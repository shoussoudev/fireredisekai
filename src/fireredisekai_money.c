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
#include "battle.h"
#include "list_menu.h"
#include "malloc.h"
#include "pokemon_storage_system.h"
#include "script_menu.h"
#include "fireredisekai_save.h"
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

static void TickHeldMons(void);

bool32 IsekaiMoneyOnStep(u16 metatileBehavior)
{
    TickHeldMons();
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

EWRAM_DATA static u8 sPendingSharkCall;
EWRAM_DATA static u8 sPendingSharkSlot;
EWRAM_DATA static u8 sPendingSoldName[POKEMON_NAME_LENGTH + 1];

bool32 IsekaiMoneyFieldIdle(void)
{
    if (gSaveBlock3Ptr->isekai.tonyPending)
    {
        LockPlayerFieldControls();
        ScriptContext_SetupScript(IsekaiEventScript_TonyTakes);
        return TRUE;
    }
    if (sPendingSharkCall != 0)
    {
        if (sPendingSharkCall == 1)
        {
            struct BoxPokemon *boxMon = (struct BoxPokemon *)gSaveBlock3Ptr->isekai.sharkHeld[sPendingSharkSlot].mon;
            GetBoxMonData(boxMon, MON_DATA_NICKNAME, gStringVar1);
            StringGet_Nickname(gStringVar1);
        }
        else
        {
            StringCopy(gStringVar1, sPendingSoldName);
        }
        gSpecialVar_0x8004 = sPendingSharkCall;
        sPendingSharkCall = 0;
        LockPlayerFieldControls();
        ScriptContext_SetupScript(IsekaiEventScript_SharkCall);
        return TRUE;
    }
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

// ---------------------------------------------------------------------------
// How much each POKéMON gets used: battles it got to act in.
// Stored in the contest stats (no contests in this game): COOL low byte, TOUGH high byte.
// ---------------------------------------------------------------------------
EWRAM_DATA static u8 sFoughtMask = 0;

void IsekaiNoteFighters(void)
{
    u32 battler;
    for (battler = 0; battler < gBattlersCount; battler++)
        if (GetBattlerSide(battler) == B_SIDE_PLAYER && !(gBattleTypeFlags & BATTLE_TYPE_MULTI && (battler & BIT_FLANK)))
            sFoughtMask |= 1 << gBattlerPartyIndexes[battler];
}

static u32 GetBoxMonUses(struct BoxPokemon *boxMon)
{
    return GetBoxMonData(boxMon, MON_DATA_COOL) | (GetBoxMonData(boxMon, MON_DATA_TOUGH) << 8);
}

void IsekaiCountFights(void)
{
    u32 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 uses;
        u8 lo, hi;
        if (!(sFoughtMask & (1 << i)) || !IsRealMon(mon))
            continue;
        uses = GetBoxMonUses(&mon->box);
        if (uses < 0xFFFF)
            uses++;
        lo = uses & 0xFF;
        hi = uses >> 8;
        SetMonData(mon, MON_DATA_COOL, &lo);
        SetMonData(mon, MON_DATA_TOUGH, &hi);
    }
    sFoughtMask = 0;
}

// ---------------------------------------------------------------------------
// SHARKEY, part two: collateral, TONY's "interest", ransom, auction
// ---------------------------------------------------------------------------
#define sIsekaiSave (gSaveBlock3Ptr->isekai)
#define SHARK_COLLATERAL_LOAN 3000
#define HELD_WARNING   4000
#define HELD_AUCTION   5000

// The 6 most-used POKéMON, party or PC. Location: box 0xFF = party slot.
struct SharkCandidate { u8 box, pos; u16 uses; };
EWRAM_DATA static struct SharkCandidate sCandidates[PARTY_SIZE] = {0};
EWRAM_DATA static u8 sCandidateCount = 0;

static bool32 IsOaksLoanerBox(struct BoxPokemon *boxMon)
{
    u32 state = VarGet(VAR_ISEKAI_LOAN_STATE);
    u32 pid = VarGet(VAR_ISEKAI_LOANER_PID_LO) | (VarGet(VAR_ISEKAI_LOANER_PID_HI) << 16);
    return state >= 1 && state <= 4 && GetBoxMonData(boxMon, MON_DATA_PERSONALITY) == pid;
}

static void ConsiderCandidate(struct BoxPokemon *boxMon, u8 box, u8 pos)
{
    u32 uses, i, j;

    if (GetBoxMonData(boxMon, MON_DATA_SPECIES) == SPECIES_NONE || GetBoxMonData(boxMon, MON_DATA_IS_EGG)
     || IsOaksLoanerBox(boxMon))
        return;
    uses = GetBoxMonUses(boxMon);
    if (uses == 0)
        return;   // never fought: worthless
    for (i = 0; i < sCandidateCount; i++)
        if (uses > sCandidates[i].uses)
            break;
    if (i >= PARTY_SIZE)
        return;
    if (sCandidateCount < PARTY_SIZE)
        sCandidateCount++;
    for (j = sCandidateCount - 1; j > i; j--)
        sCandidates[j] = sCandidates[j - 1];
    sCandidates[i].box = box;
    sCandidates[i].pos = pos;
    sCandidates[i].uses = uses;
}

static struct BoxPokemon *CandidateBoxMon(u32 i)
{
    if (sCandidates[i].box == 0xFF)
        return &gParties[B_TRAINER_PLAYER][sCandidates[i].pos].box;
    return GetBoxedMonPtr(sCandidates[i].box, sCandidates[i].pos);
}

static s32 FreeSharkSlot(void)
{
    u32 i;
    for (i = 0; i < ISEKAI_SHARK_SLOTS; i++)
        if (sIsekaiSave.sharkHeld[i].kind == ISEKAI_SHARK_NONE)
            return i;
    return -1;
}

static const u8 sText_HeldLevel[] = _("{STR_VAR_1} Lv{STR_VAR_2}");

// Builds the list of the 6 most-used and pushes it for dynmultistack.
// VAR_RESULT = how many (0 = nothing worth taking; 0xFF = his kennels are full).
void IsekaiSharkBuildList(void)
{
    u32 i, box, pos, partyCount = CalculatePlayerPartyCount();

    sCandidateCount = 0;
    if (FreeSharkSlot() < 0)
    {
        gSpecialVar_Result = 0xFF;
        return;
    }
    for (i = 0; i < PARTY_SIZE; i++)
        if (partyCount > 1) // never his last POKéMON in the party
            ConsiderCandidate(&gParties[B_TRAINER_PLAYER][i].box, 0xFF, i);
    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
            ConsiderCandidate(GetBoxedMonPtr(box, pos), box, pos);

    for (i = 0; i < sCandidateCount; i++)
    {
        struct ListMenuItem item;
        u8 *name = Alloc(40);
        struct BoxPokemon *boxMon = CandidateBoxMon(i);
        GetBoxMonData(boxMon, MON_DATA_NICKNAME, gStringVar1);
        StringGet_Nickname(gStringVar1);
        ConvertIntToDecimalStringN(gStringVar2, GetLevelFromBoxMonExp(boxMon), STR_CONV_MODE_LEFT_ALIGN, 3);
        StringExpandPlaceholders(name, sText_HeldLevel);
        item.name = name;
        item.id = i;
        MultichoiceDynamic_PushElement(item);
    }
    gSpecialVar_Result = sCandidateCount;
}

// Takes candidate VAR_0x8004 into a free slot. VAR_0x8005 = ISEKAI_SHARK_COLLATERAL / INTEREST.
// STR_VAR_1 = its name, STR_VAR_2 = the ransom.
void IsekaiSharkTake(void)
{
    u32 i = gSpecialVar_0x8004;
    s32 slot = FreeSharkSlot();
    struct IsekaiSharkHeld *held;
    struct BoxPokemon *boxMon;

    if (i >= sCandidateCount || slot < 0)
        return;
    held = &sIsekaiSave.sharkHeld[slot];
    boxMon = CandidateBoxMon(i);
    GetBoxMonData(boxMon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    memcpy(held->mon, boxMon, ISEKAI_BOXMON_SIZE);
    held->kind = gSpecialVar_0x8005;
    held->steps = 0;
    held->warned = FALSE;
    held->ransom = (held->kind == ISEKAI_SHARK_INTEREST) ? GetLevelFromBoxMonExp(boxMon) * 50 : 0;
    BufferMoney(gStringVar2, held->ransom);
    if (sCandidates[i].box == 0xFF)
    {
        ZeroMonData(&gParties[B_TRAINER_PLAYER][sCandidates[i].pos]);
        CompactPartySlots();
    }
    else
    {
        ZeroBoxMonAt(sCandidates[i].box, sCandidates[i].pos);
    }
    if (held->kind == ISEKAI_SHARK_COLLATERAL)
    {
        AddMoney(&gSaveBlock1Ptr->money, SHARK_COLLATERAL_LOAN);
        VarSet(VAR_ISEKAI_SHARK_DEBT, SHARK_COLLATERAL_LOAN);
        VarSet(VAR_ISEKAI_SHARK_STEPS, 0);
    }
}

// Back to the party, or the PC if the party is full. STR_VAR_1 = its name.
static void GiveHeldBack(u32 slot)
{
    struct Pokemon mon;
    u32 i;
    struct IsekaiSharkHeld *held = &sIsekaiSave.sharkHeld[slot];

    BoxMonToMon((struct BoxPokemon *)held->mon, &mon);
    GetMonNickname(&mon, gStringVar1);
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES) == SPECIES_NONE)
            break;
    if (i < PARTY_SIZE)
    {
        CopyMon(&gParties[B_TRAINER_PLAYER][i], &mon, sizeof(mon));
        gPartiesCount[B_TRAINER_PLAYER] = CalculatePlayerPartyCount();
    }
    else
    {
        CopyMonToPC(&mon);
    }
    memset(held, 0, sizeof(*held));
}

// Collateral comes back when the debt is gone. VAR_RESULT = TRUE if one came back (STR_VAR_1).
void IsekaiSharkReturnCollateral(void)
{
    u32 i;
    gSpecialVar_Result = FALSE;
    if (VarGet(VAR_ISEKAI_SHARK_DEBT) != 0)
        return;
    for (i = 0; i < ISEKAI_SHARK_SLOTS; i++)
    {
        if (sIsekaiSave.sharkHeld[i].kind == ISEKAI_SHARK_COLLATERAL)
        {
            GiveHeldBack(i);
            gSpecialVar_Result = TRUE;
            return;
        }
    }
}

// SHARKEY's counter menu. Pushes: pay the debt (100), each POKéMON he holds (slot id), leave (200).
static const u8 sText_PayDebt[] = _("Pay the debt");
static const u8 sText_BuyBack[] = _("{STR_VAR_1} ¥{STR_VAR_2}");
static const u8 sText_Collateral[] = _("{STR_VAR_1} (loan)");
static const u8 sText_Leave[] = _("Leave");

static void PushOption(const u8 *text, u16 id)
{
    struct ListMenuItem item;
    u8 *name = Alloc(40);
    StringExpandPlaceholders(name, text);
    item.name = name;
    item.id = id;
    MultichoiceDynamic_PushElement(item);
}

// VAR_RESULT = TRUE if there's anything to settle (debt or held POKéMON).
void IsekaiSharkBuildCounter(void)
{
    u32 i;
    bool32 any = FALSE;

    if (VarGet(VAR_ISEKAI_SHARK_DEBT) != 0)
    {
        PushOption(sText_PayDebt, 100);
        any = TRUE;
    }
    for (i = 0; i < ISEKAI_SHARK_SLOTS; i++)
    {
        struct IsekaiSharkHeld *held = &sIsekaiSave.sharkHeld[i];
        if (held->kind == ISEKAI_SHARK_NONE)
            continue;
        GetBoxMonData((struct BoxPokemon *)held->mon, MON_DATA_NICKNAME, gStringVar1);
        StringGet_Nickname(gStringVar1);
        BufferMoney(gStringVar2, held->ransom);
        PushOption(held->kind == ISEKAI_SHARK_COLLATERAL ? sText_Collateral : sText_BuyBack, i);
        any = TRUE;
    }
    if (any)
        PushOption(sText_Leave, 200);
    gSpecialVar_Result = any;
}

// VAR_0x8004 = held slot. VAR_RESULT: 0 collateral (repay the loan), 1 bought back, 2 can't afford.
// STR_VAR_1 name, STR_VAR_2 ransom.
void IsekaiSharkBuyBack(void)
{
    struct IsekaiSharkHeld *held = &sIsekaiSave.sharkHeld[gSpecialVar_0x8004];

    GetBoxMonData((struct BoxPokemon *)held->mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    BufferMoney(gStringVar2, held->ransom);
    if (held->kind == ISEKAI_SHARK_COLLATERAL)
    {
        gSpecialVar_Result = 0;
    }
    else if (GetMoney(&gSaveBlock1Ptr->money) < held->ransom)
    {
        gSpecialVar_Result = 2;
    }
    else
    {
        RemoveMoney(&gSaveBlock1Ptr->money, held->ransom);
        GiveHeldBack(gSpecialVar_0x8004);
        gSpecialVar_Result = 1;
    }
}

// Lost to TONY: he'll come for his "interest" once things are calm.
void IsekaiTonyWillTake(void)
{
    sIsekaiSave.tonyPending = TRUE;
}

void IsekaiTonyDone(void)
{
    sIsekaiSave.tonyPending = FALSE;
}

// A ROCKET grunt has the auctioned POKéMON (1 in 3 grunts until it's been seen).
void IsekaiMaybeGiveGruntSoldMon(void)
{
    u32 i, last = 0;
    struct Pokemon *party = gParties[B_TRAINER_OPPONENT_A];

    if (!sIsekaiSave.soldWaiting || (Random() % 3) != 0)
        return;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&party[i], MON_DATA_SPECIES) != SPECIES_NONE)
            last = i;
    BoxMonToMon((struct BoxPokemon *)sIsekaiSave.soldMon, &party[last]);
    {
        u16 hp = GetMonData(&party[last], MON_DATA_MAX_HP);
        SetMonData(&party[last], MON_DATA_HP, &hp);
    }
    memset(sIsekaiSave.soldMon, 0, ISEKAI_BOXMON_SIZE);
    sIsekaiSave.soldWaiting = FALSE;
}


static void TickHeldMons(void)
{
    u32 i;
    for (i = 0; i < ISEKAI_SHARK_SLOTS; i++)
    {
        struct IsekaiSharkHeld *held = &sIsekaiSave.sharkHeld[i];
        if (held->kind == ISEKAI_SHARK_NONE)
            continue;
        if (held->steps < 0xFFFF)
            held->steps++;
        if (held->kind == ISEKAI_SHARK_INTEREST && held->steps % 500 == 0)
        {
            u32 ransom = held->ransom + held->ransom / 10;   // "board and lodging"
            held->ransom = ransom > 60000 ? 60000 : ransom;
        }
        if (held->steps >= HELD_WARNING && !held->warned && sPendingSharkCall == 0)
        {
            held->warned = TRUE;
            sPendingSharkCall = 1;
            sPendingSharkSlot = i;
        }
        else if (held->steps >= HELD_AUCTION && sPendingSharkCall == 0)
        {
            GetBoxMonData((struct BoxPokemon *)held->mon, MON_DATA_NICKNAME, sPendingSoldName);
            StringGet_Nickname(sPendingSoldName);
            memcpy(sIsekaiSave.soldMon, held->mon, ISEKAI_BOXMON_SIZE);
            sIsekaiSave.soldWaiting = TRUE;
            memset(held, 0, sizeof(*held));
            sPendingSharkCall = 2;
        }
    }
}
