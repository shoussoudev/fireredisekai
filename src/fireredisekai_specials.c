#include "global.h"
#include "event_data.h"
#include "money.h"
#include "string_util.h"
#include "constants/vars.h"

// FireRed Isekai script specials.

// Oak's loan deposit: take every penny, remember how much, put the amount in STR_VAR_1.
void IsekaiTakeAllMoney(void)
{
    u32 money = GetMoney(&gSaveBlock1Ptr->money);
    VarSet(VAR_ISEKAI_LOAN_DEPOSIT, money > 0xFFFF ? 0xFFFF : money);
    SetMoney(&gSaveBlock1Ptr->money, 0);
    ConvertIntToDecimalStringN(gStringVar1, money, STR_CONV_MODE_LEFT_ALIGN, 7);
}
