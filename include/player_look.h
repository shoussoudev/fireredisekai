#ifndef GUARD_PLAYER_LOOK_H
#define GUARD_PLAYER_LOOK_H

// FireRed Isekai: the MC's chosen look (one of five recolours of the player sprite).
// The choice lives in gSaveBlock2Ptr->playerLook and recolours the overworld sprite,
// the battle back sprite, the trainer front pic and the picture in Oak's intro.

#define PLAYER_LOOK_COUNT 5

extern u16 gPlayerLookOverworldPal[16];
extern const u8 *const gPlayerLookNames[PLAYER_LOOK_COUNT];

u8 PlayerLook_Get(void);
void PlayerLook_Set(u8 look);
void PlayerLook_RefreshOverworldPal(void);
const u16 *PlayerLook_GetOverworldPal(void);
const u16 *PlayerLook_GetFrontPal(void);
const u16 *PlayerLook_GetIntroPal(u8 look);

#endif // GUARD_PLAYER_LOOK_H
