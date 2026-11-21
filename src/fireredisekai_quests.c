// FireRed Isekai (v0.14): the Chapter 1 romance quests and the Rematch Pass.
#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "event_scripts.h"
#include "fireredisekai.h"
#include "fieldmap.h"
#include "field_screen_effect.h"
#include "field_player_avatar.h"
#include "item_use.h"
#include "script_pokemon_util.h"
#include "main.h"
#include "metatile_behavior.h"
#include "money.h"
#include "overworld.h"
#include "random.h"
#include "script.h"
#include "string_util.h"
#include "task.h"
#include "constants/fireredisekai_romance.h"
#include "constants/maps.h"

// ---------------------------------------------------------------------------
// The ward nurse: three herbs, a potion, and a battle won while confused.
// VAR_ISEKAI_NURSE_QUEST: bits 0-2 herbs (Route 2, Viridian Forest, Route 22), stage in bits 4-7.
// ---------------------------------------------------------------------------
#define NURSE_STAGE(v) (((v) >> 4) & 0xF)

void IsekaiNurseQuestStage(void)
{
    gSpecialVar_Result = NURSE_STAGE(VarGet(VAR_ISEKAI_NURSE_QUEST));
}

// VAR_0x8005 = stage
void IsekaiNurseSetStage(void)
{
    VarSet(VAR_ISEKAI_NURSE_QUEST, (VarGet(VAR_ISEKAI_NURSE_QUEST) & 0xF) | (gSpecialVar_0x8005 << 4));
}

void IsekaiNurseHerbCount(void)
{
    u16 v = VarGet(VAR_ISEKAI_NURSE_QUEST);
    gSpecialVar_Result = ((v >> 0) & 1) + ((v >> 1) & 1) + ((v >> 2) & 1);
    ConvertIntToDecimalStringN(gStringVar1, gSpecialVar_Result, STR_CONV_MODE_LEFT_ALIGN, 1);
}

static s8 HerbHere(void)
{
    u8 g = gSaveBlock1Ptr->location.mapGroup, n = gSaveBlock1Ptr->location.mapNum;
    if (g == MAP_GROUP(MAP_ROUTE2) && n == MAP_NUM(MAP_ROUTE2))
        return 0;
    if (g == MAP_GROUP(MAP_VIRIDIAN_FOREST) && n == MAP_NUM(MAP_VIRIDIAN_FOREST))
        return 1;
    if (g == MAP_GROUP(MAP_ROUTE22) && n == MAP_NUM(MAP_ROUTE22))
        return 2;
    return -1;
}

static const u8 *const sHerbPlaces[] =
{
    COMPOUND_STRING("ROUTE 2"),
    COMPOUND_STRING("VIRIDIAN FOREST"),
    COMPOUND_STRING("ROUTE 22"),
};

// From IsekaiOnStep: walking through tall grass on the right routes turns up a herb.
bool32 IsekaiQuestOnStep(u16 metatileBehavior)
{
    u16 v = VarGet(VAR_ISEKAI_NURSE_QUEST);
    s8 herb;

    if (NURSE_STAGE(v) != NURSE_QUEST_HERBS || !MetatileBehavior_IsTallGrass(metatileBehavior))
        return FALSE;
    herb = HerbHere();
    if (herb < 0 || (v & (1 << herb)) || (Random() % 12) != 0)
        return FALSE;
    VarSet(VAR_ISEKAI_NURSE_QUEST, v | (1 << herb));
    StringCopy(gStringVar1, sHerbPlaces[herb]);
    IsekaiNurseHerbCount();
    ConvertIntToDecimalStringN(gStringVar2, gSpecialVar_Result, STR_CONV_MODE_LEFT_ALIGN, 1);
    ScriptContext_SetupScript(IsekaiEventScript_FoundHerb);
    return TRUE;
}

// The potion: the lead starts the next battle confused. Win it and the trial is a success.
static EWRAM_DATA bool8 sPotionBattle = FALSE;
static EWRAM_DATA bool8 sPotionApplied = FALSE;

void IsekaiQuestOnBattleStart(void)
{
    sPotionBattle = FlagGet(FLAG_ISEKAI_POTION_ACTIVE);
    sPotionApplied = FALSE;
}

// Each turn's action selection (battle_main.c): confuse the lead once.
void IsekaiQuestOnTurn(void)
{
    u32 battler;
    if (!sPotionBattle || sPotionApplied)
        return;
    sPotionApplied = TRUE;
    for (battler = 0; battler < gBattlersCount; battler++)
    {
        if (GetBattlerSide(battler) == B_SIDE_PLAYER && gBattleMons[battler].hp != 0)
        {
            gBattleMons[battler].volatiles.confusionTimer = 3;
            break;
        }
    }
}

void IsekaiQuestOnBattleEnd(void)
{
    if (!sPotionBattle)
        return;
    sPotionBattle = FALSE;
    FlagClear(FLAG_ISEKAI_POTION_ACTIVE);
    if (gBattleOutcome == B_OUTCOME_WON)
    {
        gSpecialVar_0x8005 = NURSE_QUEST_DONE;
        IsekaiNurseSetStage();
    }
}

// Waking in the ward (any Center). VAR_RESULT: 0 nothing, 1 the nurse heals the team (perk),
// 2 he's charged and lectured (penalty; STR_VAR_1 = what it cost).
void IsekaiWardWakeRomance(void)
{
    gSpecialVar_Result = 0;
    if (IsekaiRomPerk(ROM_WARD_NURSE))
    {
        HealPlayerParty();
        gSpecialVar_Result = 1;
    }
    else if (IsekaiRomPenalty(ROM_WARD_NURSE))
    {
        u32 fee = 300, money = GetMoney(&gSaveBlock1Ptr->money);
        if (fee > money)
            fee = money;
        RemoveMoney(&gSaveBlock1Ptr->money, fee);
        ConvertIntToDecimalStringN(gStringVar1, fee, STR_CONV_MODE_LEFT_ALIGN, 6);
        gSpecialVar_Result = 2;
    }
}

// ---------------------------------------------------------------------------
// Rematch Pass: remember where the last loss happened (where he stood when the battle began).
// ---------------------------------------------------------------------------
static EWRAM_DATA u16 sBattleMap = 0;
static EWRAM_DATA s16 sBattleX = 0, sBattleY = 0;

void IsekaiRematchNoteBattleStart(void)
{
    sBattleMap = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    sBattleX = gSaveBlock1Ptr->pos.x;
    sBattleY = gSaveBlock1Ptr->pos.y;
}

void IsekaiRematchNoteLoss(void)
{
    VarSet(VAR_ISEKAI_LAST_LOSS_MAP, sBattleMap + 1); // 0 = no loss yet
    VarSet(VAR_ISEKAI_LAST_LOSS_X, sBattleX);
    VarSet(VAR_ISEKAI_LAST_LOSS_Y, sBattleY);
}

// VAR_RESULT = TRUE if there's somewhere to go back to.
void IsekaiRematchPassCheck(void)
{
    gSpecialVar_Result = VarGet(VAR_ISEKAI_LAST_LOSS_MAP) != 0;
}

// Set the warp; the script fades and warps.
void IsekaiRematchPassSetWarp(void)
{
    u16 m = VarGet(VAR_ISEKAI_LAST_LOSS_MAP) - 1;
    SetWarpDestination(m >> 8, m & 0xFF, WARP_ID_NONE, VarGet(VAR_ISEKAI_LAST_LOSS_X), VarGet(VAR_ISEKAI_LAST_LOSS_Y));
    DoWarp();
    ResetInitialPlayerAvatarState();
}
