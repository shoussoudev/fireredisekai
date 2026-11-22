PewterCity_Museum_1F_Frlg_MapScripts::
	.byte 0

PewterCity_Museum_1F_EventScript_Scientist1::
	lock
	faceplayer
	goto_if_eq VAR_FACING, DIR_WEST, PewterCity_Museum_1F_EventScript_Scientist1BehindCounter
	goto_if_eq VAR_FACING, DIR_SOUTH, PewterCity_Museum_1F_EventScript_Scientist1BehindCounter
	goto_if_eq VAR_FACING, DIR_NORTH, PewterCity_Museum_1F_EventScript_Scientist1BehindCounter
	msgbox PewterCity_Museum_1F_Text_PleaseEnjoyYourself
	release
	end

PewterCity_Museum_1F_EventScript_Scientist1BehindCounter::
	msgbox PewterCity_Museum_1F_Text_DoYouKnowWhatAmberIs, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, PewterCity_Museum_1F_EventScript_AmberHasGeneticMatter
	goto_if_eq VAR_RESULT, NO, PewterCity_Museum_1F_EventScript_ExplainAmber
	end

PewterCity_Museum_1F_EventScript_AmberHasGeneticMatter::
	msgbox PewterCity_Museum_1F_Text_AmberContainsGeneticMatter
	applymovement LOCALID_MUSEUM_SCIENTIST1, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

PewterCity_Museum_1F_EventScript_ExplainAmber::
	msgbox PewterCity_Museum_1F_Text_AmberIsFossilizedSap
	applymovement LOCALID_MUSEUM_SCIENTIST1, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

PewterCity_Museum_1F_EventScript_EntranceTriggerLeft::
	lockall
	setvar VAR_TEMP_1, 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	goto PewterCity_Museum_1F_EventScript_EntranceTrigger
	end

PewterCity_Museum_1F_EventScript_EntranceTriggerMid::
	lockall
	setvar VAR_TEMP_1, 1
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	goto PewterCity_Museum_1F_EventScript_EntranceTrigger
	end

PewterCity_Museum_1F_EventScript_EntranceTriggerRight::
	lockall
	setvar VAR_TEMP_1, 2
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	goto PewterCity_Museum_1F_EventScript_EntranceTrigger
	end

PewterCity_Museum_1F_EventScript_EntranceTrigger::
	textcolor NPC_TEXT_COLOR_MALE
	showmoneybox 0, 0
	msgbox PewterCity_Museum_1F_Text_Its50YForChildsTicket, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, PewterCity_Museum_1F_EventScript_TryPayForTicket
	msgbox PewterCity_Museum_1F_Text_ComeAgain
	closemessage
	hidemoneybox
	applymovement LOCALID_PLAYER, PewterCity_Museum_1F_Movement_ForcePlayerExit
	waitmovement 0
	releaseall
	end

PewterCity_Museum_1F_EventScript_TryPayForTicket::
	checkmoney 50
	goto_if_eq VAR_RESULT, FALSE, PewterCity_Museum_1F_EventScript_NotEnoughMoney
	closemessage
	call_if_eq VAR_TEMP_1, 0, PewterCity_Museum_1F_EventScript_PlayerApproachCounterLeft
	call_if_eq VAR_TEMP_1, 1, PewterCity_Museum_1F_EventScript_PlayerApproachCounterMid
	call_if_eq VAR_TEMP_1, 2, PewterCity_Museum_1F_EventScript_PlayerApproachCounterRight
	playse SE_SHOP
	removemoney 50
	updatemoneybox
	waitse
	msgbox PewterCity_Museum_1F_Text_Right50YThankYou
	setvar VAR_MAP_SCENE_PEWTER_CITY_MUSEUM_1F, 1
	hidemoneybox
	releaseall
	end

PewterCity_Museum_1F_EventScript_PlayerApproachCounterLeft::
	applymovement LOCALID_PLAYER, PewterCity_Museum_1F_Movement_ApproachCounterLeft
	waitmovement 0
	return

PewterCity_Museum_1F_EventScript_PlayerApproachCounterMid::
	applymovement LOCALID_PLAYER, PewterCity_Museum_1F_Movement_ApproachCounterMid
	waitmovement 0
	return

PewterCity_Museum_1F_EventScript_PlayerApproachCounterRight::
	applymovement LOCALID_PLAYER, PewterCity_Museum_1F_Movement_ApproachCounterRight
	waitmovement 0
	return

@ FireRed Isekai: broke after Oak's deposit? The receptionist sighs and lets him in.
PewterCity_Museum_1F_EventScript_NotEnoughMoney::
	msgbox PewterCity_Museum_1F_Text_IsekaiSighLetIn
	closemessage
	call_if_eq VAR_TEMP_1, 0, PewterCity_Museum_1F_EventScript_PlayerApproachCounterLeft
	call_if_eq VAR_TEMP_1, 1, PewterCity_Museum_1F_EventScript_PlayerApproachCounterMid
	call_if_eq VAR_TEMP_1, 2, PewterCity_Museum_1F_EventScript_PlayerApproachCounterRight
	setvar VAR_MAP_SCENE_PEWTER_CITY_MUSEUM_1F, 1
	hidemoneybox
	releaseall
	end

PewterCity_Museum_1F_Text_IsekaiSighLetIn::
	.string "…You can't afford ¥50?\p"
	.string "(The receptionist looks at you for\n"
	.string "a long time. Then sighs.)\p"
	.string "Go on. Just go.$"

PewterCity_Museum_1F_Movement_ForcePlayerExit::
	walk_down
	step_end

PewterCity_Museum_1F_Movement_ApproachCounterLeft::
	walk_right
	walk_right
	delay_8
	walk_in_place_right
	step_end

PewterCity_Museum_1F_Movement_ApproachCounterMid::
	walk_right
	delay_8
	walk_in_place_right
	step_end

PewterCity_Museum_1F_Movement_ApproachCounterRight::
	delay_8
	walk_in_place_right
	step_end

@ FireRed Isekai (v0.14): DR. PEBBLE, researcher of strange phenomena.
PewterCity_Museum_1F_EventScript_Scientist2::
	lock
	faceplayer
	textcolor NPC_TEXT_COLOR_MALE
	goto_if_unset FLAG_ISEKAI_MUSEUM_TOUR, PewterCity_Museum_1F_EventScript_IsekaiPebbleTour
	goto_if_eq VAR_ISEKAI_MUM_QUEST, MUM_QUEST_ADAPTER, PewterCity_Museum_1F_EventScript_IsekaiPebbleAdapter
	goto_if_eq VAR_ISEKAI_MUM_QUEST, MUM_QUEST_ASSISTANT, PewterCity_Museum_1F_EventScript_IsekaiPebbleQuiz
	msgbox PewterCity_Museum_1F_Text_IsekaiPebbleAgain
	release
	end

PewterCity_Museum_1F_EventScript_IsekaiPebbleTour::
	msgbox PewterCity_Museum_1F_Text_IsekaiPebbleTour1
	msgbox PewterCity_Museum_1F_Text_IsekaiPebbleTour2
	msgbox PewterCity_Museum_1F_Text_IsekaiPebbleTour3
	setflag FLAG_ISEKAI_MUSEUM_TOUR
	release
	end

PewterCity_Museum_1F_EventScript_IsekaiPebbleAdapter::
	msgbox PewterCity_Museum_1F_Text_IsekaiPebbleAdapterAsk
	msgbox PewterCity_Museum_1F_Text_IsekaiPebbleReady, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PewterCity_Museum_1F_EventScript_IsekaiPebbleLater
	msgbox PewterCity_Museum_1F_Text_IsekaiPebbleAssistant
	closemessage
	trainerbattle_no_intro TRAINER_ISEKAI_PEBBLE_ASSISTANT, PewterCity_Museum_1F_Text_IsekaiAssistantDefeat
	setvar VAR_ISEKAI_MUM_QUEST, MUM_QUEST_ASSISTANT
	msgbox PewterCity_Museum_1F_Text_IsekaiPebbleNowQuiz
PewterCity_Museum_1F_EventScript_IsekaiPebbleQuiz::
	msgbox PewterCity_Museum_1F_Text_IsekaiQuiz1
	dynmultichoice 0, 0, TRUE, 3, 0, DYN_MULTICHOICE_CB_NONE, PewterCity_Museum_1F_Text_IsekaiQ1A, PewterCity_Museum_1F_Text_IsekaiQ1B, PewterCity_Museum_1F_Text_IsekaiQ1C
	goto_if_ne VAR_RESULT, 1, PewterCity_Museum_1F_EventScript_IsekaiQuizFail
	msgbox PewterCity_Museum_1F_Text_IsekaiQuiz2
	dynmultichoice 0, 0, TRUE, 3, 0, DYN_MULTICHOICE_CB_NONE, PewterCity_Museum_1F_Text_IsekaiQ2A, PewterCity_Museum_1F_Text_IsekaiQ2B, PewterCity_Museum_1F_Text_IsekaiQ2C
	goto_if_ne VAR_RESULT, 1, PewterCity_Museum_1F_EventScript_IsekaiQuizFail
	msgbox PewterCity_Museum_1F_Text_IsekaiQuiz2Comment
	msgbox PewterCity_Museum_1F_Text_IsekaiQuiz3
	dynmultichoice 0, 0, TRUE, 3, 0, DYN_MULTICHOICE_CB_NONE, PewterCity_Museum_1F_Text_IsekaiQ3A, PewterCity_Museum_1F_Text_IsekaiQ3B, PewterCity_Museum_1F_Text_IsekaiQ3C
	goto_if_ne VAR_RESULT, 2, PewterCity_Museum_1F_EventScript_IsekaiQuizFail
	msgbox PewterCity_Museum_1F_Text_IsekaiQuizPerfect
	setvar VAR_ISEKAI_MUM_QUEST, MUM_QUEST_GOT_IT
	release
	end

PewterCity_Museum_1F_EventScript_IsekaiQuizFail::
	msgbox PewterCity_Museum_1F_Text_IsekaiQuizFail
	release
	end

PewterCity_Museum_1F_EventScript_IsekaiPebbleLater::
	msgbox PewterCity_Museum_1F_Text_IsekaiPebbleLater
	release
	end

PewterCity_Museum_1F_Text_IsekaiPebbleTour1::
	.string "DR. PEBBLE: A visitor! A real one!\n"
	.string "Come, come!\p"
	.string "Phenomenon No. 1!\n"
	.string "LIGHTS OVER MT. MOON!\p"
	.string "(A blurry photo of a CLEFAIRY\n"
	.string "holding a torch.)\p"
	.string "Phenomenon No. 2! OBJECTS THAT\n"
	.string "APPEAR FROM NOWHERE!\p"
	.string "(A single sock.)\p"
	.string "Found in my dryer.\n"
	.string "It is NOT one of mine.$"

PewterCity_Museum_1F_Text_IsekaiPebbleTour2::
	.string "Phenomenon No. 3! A ROCK THAT\n"
	.string "LOOKS LIKE A FACE!\p"
	.string "(It's a GEODUDE. It's asleep.)\p"
	.string "And my finest case… PEOPLE WHO WAKE\n"
	.string "UP SOMEWHERE THEY'VE NEVER BEEN!\p"
	.string "(A photo of his assistant asleep\n"
	.string "on a bus.)\p"
	.string "SAFFRON. He was going to CELADON.$"

PewterCity_Museum_1F_Text_IsekaiPebbleTour3::
	.string "(You tell him everything. The truck.\n"
	.string "Waking up in RED's bed. Knowing\l"
	.string "this world from a game.)\p"
	.string "DR. PEBBLE: …\p"
	.string "…Fascinating.\n"
	.string "Have you considered that you might\l"
	.string "be very tired?\p"
	.string "Here. Take this.\n"
	.string "(He hands you a museum leaflet.)$"

PewterCity_Museum_1F_Text_IsekaiPebbleAgain::
	.string "DR. PEBBLE: Back again? I'm still\n"
	.string "analysing your… tiredness.$"

PewterCity_Museum_1F_Text_IsekaiPebbleAdapterAsk::
	.string "DR. PEBBLE: The spare adapter? From\n"
	.string "my EARLY HOME CONSOLES display?\p"
	.string "Out of the question!\p"
	.string "…Unless. My assistant needs battle\n"
	.string "data, and I need a quiz partner.\p"
	.string "Beat him, then ace my quiz on old\n"
	.string "games. Then we'll talk.$"

PewterCity_Museum_1F_Text_IsekaiPebbleReady::
	.string "Ready?$"

PewterCity_Museum_1F_Text_IsekaiPebbleLater::
	.string "Science waits for no one.\n"
	.string "Except, apparently, you.$"

PewterCity_Museum_1F_Text_IsekaiPebbleAssistant::
	.string "DR. PEBBLE: ASSISTANT!\n"
	.string "Battle data! Now!$"

PewterCity_Museum_1F_Text_IsekaiAssistantDefeat::
	.string "I was asleep on a bus last week.\n"
	.string "I'm still not over it.$"

PewterCity_Museum_1F_Text_IsekaiPebbleNowQuiz::
	.string "DR. PEBBLE: Excellent data!\n"
	.string "Now. The quiz.$"

PewterCity_Museum_1F_Text_IsekaiQuiz1::
	.string "Question one. How many face\n"
	.string "buttons on an SNES controller?$"

PewterCity_Museum_1F_Text_IsekaiQ1A:
	.string "Two$"
PewterCity_Museum_1F_Text_IsekaiQ1B:
	.string "Four$"
PewterCity_Museum_1F_Text_IsekaiQ1C:
	.string "Six$"

PewterCity_Museum_1F_Text_IsekaiQuiz2::
	.string "Question two. A cartridge won't\n"
	.string "start. What do you do?$"

PewterCity_Museum_1F_Text_IsekaiQ2A:
	.string "Hit it$"
PewterCity_Museum_1F_Text_IsekaiQ2B:
	.string "Blow on it$"
PewterCity_Museum_1F_Text_IsekaiQ2C:
	.string "Pray$"

PewterCity_Museum_1F_Text_IsekaiQuiz2Comment::
	.string "DR. PEBBLE: That does nothing,\n"
	.string "scientifically.\p"
	.string "YOU: It works.$"

PewterCity_Museum_1F_Text_IsekaiQuiz3::
	.string "Question three. Which colour were\n"
	.string "the original handheld's A and B\l"
	.string "buttons?$"

PewterCity_Museum_1F_Text_IsekaiQ3A:
	.string "Grey$"
PewterCity_Museum_1F_Text_IsekaiQ3B:
	.string "Green$"
PewterCity_Museum_1F_Text_IsekaiQ3C:
	.string "Magenta$"

PewterCity_Museum_1F_Text_IsekaiQuizPerfect::
	.string "DR. PEBBLE: …A perfect score.\p"
	.string "Nobody gets a perfect score.\n"
	.string "Deeply suspicious.\p"
	.string "…A deal's a deal. Take it.\p"
	.string "(You got the spare SNES adapter.)$"

PewterCity_Museum_1F_Text_IsekaiQuizFail::
	.string "DR. PEBBLE: Wrong! Come back when\n"
	.string "you've studied.$"

PewterCity_Museum_1F_EventScript_OldMan::
	msgbox PewterCity_Museum_1F_Text_ShouldBeGratefulForLongLife, MSGBOX_NPC
	end

PewterCity_Museum_1F_EventScript_OldAmberScientist::
	lock
	faceplayer
	goto_if_set FLAG_GOT_OLD_AMBER, PewterCity_Museum_1F_EventScript_AlreadyGotOldAmber
	msgbox PewterCity_Museum_1F_Text_WantYouToGetAmberExamined
	checkitemspace ITEM_OLD_AMBER
	goto_if_eq VAR_RESULT, FALSE, PewterCity_Museum_1F_EventScript_NoRoomForOldAmber
	setflag FLAG_GOT_OLD_AMBER
	removeobject LOCALID_OLD_AMBER
	giveitem_msg PewterCity_Museum_1F_Text_ReceivedOldAmberFromMan, ITEM_OLD_AMBER, 1, MUS_RG_OBTAIN_KEY_ITEM
	release
	end

PewterCity_Museum_1F_EventScript_NoRoomForOldAmber::
	msgbox PewterCity_Museum_1F_Text_DontHaveSpaceForThis
	release
	end

PewterCity_Museum_1F_EventScript_AlreadyGotOldAmber::
	msgbox PewterCity_Museum_1F_Text_GetOldAmberChecked
	release
	end

PewterCity_Museum_1F_EventScript_OldAmber::
	msgbox PewterCity_Museum_1F_Text_BeautifulPieceOfAmber, MSGBOX_NPC
	end

PewterCity_Museum_1F_EventScript_AerodactylFossil::
	lockall
	setvar VAR_0x8004, SPECIES_AERODACTYL
	setvar VAR_0x8005, 10
	setvar VAR_0x8006, 3
	special OpenMuseumFossilPic
	msgbox PewterCity_Museum_1F_Text_AerodactylFossil
	special CloseMuseumFossilPic
	releaseall
	end

PewterCity_Museum_1F_EventScript_KabutopsFossil::
	lockall
	setvar VAR_0x8004, SPECIES_KABUTOPS
	setvar VAR_0x8005, 10
	setvar VAR_0x8006, 3
	special OpenMuseumFossilPic
	msgbox PewterCity_Museum_1F_Text_KabutopsFossil
	special CloseMuseumFossilPic
	releaseall
	end

PewterCity_Museum_1F_Text_Its50YForChildsTicket::
	.string "Yes, yes.\n"
	.string "It's ¥50 for a child's ticket.\p"
	.string "Would you like to come in?$"

PewterCity_Museum_1F_Text_ComeAgain::
	.string "Come again!$"

PewterCity_Museum_1F_Text_Right50YThankYou::
	.string "Right, ¥50!\n"
	.string "Thank you!$"

PewterCity_Museum_1F_Text_DontHaveEnoughMoney::
	.string "You don't have enough money.$"

PewterCity_Museum_1F_Text_PleaseEnjoyYourself::
	.string "Please enjoy yourself.$"

PewterCity_Museum_1F_Text_DoYouKnowWhatAmberIs::
	.string "You can't sneak in the back way!\n"
	.string "Nice try, kid, but no.\p"
	.string "Oh, whatever!\n"
	.string "Do you know what AMBER is?$"

PewterCity_Museum_1F_Text_AmberContainsGeneticMatter::
	.string "AMBER contains the genetic matter\n"
	.string "of ancient POKéMON.\p"
	.string "There's a LAB somewhere trying to\n"
	.string "resurrect POKéMON from AMBER.$"

PewterCity_Museum_1F_Text_AmberIsFossilizedSap::
	.string "AMBER is actually sap, that gooey\n"
	.string "stuff that oozes from trees.\p"
	.string "The ancient sap fossilized over\n"
	.string "time to become rock-hard AMBER.$"

@ Unused
PewterCity_Museum_1F_Text_PleaseGoAround::
	.string "あちらへ　おまわりください$"

PewterCity_Museum_1F_Text_ShouldBeGratefulForLongLife::
	.string "I should be grateful for my long\n"
	.string "life.\p"
	.string "Never did I think I would get to\n"
	.string "see the bones of a dragon!$"

PewterCity_Museum_1F_Text_WantYouToGetAmberExamined::
	.string "Ssh! Listen, I need to share a\n"
	.string "secret with someone.\p"
	.string "I think that this chunk of AMBER\n"
	.string "contains POKéMON DNA!\p"
	.string "It would be a shattering scientific\n"
	.string "breakthrough if POKéMON could be\l"
	.string "resurrected from it.\p"
	.string "But, my colleagues just ignore\n"
	.string "what I have to say.\p"
	.string "So, I have a favor to ask!\p"
	.string "I want you to get this examined at\n"
	.string "a POKéMON LAB somewhere.$"

PewterCity_Museum_1F_Text_ReceivedOldAmberFromMan::
	.string "{PLAYER} received the OLD AMBER\n"
	.string "from the man.$"

PewterCity_Museum_1F_Text_GetOldAmberChecked::
	.string "Ssh!\n"
	.string "Get the OLD AMBER checked!$"

PewterCity_Museum_1F_Text_DontHaveSpaceForThis::
	.string "You don't have space for this.$"

PewterCity_Museum_1F_Text_WeHaveTwoFossilsOnExhibit::
	.string "We have two fossils of rare,\n"
	.string "prehistoric POKéMON on exhibit.$"

PewterCity_Museum_1F_Text_BeautifulPieceOfAmber::
	.string "There is a beautiful piece of\n"
	.string "AMBER in a clear gold color.$"

PewterCity_Museum_1F_Text_AerodactylFossil::
	.string "AERODACTYL Fossil\n"
	.string "A primitive and rare POKéMON.$"

PewterCity_Museum_1F_Text_KabutopsFossil::
	.string "KABUTOPS Fossil\n"
	.string "A primitive and rare POKéMON.$"

