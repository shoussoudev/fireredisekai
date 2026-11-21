// FireRed Isekai (v0.13): the romance system.
//
// Ten partners, each with a quest. Finishing the quest makes her interested; he can accept or
// decline. Dating several at once is allowed, but each partner has a hidden suspicion meter that
// snowballs with every extra partner. Caught = her penalty switches on until it's resolved.
// Center "calls", one-time gifts in the PC (only after a BADGE together), clean and messy
// breakups, ghosting, "I've heard about you" after three breakups, and the hidden Glacia chain.
//
// State lives in vars (SaveBlock1), so old saves keep working.
#include "global.h"
#include "event_data.h"
#include "fireredisekai.h"
#include "fieldmap.h"
#include "item.h"
#include "main.h"
#include "overworld.h"
#include "random.h"
#include "script.h"
#include "string_util.h"
#include "strings.h"
#include "event_scripts.h"
#include "constants/fireredisekai_romance.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"

// ---------------------------------------------------------------------------
// Storage
// ---------------------------------------------------------------------------
// VAR_ISEKAI_ROM_0 + p: bits 0-2 state, 3 lied, 4 caught, 5 gift given,
//                        6-9 badges when it started, 10-13 badges at the last visit.
#define ROM_LIED        (1 << 3)
#define ROM_CAUGHT      (1 << 4)
#define ROM_GIFT        (1 << 5)
#define ROM_START_SHIFT 6
#define ROM_VISIT_SHIFT 10

// VAR_ISEKAI_ROM_META: bits 0-3 breakups, 4-7 Glacia chain position, 8 chain broken, 9 Fortuna read.
#define META_BREAKUPS(m)  ((m) & 0xF)
#define META_CHAIN(m)     (((m) >> 4) & 0xF)
#define META_BROKEN       (1 << 8)
#define META_FORTUNA      (1 << 9)

#define CALL_EVERY_STEPS  400
#define SUSPICION_CAUGHT  100

static u16 RomVar(u8 p)            { return VarGet(VAR_ISEKAI_ROM_0 + p); }
static void RomSetVar(u8 p, u16 v) { VarSet(VAR_ISEKAI_ROM_0 + p, v); }
static u8 RomState(u8 p)           { return RomVar(p) & 7; }
static void RomSetState(u8 p, u8 s){ RomSetVar(p, (RomVar(p) & ~7) | s); }

static u8 Suspicion(u8 p)
{
    u16 v = VarGet(VAR_ISEKAI_ROM_SUSP_01 + p / 2);
    return (p & 1) ? (v >> 8) : (v & 0xFF);
}

static void SetSuspicion(u8 p, u16 value)
{
    u16 v = VarGet(VAR_ISEKAI_ROM_SUSP_01 + p / 2);
    if (value > 255)
        value = 255;
    if (p & 1)
        v = (v & 0x00FF) | (value << 8);
    else
        v = (v & 0xFF00) | value;
    VarSet(VAR_ISEKAI_ROM_SUSP_01 + p / 2, v);
}

static u8 BadgeCount(void)
{
    u8 i, n = 0;
    for (i = 0; i < 8; i++)
        if (FlagGet(FLAG_BADGE01_GET + i))
            n++;
    return n;
}

static u8 DatingCount(void)
{
    u8 p, n = 0;
    for (p = 0; p < ROM_COUNT; p++)
        if (RomState(p) == ROM_STATE_DATING)
            n++;
    return n;
}

static u16 Meta(void)          { return VarGet(VAR_ISEKAI_ROM_META); }
static void SetMeta(u16 m)     { VarSet(VAR_ISEKAI_ROM_META, m); }
static void BreakChain(void)   { SetMeta(Meta() | META_BROKEN); }

static void AddBreakup(void)
{
    u16 m = Meta();
    if (META_BREAKUPS(m) < 15)
        m++;
    SetMeta(m);
}

// Where each partner lives (her suspicion rises when he's careless there).
static const u8 sHometown[ROM_COUNT] =
{
    [ROM_WARD_NURSE] = MAPSEC_VIRIDIAN_CITY,
    [ROM_REDS_MUM]   = MAPSEC_PALLET_TOWN,
    [ROM_DAISY]      = MAPSEC_PALLET_TOWN,
    [ROM_JENNY]      = MAPSEC_CERULEAN_CITY,
    [ROM_ROSA]       = MAPSEC_VERMILION_CITY,
    [ROM_CHANNELER]  = MAPSEC_LAVENDER_TOWN,
    [ROM_ROCKET]     = MAPSEC_CELADON_CITY,
    [ROM_JOY]        = MAPSEC_FUCHSIA_CITY,
    [ROM_LORELEI]    = MAPSEC_NONE,
    [ROM_AGATHA]     = MAPSEC_PALLET_TOWN,
};

// Madame Fortuna's hidden order (Glacia's clean path).
static const u8 sChainOrder[ROM_COUNT] =
{
    ROM_JENNY, ROM_ROSA, ROM_WARD_NURSE, ROM_CHANNELER, ROM_REDS_MUM,
    ROM_JOY, ROM_ROCKET, ROM_DAISY, ROM_LORELEI, ROM_AGATHA,
};

// One-time gifts (arrive in the PC once they've been together through a BADGE).
static const u16 sGifts[ROM_COUNT][3] =
{
    [ROM_WARD_NURSE] = {ITEM_HYPER_POTION, ITEM_FULL_HEAL, ITEM_NONE},
    [ROM_REDS_MUM]   = {ITEM_EXP_SHARE, ITEM_NONE, ITEM_NONE},
    [ROM_DAISY]      = {ITEM_LUCKY_EGG, ITEM_WATER_STONE, ITEM_THUNDER_STONE},
    [ROM_JENNY]      = {ITEM_NONE, ITEM_NONE, ITEM_NONE},
    [ROM_ROSA]       = {ITEM_NONE, ITEM_NONE, ITEM_NONE},
    [ROM_CHANNELER]  = {ITEM_NONE, ITEM_NONE, ITEM_NONE},
    [ROM_ROCKET]     = {ITEM_NONE, ITEM_NONE, ITEM_NONE},
    [ROM_JOY]        = {ITEM_NONE, ITEM_NONE, ITEM_NONE},
    [ROM_LORELEI]    = {ITEM_TM_ICE_BEAM, ITEM_TM_BLIZZARD, ITEM_NONE},
    [ROM_AGATHA]     = {ITEM_FOCUS_BAND, ITEM_NONE, ITEM_NONE},
};

static const u8 *const sNames[ROM_COUNT] =
{
    [ROM_WARD_NURSE] = COMPOUND_STRING("the ward nurse"),
    [ROM_REDS_MUM]   = COMPOUND_STRING("RED's mum"),
    [ROM_DAISY]      = COMPOUND_STRING("DAISY"),
    [ROM_JENNY]      = COMPOUND_STRING("JENNY"),
    [ROM_ROSA]       = COMPOUND_STRING("ROSA"),
    [ROM_CHANNELER]  = COMPOUND_STRING("the CHANNELER"),
    [ROM_ROCKET]     = COMPOUND_STRING("the ROCKET clerk"),
    [ROM_JOY]        = COMPOUND_STRING("NURSE JOY"),
    [ROM_LORELEI]    = COMPOUND_STRING("LORELEI"),
    [ROM_AGATHA]     = COMPOUND_STRING("AGATHA"),
};

// ---------------------------------------------------------------------------
// Lines. Partners not built yet fall back to the generic set.
// ---------------------------------------------------------------------------
#define ROMLINE_COUNT 15

static const u8 *const sLinesGeneric[ROMLINE_COUNT] =
{
    [ROMLINE_INTEREST]    = COMPOUND_STRING("…You're not like the others, are you?\pWould you like to… see where this goes?"),
    [ROMLINE_ACCEPT]      = COMPOUND_STRING("Good. Don't make me regret it."),
    [ROMLINE_DECLINE]     = COMPOUND_STRING("…Alright. Friends, then.\nThe offer stands, if you change your mind."),
    [ROMLINE_ASK]         = COMPOUND_STRING("Wait. Are you seeing anyone?"),
    [ROMLINE_HOLD]        = COMPOUND_STRING("…Thank you for being honest.\nCome back if that changes."),
    [ROMLINE_HEARD]       = COMPOUND_STRING("…I've heard about you.\pKanto is small. No thank you."),
    [ROMLINE_DATING]      = COMPOUND_STRING("There you are."),
    [ROMLINE_CONFRONT]    = COMPOUND_STRING("I know about the others.\nDon't insult me by denying it."),
    [ROMLINE_BREAK_CLEAN] = COMPOUND_STRING("…Thank you for telling me to my face.\nTake care of yourself."),
    [ROMLINE_BREAK_MESSY] = COMPOUND_STRING("Get out."),
    [ROMLINE_EX]          = COMPOUND_STRING("Oh. Hi.\nYou look tired. Well, more than usual."),
    [ROMLINE_EX_MESSY]    = COMPOUND_STRING("…"),
    [ROMLINE_SORRY_DONE]  = COMPOUND_STRING("…Fine. Apology accepted.\nWe're done, but we're even."),
    [ROMLINE_CALL]        = COMPOUND_STRING("Just checking you're alive."),
    [ROMLINE_PROMISE]     = COMPOUND_STRING("…One chance. That's all you get."),
};

static const u8 *const sLinesNurse[ROMLINE_COUNT] =
{
    [ROMLINE_INTEREST]    = COMPOUND_STRING("Your results came back.\pElevated heart rate whenever you walk\nin here. Mine, not yours.\pI'd like to see you outside of the\nward, for once. Interested?"),
    [ROMLINE_ACCEPT]      = COMPOUND_STRING("Good. Try not to get carried in\non a stretcher before dinner."),
    [ROMLINE_DECLINE]     = COMPOUND_STRING("…Noted. For the file.\pThe ward's still here.\nSo am I, if you change your mind."),
    [ROMLINE_ASK]         = COMPOUND_STRING("Before I fill in the form.\nAre you seeing anyone?"),
    [ROMLINE_HOLD]        = COMPOUND_STRING("…Honest. That's rare in here.\pCome back if that changes."),
    [ROMLINE_HEARD]       = COMPOUND_STRING("…I've heard about you.\pNurses talk. All of them.\nNo thank you."),
    [ROMLINE_DATING]      = COMPOUND_STRING("You're back. Of course you're back.\pAny new bruises I should know about?"),
    [ROMLINE_CONFRONT]    = COMPOUND_STRING("I've patched up half of Kanto.\nPeople talk on the table.\pI know about the others."),
    [ROMLINE_BREAK_CLEAN] = COMPOUND_STRING("…Thank you for telling me yourself.\pI'll still patch you up.\nThat's the job."),
    [ROMLINE_BREAK_MESSY] = COMPOUND_STRING("Out. And next time you're carried\nin here, it's someone else's shift."),
    [ROMLINE_EX]          = COMPOUND_STRING("Still in one piece? Good.\nThat's all I wanted to know."),
    [ROMLINE_EX_MESSY]    = COMPOUND_STRING("Take a number."),
    [ROMLINE_SORRY_DONE]  = COMPOUND_STRING("…Fine. You've apologised enough to\ncount as treatment. We're even."),
    [ROMLINE_CALL]        = COMPOUND_STRING("NURSE: It's me. From the ward.\nI heard a CENTER buzz you in.\pNo stretcher this time? Good.\nKeep it that way."),
    [ROMLINE_PROMISE]     = COMPOUND_STRING("…One chance. Like a second\nopinion. Don't waste it."),
};

static const u8 *const sLinesMum[ROMLINE_COUNT] =
{
    [ROMLINE_INTEREST]    = COMPOUND_STRING("You know… it's been a long time since\nanyone fixed anything in this house.\pStay for dinner? And, maybe…\nnot just this once?"),
    [ROMLINE_ACCEPT]      = COMPOUND_STRING("Good. I'll set another plate.\p…I still have the frying pan,\nyou know.\pYOU: I know."),
    [ROMLINE_DECLINE]     = COMPOUND_STRING("Oh. Of course. Silly of me.\pDinner's still on, whenever.\nThat doesn't change."),
    [ROMLINE_ASK]         = COMPOUND_STRING("…Wait. Are you seeing anyone?\nI'm too old for surprises."),
    [ROMLINE_HOLD]        = COMPOUND_STRING("At least you're honest.\nMore than I can say for most.\pCome back when you're free."),
    [ROMLINE_HEARD]       = COMPOUND_STRING("…I've heard about you.\pPallet's a small town.\nI think dinner is all this will be."),
    [ROMLINE_DATING]      = COMPOUND_STRING("There you are. Sit. You look\nlike you haven't eaten."),
    [ROMLINE_CONFRONT]    = COMPOUND_STRING("Sit down.\pThe whole of PALLET knows.\nDid you think I wouldn't?"),
    [ROMLINE_BREAK_CLEAN] = COMPOUND_STRING("…Thank you for saying it to my face.\pThe door's open for dinner.\nJust dinner."),
    [ROMLINE_BREAK_MESSY] = COMPOUND_STRING("Out. Before I find the pan."),
    [ROMLINE_EX]          = COMPOUND_STRING("Oh, it's you. Have you eaten?\n…Of course you haven't."),
    [ROMLINE_EX_MESSY]    = COMPOUND_STRING("The pan is right here."),
    [ROMLINE_SORRY_DONE]  = COMPOUND_STRING("…Alright. I forgive you.\nDon't make me regret it twice."),
    [ROMLINE_CALL]        = COMPOUND_STRING("MUM: Hello? Is this the CENTER?\nI'm looking for a tall, tired man.\p…It's you! Are you eating properly?\nRED never calls either."),
    [ROMLINE_PROMISE]     = COMPOUND_STRING("…One chance.\nThe pan stays by the door."),
};

static const u8 *const sLinesDaisy[ROMLINE_COUNT] =
{
    [ROMLINE_INTEREST]    = COMPOUND_STRING("He called. He actually called.\nThirty seconds, but he called.\pI owe you. And… I don't hate\nhaving you around.\pDinner? Not with Grandpa."),
    [ROMLINE_ACCEPT]      = COMPOUND_STRING("Good.\pGrandpa's going to hate this.\n…That's a bonus, honestly."),
    [ROMLINE_DECLINE]     = COMPOUND_STRING("Fine. Probably smarter.\pYou're still welcome here.\nFor the grooming, obviously."),
    [ROMLINE_ASK]         = COMPOUND_STRING("Hold on. Are you seeing anyone?\nI run a house. I don't do messes."),
    [ROMLINE_HOLD]        = COMPOUND_STRING("Honest. Annoyingly.\pCome back when you're not\nsomeone else's problem."),
    [ROMLINE_HEARD]       = COMPOUND_STRING("…I've heard about you.\pPallet has two houses and a lab.\nNo thank you."),
    [ROMLINE_DATING]      = COMPOUND_STRING("You again. Sit. I'm doing the\naccounts, so be quiet and pretty."),
    [ROMLINE_CONFRONT]    = COMPOUND_STRING("I keep this whole house running.\nYou think I can't keep track of you?\pI know about the others."),
    [ROMLINE_BREAK_CLEAN] = COMPOUND_STRING("…Fine. At least you said it to\nmy face. Most people don't."),
    [ROMLINE_BREAK_MESSY] = COMPOUND_STRING("Out.\p…And I'm telling BLUE."),
    [ROMLINE_EX]          = COMPOUND_STRING("Oh. It's you.\nDo you need a groom or something?"),
    [ROMLINE_EX_MESSY]    = COMPOUND_STRING("BLUE knows.\nGood luck with that."),
    [ROMLINE_SORRY_DONE]  = COMPOUND_STRING("…Fine. Apology accepted.\nI'll tell BLUE to calm down. A bit."),
    [ROMLINE_CALL]        = COMPOUND_STRING("DAISY: It's me. Grandpa's finally\nasleep.\pThe house is quiet. It's weird.\n…Come by sometime."),
    [ROMLINE_PROMISE]     = COMPOUND_STRING("One chance.\pI'm writing this down.\nIn the ledger. In ink."),
};

static const u8 *const *const sLines[ROM_COUNT] =
{
    [ROM_WARD_NURSE] = sLinesNurse,
    [ROM_REDS_MUM]   = sLinesMum,
    [ROM_DAISY]      = sLinesDaisy,
};

static const u8 *RomLine(u8 p, u8 line)
{
    if (p < ROM_COUNT && sLines[p] != NULL && sLines[p][line] != NULL)
        return sLines[p][line];
    return sLinesGeneric[line];
}

// ---------------------------------------------------------------------------
// Specials. VAR_0x8004 = partner everywhere.
// ---------------------------------------------------------------------------

// VAR_0x8005 = line: puts it in gStringVar4 (msgbox gStringVar4).
void IsekaiRomBufferLine(void)
{
    u8 p = gSpecialVar_0x8004;
    StringCopy(gStringVar1, sNames[p < ROM_COUNT ? p : 0]);
    StringExpandPlaceholders(gStringVar4, RomLine(p, gSpecialVar_0x8005));
}

void IsekaiRomGetState(void)
{
    gSpecialVar_Result = RomState(gSpecialVar_0x8004);
}

// Her quest is done: she's interested (only the first time).
void IsekaiRomQuestDone(void)
{
    u8 p = gSpecialVar_0x8004;
    if (RomState(p) == ROM_STATE_NONE)
        RomSetState(p, ROM_STATE_INTEREST);
}

// Before she asks him out. VAR_RESULT: 0 single, 1 dating someone else, 2 "I've heard about you".
void IsekaiRomOfferCheck(void)
{
    u8 p = gSpecialVar_0x8004;
    if (META_BREAKUPS(Meta()) >= 3)
        gSpecialVar_Result = 2;
    else if (DatingCount() > (RomState(p) == ROM_STATE_DATING ? 1 : 0))
        gSpecialVar_Result = 1;
    else
        gSpecialVar_Result = 0;
}

// VAR_0x8005 = TRUE if he lied about seeing someone.
void IsekaiRomAccept(void)
{
    u8 p = gSpecialVar_0x8004;
    u16 v = RomVar(p) & ~(7 | ROM_LIED | ROM_CAUGHT | (0xF << ROM_START_SHIFT) | (0xF << ROM_VISIT_SHIFT));
    u8 badges = BadgeCount();
    bool32 others = DatingCount() > 0;
    u16 m = Meta();

    v |= ROM_STATE_DATING | (badges << ROM_START_SHIFT) | (badges << ROM_VISIT_SHIFT);
    if (gSpecialVar_0x8005)
        v |= ROM_LIED;
    RomSetVar(p, v);
    SetSuspicion(p, 0);

    // Glacia's clean path: one at a time, in Fortuna's order, after the first reading.
    if ((m & META_FORTUNA) && !(m & META_BROKEN))
    {
        if (others || META_CHAIN(m) >= ROM_COUNT || sChainOrder[META_CHAIN(m)] != p)
            BreakChain();
        else
            SetMeta((m & ~(0xF << 4)) | ((META_CHAIN(m) + 1) << 4));
    }
}

void IsekaiRomHold(void)    { RomSetState(gSpecialVar_0x8004, ROM_STATE_ON_HOLD); }
void IsekaiRomDecline(void) { RomSetState(gSpecialVar_0x8004, ROM_STATE_FRIEND); }

// VAR_0x8005: 0 honest, in person; 1 messy (blamed her, lied, or caught lying).
void IsekaiRomBreakUp(void)
{
    u8 p = gSpecialVar_0x8004;
    bool32 messy = gSpecialVar_0x8005 || ((RomVar(p) & ROM_LIED) && (RomVar(p) & ROM_CAUGHT));

    RomSetVar(p, (RomVar(p) & ~(7 | ROM_CAUGHT)) | (messy ? ROM_STATE_EX_MESSY : ROM_STATE_EX_CLEAN));
    SetSuspicion(p, 0);
    AddBreakup();
    if (messy)
        BreakChain();
    gSpecialVar_Result = messy;
}

// He promised to end it with everyone else: each of them gets a breakup by PC message (messy).
void IsekaiRomPromise(void)
{
    u8 p = gSpecialVar_0x8004, q;
    for (q = 0; q < ROM_COUNT; q++)
    {
        if (q == p || RomState(q) != ROM_STATE_DATING)
            continue;
        RomSetVar(q, (RomVar(q) & ~(7 | ROM_CAUGHT)) | ROM_STATE_EX_MESSY);
        SetSuspicion(q, 0);
        AddBreakup();
    }
    RomSetVar(p, RomVar(p) & ~ROM_CAUGHT);
    SetSuspicion(p, 50);  // she's still watching
    BreakChain();
}

// The apology quest for a messy breakup.
void IsekaiRomApologised(void)
{
    RomSetState(gSpecialVar_0x8004, ROM_STATE_EX_SORRY);
}

// Talking to her. Records the visit (ghosting) and says how to open.
// VAR_RESULT: ROM_TALK_NORMAL or ROM_TALK_CAUGHT.
void IsekaiRomTalk(void)
{
    u8 p = gSpecialVar_0x8004;
    u16 v = RomVar(p);

    gSpecialVar_Result = ROM_TALK_NORMAL;
    if ((v & 7) != ROM_STATE_DATING)
        return;
    v = (v & ~(0xF << ROM_VISIT_SHIFT)) | (BadgeCount() << ROM_VISIT_SHIFT);
    RomSetVar(p, v);
    if (v & ROM_CAUGHT)
        gSpecialVar_Result = ROM_TALK_CAUGHT;
}

// The Fortuna reading starts the Glacia chain (v0.15 builds her tent).
void IsekaiRomFortunaRead(void)
{
    SetMeta(Meta() | META_FORTUNA);
}

// ---------------------------------------------------------------------------
// Perks and penalties, for the rest of the game to ask.
// ---------------------------------------------------------------------------
bool32 IsekaiRomPerk(u8 p)
{
    return RomState(p) == ROM_STATE_DATING && !(RomVar(p) & ROM_CAUGHT);
}

bool32 IsekaiRomPenalty(u8 p)
{
    return (RomState(p) == ROM_STATE_DATING && (RomVar(p) & ROM_CAUGHT))
        || RomState(p) == ROM_STATE_EX_MESSY;
}

// Script versions: VAR_RESULT.
void IsekaiRomCheckPerk(void)    { gSpecialVar_Result = IsekaiRomPerk(gSpecialVar_0x8004); }
void IsekaiRomCheckPenalty(void) { gSpecialVar_Result = IsekaiRomPenalty(gSpecialVar_0x8004); }

// ---------------------------------------------------------------------------
// Suspicion
// ---------------------------------------------------------------------------
// Raise a partner's suspicion. It snowballs: x2 with two partners, x3 with three...
static void RaiseSuspicion(u8 p, u8 amount)
{
    u16 s;
    u8 n = DatingCount();
    if (RomState(p) != ROM_STATE_DATING || n < 2 || (RomVar(p) & ROM_CAUGHT))
        return;
    s = Suspicion(p) + amount * n;
    if (s >= SUSPICION_CAUGHT)
    {
        RomSetVar(p, RomVar(p) | ROM_CAUGHT);
        BreakChain();
        s = SUSPICION_CAUGHT;
    }
    SetSuspicion(p, s);
}

// Someone else's call or gift arrives while he's in another partner's town.
static void CallSeenInTown(u8 caller)
{
    u8 q, sec = gMapHeader.regionMapSectionId;
    for (q = 0; q < ROM_COUNT; q++)
        if (q != caller && sHometown[q] == sec)
            RaiseSuspicion(q, 20);
}

// LONG tells everyone when he beats the MC (v0.17).
void IsekaiRomLongTellsEveryone(void)
{
    u8 q;
    for (q = 0; q < ROM_COUNT; q++)
        RaiseSuspicion(q, 15);
}

// ---------------------------------------------------------------------------
// Center calls and gifts
// ---------------------------------------------------------------------------
static EWRAM_DATA u8 sCallPending = 0;      // partner + 1
static EWRAM_DATA u8 sGhostedPending = 0;   // bitmask of partners who left (ghosting), shown one at a time

static const u16 sCenters[][2] =
{
    {MAP_GROUP(MAP_VIRIDIAN_CITY_POKEMON_CENTER_1F), MAP_NUM(MAP_VIRIDIAN_CITY_POKEMON_CENTER_1F)},
    {MAP_GROUP(MAP_PEWTER_CITY_POKEMON_CENTER_1F),   MAP_NUM(MAP_PEWTER_CITY_POKEMON_CENTER_1F)},
    {MAP_GROUP(MAP_CERULEAN_CITY_POKEMON_CENTER_1F), MAP_NUM(MAP_CERULEAN_CITY_POKEMON_CENTER_1F)},
    {MAP_GROUP(MAP_VERMILION_CITY_POKEMON_CENTER_1F),MAP_NUM(MAP_VERMILION_CITY_POKEMON_CENTER_1F)},
    {MAP_GROUP(MAP_LAVENDER_TOWN_POKEMON_CENTER_1F), MAP_NUM(MAP_LAVENDER_TOWN_POKEMON_CENTER_1F)},
    {MAP_GROUP(MAP_CELADON_CITY_POKEMON_CENTER_1F),  MAP_NUM(MAP_CELADON_CITY_POKEMON_CENTER_1F)},
    {MAP_GROUP(MAP_FUCHSIA_CITY_POKEMON_CENTER_1F),  MAP_NUM(MAP_FUCHSIA_CITY_POKEMON_CENTER_1F)},
    {MAP_GROUP(MAP_SAFFRON_CITY_POKEMON_CENTER_1F),  MAP_NUM(MAP_SAFFRON_CITY_POKEMON_CENTER_1F)},
    {MAP_GROUP(MAP_CINNABAR_ISLAND_POKEMON_CENTER_1F),MAP_NUM(MAP_CINNABAR_ISLAND_POKEMON_CENTER_1F)},
    {MAP_GROUP(MAP_ROUTE4_POKEMON_CENTER_1F),        MAP_NUM(MAP_ROUTE4_POKEMON_CENTER_1F)},
    {MAP_GROUP(MAP_ROUTE10_POKEMON_CENTER_1F),       MAP_NUM(MAP_ROUTE10_POKEMON_CENTER_1F)},
};

static bool32 InCenter(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sCenters); i++)
        if (gSaveBlock1Ptr->location.mapGroup == sCenters[i][0] && gSaveBlock1Ptr->location.mapNum == sCenters[i][1])
            return TRUE;
    return FALSE;
}

static bool32 GiftReady(u8 p)
{
    u16 v = RomVar(p);
    return (v & 7) == ROM_STATE_DATING && !(v & (ROM_GIFT | ROM_CAUGHT))
        && sGifts[p][0] != ITEM_NONE
        && BadgeCount() > ((v >> ROM_START_SHIFT) & 0xF);
}

void IsekaiRomOnStep(void)
{
    if (VarGet(VAR_ISEKAI_ROM_CALL_STEPS) < 0xFFFF)
        VarSet(VAR_ISEKAI_ROM_CALL_STEPS, VarGet(VAR_ISEKAI_ROM_CALL_STEPS) + 1);
}

void IsekaiRomOnMapWarp(void)
{
    u8 p, n, pick, sec;

    if (!IS_FRLG || DatingCount() == 0)
        return;

    // Seen around a partner's town with more than one on the go.
    sec = gMapHeader.regionMapSectionId;
    for (p = 0; p < ROM_COUNT; p++)
        if (sHometown[p] == sec && gMapHeader.mapType != MAP_TYPE_INDOOR)
            RaiseSuspicion(p, 3);

    if (!InCenter() || VarGet(VAR_ISEKAI_ROM_CALL_STEPS) < CALL_EVERY_STEPS)
        return;
    // Someone with a gift waiting calls first; otherwise anyone.
    for (p = 0; p < ROM_COUNT; p++)
    {
        if (GiftReady(p))
        {
            sCallPending = p + 1;
            return;
        }
    }
    n = DatingCount();
    pick = Random() % n;
    for (p = 0; p < ROM_COUNT; p++)
    {
        if (RomState(p) != ROM_STATE_DATING)
            continue;
        if (pick-- == 0)
        {
            sCallPending = p + 1;
            return;
        }
    }
}

// Partners he hasn't visited for two BADGES leave him (by PC message: messy).
void IsekaiRomOnBadge(void)
{
    u8 p, badges = BadgeCount();
    for (p = 0; p < ROM_COUNT; p++)
    {
        u16 v = RomVar(p);
        if ((v & 7) != ROM_STATE_DATING)
            continue;
        if (badges >= ((v >> ROM_VISIT_SHIFT) & 0xF) + 2)
        {
            RomSetVar(p, (v & ~(7 | ROM_CAUGHT)) | ROM_STATE_EX_MESSY);
            AddBreakup();
            BreakChain();
            sGhostedPending |= 1 << p;
        }
    }
}

// From IsekaiFieldIdle: the phone, or a "Don't bother" message.
bool32 IsekaiRomFieldIdle(void)
{
    u8 p;

    if (sGhostedPending)
    {
        for (p = 0; p < ROM_COUNT; p++)
            if (sGhostedPending & (1 << p))
                break;
        sGhostedPending &= ~(1 << p);
        StringCopy(gStringVar1, sNames[p]);
        LockPlayerFieldControls();
        ScriptContext_SetupScript(IsekaiEventScript_RomGhosted);
        return TRUE;
    }
    if (sCallPending)
    {
        p = sCallPending - 1;
        sCallPending = 0;
        VarSet(VAR_ISEKAI_ROM_CALL_STEPS, 0);
        FlagSet(FLAG_ISEKAI_ROM_CALLED_ONCE);
        gSpecialVar_0x8004 = p;
        StringCopy(gStringVar1, sNames[p]);
        gSpecialVar_0x8006 = FALSE;
        if (GiftReady(p))
        {
            u8 i;
            for (i = 0; i < 3 && sGifts[p][i] != ITEM_NONE; i++)
                AddPCItem(sGifts[p][i], 1);
            RomSetVar(p, RomVar(p) | ROM_GIFT);
            CopyItemName(sGifts[p][0], gStringVar2);
            gSpecialVar_0x8006 = TRUE;
        }
        CallSeenInTown(p);
        LockPlayerFieldControls();
        ScriptContext_SetupScript(IsekaiEventScript_RomCall);
        return TRUE;
    }
    return FALSE;
}
