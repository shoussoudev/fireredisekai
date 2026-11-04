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

#endif // GUARD_FIREREDISEKAI_H
