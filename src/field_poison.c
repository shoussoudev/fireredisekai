#include "global.h"
#include "fireredisekai.h"
#include "event_data.h"
#include "party_menu.h"
#include "battle.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_poison.h"
#include "fldeff_misc.h"
#include "frontier_util.h"
#include "party_menu.h"
#include "pokenav.h"
#include "script.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "trainer_hill.h"
#include "constants/field_poison.h"
#include "constants/form_change_types.h"
#include "constants/party_menu.h"

// FireRed Isekai: before the first BADGE, poison stops at 1 HP (4th-gen style).
// After it, poison can kill: a POKéMON that drops to 0 HP while walking dies.
static bool32 PoisonKills(void)
{
    return IS_FRLG && FlagGet(FLAG_BADGE01_GET);
}

static const u8 sText_IsekaiPoisonDeath[] = _("{STR_VAR_1} collapsed.\nThe poison was too much…\p{STR_VAR_1} is gone. It was taken to\nthe GARDEN OF REST in VIRIDIAN.\pIt is no longer in your team.\p");

static bool32 IsMonValidSpecies(struct Pokemon *pokemon)
{
    enum Species species = GetMonData(pokemon, MON_DATA_SPECIES_OR_EGG);
    if (species == SPECIES_NONE || species == SPECIES_EGG)
        return FALSE;

    return TRUE;
}

static bool32 AllMonsFainted(void)
{
    int i;
    struct Pokemon *pokemon = gParties[B_TRAINER_PLAYER];

    for (i = 0; i < PARTY_SIZE; i++, pokemon++)
    {
        if (IsMonValidSpecies(pokemon) && GetMonData(pokemon, MON_DATA_HP) != 0)
            return FALSE;
    }
    return TRUE;
}

static void FaintFromFieldPoison(u8 partyIdx)
{
    struct Pokemon *pokemon = &gParties[B_TRAINER_PLAYER][partyIdx];
    u32 status = STATUS1_NONE;

    if (OW_POISON_DAMAGE < GEN_4)
        AdjustFriendship(pokemon, FRIENDSHIP_EVENT_FAINT_FIELD_PSN);

    SetMonData(pokemon, MON_DATA_STATUS, &status);
    GetMonData(pokemon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    if (PoisonKills())
        IsekaiPoisonDeath(pokemon); // buried; the party is tidied up once the messages are done
}

static bool32 MonFaintedFromPoison(u8 partyIdx)
{
    struct Pokemon *pokemon = &gParties[B_TRAINER_PLAYER][partyIdx];
    if (IsMonValidSpecies(pokemon) && GetMonData(pokemon, MON_DATA_HP) == ((OW_POISON_DAMAGE < GEN_4 || PoisonKills()) ? 0 : 1) && GetAilmentFromStatus(GetMonData(pokemon, MON_DATA_STATUS)) == AILMENT_PSN)
        return TRUE;

    return FALSE;
}

#define tState    data[0]
#define tPartyIdx data[1]

static void Task_TryFieldPoisonWhiteOut(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (tState)
    {
    case 0:
        for (; tPartyIdx < PARTY_SIZE; tPartyIdx++)
        {
            if (MonFaintedFromPoison(tPartyIdx))
            {
                bool32 kills = PoisonKills();
                FaintFromFieldPoison(tPartyIdx);
                ShowFieldMessage(kills ? sText_IsekaiPoisonDeath : gText_PkmnFainted_FldPsn);
                tState++;
                return;
            }
        }
        tState = 2; // Finished checking party
        break;
    case 1:
        // Wait for "{mon} fainted" message, then return to party loop
        if (IsFieldMessageBoxHidden())
            tState--;
        break;
    case 2:
        if (PoisonKills())
            CompactPartySlots();
        if (AllMonsFainted())
        {
            // Battle facilities have their own white out script to handle the challenge loss
#ifdef BUGFIX
            if (CurrentBattlePyramidLocation() || InBattlePike() || InTrainerHillChallenge())
#else
            if (CurrentBattlePyramidLocation() | InBattlePike() || InTrainerHillChallenge())
#endif
                gSpecialVar_Result = FLDPSN_FRONTIER_WHITEOUT;
            else
                gSpecialVar_Result = FLDPSN_WHITEOUT;
        }
        else
        {
            gSpecialVar_Result = FLDPSN_NO_WHITEOUT;
            UpdateFollowingPokemon();
        }
        ScriptContext_Enable();
        DestroyTask(taskId);
        break;
    }
}

#undef tState
#undef tPartyIdx

void TryFieldPoisonWhiteOut(void)
{
    CreateTask(Task_TryFieldPoisonWhiteOut, 80);
    ScriptContext_Stop();
}

s32 DoPoisonFieldEffect(void)
{
    int i;
    u32 hp;
    struct Pokemon *pokemon = gParties[B_TRAINER_PLAYER];
    u32 numPoisoned = 0;
    u32 numFainted = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(pokemon, MON_DATA_SANITY_HAS_SPECIES) && GetAilmentFromStatus(GetMonData(pokemon, MON_DATA_STATUS)) == AILMENT_PSN)
        {
            // Apply poison damage
            hp = GetMonData(pokemon, MON_DATA_HP);
            if ((OW_POISON_DAMAGE < GEN_4 || PoisonKills()) && (hp == 0 || --hp == 0))
            {
                TryFormChange(&gParties[B_TRAINER_PLAYER][i], FORM_CHANGE_FAINT, B_TRAINER_PLAYER);
                numFainted++;
            }
            else if (OW_POISON_DAMAGE >= GEN_4 && !PoisonKills() && (hp == 1 || --hp == 1))
                numFainted++;

            SetMonData(pokemon, MON_DATA_HP, &hp);
            numPoisoned++;
        }
        pokemon++;
    }

    // Do screen flash effect
    if (numFainted != 0 || numPoisoned != 0)
        FldEffPoison_Start();

    if (numFainted != 0)
        return FLDPSN_FNT;

    if (numPoisoned != 0)
        return FLDPSN_PSN;

    return FLDPSN_NONE;
}
