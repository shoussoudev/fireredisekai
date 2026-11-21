#include "global.h"
#include "constants/fireredisekai_romance.h"
#include "config/fireredisekai.h"
#include "fireredisekai.h"
#include "clock.h"
#include "new_game.h"
#include "random.h"
#include "clock.h"
#include "pokemon.h"
#include "roamer.h"
#include "pokemon_size_record.h"
#include "script.h"
#include "lottery_corner.h"
#include "play_time.h"
#include "mauville_old_man.h"
#include "match_call.h"
#include "lilycove_lady.h"
#include "load_save.h"
#include "pokeblock.h"
#include "dewford_trend.h"
#include "berry.h"
#include "rtc.h"
#include "easy_chat.h"
#include "event_data.h"
#include "money.h"
#include "trainer_hill.h"
#include "trainer_tower.h"
#include "tv.h"
#include "coins.h"
#include "text.h"
#include "overworld.h"
#include "mail.h"
#include "battle_records.h"
#include "item.h"
#include "pokedex.h"
#include "apprentice.h"
#include "frontier_util.h"
#include "pokedex.h"
#include "save.h"
#include "link_rfu.h"
#include "main.h"
#include "contest.h"
#include "item_menu.h"
#include "pokemon_storage_system.h"
#include "pokemon_jump.h"
#include "decoration_inventory.h"
#include "secret_base.h"
#include "string_util.h"
#include "player_pc.h"
#include "field_specials.h"
#include "berry_powder.h"
#include "mystery_gift.h"
#include "union_room_chat.h"
#include "constants/map_groups.h"
#include "constants/items.h"
#include "difficulty.h"
#include "follower_npc.h"
#include "script_pokemon_util.h"
#include "constants/heal_locations.h"

extern const u8 EventScript_ResetAllMapFlags[];
extern const u8 EventScript_ResetAllMapFlagsFrlg[];

static void ClearFrontierRecord(void);
static void WarpToTruck(void);
static void ResetMiniGamesRecords(void);
static void ResetItemFlags(void);
static void ResetDexNav(void);

EWRAM_DATA bool8 gDifferentSaveFile = FALSE;
EWRAM_DATA bool8 gEnableContestDebugging = FALSE;

static const struct ContestWinner sContestWinnerPicDummy =
{
    .monName = _(""),
    .trainerName = _("")
};

void SetTrainerId(u32 trainerId, u8 *dst)
{
    dst[0] = trainerId;
    dst[1] = trainerId >> 8;
    dst[2] = trainerId >> 16;
    dst[3] = trainerId >> 24;
}

u32 GetTrainerId(u8 *trainerId)
{
    return (trainerId[3] << 24) | (trainerId[2] << 16) | (trainerId[1] << 8) | (trainerId[0]);
}

void CopyTrainerId(u8 *dst, u8 *src)
{
    s32 i;
    for (i = 0; i < TRAINER_ID_LENGTH; i++)
        dst[i] = src[i];
}

static void InitPlayerTrainerId(void)
{
    u32 trainerId = (Random() << 16) | GetGeneratedTrainerIdLower();
    SetTrainerId(trainerId, gSaveBlock2Ptr->playerTrainerId);
}

// L=A isnt set here for some reason.
static void SetDefaultOptions(void)
{
    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_MID;
    gSaveBlock2Ptr->optionsWindowFrameType = 0;
    gSaveBlock2Ptr->optionsSound = OPTIONS_SOUND_MONO;
    gSaveBlock2Ptr->optionsBattleStyle = OPTIONS_BATTLE_STYLE_SHIFT;
    gSaveBlock2Ptr->optionsBattleSceneOff = FALSE;
    gSaveBlock2Ptr->regionMapZoom = FALSE;
}

static void ClearPokedexFlags(void)
{
    memset(&gSaveBlock1Ptr->dexCaught, 0, sizeof(gSaveBlock1Ptr->dexCaught));
    memset(&gSaveBlock1Ptr->dexSeen, 0, sizeof(gSaveBlock1Ptr->dexSeen));
}

void ClearAllContestWinnerPics(void)
{
    s32 i;

    ClearContestWinnerPicsInContestHall();

    // Clear Museum paintings
    for (i = MUSEUM_CONTEST_WINNERS_START; i < NUM_CONTEST_WINNERS; i++)
        gSaveBlock1Ptr->contestWinners[i] = sContestWinnerPicDummy;
}

static void ClearFrontierRecord(void)
{
    CpuFill32(0, &gSaveBlock2Ptr->frontier, sizeof(gSaveBlock2Ptr->frontier));

    gSaveBlock2Ptr->frontier.opponentNames[0][0] = EOS;
    gSaveBlock2Ptr->frontier.opponentNames[1][0] = EOS;
}

static void WarpToTruck(void)
{
#if ISEKAI_START_AT == ISEKAI_START_VIRIDIAN
    // Developer checkpoint: just arrived in Viridian with the loaner, holding Oak's Parcel.
    VarSet(VAR_ISEKAI_INTRO, 4);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 4);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_OAK, 1);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_PLAYERS_HOUSE_2F, 1);
    VarSet(VAR_MAP_SCENE_VIRIDIAN_CITY_MART, 1);
    VarSet(VAR_STARTER_MON, 0);
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_ISEKAI_HAS_LOANER);
    FlagSet(FLAG_ISEKAI_HAS_PHONE);
    FlagSet(FLAG_BEAT_RIVAL_IN_OAKS_LAB);
    FlagClear(FLAG_HIDE_OAK_IN_HIS_LAB);
    FlagSet(FLAG_HIDE_OAK_IN_PALLET_TOWN);
    FlagSet(FLAG_HIDE_RIVAL_IN_LAB);
    FlagSet(FLAG_HIDE_BULBASAUR_BALL);
    VarSet(VAR_ISEKAI_LOAN_STATE, 2);
    AddBagItem(ITEM_POKE_BALL, 5);
    VarSet(VAR_ISEKAI_LOAN_DEPOSIT, 3000);
    SetMoney(&gSaveBlock1Ptr->money, 0);
    ScriptGiveMon(SPECIES_PSYDUCK, 7, ITEM_NONE);
    VarSet(VAR_ISEKAI_LOANER_SPECIES, SPECIES_PSYDUCK);
    IsekaiRememberLoaner();
    AddBagItem(ITEM_OAKS_PARCEL, 1);
    AddBagItem(ITEM_POTION, 2);
    SetLastHealLocationWarp(HEAL_LOCATION_VIRIDIAN_CITY);
    SetWarpDestination(MAP_GROUP(MAP_VIRIDIAN_CITY), MAP_NUM(MAP_VIRIDIAN_CITY), WARP_ID_NONE, 36, 20);
    WarpIntoMap();
    return;
#endif
#if ISEKAI_START_AT == ISEKAI_START_PEWTER
    // Developer checkpoint: arrived in Pewter. Parcel delivered, PSYDUCK returned on time and kept,
    // the POKéDEX, and a team that can take BROCK (Water, Grass, Fighting).
    VarSet(VAR_ISEKAI_INTRO, 4);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 6);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_OAK, 1);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_PLAYERS_HOUSE_2F, 1);
    VarSet(VAR_MAP_SCENE_VIRIDIAN_CITY_MART, 2);
    VarSet(VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 1);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_RIVALS_HOUSE, 1);
    VarSet(VAR_STARTER_MON, 0);
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    FlagSet(FLAG_ISEKAI_HAS_LOANER);
    FlagSet(FLAG_ISEKAI_HAS_PHONE);
    FlagSet(FLAG_BEAT_RIVAL_IN_OAKS_LAB);
    FlagSet(FLAG_ISEKAI_WARD_NURSE_MET);
    VarSet(VAR_ISEKAI_LOAN_STATE, 5);
    // Met at low levels (so they obey before the first BADGE), then trained up.
    ScriptGiveMon(SPECIES_PSYDUCK, 7, ITEM_NONE);
    VarSet(VAR_ISEKAI_LOANER_SPECIES, SPECIES_PSYDUCK);
    IsekaiRememberLoaner();
    ScriptGiveMon(SPECIES_ODDISH, 6, ITEM_NONE);
    ScriptGiveMon(SPECIES_MANKEY, 6, ITEM_NONE);
    {
        static const u8 sLevels[] = {14, 13, 13};
        u32 i;
        for (i = 0; i < 3; i++)
        {
            struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
            u32 exp = gExperienceTables[gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES)].growthRate][sLevels[i]];
            SetMonData(mon, MON_DATA_EXP, &exp);
            CalculateMonStats(mon);
        }
        {
            static const u16 sMoves[3][4] = {
                {MOVE_WATER_GUN, MOVE_CONFUSION, MOVE_SCRATCH, MOVE_TAIL_WHIP},
                {MOVE_ABSORB, MOVE_ACID, MOVE_POISON_POWDER, MOVE_GROWTH},
                {MOVE_LOW_KICK, MOVE_KARATE_CHOP, MOVE_FURY_SWIPES, MOVE_LEER},
            };
            u32 m, k;
            for (m = 0; m < 3; m++)
                for (k = 0; k < 4; k++)
                    SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][m], sMoves[m][k], k);
        }
    }
    AddBagItem(ITEM_POKE_BALL, 5);
    AddBagItem(ITEM_POTION, 8);
    AddBagItem(ITEM_ANTIDOTE, 2);
    SetMoney(&gSaveBlock1Ptr->money, 1500);
    SetLastHealLocationWarp(HEAL_LOCATION_PEWTER_CITY);
    SetWarpDestination(MAP_GROUP(MAP_PEWTER_CITY), MAP_NUM(MAP_PEWTER_CITY), WARP_ID_NONE, 17, 26);
    WarpIntoMap();
    return;
#endif
#if ISEKAI_START_AT == ISEKAI_START_RULES_TEST
    // Developer checkpoint: the core rules sandbox. Route 22 tall grass (next to Viridian), Oak's PSYDUCK at Lv10
    // (so its faints count) on 1 HP, a few healing items, Viridian as the last heal spot.
    {
        u16 hp = 1;
        VarSet(VAR_ISEKAI_INTRO, 4);
        VarSet(VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 3);
        VarSet(VAR_MAP_SCENE_PALLET_TOWN_OAK, 1);
        VarSet(VAR_MAP_SCENE_PALLET_TOWN_PLAYERS_HOUSE_2F, 1);
        VarSet(VAR_STARTER_MON, 0);
        FlagSet(FLAG_SYS_POKEMON_GET);
        FlagSet(FLAG_ISEKAI_HAS_LOANER);
        FlagSet(FLAG_BEAT_RIVAL_IN_OAKS_LAB);
        FlagSet(FLAG_ISEKAI_LIMPING); // to test the nurse's ankle line
        ScriptGiveMon(SPECIES_PSYDUCK, 10, ITEM_NONE);
        SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP, &hp);
        VarSet(VAR_ISEKAI_LOANER_SPECIES, SPECIES_PSYDUCK);
        IsekaiRememberLoaner();
        AddBagItem(ITEM_POKE_BALL, 10);
        AddBagItem(ITEM_POTION, 3);
        AddBagItem(ITEM_REVIVE, 2);
        AddBagItem(ITEM_MAX_REVIVE, 1);
        SetLastHealLocationWarp(HEAL_LOCATION_VIRIDIAN_CITY);
        SetWarpDestination(MAP_GROUP(MAP_ROUTE22), MAP_NUM(MAP_ROUTE22), WARP_ID_NONE, 37, 11); // in the tall grass
        WarpIntoMap();
        return;
    }
#endif
#if ISEKAI_START_AT == ISEKAI_START_OAKS_LAB
    // Developer checkpoint: skip the bedroom and Oak's escort, start the lab scene.
    VarSet(VAR_ISEKAI_INTRO, 4);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 1);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_OAK, 1);
    VarSet(VAR_MAP_SCENE_PALLET_TOWN_PLAYERS_HOUSE_2F, 1);
    FlagClear(FLAG_HIDE_OAK_IN_HIS_LAB);
    FlagSet(FLAG_HIDE_OAK_IN_PALLET_TOWN);
    FlagSet(FLAG_DONT_TRANSITION_MUSIC);
    SetWarpDestination(MAP_GROUP(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB), MAP_NUM(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB), WARP_ID_NONE, 6, 12);
    WarpIntoMap();
    return;
#endif
    if (IS_FRLG)
        SetWarpDestination(MAP_GROUP(MAP_PALLET_TOWN_PLAYERS_HOUSE_2F), MAP_NUM(MAP_PALLET_TOWN_PLAYERS_HOUSE_2F), WARP_ID_NONE, 3, 6); // FireRed Isekai: wake up beside the bed
    else
        SetWarpDestination(MAP_GROUP(MAP_INSIDE_OF_TRUCK), MAP_NUM(MAP_INSIDE_OF_TRUCK), WARP_ID_NONE, -1, -1);
    WarpIntoMap();
}

void Sav2_ClearSetDefault(void)
{
    ClearSav2();
    SetDefaultOptions();
}

void ResetMenuAndMonGlobals(void)
{
    gDifferentSaveFile = FALSE;
    ResetPokedexScrollPositions();
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    ResetBagScrollPositions();
    ResetPokeblockScrollPositions();
}

void NewGameInitData(void)
{
#if IS_FRLG
    u8 rivalName[PLAYER_NAME_LENGTH + 1];
#endif
    if (gSaveFileStatus == SAVE_STATUS_EMPTY || gSaveFileStatus == SAVE_STATUS_CORRUPT)
        RtcReset();

#if IS_FRLG
    StringCopy(rivalName, gSaveBlock1Ptr->rivalName);
#endif
    gDifferentSaveFile = TRUE;
    gSaveBlock2Ptr->encryptionKey = 0;
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    ResetPokedex();
    ClearFrontierRecord();
    ClearSav1();
    ClearSav3();
    ClearAllMail();
    gSaveBlock2Ptr->specialSaveWarpFlags = 0;
    gSaveBlock2Ptr->gcnLinkFlags = 0;
    InitPlayerTrainerId();
    PlayTimeCounter_Reset();
    ClearPokedexFlags();
    InitEventData();
    ClearTVShowData();
    ResetGabbyAndTy();
    ClearSecretBases();
    ClearBerryTrees();
    SetMoney(&gSaveBlock1Ptr->money, 3000);
    SetCoins(0);
    ResetLinkContestBoolean();
    ResetGameStats();
    ClearAllContestWinnerPics();
    ClearPlayerLinkBattleRecords();
    InitSeedotSizeRecord();
    InitLotadSizeRecord();
    gPartiesCount[B_TRAINER_PLAYER] = 0;
    ZeroPlayerPartyMons();
    ResetPokemonStorageSystem();
    DeactivateAllRoamers();
    gSaveBlock1Ptr->registeredItem = ITEM_NONE;
    ClearBag();
    NewGameInitPCItems();
    ClearPokeblocks();
    ClearDecorationInventories();
    InitEasyChatPhrases();
    SetMauvilleOldMan();
    InitDewfordTrend();
    ResetFanClub();
    ResetLotteryCorner();
    UpdateDailySeed();
    WarpToTruck();
    if (IS_FRLG)
        RunScriptImmediately(EventScript_ResetAllMapFlagsFrlg);
    else
        RunScriptImmediately(EventScript_ResetAllMapFlags);
#if IS_FRLG
        StringCopy(gSaveBlock1Ptr->rivalName, rivalName);
#endif
    ResetMiniGamesRecords();
    InitUnionRoomChatRegisteredTexts();
    InitLilycoveLady();
    ResetAllApprenticeData();
    ClearRankingHallRecords();
    InitMatchCallCounters();
    ClearMysteryGift();
    WipeTrainerNameRecords();
    ResetTrainerHillResults();
    ResetTrainerTowerResults();
    ResetContestLinkResults();
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
#if IS_FRLG
    // FireRed Isekai: the answer to Oak's sleep question (stored +1; 0 = not asked)
    if (gIsekaiChosenDifficulty != 0)
        SetCurrentDifficultyLevel(gIsekaiChosenDifficulty - 1);
    IsekaiNewGameInit(); // lives, running shoes
#if ISEKAI_START_AT == ISEKAI_START_VIRIDIAN || ISEKAI_START_AT == ISEKAI_START_PEWTER
    // (after the map flags were reset) Oak is back in his lab, Blue has left.
    FlagClear(FLAG_HIDE_OAK_IN_HIS_LAB);
    FlagSet(FLAG_HIDE_RIVAL_IN_LAB);
    FlagSet(FLAG_HIDE_BULBASAUR_BALL);
#endif
#endif
    ResetItemFlags();
    ResetDexNav();
    ClearFollowerNPCData();
}

static void ResetMiniGamesRecords(void)
{
    CpuFill16(0, &gSaveBlock2Ptr->berryCrush, sizeof(struct BerryCrush));
    SetBerryPowder(&gSaveBlock2Ptr->berryCrush.berryPowderAmount, 0);
    ResetPokemonJumpRecords();
    CpuFill16(0, &gSaveBlock2Ptr->berryPick, sizeof(struct BerryPickingResults));
}

static void ResetItemFlags(void)
{
#if OW_SHOW_ITEM_DESCRIPTIONS == OW_ITEM_DESCRIPTIONS_FIRST_TIME
    memset(&gSaveBlock3Ptr->itemFlags, 0, sizeof(gSaveBlock3Ptr->itemFlags));
#endif
}

static void ResetDexNav(void)
{
#if USE_DEXNAV_SEARCH_LEVELS == TRUE
    memset(gSaveBlock3Ptr->dexNavSearchLevels, 0, sizeof(gSaveBlock3Ptr->dexNavSearchLevels));
#endif
    gSaveBlock3Ptr->dexNavChain = 0;
}
