#ifndef GUARD_FIREREDISEKAI_H
#define GUARD_FIREREDISEKAI_H

// FireRed Isekai shared state and helpers.

// Oak's sleep question, answered before NewGameInitData wipes the vars.
// Stored as DIFFICULTY_* + 1; 0 means the question wasn't asked.
extern u8 gIsekaiChosenDifficulty;

// Oak's loaner: remember which party Pokémon it is (by personality), and decide in
// battle whether it loses its turn. Its quirk lasts until the Boulder Badge.
void IsekaiRememberLoaner(void);
bool32 IsekaiIsLoaner(struct Pokemon *mon);
bool32 IsekaiLoanerLosesTurn(u32 battler);

// ---- Core rules (src/fireredisekai_rules.c) ----
#define ISEKAI_DEFEAT_NONE       0
#define ISEKAI_DEFEAT_PROTECTED  1   // the level-10 rule: everyone just fainted
#define ISEKAI_DEFEAT_WILD       2   // the wild Pokémon turned on the MC
#define ISEKAI_DEFEAT_TRAINER    3   // he threw himself in front of his last Pokémon
#define ISEKAI_DEFEAT_ROCKET     4   // Team Rocket hurt him on purpose
#define ISEKAI_DEFEAT_GRASS      5   // no Pokémon left, attacked walking through grass

#define ISEKAI_NOFORFEIT_LEADER  1   // gym leaders, Elite Four, Champion
#define ISEKAI_NOFORFEIT_ROCKET  2   // Giovanni
#define ISEKAI_NOFORFEIT_OAK     3   // Blue in the lab
#define ISEKAI_NOFORFEIT_PEAK    4   // Red, and the hidden trainers

struct Pokemon;
struct BoxPokemon;
extern const u8 *gIsekaiTrainerArgs;
extern bool8 gIsekaiWildUnwilling;

void IsekaiNewGameInit(void);
u8 IsekaiMaxLives(void);
u8 IsekaiWardBattles(void);
u8 IsekaiLives(void);
void IsekaiBadgeEarned(void);
bool32 IsekaiIsBadlyInjured(struct Pokemon *mon);
void IsekaiSetBadlyInjured(struct Pokemon *mon, bool32 injured);
bool32 IsekaiBoxMonIsBadlyInjured(struct BoxPokemon *boxMon);
void IsekaiOnReviveUsed(struct Pokemon *mon, u16 item);
u8 IsekaiGraveCount(void);
void IsekaiAutosave(void);
void IsekaiRequestAutosave(void);
void IsekaiOnMapWarp(void);
bool32 IsekaiOnStep(u16 metatileBehavior);
void IsekaiOnFirstCatch(void);
void IsekaiOnBattleInit(void);
void IsekaiOnTrainerBattleStart(void);
void IsekaiOnWildBattleStart(void);
void IsekaiOnBattleEnd(void);
u8 IsekaiLastDefeat(void);
u16 IsekaiLastWildSpecies(void);
bool32 IsekaiFieldIdle(void);
u32 IsekaiForfeitForbidden(void);
void IsekaiWardBattleFought(void);
bool32 IsekaiReturnCenterLoaner(void);
bool32 IsekaiCenterLoanerIgnores(u32 battler);
u8 IsekaiWillingnessPercent(struct Pokemon *wild);
bool8 IsekaiRollWillingness(struct Pokemon *wild);
void IsekaiOnMonCaught(u16 species);
void IsekaiCatchCooldownTick(void);
void IsekaiOnMonReleased(struct BoxPokemon *boxMon);
bool32 IsekaiWildIsOverCaught(void);
void IsekaiRestartTrainerBattleFromArgs(const u8 *args); // battle_setup.c

#endif // GUARD_FIREREDISEKAI_H
