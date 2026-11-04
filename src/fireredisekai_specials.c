#include "global.h"
#include "battle.h"
#include "pokemon.h"
#include "random.h"
#include "constants/battle.h"
#include "constants/flags.h"
#include "constants/species.h"
#include "event_data.h"
#include "fireredisekai.h"
#include "money.h"
#include "string_util.h"
#include "constants/vars.h"

// FireRed Isekai script specials.

EWRAM_DATA u8 gIsekaiChosenDifficulty = 0;

// Oak's loan deposit: take every penny, remember how much, put the amount in STR_VAR_1.
void IsekaiTakeAllMoney(void)
{
    u32 money = GetMoney(&gSaveBlock1Ptr->money);
    VarSet(VAR_ISEKAI_LOAN_DEPOSIT, money > 0xFFFF ? 0xFFFF : money);
    SetMoney(&gSaveBlock1Ptr->money, 0);
    ConvertIntToDecimalStringN(gStringVar1, money, STR_CONV_MODE_LEFT_ALIGN, 7);
}

// Called right after Oak hands over the loaner: it's the last Pokémon in the party.
void IsekaiRememberLoaner(void)
{
    u8 count = CalculatePlayerPartyCount();
    u32 personality;

    if (count == 0)
        return;
    personality = GetMonData(&gParties[B_TRAINER_PLAYER][count - 1], MON_DATA_PERSONALITY);
    VarSet(VAR_ISEKAI_LOANER_SPECIES, GetMonData(&gParties[B_TRAINER_PLAYER][count - 1], MON_DATA_SPECIES));
    VarSet(VAR_ISEKAI_LOANER_PID_LO, personality & 0xFFFF);
    VarSet(VAR_ISEKAI_LOANER_PID_HI, personality >> 16);
}

static bool32 IsLoanerIdentity(u32 species, u32 monPersonality)
{
    u32 personality = VarGet(VAR_ISEKAI_LOANER_PID_LO) | (VarGet(VAR_ISEKAI_LOANER_PID_HI) << 16);
    u32 loanerSpecies = VarGet(VAR_ISEKAI_LOANER_SPECIES);

    return loanerSpecies != SPECIES_NONE && species == loanerSpecies && monPersonality == personality;
}

bool32 IsekaiIsLoaner(struct Pokemon *mon)
{
    return IsLoanerIdentity(GetMonData(mon, MON_DATA_SPECIES), GetMonData(mon, MON_DATA_PERSONALITY));
}

// About one turn in eight is lost; on the very first turn of the first battle
// (Blue, in the lab) it's a coin flip. Sets the message for BattleScript_IsekaiLoanerQuirk.
bool32 IsekaiLoanerLosesTurn(u32 battler)
{
    u32 chance;

    if (!IS_FRLG || FlagGet(FLAG_BADGE01_GET))
        return FALSE;
    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED))
        return FALSE;
    if (GetBattlerSide(battler) != B_SIDE_PLAYER)
        return FALSE;
    if (!IsLoanerIdentity(gBattleMons[battler].species, gBattleMons[battler].personality))
        return FALSE;

    if (!FlagGet(FLAG_BEAT_RIVAL_IN_OAKS_LAB) && gBattleResults.battleTurnCounter == 0)
        chance = 50;
    else
        chance = 12;
    if ((Random() % 100) >= chance)
        return FALSE;

    gBattleCommunication[MULTISTRING_CHOOSER] = (VarGet(VAR_ISEKAI_LOANER_SPECIES) == SPECIES_SLOWPOKE) ? 1 : 0;
    return TRUE;
}
