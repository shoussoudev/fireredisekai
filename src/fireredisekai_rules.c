// FireRed Isekai: the core rules.
//   v0.7  autosave, mid-battle reset penalty, running, forfeits
//   v0.8  level-10 rule, Badly Injured, death, graveyard
//   v0.9  human lives, the ward, the game-over ending
//   v0.10 healing injuries at a Center, the grumpy loaner, capture willingness
#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "difficulty.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "fireredisekai.h"
#include "item.h"
#include "main.h"
#include "metatile_behavior.h"
#include "money.h"
#include "constants/map_groups.h"
#include "menu.h"
#include "text_window.h"
#include "window.h"
#include "overworld.h"
#include "party_menu.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "region_map.h"
#include "save.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "string_util.h"
#include "new_game.h"
#include "save_location.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/difficulty.h"
#include "constants/items.h"
#include "constants/map_types.h"
#include "constants/region_map_sections.h"
#include "constants/species.h"
#include "constants/trainers.h"

STATIC_ASSERT(sizeof(struct BoxPokemon) == ISEKAI_BOXMON_SIZE, IsekaiBoxMonSize);

#include "data/fireredisekai_save_maps.h"

#define sIsekai (gSaveBlock3Ptr->isekai)

// Not saved: things that only matter until the next autosave or field message.
EWRAM_DATA const u8 *gIsekaiTrainerArgs = NULL;       // set by TrainerBattleLoadArgs
EWRAM_DATA static bool8 sResumingBattle = FALSE;       // this battle is a restart after a reset
EWRAM_DATA static u8 sAliveAtStart = 0;                // party slots with HP > 0 when the battle began
EWRAM_DATA static bool8 sPendingProtectedNote = FALSE;  // explain the level-10 rule (once)
EWRAM_DATA static u8 sProtectedName[POKEMON_NAME_LENGTH + 1] = {0};
EWRAM_DATA static u8 sPendingDeaths = 0;               // how many "didn't get back up" messages to show
EWRAM_DATA static u8 sPendingDeathNames[PARTY_SIZE][POKEMON_NAME_LENGTH + 1] = {0};
EWRAM_DATA static u8 sPendingInjuries = 0;             // how many "Badly Injured" messages to show
EWRAM_DATA static u8 sPendingInjuryNames[PARTY_SIZE][POKEMON_NAME_LENGTH + 1] = {0};
EWRAM_DATA static u8 sLastDefeat = ISEKAI_DEFEAT_NONE; // how the last whiteout happened
EWRAM_DATA static u16 sLastWildSpecies = SPECIES_NONE;
EWRAM_DATA static bool8 sAutosavePending = FALSE;
EWRAM_DATA static bool8 sZoneSavePending = FALSE;
EWRAM_DATA static bool8 sArrivalSavePending = FALSE;  // entered a save building; save once it has faded in
EWRAM_DATA static bool8 sBattleStartSavePending = FALSE;
EWRAM_DATA static u8 sPendingCall = 0;               // Oak's phone call to play: 1 first catch, 2 overdue, 3 too late
EWRAM_DATA static bool8 sPendingWantedReminder = FALSE;
EWRAM_DATA static u16 sLastAutosaveMap = 0;            // (group << 8 | num) + 1 of the last autosave; 0 = none yet
EWRAM_DATA bool8 gIsekaiWildUnwilling = FALSE;         // this wild Pokémon won't accept a ball

// ---------------------------------------------------------------------------
// Difficulty helpers
// ---------------------------------------------------------------------------

u8 IsekaiMaxLives(void)
{
    switch (GetCurrentDifficultyLevel())
    {
    case DIFFICULTY_EASY: return 5;
    case DIFFICULTY_HARD: return 2;
    default:              return 3;
    }
}

u8 IsekaiWardBattles(void)
{
    switch (GetCurrentDifficultyLevel())
    {
    case DIFFICULTY_EASY: return 2;
    case DIFFICULTY_HARD: return 4;
    default:              return 3;
    }
}

static u8 CountBadges(void)
{
    u8 i, count = 0;
    for (i = 0; i < NUM_BADGES; i++)
        if (FlagGet(gBadgeFlags[i]))
            count++;
    return count;
}

void IsekaiNewGameInit(void)
{
    memset(&sIsekai, 0, sizeof(sIsekai));
    sIsekai.lives = IsekaiMaxLives();
    if (VarGet(VAR_ISEKAI_LOAN_STATE) == 0)
        FlagSet(FLAG_HIDE_ISEKAI_SHADY_MAN);  // appears once the loan starts
    FlagSet(FLAG_HIDE_ISEKAI_LAB_JENNY);  // appears if the loaner is overdue
    FlagSet(FLAG_SYS_B_DASH); // running from the very start
    if (GetCurrentDifficultyLevel() == DIFFICULTY_HARD)
        gSaveBlock2Ptr->optionsBattleStyle = OPTIONS_BATTLE_STYLE_SET; // no free switch after a KO
}

// ---------------------------------------------------------------------------
// Badly Injured: a hidden flag carried by the Pokémon itself (an unused ribbon)
// ---------------------------------------------------------------------------

bool32 IsekaiIsBadlyInjured(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_MARINE_RIBBON);
}

void IsekaiSetBadlyInjured(struct Pokemon *mon, bool32 injured)
{
    u8 value = injured ? 1 : 0;
    SetMonData(mon, MON_DATA_MARINE_RIBBON, &value);
    if (injured)
        SetMonData(mon, MON_DATA_SKY_RIBBON, &value);  // ever Badly Injured (Celebi's check); never cleared
    else
        SetMonData(mon, MON_DATA_LAND_RIBBON, &value); // healed: no longer a pre-existing condition
}

bool32 IsekaiBoxMonIsBadlyInjured(struct BoxPokemon *boxMon)
{
    return GetBoxMonData(boxMon, MON_DATA_MARINE_RIBBON);
}

// The level-10 rule. Before the Boulder Badge, a Pokémon under level 10 is protected
// whatever it faces. After it, a fight only counts if the opponent is level 10+.
static bool32 IsProtectedFaint(struct Pokemon *mon, u8 opponentMaxLevel)
{
    if (!FlagGet(FLAG_BADGE01_GET))
        return GetMonData(mon, MON_DATA_LEVEL) < 10;
    return opponentMaxLevel < 10;
}

static u8 OpponentMaxLevel(void)
{
    u8 i, maxLevel = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && GetMonData(mon, MON_DATA_LEVEL) > maxLevel)
            maxLevel = GetMonData(mon, MON_DATA_LEVEL);
        mon = &gParties[B_TRAINER_OPPONENT_B][i];
        if ((gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) && GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && GetMonData(mon, MON_DATA_LEVEL) > maxLevel)
            maxLevel = GetMonData(mon, MON_DATA_LEVEL);
    }
    return maxLevel;
}

// Revive used in battle: the Pokémon gets back up, but it's now Badly Injured
// (unless the level-10 rule protects it). Max Revive clears the injury.
void IsekaiOnReviveUsed(struct Pokemon *mon, u16 item)
{
    if (item == ITEM_MAX_REVIVE)
    {
        IsekaiSetBadlyInjured(mon, FALSE);
        return;
    }
    if (gMain.inBattle && !IsProtectedFaint(mon, OpponentMaxLevel()))
        IsekaiSetBadlyInjured(mon, TRUE);
}

// ---------------------------------------------------------------------------
// The graveyard
// ---------------------------------------------------------------------------

static void BuryMon(struct Pokemon *mon)
{
    IsekaiCarePlanOnDeath();
    u8 slot = sIsekai.graveCount;
    u8 name[POKEMON_NAME_LENGTH + 1];

    GetMonData(mon, MON_DATA_NICKNAME, name);
    if (slot >= ISEKAI_GRAVES)
    {
        // Full: the oldest names are kept, the count still goes up for the reputation rule.
        if (sIsekai.graveCount < 255)
            sIsekai.graveCount++;
        return;
    }
    sIsekai.graves[slot].species = GetMonData(mon, MON_DATA_SPECIES);
    memset(sIsekai.graves[slot].nickname, EOS, ISEKAI_GRAVE_NAME_LEN);
    memcpy(sIsekai.graves[slot].nickname, name, min(StringLength(name), ISEKAI_GRAVE_NAME_LEN));
    sIsekai.graveCount++;
}

u8 IsekaiGraveCount(void)
{
    return sIsekai.graveCount;
}

// For the graveyard sign: puts the names into gStringVar4, one per line.
// VAR_RESULT = how many names.
void IsekaiBufferGraveNames(void)
{
    u8 i, count = min(sIsekai.graveCount, ISEKAI_GRAVES);
    u8 *dst = gStringVar4;
    u8 name[ISEKAI_GRAVE_NAME_LEN + 1];

    *dst = EOS;
    gSpecialVar_Result = count;
    for (i = 0; i < count; i++)
    {
        memcpy(name, sIsekai.graves[i].nickname, ISEKAI_GRAVE_NAME_LEN);
        name[ISEKAI_GRAVE_NAME_LEN] = EOS;
        if (i == 1)
            *dst++ = CHAR_NEWLINE;
        else if (i > 1)
            *dst++ = CHAR_PROMPT_SCROLL;
        dst = StringCopy(dst, name);
    }
    *dst = EOS;
}

// ---------------------------------------------------------------------------
// Autosave
// ---------------------------------------------------------------------------

static u16 CurrentMapKey(void)
{
    return ((gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum) + 1;
}

// All autosaves are quick saves (only the parts of the save that changed are written)
// and happen at moments the player is already waiting: black screens, transitions, stops.
static void SaveNow(void)
{
    BackgroundSave_Finish();
    if (gDifferentSaveFile == TRUE)
    {
        // First save over someone else's file: erase it properly, in one go.
        TrySavingData(SAVE_OVERWRITE_DIFFERENT_FILE);
        gDifferentSaveFile = FALSE;
    }
    else
    {
        QuickSaveGame();
    }
    sAutosavePending = FALSE;
    sZoneSavePending = FALSE; // this save already covers a zone change
}

void IsekaiAutosave(void)
{
    SaveNow();
}

static bool32 IsSaveBuilding(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sIsekaiSaveMaps); i++)
        if (sIsekaiSaveMaps[i] == map)
            return TRUE;
    return FALSE;
}

// Every warp: entering an important building (Center, gym, Mart, story place) queues a
// save; leaving one means coming back saves again.
void IsekaiOnMapWarp(void)
{
    // While wanted, Centers and Marts now and then remind him (posters by the door).
    // (Buildings no longer autosave since v0.10.5; manual SAVE is back.)
    sPendingWantedReminder = FALSE;
    if (FlagGet(FLAG_ISEKAI_WANTED) && IsSaveBuilding()
     && gSaveBlock1Ptr->location.mapGroup != MAP_GROUP(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB)
     && (Random() % 2) == 0)
        sPendingWantedReminder = TRUE;
}

// ---------------------------------------------------------------------------
// Oak's loan (v0.11): the first catch starts a 1,000-step clock.
// ---------------------------------------------------------------------------
#define ISEKAI_LOAN_DEADLINE 1000

void IsekaiOnFirstCatch(void)
{
    if (VarGet(VAR_ISEKAI_LOAN_STATE) != 2)
        return;
    VarSet(VAR_ISEKAI_LOAN_STATE, 3);
    VarSet(VAR_ISEKAI_LOAN_STEPS, 0);
    sPendingCall = 1;
}

bool32 IsekaiOnStep(u16 metatileBehavior)
{
    // DAISY's grooming cooldown (FireRed counts steps for it; the expansion doesn't).
    if (VarGet(VAR_MASSAGE_COOLDOWN_STEP_COUNTER) < 500)
        VarSet(VAR_MASSAGE_COOLDOWN_STEP_COUNTER, VarGet(VAR_MASSAGE_COOLDOWN_STEP_COUNTER) + 1);
    if (VarGet(VAR_ISEKAI_LOAN_STATE) == 3)
    {
        u16 steps = VarGet(VAR_ISEKAI_LOAN_STEPS) + 1;
        VarSet(VAR_ISEKAI_LOAN_STEPS, steps);
        if (steps >= ISEKAI_LOAN_DEADLINE)
        {
            VarSet(VAR_ISEKAI_LOAN_STATE, 4);
            FlagSet(FLAG_ISEKAI_WANTED);
            FlagClear(FLAG_HIDE_ISEKAI_LAB_JENNY);
            sPendingCall = 2;
        }
    }
    if (IsekaiMoneyOnStep(metatileBehavior))
        return TRUE;
    // The first step into tall grass on Route 1.
    if (!FlagGet(FLAG_ISEKAI_SEEN_FIRST_GRASS)
     && gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_ROUTE1)
     && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_ROUTE1)
     && MetatileBehavior_IsTallGrass(metatileBehavior))
    {
        FlagSet(FLAG_ISEKAI_SEEN_FIRST_GRASS);
        ScriptContext_SetupScript(IsekaiEventScript_FirstGrass);
        return TRUE;
    }
    return FALSE;
}

// VAR_RESULT = TRUE if Oak's loaner is in the party.
void IsekaiLoanerInParty(void)
{
    u8 i;
    gSpecialVar_Result = FALSE;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES) != SPECIES_NONE
         && IsekaiIsLoaner(&gParties[B_TRAINER_PLAYER][i]))
            gSpecialVar_Result = TRUE;
}

// VAR_RESULT = TRUE if the party has a POKéMON other than the loaner.
void IsekaiHasOwnMon(void)
{
    u8 i;
    gSpecialVar_Result = FALSE;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG) && !IsekaiIsLoaner(mon))
            gSpecialVar_Result = TRUE;
    }
}

// Honest return: Oak keeps ₽500 and refunds the rest. STR_VAR_1 = the refund.
void IsekaiRefundDeposit(void)
{
    u32 deposit = VarGet(VAR_ISEKAI_LOAN_DEPOSIT);
    u32 refund = deposit > 500 ? deposit - 500 : 0;
    AddMoney(&gSaveBlock1Ptr->money, refund);
    ConvertIntToDecimalStringN(gStringVar1, refund, STR_CONV_MODE_LEFT_ALIGN, 6);
}

// Jenny's fine: up to ₽1000, whatever he has. STR_VAR_1 = the amount.
// The late-return fine is ¥1000. Whatever he can't pay now comes out of his
// battle winnings later. VAR_RESULT: 0 paid in full, 1 paid part, 2 broke.
#define ISEKAI_FINE 1000
void IsekaiPayFine(void)
{
    u32 money = GetMoney(&gSaveBlock1Ptr->money);
    u32 paid = money < ISEKAI_FINE ? money : ISEKAI_FINE;
    u32 debt = ISEKAI_FINE - paid;

    RemoveMoney(&gSaveBlock1Ptr->money, paid);
    VarSet(VAR_ISEKAI_FINE_DEBT, debt);
    ConvertIntToDecimalStringN(gStringVar1, paid, STR_CONV_MODE_LEFT_ALIGN, 6);
    ConvertIntToDecimalStringN(gStringVar2, debt, STR_CONV_MODE_LEFT_ALIGN, 6);
    gSpecialVar_Result = debt == 0 ? 0 : (paid != 0 ? 1 : 2);
}

// After a battle: the police take what's owed from his winnings.
EWRAM_DATA static u16 sPendingDebtTaken = 0;
static void CollectFineDebt(void)
{
    u32 debt = VarGet(VAR_ISEKAI_FINE_DEBT);
    u32 money = GetMoney(&gSaveBlock1Ptr->money);
    u32 take;

    if (debt == 0 || money == 0)
        return;
    take = money < debt ? money : debt;
    RemoveMoney(&gSaveBlock1Ptr->money, take);
    VarSet(VAR_ISEKAI_FINE_DEBT, debt - take);
    sPendingDebtTaken += take;
}

// A small "Saving..." box in the bottom-right corner, shown while a field autosave runs.
static const struct WindowTemplate sSavingWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 21,     // bottom-right corner, clear of the map name popup
    .tilemapTop = 17,
    .width = 8,
    .height = 2,
    .paletteNum = 15,
    .baseBlock = 0x170,    // free tiles between the map popup (0x107-) and the textbox (0x194-)
};
static const u8 sText_Saving[] = _("Saving…");
EWRAM_DATA static u8 sSavingWindowId = 0;
EWRAM_DATA static bool8 sSavingBoxShown = FALSE;

void IsekaiShowSavingBox(void)
{
    sSavingWindowId = AddWindow(&sSavingWindowTemplate);
    if (sSavingWindowId == WINDOW_NONE)
        return;
    sSavingBoxShown = TRUE;
    DrawStdWindowFrame(sSavingWindowId, FALSE);
    FillWindowPixelBuffer(sSavingWindowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(sSavingWindowId, FONT_NORMAL, sText_Saving, 0, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sSavingWindowId, COPYWIN_FULL);
}

// Called by the "Saving..." field script, a couple of frames after the box is shown.
void IsekaiDoFieldAutosave(void)
{
    if (sIsekai.battleType == ISEKAI_BATTLE_NONE) // (a reset battle about to resume isn't overwritten)
    {
        SaveNow();
        if (IsSaveBuilding())
            sLastAutosaveMap = CurrentMapKey();
    }
    if (sSavingBoxShown)
    {
        ClearStdWindowAndFrame(sSavingWindowId, TRUE);
        RemoveWindow(sSavingWindowId);
        sSavingBoxShown = FALSE;
    }
}

void IsekaiRequestAutosave(void)
{
    sAutosavePending = TRUE;
}

// ---------------------------------------------------------------------------
// Battle start: mark the battle as in progress, apply the reset penalty, save.
// ---------------------------------------------------------------------------

static void BoostMonLevel(struct Pokemon *mon, u8 boost)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u32 level, exp;

    if (species == SPECIES_NONE || boost == 0)
        return;
    level = GetMonData(mon, MON_DATA_LEVEL) + boost;
    if (level > MAX_LEVEL)
        level = MAX_LEVEL;
    exp = gExperienceTables[gSpeciesInfo[species].growthRate][level];
    SetMonData(mon, MON_DATA_EXP, &exp);
    CalculateMonStats(mon);
    exp = GetMonData(mon, MON_DATA_MAX_HP);
    SetMonData(mon, MON_DATA_HP, &exp);
}

static bool32 IsekaiRulesApply(void)
{
    if (!IS_FRLG)
        return FALSE;
    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED | BATTLE_TYPE_FRONTIER
                          | BATTLE_TYPE_SAFARI | BATTLE_TYPE_CATCH_TUTORIAL | BATTLE_TYPE_TRAINER_HILL))
        return FALSE;
    return TRUE;
}

static void RecordAliveAtStart(void)
{
    u8 i;
    sAliveAtStart = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && GetMonData(mon, MON_DATA_HP) != 0)
            sAliveAtStart |= 1 << i;
    }
}

// Special foes: autosaved at the start and end of the battle, and the only ones who
// carry the reset penalty (and notice it). Gym leaders, BLUE, GIOVANNI, the ELITE FOUR
// and the CHAMPION by class; a few hand-picked tough trainers by id (added as their
// scenes are built: the staring Viridian Forest kid, KEVIN, AMEDEE).
static const u16 sIsekaiSpecialTrainers[] =
{
    TRAINER_ISEKAI_KEVIN,
};

static bool32 IsSpecialFoe(void)
{
    u32 i;
    u16 id = TRAINER_BATTLE_PARAM.opponentA;

    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return FALSE;
    switch (GetTrainerClassFromId(id))
    {
    case TRAINER_CLASS_LEADER_FRLG:
    case TRAINER_CLASS_ELITE_FOUR_FRLG:
    case TRAINER_CLASS_CHAMPION_FRLG:
    case TRAINER_CLASS_RIVAL_EARLY_FRLG:
    case TRAINER_CLASS_RIVAL_LATE_FRLG:
    case TRAINER_CLASS_BOSS_FRLG:
    case TRAINER_CLASS_LEADER:
    case TRAINER_CLASS_ELITE_FOUR:
    case TRAINER_CLASS_CHAMPION:
        return TRUE;
    default:
        break;
    }
    for (i = 0; i < ARRAY_COUNT(sIsekaiSpecialTrainers); i++)
        if (sIsekaiSpecialTrainers[i] != TRAINER_NONE && sIsekaiSpecialTrainers[i] == id)
            return TRUE;
    return FALSE;
}

// Difficulty: trainers are a little tougher on NORMAL, tougher again on HARD (on top of the
// hand-made HARD teams in trainers_frlg.party). Not the lab fight, not KEVIN (fixed).
static u8 DifficultyLevelBoost(void)
{
    if (IsekaiForfeitForbidden() == ISEKAI_NOFORFEIT_OAK
     || TRAINER_BATTLE_PARAM.opponentA == TRAINER_ISEKAI_KEVIN)
        return 0;
    switch (VarGet(VAR_ISEKAI_DIFFICULTY))
    {
    case DIFFICULTY_HARD:   return 2;
    case DIFFICULTY_NORMAL: return 1;
    default:                return 0;
    }
}

static void BoostOpponents(u8 boost)
{
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        BoostMonLevel(&gParties[B_TRAINER_OPPONENT_A][i], boost);
        if (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS)
            BoostMonLevel(&gParties[B_TRAINER_OPPONENT_B][i], boost);
    }
}

STATIC_ASSERT(offsetof(struct IsekaiSaveData, wardMons) <= SAVE_BLOCK_3_CHUNK_SIZE, IsekaiHotFieldsInSectorZero);

// Called by DoTrainerBattle once the opponent's party exists.
void IsekaiOnTrainerBattleStart(void)
{
    if (!IsekaiRulesApply())
        return;
    RecordAliveAtStart();
    if (GetTrainerClassFromId(TRAINER_BATTLE_PARAM.opponentA) == TRAINER_CLASS_TEAM_ROCKET_FRLG)
        IsekaiMaybeGiveGruntSoldMon(); // the auctioned POKéMON, still wearing his nickname
    if (!IsSpecialFoe() || gIsekaiTrainerArgs == NULL)
    {
        BoostOpponents(DifficultyLevelBoost());
        // Ordinary trainer: no save at the start, no reset penalty.
        sIsekai.battleType = ISEKAI_BATTLE_NONE;
        sResumingBattle = FALSE;
        return;
    }
    if (!sResumingBattle)
        sIsekai.resetCount = 0;
    sResumingBattle = FALSE;
    sIsekai.battleType = ISEKAI_BATTLE_TRAINER;
    sIsekai.trainerArgs = (u32)gIsekaiTrainerArgs;
    sIsekai.battleOpponent = TRAINER_BATTLE_PARAM.opponentA;
    BoostOpponents(DifficultyLevelBoost() + sIsekai.resetCount * 3);
    // Saved once the battle transition has gone black (IsekaiOnBattleInit), so the pause is hidden.
    sBattleStartSavePending = TRUE;
}

// Called as the battle screen sets up, while it's still black.
void IsekaiOnBattleInit(void)
{
    if (!sBattleStartSavePending)
        return;
    sBattleStartSavePending = FALSE;
    SaveNow();
}

// Called when a wild battle starts (grass, water, scripted).
void IsekaiOnWildBattleStart(void)
{
    struct Pokemon *wild = &gParties[B_TRAINER_OPPONENT_A][0];

    if (!IsekaiRulesApply())
        return;
    if (!sResumingBattle)
    {
        sIsekai.resetCount = 0;
        gIsekaiWildUnwilling = !IsekaiRollWillingness(wild);
    }
    else
    {
        gIsekaiWildUnwilling = !sIsekai.wildWilling;
    }
    sResumingBattle = FALSE;
    sLastWildSpecies = GetMonData(wild, MON_DATA_SPECIES);
    // Wild battles aren't saved at the start: resetting during one just reloads the last
    // save (only trainer battles carry the reset penalty). The end of the battle is saved.
    sIsekai.battleType = ISEKAI_BATTLE_NONE;
    RecordAliveAtStart();
}

// ---------------------------------------------------------------------------
// After a reset: the battle that was in progress starts again, harder.
// ---------------------------------------------------------------------------

static const u8 sText_TryToReload[] = _("You try to reload.\nNothing happens.\pThis isn't a game.");

// Field: is there a battle to resume? Fills VAR_0x8004 (type), VAR_0x8005 (first time).
void IsekaiPrepareResume(void)
{
    gSpecialVar_0x8004 = sIsekai.battleType;
    {
        u8 cls = GetTrainerClassFromId(sIsekai.battleOpponent);
        gSpecialVar_0x8006 = (sIsekai.battleType == ISEKAI_BATTLE_TRAINER
                           && (cls == TRAINER_CLASS_RIVAL_EARLY_FRLG || cls == TRAINER_CLASS_RIVAL_LATE_FRLG));
    }
    gSpecialVar_0x8005 = !sIsekai.resetMsgShown;
    sIsekai.resetMsgShown = TRUE;
    if (sIsekai.resetCount < 30)
        sIsekai.resetCount++;
    if (sIsekai.battleType == ISEKAI_BATTLE_WILD)
        StringCopy(gStringVar1, GetSpeciesName(sIsekai.battleOpponent));
    StringCopy(gStringVar2, sText_TryToReload);
}

void IsekaiCreateResumeWildMon(void)
{
    u32 level = sIsekai.wildLevel + sIsekai.resetCount * 3;
    if (level > MAX_LEVEL)
        level = MAX_LEVEL;
    CreateScriptedWildMon(sIsekai.battleOpponent, level, ITEM_NONE);
    sResumingBattle = TRUE;
}

void IsekaiResumeTrainerBattle(void)
{
    sResumingBattle = TRUE;
    IsekaiRestartTrainerBattleFromArgs((const u8 *)sIsekai.trainerArgs);
}

// ---------------------------------------------------------------------------
// Battle end: injuries, deaths, lives, then save before anything can be undone.
// ---------------------------------------------------------------------------

static bool32 PlayerLost(void)
{
    return gBattleOutcome == B_OUTCOME_LOST || gBattleOutcome == B_OUTCOME_DREW;
}

static bool32 IsRocketBattle(void)
{
    u8 trainerClass;
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return FALSE;
    trainerClass = GetTrainerClassFromId(TRAINER_BATTLE_PARAM.opponentA);
    return trainerClass == TRAINER_CLASS_TEAM_ROCKET_FRLG || trainerClass == TRAINER_CLASS_BOSS_FRLG;
}

void IsekaiOnBattleEnd(void)
{
    u8 i, oppLevel;
    bool32 anyUnprotected = FALSE;
    bool32 forfeited = (gBattleOutcome == B_OUTCOME_FORFEITED);
    bool32 special = IsSpecialFoe();

    if (!IsekaiRulesApply())
        return;

    IsekaiCountFights();
    sIsekai.battleType = ISEKAI_BATTLE_NONE;
    sIsekai.resetCount = 0;
    if (gBattleOutcome == B_OUTCOME_WON)
        CollectFineDebt();
    if ((gBattleTypeFlags & BATTLE_TYPE_TRAINER) && TRAINER_BATTLE_PARAM.opponentA == TRAINER_ISEKAI_SHARK_GOON
     && gBattleOutcome != B_OUTCOME_WON)
    {
        IsekaiSharkTakesAll();
        IsekaiTonyWillTake();
    }
    if (gBattleTypeFlags & BATTLE_TYPE_TRAINER || gBattleOutcome == B_OUTCOME_WON
     || gBattleOutcome == B_OUTCOME_CAUGHT || gBattleOutcome == B_OUTCOME_LOST)
        IsekaiWardBattleFought();
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        IsekaiCatchCooldownTick();

    oppLevel = OpponentMaxLevel();
    sPendingInjuries = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u16 one = 1;

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        if (GetMonData(mon, MON_DATA_HP) != 0 || !(sAliveAtStart & (1 << i)))
            continue;
        if (IsProtectedFaint(mon, oppLevel))
        {
            // The first time a faint doesn't count, say why (once per game).
            if (!FlagGet(FLAG_ISEKAI_SEEN_PROTECTED) && !sPendingProtectedNote)
            {
                GetMonData(mon, MON_DATA_NICKNAME, sProtectedName);
                sPendingProtectedNote = TRUE;
            }
            continue;
        }
        anyUnprotected = TRUE;
        if (IsekaiIsBadlyInjured(mon))
        {
            // Second time down: it doesn't get back up.
            if (sPendingDeaths < PARTY_SIZE)
                GetMonData(mon, MON_DATA_NICKNAME, sPendingDeathNames[sPendingDeaths++]);
            if (IsekaiIsLoaner(mon))
                FlagSet(FLAG_ISEKAI_LOANER_DIED);
            BuryMon(mon);
            ZeroMonData(mon);
        }
        else
        {
            IsekaiSetBadlyInjured(mon, TRUE);
            SetMonData(mon, MON_DATA_HP, &one);
            GetMonData(mon, MON_DATA_NICKNAME, sPendingInjuryNames[sPendingInjuries++]);
        }
    }
    CompactPartySlots();
    CalculatePlayerPartyCount();

    sLastDefeat = ISEKAI_DEFEAT_NONE;
    if (PlayerLost() && !forfeited)
    {
        if (!anyUnprotected)
            sLastDefeat = ISEKAI_DEFEAT_PROTECTED;
        else
        {
            if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
                sLastDefeat = ISEKAI_DEFEAT_WILD;
            else if (IsRocketBattle())
                sLastDefeat = ISEKAI_DEFEAT_ROCKET;
            else
                sLastDefeat = ISEKAI_DEFEAT_TRAINER;
            if (sIsekai.lives > 0)
                sIsekai.lives--;
        }
    }

    // Save only when something important happened (a death, a new injury, a lost life),
    // or after a special foe. Otherwise the last save stands.
    if (!special && sPendingDeaths == 0 && sPendingInjuries == 0
     && (sLastDefeat == ISEKAI_DEFEAT_NONE || sLastDefeat == ISEKAI_DEFEAT_PROTECTED))
        return;

    // Whiteouts heal and warp to a Center; write that outcome so a reset can't dodge it.
    if (PlayerLost() || forfeited || CalculatePlayerPartyCount() == 0)
    {
        if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER) || !TRAINER_BATTLE_PARAM.earlyRival)
        {
            HealPlayerParty();
            gSaveBlock1Ptr->continueGameWarp = gSaveBlock1Ptr->lastHealLocation;
            gSaveBlock2Ptr->specialSaveWarpFlags |= CONTINUE_GAME_WARP;
        }
    }
    SaveNow(); // the screen is black here
    gSaveBlock2Ptr->specialSaveWarpFlags &= ~CONTINUE_GAME_WARP;
}

// Walking through grass with no Pokémon at all: it costs a life, and he wakes up in the ward.
void IsekaiAttackedWithNoTeam(void)
{
    sLastDefeat = ISEKAI_DEFEAT_GRASS;
    if (sIsekai.lives > 0)
        sIsekai.lives--;
    gSaveBlock1Ptr->continueGameWarp = gSaveBlock1Ptr->lastHealLocation;
    gSaveBlock2Ptr->specialSaveWarpFlags |= CONTINUE_GAME_WARP;
    SaveNow();
    gSaveBlock2Ptr->specialSaveWarpFlags &= ~CONTINUE_GAME_WARP;
    SetMainCallback2(CB2_WhiteOut);
}

u8 IsekaiLastDefeat(void)
{
    return sLastDefeat;
}

u16 IsekaiLastWildSpecies(void)
{
    return sLastWildSpecies;
}

u8 IsekaiLives(void)
{
    return sIsekai.lives;
}

// A badge gives a life back, up to the starting number.
void IsekaiBadgeEarned(void)
{
    // Still hasn't brought Oak's POKéMON back by the first badge: too late now.
    if (VarGet(VAR_ISEKAI_LOAN_STATE) == 4)
    {
        VarSet(VAR_ISEKAI_LOAN_STATE, 7);
        FlagSet(FLAG_HIDE_ISEKAI_LAB_JENNY);
        sPendingCall = 3;
    }
    if (sIsekai.lives < IsekaiMaxLives())
        sIsekai.lives++;
}

// For the nurse: puts lives left in STR_VAR_1 and VAR_RESULT.
void IsekaiBufferLives(void)
{
    ConvertIntToDecimalStringN(gStringVar1, sIsekai.lives, STR_CONV_MODE_LEFT_ALIGN, 1);
    gSpecialVar_Result = sIsekai.lives;
}

// After the whiteout: which wake-up scene to play.
// VAR_RESULT: 0 waiting room, 1 ward, 2 game over, 3 forfeit.
// VAR_0x8004: how it happened (ISEKAI_DEFEAT_*). STR_VAR_1: lives left. STR_VAR_2: the wild Pokémon.
void IsekaiGetWakeUpScene(void)
{
    if (sLastDefeat == ISEKAI_DEFEAT_NONE)
        gSpecialVar_Result = 3;
    else if (sLastDefeat == ISEKAI_DEFEAT_PROTECTED)
        gSpecialVar_Result = 0;
    else if (sIsekai.lives == 0)
        gSpecialVar_Result = 2;
    else
        gSpecialVar_Result = 1;
    gSpecialVar_0x8004 = sLastDefeat;
    sPendingInjuries = 0; // the nurse is about to talk about them anyway
    sPendingProtectedNote = FALSE;
    ConvertIntToDecimalStringN(gStringVar1, sIsekai.lives, STR_CONV_MODE_LEFT_ALIGN, 1);
    StringCopy(gStringVar2, GetSpeciesName(sLastWildSpecies));
    sLastDefeat = ISEKAI_DEFEAT_NONE;
}

// Is the saved trainer battle still a valid script address in this ROM? If not, forget it.
void IsekaiCheckResumeValid(void)
{
    u32 args = sIsekai.trainerArgs;
    gSpecialVar_Result = TRUE;
    if (sIsekai.battleType == ISEKAI_BATTLE_TRAINER && (args < 0x08000000 || args >= 0x0A000000))
    {
        sIsekai.battleType = ISEKAI_BATTLE_NONE;
        sIsekai.resetCount = 0;
        gSpecialVar_Result = FALSE;
    }
}

// After the first protected faint: STR_VAR_1 = its name. VAR_RESULT: 0 none, 1 before the badge, 2 after.
void IsekaiNextProtectedNote(void)
{
    gSpecialVar_Result = 0;
    if (!sPendingProtectedNote)
        return;
    sPendingProtectedNote = FALSE;
    FlagSet(FLAG_ISEKAI_SEEN_PROTECTED);
    StringCopy(gStringVar1, sProtectedName);
    StringGet_Nickname(gStringVar1);
    gSpecialVar_Result = FlagGet(FLAG_BADGE01_GET) ? 2 : 1;
}

// The end. The save is wiped: there's no loading your way out of this one.
void IsekaiGameOver(void)
{
    BackgroundSave_Finish();
    ClearSaveData();
    DoSoftReset();
}

// ---------------------------------------------------------------------------
// Field messages after a battle (deaths are always quiet, never a joke)
// ---------------------------------------------------------------------------

// VAR_RESULT = TRUE if there's a death message to show; the name goes in STR_VAR_1.
void IsekaiNextDeathMessage(void)
{
    u8 i;
    if (sPendingDeaths == 0)
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    StringCopy(gStringVar1, sPendingDeathNames[0]);
    StringGet_Nickname(gStringVar1);
    for (i = 1; i < sPendingDeaths; i++)
        memcpy(sPendingDeathNames[i - 1], sPendingDeathNames[i], sizeof(sPendingDeathNames[0]));
    sPendingDeaths--;
    gSpecialVar_Result = TRUE;
}

// VAR_RESULT = TRUE if a Pokémon was newly Badly Injured; its name goes in STR_VAR_1.
void IsekaiNextInjuryMessage(void)
{
    u8 i;
    if (sPendingInjuries == 0)
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    StringCopy(gStringVar1, sPendingInjuryNames[0]);
    StringGet_Nickname(gStringVar1);
    for (i = 1; i < sPendingInjuries; i++)
        memcpy(sPendingInjuryNames[i - 1], sPendingInjuryNames[i], sizeof(sPendingInjuryNames[0]));
    sPendingInjuries--;
    gSpecialVar_Result = TRUE;
}

// ---------------------------------------------------------------------------
// The field hook: runs whenever the player is free to move.
// ---------------------------------------------------------------------------

bool32 IsekaiFieldIdle(void)
{
    if (!IS_FRLG || gMain.callback2 != CB2_Overworld)
        return FALSE;

    if (sIsekai.battleType != ISEKAI_BATTLE_NONE)
    {
        LockPlayerFieldControls();
        ScriptContext_SetupScript(IsekaiEventScript_ResumeBattle);
        return TRUE;
    }
    if (sIsekai.lives == 0 && FlagGet(FLAG_SYS_POKEMON_GET))
    {
        LockPlayerFieldControls();
        ScriptContext_SetupScript(IsekaiEventScript_GameOver);
        return TRUE;
    }
    if (sPendingDeaths != 0 || sPendingInjuries != 0 || sPendingProtectedNote)
    {
        LockPlayerFieldControls();
        ScriptContext_SetupScript(IsekaiEventScript_AfterBattleNews);
        return TRUE;
    }
    if (sPendingCall != 0)
    {
        gSpecialVar_0x8004 = sPendingCall;
        sPendingCall = 0;
        LockPlayerFieldControls();
        ScriptContext_SetupScript(IsekaiEventScript_OakCall);
        return TRUE;
    }
    if (IsekaiMoneyFieldIdle())
        return TRUE;
    if (sPendingDebtTaken != 0)
    {
        ConvertIntToDecimalStringN(gStringVar1, sPendingDebtTaken, STR_CONV_MODE_LEFT_ALIGN, 6);
        ConvertIntToDecimalStringN(gStringVar2, VarGet(VAR_ISEKAI_FINE_DEBT), STR_CONV_MODE_LEFT_ALIGN, 6);
        gSpecialVar_0x8004 = VarGet(VAR_ISEKAI_FINE_DEBT) == 0;
        sPendingDebtTaken = 0;
        LockPlayerFieldControls();
        ScriptContext_SetupScript(IsekaiEventScript_FineFromWinnings);
        return TRUE;
    }
    if (sPendingWantedReminder)
    {
        sPendingWantedReminder = FALSE;
        gSpecialVar_0x8004 = Random() % 3;
        LockPlayerFieldControls();
        ScriptContext_SetupScript(IsekaiEventScript_WantedReminder);
        return TRUE;
    }
    // (v0.10.5: no building or zone autosaves; players save from the START menu.
    // Something can still ask for a field autosave, which shows a small "Saving..." box.)
    if (sAutosavePending && !(gMain.heldKeys & DPAD_ANY))
    {
        sArrivalSavePending = FALSE;
        sZoneSavePending = FALSE;
        sAutosavePending = FALSE;
        LockPlayerFieldControls();
        ScriptContext_SetupScript(IsekaiEventScript_Saving);
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// Forfeiting
// ---------------------------------------------------------------------------

// Gym leaders, the Elite Four and the Champion, Giovanni, Red and the hidden trainers.
// Returns 0 if forfeiting is allowed, else ISEKAI_NOFORFEIT_* (picks the message).
u32 IsekaiForfeitForbidden(void)
{
    u8 trainerClass;
    if (!IS_FRLG || !(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return 0;
    if (TRAINER_BATTLE_PARAM.earlyRival
     && GetTrainerClassFromId(TRAINER_BATTLE_PARAM.opponentA) == TRAINER_CLASS_RIVAL_EARLY_FRLG
     && gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB)
     && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB))
        return ISEKAI_NOFORFEIT_OAK; // Oak is watching
    // Red and the hidden trainers (Chapter 3) will return ISEKAI_NOFORFEIT_PEAK.
    trainerClass = GetTrainerClassFromId(TRAINER_BATTLE_PARAM.opponentA);
    switch (trainerClass)
    {
    case TRAINER_CLASS_LEADER_FRLG:
    case TRAINER_CLASS_ELITE_FOUR_FRLG:
    case TRAINER_CLASS_CHAMPION_FRLG:
    case TRAINER_CLASS_LEADER:
    case TRAINER_CLASS_ELITE_FOUR:
    case TRAINER_CLASS_CHAMPION:
        return ISEKAI_NOFORFEIT_LEADER;
    case TRAINER_CLASS_BOSS_FRLG:
        return ISEKAI_NOFORFEIT_ROCKET;
    default:
        return 0;
    }
}

// ---------------------------------------------------------------------------
// Healing injuries at a Center (the ward)
// ---------------------------------------------------------------------------

static u8 CountWardMons(void)
{
    u8 i, count = 0;
    for (i = 0; i < ISEKAI_WARD_SLOTS; i++)
        if (GetBoxMonData((struct BoxPokemon *)sIsekai.wardMons[i], MON_DATA_SPECIES) != SPECIES_NONE)
            count++;
    return count;
}

// VAR_RESULT = how many party Pokémon are Badly Injured; STR_VAR_1 = the first one's name.
void IsekaiCountInjuredInParty(void)
{
    u8 i, count = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && IsekaiIsBadlyInjured(mon))
        {
            if (count == 0)
            {
                GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
                StringGet_Nickname(gStringVar1);
            }
            count++;
        }
    }
    gSpecialVar_Result = count;
    ConvertIntToDecimalStringN(gStringVar2, IsekaiWardBattles(), STR_CONV_MODE_LEFT_ALIGN, 1);
}

// Leaves every Badly Injured party Pokémon at the Center. If that would leave the MC
// with nothing, VAR_0x8004 = TRUE and the caller offers the grumpy loaner.
void IsekaiLeaveInjuredAtCenter(void)
{
    u8 i, j;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || !IsekaiIsBadlyInjured(mon))
            continue;
        for (j = 0; j < ISEKAI_WARD_SLOTS; j++)
        {
            if (GetBoxMonData((struct BoxPokemon *)sIsekai.wardMons[j], MON_DATA_SPECIES) == SPECIES_NONE)
            {
                memcpy(sIsekai.wardMons[j], &mon->box, ISEKAI_BOXMON_SIZE);
                sIsekai.wardBattlesLeft[j] = IsekaiWardBattles();
                ZeroMonData(mon);
                break;
            }
        }
    }
    CompactPartySlots();
    gSpecialVar_0x8004 = (CalculatePlayerPartyCount() == 0);
}

// Would leaving the injured ones empty the party? VAR_RESULT = TRUE if so.
void IsekaiWouldLeaveEmptyParty(void)
{
    u8 i, healthy = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG) && !IsekaiIsBadlyInjured(mon))
            healthy++;
    }
    gSpecialVar_Result = (healthy == 0);
}

void IsekaiWardBattleFought(void)
{
    u8 i;
    for (i = 0; i < ISEKAI_WARD_SLOTS; i++)
        if (sIsekai.wardBattlesLeft[i] > 0)
            sIsekai.wardBattlesLeft[i]--;
}

// At a Center: give back every Pokémon that has finished healing.
// VAR_RESULT = how many came back; STR_VAR_1 = the first one's name.
void IsekaiCollectHealedMons(void)
{
    u8 i, count = 0;
    struct Pokemon mon;

    for (i = 0; i < ISEKAI_WARD_SLOTS; i++)
    {
        struct BoxPokemon *boxMon = (struct BoxPokemon *)sIsekai.wardMons[i];
        if (GetBoxMonData(boxMon, MON_DATA_SPECIES) == SPECIES_NONE || sIsekai.wardBattlesLeft[i] > 0)
            continue;
        BoxMonToMon(boxMon, &mon);
        IsekaiSetBadlyInjured(&mon, FALSE);
        MonRestorePP(&mon);
        {
            u16 hp = GetMonData(&mon, MON_DATA_MAX_HP);
            SetMonData(&mon, MON_DATA_HP, &hp);
        }
        if (count == 0)
        {
            GetMonData(&mon, MON_DATA_NICKNAME, gStringVar1);
            StringGet_Nickname(gStringVar1);
        }
        if (CalculatePlayerPartyCount() < PARTY_SIZE)
        {
            gParties[B_TRAINER_PLAYER][CalculatePlayerPartyCount()] = mon;
            CalculatePlayerPartyCount();
        }
        else
        {
            CopyMonToPC(&mon);
        }
        memset(sIsekai.wardMons[i], 0, ISEKAI_BOXMON_SIZE);
        count++;
    }
    gSpecialVar_Result = count;
    // When the last one is back, the grumpy loaner goes home.
    if (count != 0 && sIsekai.loanerOut && CountWardMons() == 0)
        gSpecialVar_0x8004 = IsekaiReturnCenterLoaner();
    else
        gSpecialVar_0x8004 = FALSE;
}

// VAR_RESULT = Pokémon still healing; STR_VAR_2 = battles left for the closest one.
void IsekaiWardStatus(void)
{
    u8 i, count = 0, best = 255;
    for (i = 0; i < ISEKAI_WARD_SLOTS; i++)
    {
        if (GetBoxMonData((struct BoxPokemon *)sIsekai.wardMons[i], MON_DATA_SPECIES) != SPECIES_NONE)
        {
            count++;
            if (sIsekai.wardBattlesLeft[i] < best)
                best = sIsekai.wardBattlesLeft[i];
        }
    }
    gSpecialVar_Result = count;
    ConvertIntToDecimalStringN(gStringVar2, best == 255 ? 0 : best, STR_CONV_MODE_LEFT_ALIGN, 1);
}

// The grumpy loaner: a MANKEY at about the level of the MC's strongest Pokémon.
void IsekaiGiveCenterLoaner(void)
{
    u8 i, level = 5;
    struct Pokemon *mon;

    for (i = 0; i < ISEKAI_WARD_SLOTS; i++)
    {
        struct BoxPokemon *boxMon = (struct BoxPokemon *)sIsekai.wardMons[i];
        if (GetBoxMonData(boxMon, MON_DATA_SPECIES) != SPECIES_NONE && GetLevelFromBoxMonExp(boxMon) > level)
            level = GetLevelFromBoxMonExp(boxMon);
    }
    ScriptGiveMon(SPECIES_MANKEY, level, ITEM_NONE);
    mon = &gParties[B_TRAINER_PLAYER][CalculatePlayerPartyCount() - 1];
    sIsekai.loanerPersonality = GetMonData(mon, MON_DATA_PERSONALITY);
    sIsekai.loanerOut = TRUE;
}

static bool32 IsCenterLoaner(struct Pokemon *mon)
{
    return sIsekai.loanerOut
        && GetMonData(mon, MON_DATA_SPECIES) == SPECIES_MANKEY
        && GetMonData(mon, MON_DATA_PERSONALITY) == sIsekai.loanerPersonality;
}

bool32 IsekaiReturnCenterLoaner(void)
{
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (IsCenterLoaner(mon) && CalculatePlayerPartyCount() > 1)
        {
            ZeroMonData(mon);
            CompactPartySlots();
            CalculatePlayerPartyCount();
            sIsekai.loanerOut = FALSE;
            return TRUE;
        }
    }
    return FALSE;
}

// The grumpy loaner ignores the MC about one turn in four.
bool32 IsekaiCenterLoanerIgnores(u32 battler)
{
    if (!IS_FRLG || !sIsekai.loanerOut || GetBattlerSide(battler) != B_SIDE_PLAYER)
        return FALSE;
    if (gBattleMons[battler].species != SPECIES_MANKEY || gBattleMons[battler].personality != sIsekai.loanerPersonality)
        return FALSE;
    return (Random() % 4) == 0;
}

// ---------------------------------------------------------------------------
// Capture willingness
// ---------------------------------------------------------------------------

static u8 CountBoxedMons(void)
{
    u32 box, pos, count = 0;
    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
            if (GetBoxMonDataAt(box, pos, MON_DATA_SPECIES) != SPECIES_NONE)
                count++;
    return count > 255 ? 255 : count;
}

static u8 StrongestPartyLevel(void)
{
    u8 i, level = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG)
         && GetMonData(mon, MON_DATA_LEVEL) > level)
            level = GetMonData(mon, MON_DATA_LEVEL);
    }
    return level;
}

static struct IsekaiCatch *FindCatchSlot(u16 mapsec, u16 species, bool32 create)
{
    u8 i, oldest = 0;
    for (i = 0; i < ISEKAI_CATCH_SLOTS; i++)
        if (sIsekai.catches[i].count != 0 && sIsekai.catches[i].mapsec == mapsec && sIsekai.catches[i].species == species)
            return &sIsekai.catches[i];
    if (!create)
        return NULL;
    for (i = 0; i < ISEKAI_CATCH_SLOTS; i++)
    {
        if (sIsekai.catches[i].count == 0)
            break;
        if (sIsekai.catches[i].cooldown < sIsekai.catches[oldest].cooldown)
            oldest = i;
    }
    if (i == ISEKAI_CATCH_SLOTS)
        i = oldest;
    sIsekai.catches[i].mapsec = mapsec;
    sIsekai.catches[i].species = species;
    sIsekai.catches[i].count = 0;
    sIsekai.catches[i].cooldown = 0;
    return &sIsekai.catches[i];
}

static bool32 IsOverCaught(u16 species)
{
    struct IsekaiCatch *slot = FindCatchSlot(gMapHeader.regionMapSectionId, species, FALSE);
    return slot != NULL && slot->count >= 3 && slot->cooldown > 0;
}

// Percent chance (50-95) that a wild Pokémon here is willing. Romances arrive in v0.13.
u8 IsekaiWillingnessPercent(struct Pokemon *wild)
{
    s32 chance = 75;
    u8 strongest = StrongestPartyLevel();
    u8 wildLevel = GetMonData(wild, MON_DATA_LEVEL);

    if (strongest >= wildLevel + 10)
        chance += 10;
    else if (strongest + 10 <= wildLevel)
        chance -= 10;
    chance += 2 * CountBadges();
    chance -= min(2 * sIsekai.graveCount, 10);
    chance += min(2 * sIsekai.releasedCount, 10);
    if (CountBoxedMons() > 30)
        chance -= 5;
    if (chance < 50)
        chance = 50;
    if (chance > 95)
        chance = 95;
    return chance;
}

bool8 IsekaiRollWillingness(struct Pokemon *wild)
{
    u16 species = GetMonData(wild, MON_DATA_SPECIES);

    if (!IS_FRLG)
        return TRUE;
    // Route 1 never soft-locks the opening, and scripted legendaries are their own story.
    // ...except the very first PIDGEY there, which teaches that not everything wants to come along.
    if (gMapHeader.regionMapSectionId == MAPSEC_ROUTE_1 && species == SPECIES_PIDGEY && !FlagGet(FLAG_ISEKAI_PIDGEY_REFUSED)
     && CheckBagHasItem(ITEM_POKE_BALL, 1))
    {
        FlagSet(FLAG_ISEKAI_PIDGEY_REFUSED);
        return FALSE;
    }
    if (gMapHeader.regionMapSectionId == MAPSEC_ROUTE_1 || (gBattleTypeFlags & BATTLE_TYPE_LEGENDARY))
        return TRUE;
    if (IsOverCaught(species))
        return FALSE;
    return (Random() % 100) < IsekaiWillingnessPercent(wild);
}

void IsekaiOnMonCaught(u16 species)
{
    struct IsekaiCatch *slot = FindCatchSlot(gMapHeader.regionMapSectionId, species, TRUE);
    if (slot->count < 255)
        slot->count++;
    if (slot->count >= 3)
        slot->cooldown = 10; // ten wild battles before they forget
}

void IsekaiCatchCooldownTick(void)
{
    u8 i;
    for (i = 0; i < ISEKAI_CATCH_SLOTS; i++)
    {
        if (sIsekai.catches[i].cooldown > 0)
        {
            sIsekai.catches[i].cooldown--;
            if (sIsekai.catches[i].cooldown == 0 && sIsekai.catches[i].count >= 3)
                sIsekai.catches[i].count = 0;
        }
    }
}

// Releasing a Pokémon on the route it came from (from the PC, the met location counts).
void IsekaiOnMonReleased(struct BoxPokemon *boxMon)
{
    if (GetBoxMonData(boxMon, MON_DATA_MET_LOCATION) == gMapHeader.regionMapSectionId
     && sIsekai.releasedCount < 5)
        sIsekai.releasedCount++;
}

// The over-caught message for the battle intro.
bool32 IsekaiWildIsOverCaught(void)
{
    return IsOverCaught(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES));
}
