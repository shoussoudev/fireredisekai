EventScript_AfterWhiteOutHeal_Frlg::
	lockall
	textcolor NPC_TEXT_COLOR_FEMALE
	call IsekaiEventScript_WakeUp @ FireRed Isekai: waiting room, human ward, or the end
	textcolor NPC_TEXT_COLOR_FEMALE
	msgbox gText_FirstShouldRestoreMonsHealth
	call EventScript_PkmnCenterNurse_TakeAndHealPkmn_Frlg
	call IsekaiEventScript_CenterWard
	applymovement VAR_LAST_TALKED, Movement_Bow
	waitmovement 0
	fadedefaultbgm
	releaseall
	end

EventScript_PkmnCenterNurse_Frlg::
	message Text_WelcomeWantToHealPkmn_Frlg
	waitmessage
	multichoice 19, 8, MULTI_YESNO, FALSE | (TRUE << 1)
	switch VAR_RESULT
	case 0, EventScript_PkmnCenterNurse_HealPkmn_Frlg
	case 1 EventScript_PkmnCenterNurse_Goodbye_Frlg
	case MULTI_B_PRESSED, EventScript_PkmnCenterNurse_Goodbye_Frlg
	end

EventScript_PkmnCenterNurse_HealPkmn_Frlg::
	incrementgamestat GAME_STAT_USED_POKECENTER
	@ FireRed Isekai: healing costs money (free until the first badge, or on the POKé CARE PLAN)
	special IsekaiHealQuote
	switch VAR_RESULT
	case 0, IsekaiEventScript_Nurse_Newbie
	case 1, IsekaiEventScript_Nurse_Insured
	case 3, IsekaiEventScript_Nurse_Quote
	case 4, IsekaiEventScript_Nurse_CantAfford
EventScript_PkmnCenterNurse_DoHeal_Frlg::
	message Text_TakeYourPkmnForFewSeconds_Frlg
	waitmessage
	call EventScript_PkmnCenterNurse_TakeAndHealPkmn_Frlg
	call IsekaiEventScript_CenterWard @ FireRed Isekai: Badly Injured POKéMON
	goto EventScript_PkmnCenterNurse_CheckTrainerTowerAndUnionRoom_Frlg
	end

EventScript_PkmnCenterNurse_TakeAndHealPkmn_Frlg::
	getpartysize
	goto_if_eq VAR_RESULT, 0, EventScript_PkmnCenterNurse_NoPkmnToHeal_Frlg @ FireRed Isekai: nobody left to heal
	applymovement VAR_LAST_TALKED, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	dofieldeffect FLDEFF_POKECENTER_HEAL
	waitfieldeffect FLDEFF_POKECENTER_HEAL
	applymovement VAR_LAST_TALKED, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	special HealPlayerParty
	return

EventScript_PkmnCenterNurse_NoPkmnToHeal_Frlg:
	special HealPlayerParty
	return

EventScript_PkmnCenterNurse_CheckTrainerTowerAndUnionRoom_Frlg::
	specialvar VAR_RESULT, IsPlayerNotInTrainerTowerLobby
	goto_if_eq VAR_RESULT, FALSE, EventScript_PkmnCenterNurse_ReturnPkmn_Frlg
	specialvar VAR_RESULT, BufferUnionRoomPlayerName
	copyvar VAR_0x8008, VAR_RESULT
	goto_if_eq VAR_0x8008, 0, EventScript_PkmnCenterNurse_ReturnPkmn_Frlg
	goto_if_eq VAR_0x8008, 1, EventScript_PkmnCenterNurse_PlayerWaitingInUnionRoom_Frlg
	end

EventScript_PkmnCenterNurse_ReturnPkmn_Frlg::
	message Text_RestoredPkmnToFullHealth_Frlg
	waitmessage
	applymovement VAR_LAST_TALKED, Movement_Bow
	waitmovement 0
	msgbox Text_WeHopeToSeeYouAgain_Frlg
	return

EventScript_PkmnCenterNurse_PlayerWaitingInUnionRoom_Frlg::
	goto_if_set FLAG_NURSE_UNION_ROOM_REMINDER, EventScript_PkmnCenterNurse_ReturnPkmn_Frlg
	msgbox Text_RestoredPkmnToFullHealth_Frlg
	setflag FLAG_NURSE_UNION_ROOM_REMINDER
	message Text_PlayerIsPlayingRightNowGoForIt_Frlg
	waitmessage
	applymovement VAR_LAST_TALKED, Movement_Bow
	waitmovement 0
	msgbox Text_WeHopeToSeeYouAgain_Frlg
	return

EventScript_PkmnCenterNurse_Goodbye_Frlg::
	msgbox Text_WeHopeToSeeYouAgain_Frlg
	return

Movement_Bow::
	nurse_joy_bow
	delay_4
	step_end

Text_WelcomeWantToHealPkmn_Frlg::
	.string "Welcome to our POKéMON CENTER!\p"
	.string "Would you like me to heal your\n"
	.string "POKéMON back to perfect health?$"

Text_TakeYourPkmnForFewSeconds_Frlg::
	.string "Okay, I'll take your POKéMON for a\n"
	.string "few seconds.$"

Text_WeHopeToSeeYouAgain_Frlg::
	.string "We hope to see you again!$"

Text_RestoredPkmnToFullHealth_Frlg::
	.string "Thank you for waiting.\n"
	.string "We've restored your POKéMON to\l"
	.string "full health.$"

@ ---------------------------------------------------------------------------
@ FireRed Isekai: paying for healing, and the POKé CARE PLAN
@ ---------------------------------------------------------------------------
IsekaiEventScript_Nurse_Newbie::
	goto_if_set FLAG_ISEKAI_NURSE_FREE_SAID, EventScript_PkmnCenterNurse_DoHeal_Frlg
	setflag FLAG_ISEKAI_NURSE_FREE_SAID
	msgbox IsekaiText_Nurse_Newbie
	goto EventScript_PkmnCenterNurse_DoHeal_Frlg

IsekaiEventScript_Nurse_Insured::
	msgbox IsekaiText_Nurse_Insured
	goto EventScript_PkmnCenterNurse_DoHeal_Frlg

IsekaiEventScript_Nurse_Quote::
	msgbox IsekaiText_Nurse_Quote, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IsekaiEventScript_Nurse_NotToday
	special IsekaiPayHeal
	playse SE_SHOP
	message Text_TakeYourPkmnForFewSeconds_Frlg
	waitmessage
	call EventScript_PkmnCenterNurse_TakeAndHealPkmn_Frlg
	call IsekaiEventScript_CenterWard
	call IsekaiEventScript_Nurse_OfferPlan
	goto EventScript_PkmnCenterNurse_CheckTrainerTowerAndUnionRoom_Frlg
	end

IsekaiEventScript_Nurse_NotToday::
	call IsekaiEventScript_CenterWard
	msgbox Text_WeHopeToSeeYouAgain_Frlg
	return

@ Not enough money for everyone: one at a time.
IsekaiEventScript_Nurse_CantAfford::
	msgbox IsekaiText_Nurse_CantAfford
IsekaiEventScript_Nurse_PickOne:
	msgbox IsekaiText_Nurse_WhoFirst
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, IsekaiEventScript_Nurse_PickDone
	special IsekaiHealOneQuote
	goto_if_eq VAR_RESULT, 0, IsekaiEventScript_Nurse_PickFine
	goto_if_eq VAR_RESULT, 2, IsekaiEventScript_Nurse_PickTooMuch
	msgbox IsekaiText_Nurse_OneQuote, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IsekaiEventScript_Nurse_PickOne
	special IsekaiHealOne
	playse SE_SHOP
	waitse
	playfanfare MUS_HEAL
	msgbox IsekaiText_Nurse_OneHealed
	waitfanfare
	goto IsekaiEventScript_Nurse_PickOne

IsekaiEventScript_Nurse_PickFine:
	msgbox IsekaiText_Nurse_PickFine
	goto IsekaiEventScript_Nurse_PickOne

IsekaiEventScript_Nurse_PickTooMuch:
	msgbox IsekaiText_Nurse_PickTooMuch
	goto IsekaiEventScript_Nurse_PickOne

IsekaiEventScript_Nurse_PickDone:
	call IsekaiEventScript_CenterWard
	msgbox Text_WeHopeToSeeYouAgain_Frlg
	return

IsekaiEventScript_Nurse_OfferPlan::
	special IsekaiCarePlanShouldOffer
	goto_if_eq VAR_RESULT, FALSE, IsekaiEventScript_Nurse_OfferPlanEnd
	msgbox IsekaiText_Nurse_PlanPitch, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IsekaiEventScript_Nurse_PlanDeclined
	special IsekaiCarePlanSignUp
	playfanfare MUS_OBTAIN_ITEM
	msgbox IsekaiText_Nurse_PlanSigned
	waitfanfare
	msgbox IsekaiText_Nurse_PlanFinePrint
	return
IsekaiEventScript_Nurse_PlanDeclined:
	special IsekaiCarePlanDecline
	msgbox IsekaiText_Nurse_PlanDeclined
IsekaiEventScript_Nurse_OfferPlanEnd:
	return

IsekaiText_Nurse_Newbie:
	.string "You're new around here, aren't you?\p"
	.string "First visits are on the house until\n"
	.string "you've earned a BADGE.\l"
	.string "Everyone's new once.$"

IsekaiText_Nurse_Insured:
	.string "You're on the POKé CARE PLAN, so\n"
	.string "this one's covered!\p"
	.string "Your premium is ¥{STR_VAR_3} every\n"
	.string "500 steps.$"

IsekaiText_Nurse_Quote:
	.string "That'll be ¥{STR_VAR_1}.\n"
	.string "Is that all right?$"

IsekaiText_Nurse_CantAfford:
	.string "Healing everyone would be ¥{STR_VAR_1}…\n"
	.string "and you have ¥{STR_VAR_2}.\p"
	.string "I can do them one at a time.$"

IsekaiText_Nurse_WhoFirst:
	.string "Who should I look at?$"

IsekaiText_Nurse_OneQuote:
	.string "{STR_VAR_2} will be ¥{STR_VAR_1}.\n"
	.string "All right?$"

IsekaiText_Nurse_OneHealed:
	.string "{STR_VAR_2} is good as new.$"

IsekaiText_Nurse_PickFine:
	.string "{STR_VAR_2} is perfectly fine!$"

IsekaiText_Nurse_PickTooMuch:
	.string "{STR_VAR_2} would be ¥{STR_VAR_1}.\n"
	.string "…I'm sorry.$"

IsekaiText_Nurse_PlanPitch:
	.string "By the way… have you heard of the\n"
	.string "POKé CARE PLAN?\p"
	.string "¥{STR_VAR_3} every 500 steps, and all\n"
	.string "your healing is free!\l"
	.string "Ward care is half price!\p"
	.string "And the longer you stay with us,\n"
	.string "the cheaper it gets!\p"
	.string "Would you like to sign up?$"

IsekaiText_Nurse_PlanSigned:
	.string "Welcome to the POKé CARE PLAN!$"

IsekaiText_Nurse_PlanFinePrint:
	.string "Please read the fine print.\n"
	.string "Nobody ever does.\p"
	.string "…Injuries you already have aren't\n"
	.string "covered. Each loss raises your\l"
	.string "premium. Miss one payment and the\l"
	.string "plan ends, discount and all.$"

IsekaiText_Nurse_PlanDeclined:
	.string "No pressure! I'll ask again after\n"
	.string "your next BADGE.$"
