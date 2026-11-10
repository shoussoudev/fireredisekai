// FireRed Isekai: money troubles. Paid POKéMON CENTERS, the POKé CARE PLAN (insurance),
// and SHARKEY the loan shark.
#include "global.h"
#include "event_data.h"
#include "event_scripts.h"
#include "fireredisekai.h"
#include "main.h"
#include "metatile_behavior.h"
#include "money.h"
#include "overworld.h"
#include "party_menu.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/vars.h"

// ---------------------------------------------------------------------------
// Prices
// ---------------------------------------------------------------------------
#define PRICE_HURT        20   // HP, status or PP missing
#define PRICE_FAINTED     60
#define PRICE_WARD       200   // Badly Injured, per POKéMON
#define PLAN_BASE        100   // premium every PLAN_PERIOD steps
#define PLAN_PERIOD      500
#define PLAN_DEATH_FEE    50   // each death on the plan raises the premium
#define SHARK_LOAN      1000
#define SHARK_PERIOD     500   // interest every 500 steps
#define SHARK_GOONS     3000   // owe this much and his associates come calling

// VAR_ISEKAI_CARE_PLAN: low byte 1 = on the plan; high byte = (badges + 1) when he last said no.
#define PLAN_ACTIVE()  ((VarGet(VAR_ISEKAI_CARE_PLAN) & 0xFF) == 1)

static u32 BadgeCount(void)
{
    u32 i, n = 0;
    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
        if (FlagGet(i))
            n++;
    return n;
}

// Prices go up a little with each badge: x1 with one badge, x3 with eight.
static u32 Scaled(u32 base)
{
    u32 badges = BadgeCount();
    return base * (3 + badges) / 4;
}

static bool32 IsNewbie(void)
{
    return !FlagGet(FLAG_BADGE01_GET);
}

static bool32 IsRealMon(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG);
}

static u32 MonHealCost(struct Pokemon *mon)
{
    u32 i;

    if (!IsRealMon(mon))
        return 0;
    if (GetMonData(mon, MON_DATA_HP) == 0)
        return Scaled(PRICE_FAINTED);
    if (GetMonData(mon, MON_DATA_HP) < GetMonData(mon, MON_DATA_MAX_HP)
     || GetMonData(mon, MON_DATA_STATUS) != 0)
        return Scaled(PRICE_HURT);
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        enum Move move = GetMonData(mon, MON_DATA_MOVE1 + i);
        if (move != MOVE_NONE
         && GetMonData(mon, MON_DATA_PP1 + i) < CalculatePPWithBonus(move, GetMonData(mon, MON_DATA_PP_BONUSES), i))
            return Scaled(PRICE_HURT);
    }
    return 0;
}

static u32 PartyHealCost(void)
{
    u32 i, total = 0;
    for (i = 0; i < PARTY_SIZE; i++)
        total += MonHealCost(&gParties[B_TRAINER_PLAYER][i]);
    return total;
}

static void BufferMoney(u8 *dest, u32 amount)
{
    ConvertIntToDecimalStringN(dest, amount, STR_CONV_MODE_LEFT_ALIGN, 6);
}

static u32 PlanPremium(void)
{
    u32 payments = VarGet(VAR_ISEKAI_CARE_PAYMENTS);
    u32 discount = (payments / 5) * 10;     // -10% for every 5 payments...
    if (discount > 50)
        discount = 50;                      // ...down to half price
    return (Scaled(PLAN_BASE) + VarGet(VAR_ISEKAI_CARE_SURCHARGE)) * (100 - discount) / 100;
}

// ---------------------------------------------------------------------------
// The nurse
// ---------------------------------------------------------------------------

// VAR_RESULT: 0 free (new trainer), 1 free (on the plan), 2 nothing to pay,
// 3 pay STR_VAR_1, 4 can't pay it all (STR_VAR_1 cost, STR_VAR_2 money). STR_VAR_3: premium.
void IsekaiHealQuote(void)
{
    u32 cost = PartyHealCost();
    u32 money = GetMoney(&gSaveBlock1Ptr->money);

    BufferMoney(gStringVar1, cost);
    BufferMoney(gStringVar2, money);
    BufferMoney(gStringVar3, PlanPremium());
    if (IsNewbie())
        gSpecialVar_Result = 0;
    else if (PLAN_ACTIVE())
        gSpecialVar_Result = 1;
    else if (cost == 0)
        gSpecialVar_Result = 2;
    else if (cost <= money)
        gSpecialVar_Result = 3;
    else
        gSpecialVar_Result = 4;
}

void IsekaiPayHeal(void)
{
    RemoveMoney(&gSaveBlock1Ptr->money, PartyHealCost());
}

// One at a time, for when he can't pay for everyone. VAR_0x8004 = party slot.
// VAR_RESULT: 0 nothing to heal, 1 affordable, 2 too expensive. STR_VAR_1 cost, STR_VAR_2 name.
void IsekaiHealOneQuote(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];
    u32 cost = MonHealCost(mon);

    GetMonNickname(mon, gStringVar2);
    BufferMoney(gStringVar1, cost);
    if (cost == 0)
        gSpecialVar_Result = 0;
    else if (cost <= GetMoney(&gSaveBlock1Ptr->money))
        gSpecialVar_Result = 1;
    else
        gSpecialVar_Result = 2;
}

void IsekaiHealOne(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];
    RemoveMoney(&gSaveBlock1Ptr->money, MonHealCost(mon));
    HealPokemon(mon);
}

// Ward care for the Badly Injured in the party. VAR_RESULT: 0 free, 1 pay STR_VAR_1, 2 can't.
// On the plan: half price, except for injuries he already had when he signed up.
static u32 WardCost(void)
{
    u32 i, total = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 price;
        if (!IsRealMon(mon) || !IsekaiIsBadlyInjured(mon))
            continue;
        price = Scaled(PRICE_WARD);
        if (PLAN_ACTIVE() && !GetMonData(mon, MON_DATA_LAND_RIBBON))
            price /= 2;
        total += price;
    }
    return total;
}

void IsekaiWardQuote(void)
{
    u32 cost = WardCost();
    BufferMoney(gStringVar1, cost);
    if (IsNewbie() || cost == 0)
        gSpecialVar_Result = 0;
    else if (cost <= GetMoney(&gSaveBlock1Ptr->money))
        gSpecialVar_Result = 1;
    else
        gSpecialVar_Result = 2;
}

void IsekaiPayWard(void)
{
    if (!IsNewbie())
        RemoveMoney(&gSaveBlock1Ptr->money, WardCost());
}

// ---------------------------------------------------------------------------
// The POKé CARE PLAN
// ---------------------------------------------------------------------------

// VAR_RESULT = TRUE if the nurse should pitch it (STR_VAR_3 = premium).
void IsekaiCarePlanShouldOffer(void)
{
    u32 declinedAt = VarGet(VAR_ISEKAI_CARE_PLAN) >> 8;
    BufferMoney(gStringVar3, PlanPremium());
    gSpecialVar_Result = !IsNewbie() && !PLAN_ACTIVE() && declinedAt != BadgeCount() + 1;
}

void IsekaiCarePlanDecline(void)
{
    VarSet(VAR_ISEKAI_CARE_PLAN, (BadgeCount() + 1) << 8);
}

void IsekaiCarePlanSignUp(void)
{
    u32 i;
    u8 yes = 1;

    VarSet(VAR_ISEKAI_CARE_PLAN, 1);
    VarSet(VAR_ISEKAI_CARE_PAYMENTS, 0);
    VarSet(VAR_ISEKAI_CARE_STEPS, 0);
    VarSet(VAR_ISEKAI_CARE_SURCHARGE, 0);
    // The fine print: injuries he already has aren't covered.
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (IsRealMon(mon) && IsekaiIsBadlyInjured(mon))
            SetMonData(mon, MON_DATA_LAND_RIBBON, &yes);
    }
}

void IsekaiCarePlanOnDeath(void)
{
    if (PLAN_ACTIVE())
        VarSet(VAR_ISEKAI_CARE_SURCHARGE, VarGet(VAR_ISEKAI_CARE_SURCHARGE) + PLAN_DEATH_FEE);
}

// ---------------------------------------------------------------------------
// SHARKEY
// ---------------------------------------------------------------------------

// VAR_RESULT = debt; STR_VAR_1 = debt.
void IsekaiSharkDebt(void)
{
    gSpecialVar_Result = VarGet(VAR_ISEKAI_SHARK_DEBT);
    BufferMoney(gStringVar1, gSpecialVar_Result);
}

void IsekaiSharkLend(void)
{
    AddMoney(&gSaveBlock1Ptr->money, SHARK_LOAN);
    VarSet(VAR_ISEKAI_SHARK_DEBT, SHARK_LOAN);
    VarSet(VAR_ISEKAI_SHARK_STEPS, 0);
}

// Pays what he can. VAR_RESULT: 0 broke, 1 part (STR_VAR_1 paid, STR_VAR_2 left), 2 all.
void IsekaiSharkRepay(void)
{
    u32 debt = VarGet(VAR_ISEKAI_SHARK_DEBT);
    u32 money = GetMoney(&gSaveBlock1Ptr->money);
    u32 paid = money < debt ? money : debt;

    RemoveMoney(&gSaveBlock1Ptr->money, paid);
    VarSet(VAR_ISEKAI_SHARK_DEBT, debt - paid);
    BufferMoney(gStringVar1, paid);
    BufferMoney(gStringVar2, debt - paid);
    if (paid == 0)
        gSpecialVar_Result = 0;
    else if (debt - paid != 0)
        gSpecialVar_Result = 1;
    else
        gSpecialVar_Result = 2;
}

// Lost to his associate: they empty his pockets toward the debt.
void IsekaiSharkTakesAll(void)
{
    u32 debt = VarGet(VAR_ISEKAI_SHARK_DEBT);
    u32 money = GetMoney(&gSaveBlock1Ptr->money);
    u32 taken = money < debt ? money : debt;
    RemoveMoney(&gSaveBlock1Ptr->money, taken);
    VarSet(VAR_ISEKAI_SHARK_DEBT, debt - taken);
}

// ---------------------------------------------------------------------------
// Every step / when the field is idle
// ---------------------------------------------------------------------------
EWRAM_DATA static u16 sPendingLapse = 0;      // premium he couldn't pay
EWRAM_DATA static bool8 sGoonsWaiting = FALSE; // next tall grass, SHARKEY's associate steps out

bool32 IsekaiMoneyOnStep(u16 metatileBehavior)
{
    if (PLAN_ACTIVE())
    {
        u16 steps = VarGet(VAR_ISEKAI_CARE_STEPS) + 1;
        if (steps >= PLAN_PERIOD)
        {
            u32 premium = PlanPremium();
            steps = 0;
            if (GetMoney(&gSaveBlock1Ptr->money) >= premium)
            {
                RemoveMoney(&gSaveBlock1Ptr->money, premium);
                if (VarGet(VAR_ISEKAI_CARE_PAYMENTS) < 0xFFFF)
                    VarSet(VAR_ISEKAI_CARE_PAYMENTS, VarGet(VAR_ISEKAI_CARE_PAYMENTS) + 1);
            }
            else
            {
                VarSet(VAR_ISEKAI_CARE_PLAN, 0);      // lapsed: the loyalty discount is gone too
                VarSet(VAR_ISEKAI_CARE_PAYMENTS, 0);
                sPendingLapse = premium;
            }
        }
        VarSet(VAR_ISEKAI_CARE_STEPS, steps);
    }

    if (VarGet(VAR_ISEKAI_SHARK_DEBT) != 0)
    {
        u16 steps = VarGet(VAR_ISEKAI_SHARK_STEPS) + 1;
        if (steps >= SHARK_PERIOD)
        {
            u32 debt = VarGet(VAR_ISEKAI_SHARK_DEBT);
            u32 interest = debt / 10 < 100 ? 100 : debt / 10;
            debt = debt + interest > 60000 ? 60000 : debt + interest;
            VarSet(VAR_ISEKAI_SHARK_DEBT, debt);
            steps = 0;
            if (debt >= SHARK_GOONS)
                sGoonsWaiting = TRUE;
        }
        VarSet(VAR_ISEKAI_SHARK_STEPS, steps);
        if (sGoonsWaiting && MetatileBehavior_IsTallGrass(metatileBehavior) && CalculatePlayerPartyCount() != 0)
        {
            sGoonsWaiting = FALSE;
            ScriptContext_SetupScript(IsekaiEventScript_SharkGoon);
            return TRUE;
        }
    }
    else
    {
        sGoonsWaiting = FALSE;
    }
    return FALSE;
}

bool32 IsekaiMoneyFieldIdle(void)
{
    if (sPendingLapse != 0)
    {
        BufferMoney(gStringVar1, sPendingLapse);
        sPendingLapse = 0;
        LockPlayerFieldControls();
        ScriptContext_SetupScript(IsekaiEventScript_CarePlanLapsed);
        return TRUE;
    }
    return FALSE;
}
