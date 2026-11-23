#ifndef GUARD_CONSTANTS_FIREREDISEKAI_ROMANCE_H
#define GUARD_CONSTANTS_FIREREDISEKAI_ROMANCE_H

// FireRed Isekai (v0.13): the romance system.

// The ten partners, in the order of their state vars (VAR_ISEKAI_ROM_0 + id).
#define ROM_WARD_NURSE   0   // Viridian
#define ROM_REDS_MUM     1   // Pallet
#define ROM_DAISY        2   // Pallet
#define ROM_JENNY        3   // Cerulean
#define ROM_ROSA         4   // Vermilion
#define ROM_CHANNELER    5   // Lavender
#define ROM_ROCKET       6   // Celadon
#define ROM_JOY          7   // Fuchsia
#define ROM_LORELEI      8   // Seafoam
#define ROM_AGATHA       9   // Pallet, late
#define ROM_COUNT        10

// A partner's state (low 3 bits of her var).
#define ROM_STATE_NONE      0   // quest not done
#define ROM_STATE_INTEREST  1   // quest done: she's interested, he hasn't answered (or declined: see FRIEND)
#define ROM_STATE_DATING    2
#define ROM_STATE_ON_HOLD   3   // he was honest that he's seeing someone: come back if that changes
#define ROM_STATE_FRIEND    4   // he declined: no penalty, he can bring it up again
#define ROM_STATE_EX_CLEAN  5
#define ROM_STATE_EX_MESSY  6   // her penalty is permanent (until the apology quest)
#define ROM_STATE_EX_SORRY  7   // apologised: the messy breakup is cleared

// What a talk with a partner should open with (IsekaiRomTalk -> VAR_RESULT).
#define ROM_TALK_NORMAL     0
#define ROM_TALK_CAUGHT     1   // she knows about the others: confrontation
#define ROM_TALK_HEARD      2   // three breakups: "I've heard about you."

// Lines (IsekaiRomBufferLine, VAR_0x8005)
#define ROMLINE_INTEREST     0
#define ROMLINE_ACCEPT       1
#define ROMLINE_DECLINE      2
#define ROMLINE_ASK          3
#define ROMLINE_HOLD         4
#define ROMLINE_HEARD        5
#define ROMLINE_DATING       6
#define ROMLINE_CONFRONT     7
#define ROMLINE_BREAK_CLEAN  8
#define ROMLINE_BREAK_MESSY  9
#define ROMLINE_EX           10
#define ROMLINE_EX_MESSY     11
#define ROMLINE_SORRY_DONE   12
#define ROMLINE_CALL         13
#define ROMLINE_PROMISE      14
#define ROMLINE_THANKS       15
#define ROMLINE_MISS_YOU     16

// Quest progress vars (see vars_frlg.h)
// Ward nurse: bits 0-2 herbs found (Route 2, Viridian Forest, Route 22), stage in bits 4-7.
#define NURSE_QUEST_NONE      0
#define NURSE_QUEST_HERBS     1   // gathering herbs
#define NURSE_QUEST_DRANK     2   // drank it: win the next battle
#define NURSE_QUEST_DONE      3
// Red's mum
#define MUM_QUEST_NONE        0
#define MUM_QUEST_ADAPTER     1   // needs the museum's spare adapter
#define MUM_QUEST_ASSISTANT   2   // beat the assistant: quiz next
#define MUM_QUEST_GOT_IT      3   // has the adapter
#define MUM_QUEST_DONE        4
// Daisy
#define DAISY_QUEST_NONE      0
#define DAISY_QUEST_MESSAGE   1   // pass her message to BLUE (after beating him)
#define DAISY_QUEST_DELIVERED 2
#define DAISY_QUEST_DONE      3

#endif // GUARD_CONSTANTS_FIREREDISEKAI_ROMANCE_H
