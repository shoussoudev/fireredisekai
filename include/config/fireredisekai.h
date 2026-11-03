#ifndef GUARD_CONFIG_FIREREDISEKAI_H
#define GUARD_CONFIG_FIREREDISEKAI_H

// FireRed Isekai developer settings.
// ISEKAI_START_AT: where a NEW GAME begins (0 = normal). Used for test and
// checkpoint builds only; release builds always use 0.
//   1 = Oak's lab, at the start of the loaner scene
#define ISEKAI_START_NORMAL     0
#define ISEKAI_START_OAKS_LAB   1

#ifndef ISEKAI_START_AT
#define ISEKAI_START_AT ISEKAI_START_NORMAL
#endif

#endif // GUARD_CONFIG_FIREREDISEKAI_H
