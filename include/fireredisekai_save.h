#ifndef GUARD_FIREREDISEKAI_SAVE_H
#define GUARD_FIREREDISEKAI_SAVE_H

// FireRed Isekai save data, kept in SaveBlock3 (wiped on NEW GAME).

#define ISEKAI_BOXMON_SIZE      80   // sizeof(struct BoxPokemon), checked in fireredisekai_rules.c
#define ISEKAI_WARD_SLOTS       6    // Badly Injured Pokémon left at a Center
#define ISEKAI_GRAVES           30   // names kept for the Viridian graveyard and the credits
#define ISEKAI_CATCH_SLOTS      8    // recent catches, for over-catching
#define ISEKAI_GRAVE_NAME_LEN   12   // POKEMON_NAME_LENGTH

// What kind of battle was "in progress" when the game was last saved
#define ISEKAI_BATTLE_NONE      0
#define ISEKAI_BATTLE_WILD      1
#define ISEKAI_BATTLE_TRAINER   2

struct IsekaiGrave
{
    u16 species;
    u8 nickname[ISEKAI_GRAVE_NAME_LEN];
};

struct IsekaiCatch
{
    u16 mapsec;
    u16 species;
    u8 count;
    u8 cooldown;   // battles left before the species there calms down
};

struct IsekaiSaveData
{
    // Human lives
    u8 lives;
    u8 livesShown:1;          // the nurse has explained the rule
    u8 resetMsgShown:1;       // "You try to reload. Nothing happens."
    u8 loanerOut:1;           // the Center's grumpy loaner is in the party
    u8 unusedBits:5;
    // Battle in progress (mid-battle reset penalty)
    u8 battleType;            // ISEKAI_BATTLE_*
    u8 resetCount;            // how many times this battle was reset
    u32 trainerArgs;          // ROM address of the trainerbattle arguments
    u16 battleOpponent;       // trainer id, or wild species
    u8 wildLevel;
    u8 wildWilling;
    // Center ward: Badly Injured Pokémon left to heal
    u8 wardMons[ISEKAI_WARD_SLOTS][ISEKAI_BOXMON_SIZE];
    u8 wardBattlesLeft[ISEKAI_WARD_SLOTS];
    u32 loanerPersonality;
    // Graveyard
    u8 graveCount;
    u8 releasedCount;         // released on their home route (capped bonus)
    struct IsekaiGrave graves[ISEKAI_GRAVES];
    // Over-catching
    struct IsekaiCatch catches[ISEKAI_CATCH_SLOTS];
};

#endif // GUARD_FIREREDISEKAI_SAVE_H
