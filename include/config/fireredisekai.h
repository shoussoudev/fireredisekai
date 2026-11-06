#ifndef GUARD_CONFIG_FIREREDISEKAI_H
#define GUARD_CONFIG_FIREREDISEKAI_H

// FireRed Isekai developer settings.
// ISEKAI_START_AT: where a NEW GAME begins (0 = normal). Used for test and
// checkpoint builds only; release builds always use 0.
//   1 = Oak's lab, at the start of the loaner scene
#define ISEKAI_START_NORMAL     0
#define ISEKAI_START_OAKS_LAB   1
//   2 = rules sandbox: Route 22 tall grass with Oak's PSYDUCK at Lv10 on 1 HP, after Blue's battle
#define ISEKAI_START_RULES_TEST 2
//   3 = Viridian City with Oak's PSYDUCK and his parcel (the v0.11 loan story)
#define ISEKAI_START_VIRIDIAN   3

#ifndef ISEKAI_START_AT
#define ISEKAI_START_AT ISEKAI_START_NORMAL
#endif

// Difficulty ("How much sleep did you get this week?") uses the expansion's
// DIFFICULTY_EASY/NORMAL/HARD, stored in VAR_ISEKAI_DIFFICULTY (B_VAR_DIFFICULTY).
// PLENTY = hard, SOME = normal, I HAVE A NEWBORN = easy.

#endif // GUARD_CONFIG_FIREREDISEKAI_H
