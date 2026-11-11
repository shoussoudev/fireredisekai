ViridianCity_PokemonCenter_1F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, ViridianCity_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

ViridianCity_PokemonCenter_1F_OnTransition::
	setrespawn HEAL_LOCATION_VIRIDIAN_CITY
	end

ViridianCity_PokemonCenter_1F_EventScript_Nurse::
	lock
	faceplayer
	call EventScript_PkmnCenterNurse_Frlg
	release
	end

ViridianCity_PokemonCenter_1F_EventScript_Boy::
	msgbox ViridianCity_PokemonCenter_1F_Text_PokeCenterInEveryTown, MSGBOX_NPC
	end

ViridianCity_PokemonCenter_1F_EventScript_Gentleman::
	msgbox ViridianCity_PokemonCenter_1F_Text_FeelFreeToUsePC, MSGBOX_NPC
	end

ViridianCity_PokemonCenter_1F_EventScript_Youngster::
	msgbox ViridianCity_PokemonCenter_1F_Text_PokeCentersHealMons, MSGBOX_NPC
	end

ViridianCity_PokemonCenter_1F_Text_FeelFreeToUsePC::
	.string "Please feel free to use that PC in\n"
	.string "the corner.\p"
	.string "The receptionist told me so.\n"
	.string "It's so kind of her!$"

ViridianCity_PokemonCenter_1F_Text_PokeCenterInEveryTown::
	.string "There's a POKéMON CENTER in every\n"
	.string "town ahead.\p"
	.string "They charge no money, so don't\n"
	.string "be shy about healing POKéMON.$"

ViridianCity_PokemonCenter_1F_Text_PokeCentersHealMons::
	.string "POKéMON CENTERS heal your tired,\n"
	.string "hurt, or fainted POKéMON.\p"
	.string "They make all POKéMON completely\n"
	.string "healthy.$"

@ FireRed Isekai: the ward nurse. The HUMAN WARD is through the door behind her desk.
ViridianCity_PokemonCenter_1F_EventScript_IsekaiWardNurse::
	lock
	faceplayer
	textcolor NPC_TEXT_COLOR_FEMALE
	call_if_set FLAG_ISEKAI_LIMPING, IsekaiEventScript_TreatAnkle
	goto_if_set FLAG_ISEKAI_WARD_NURSE_MET, ViridianCity_PokemonCenter_1F_EventScript_IsekaiWardNurseAgain
	setflag FLAG_ISEKAI_WARD_NURSE_MET
	special IsekaiBufferLives
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiWardNurseFirst
	release
	end

ViridianCity_PokemonCenter_1F_EventScript_IsekaiWardNurseAgain::
	special IsekaiBufferLives
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiWardNurseAgain
	release
	end

ViridianCity_PokemonCenter_1F_Text_IsekaiWardNurseFirst::
	.string "I run the HUMAN WARD, through\n"
	.string "that door.\p"
	.string "Yes, we have one. People get hurt\n"
	.string "out there too. We just don't put\l"
	.string "it on the posters.\p"
	.string "Young POKéMON faint as a reflex.\n"
	.string "It's how they survive each other.\p"
	.string "And people go easy on beginners.\n"
	.string "Under level 10, nothing sticks.\p"
	.string "That stops once you've earned your\n"
	.string "first BADGE.\p"
	.string "After that, if your team goes down,\n"
	.string "you come through my door.\p"
	.string "{STR_VAR_1} times, and you won't walk\n"
	.string "back out of it. I mean that kindly.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiWardNurseAgain::
	.string "Still in one piece? Good.\p"
	.string "You can come through my door\n"
	.string "{STR_VAR_1} more times. Let's keep it\l"
	.string "that way.$"

@ FireRed Isekai: SHARKEY, the loan shark. Right outside the nurse's counter.
ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkey::
	lock
	faceplayer
	special IsekaiSharkBuildCounter
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyCounter
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyPitch
	dynmultichoice 0, 0, FALSE, 3, 0, DYN_MULTICHOICE_CB_NONE, ViridianCity_PokemonCenter_1F_Text_IsekaiOpt1000, ViridianCity_PokemonCenter_1F_Text_IsekaiOpt3000, ViridianCity_PokemonCenter_1F_Text_IsekaiOptNo
	goto_if_eq VAR_RESULT, 0, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyLend
	goto_if_eq VAR_RESULT, 1, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyCollateral
	goto ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyNo

ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyLend::
	special IsekaiSharkLend
	playse SE_SHOP
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyLent
	release
	end

ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyCollateral::
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyCollateral
	special IsekaiSharkBuildList
	goto_if_eq VAR_RESULT, 0, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyNothingGood
	goto_if_eq VAR_RESULT, 0xFF, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyFull
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_ge VAR_RESULT, 6, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyNo
	copyvar VAR_0x8004, VAR_RESULT
	setvar VAR_0x8005, 1
	special IsekaiSharkTake
	playse SE_SHOP
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyCollateralTaken
	release
	end

ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyNothingGood::
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyNothingGood
	release
	end

ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyFull::
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyFull
	release
	end

ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyNo::
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyNo
	release
	end

@ He owes, or SHARKEY holds one of his POKéMON.
ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyCounter::
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyWhatllItBe
	setvar VAR_0x8006, 100
	dynmultistack 0, 0, FALSE, 6, FALSE, VAR_0x8006, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 100, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyPayDebt
	goto_if_ge VAR_RESULT, 3, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyLater
	copyvar VAR_0x8004, VAR_RESULT
	special IsekaiSharkBuyBack
	goto_if_eq VAR_RESULT, 0, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyIsCollateral
	goto_if_eq VAR_RESULT, 2, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyCantAfford
	playse SE_SHOP
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyBoughtBack
	release
	end

ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyIsCollateral::
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyIsCollateral
	release
	end

ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyCantAfford::
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyCantAfford
	release
	end

ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyPayDebt::
	special IsekaiSharkDebt
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyOwed, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyLater
	special IsekaiSharkRepay
	goto_if_eq VAR_RESULT, 0, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyBroke
	playse SE_SHOP
	goto_if_eq VAR_RESULT, 1, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyPart
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyPaid
	special IsekaiSharkReturnCollateral
	goto_if_eq VAR_RESULT, FALSE, ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyEnd
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyCollateralBack
ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyEnd:
	release
	end

ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyPart::
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyPart
	release
	end

ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyBroke::
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyBroke
	release
	end

ViridianCity_PokemonCenter_1F_EventScript_IsekaiSharkeyLater::
	msgbox ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyLater
	release
	end

ViridianCity_PokemonCenter_1F_Text_IsekaiOpt1000:
	.string "¥1000$"
ViridianCity_PokemonCenter_1F_Text_IsekaiOpt3000:
	.string "¥3000 (collateral)$"
ViridianCity_PokemonCenter_1F_Text_IsekaiOptNo:
	.string "No thanks$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyCollateral::
	.string "SHARKEY: ¥3000. But I hold one of\n"
	.string "your POKéMON till it's paid back.\p"
	.string "Nothing personal.\n"
	.string "Just paperwork.\p"
	.string "And not any old POKéMON. Show me\n"
	.string "the ones that do the work.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyCollateralTaken::
	.string "{PLAYER} handed over {STR_VAR_1}\n"
	.string "and got ¥3000.\p"
	.string "SHARKEY: It'll be fed.\n"
	.string "Mostly.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyNothingGood::
	.string "SHARKEY: Those? They've never seen\n"
	.string "a fight. Worthless.\p"
	.string "Take the ¥1000 deal.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyFull::
	.string "SHARKEY: My kennels are full.\n"
	.string "Some of them are yours, actually.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyWhatllItBe::
	.string "SHARKEY: There he is!\n"
	.string "What'll it be?$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyIsCollateral::
	.string "SHARKEY: {STR_VAR_1}'s collateral.\n"
	.string "Pay off the loan and it's yours.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyCantAfford::
	.string "SHARKEY: {STR_VAR_1}'s ¥{STR_VAR_2}.\n"
	.string "Come back when you're serious.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyBoughtBack::
	.string "{PLAYER} got {STR_VAR_1} back.\p"
	.string "SHARKEY: See? Everybody wins.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyCollateralBack::
	.string "SHARKEY: And here's your {STR_VAR_1}.\n"
	.string "Not a scratch on it.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyPitch::
	.string "SHARKEY: Short on cash, friend?\n"
	.string "Everybody is, in a place like this.\p"
	.string "I'll lend you ¥1000. Today.\n"
	.string "No questions.\p"
	.string "Or ¥3000, if you leave me something\n"
	.string "to hold on to.\p"
	.string "Pay me back whenever you like.\n"
	.string "Plus a little something every\l"
	.string "500 steps. Ten percent.\p"
	.string "Friendly rates.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyWant::
	.string "SHARKEY: So. ¥1000?$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyLent::
	.string "{PLAYER} got ¥1000.\p"
	.string "SHARKEY: Pleasure doing business.\n"
	.string "I'll be right here. I'm always here.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyNo::
	.string "SHARKEY: Suit yourself.\n"
	.string "The offer doesn't expire.\l"
	.string "Neither do I.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyOwed::
	.string "SHARKEY: There he is!\n"
	.string "You owe me ¥{STR_VAR_1}.\p"
	.string "Paying today?$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyPaid::
	.string "SHARKEY: Paid in full!\n"
	.string "See? Easy.\p"
	.string "Come back anytime. You will.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyPart::
	.string "SHARKEY: ¥{STR_VAR_1}. It's a start.\n"
	.string "Still ¥{STR_VAR_2} to go.$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyBroke::
	.string "SHARKEY: With what?\n"
	.string "Your good looks?$"

ViridianCity_PokemonCenter_1F_Text_IsekaiSharkeyLater::
	.string "SHARKEY: No rush.\p"
	.string "…Well. A little rush.$"
