.equ RECEIVED_TOWN_MAP, VAR_TEMP_1

PalletTown_RivalsHouse_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PalletTown_RivalsHouse_OnTransition
	.byte 0

PalletTown_RivalsHouse_OnTransition::
	call_if_lt VAR_MAP_SCENE_PALLET_TOWN_RIVALS_HOUSE, 2, PalletTown_RivalsHouse_EventScript_MoveDaisyToTable
	call_if_ge VAR_MAP_SCENE_PALLET_TOWN_RIVALS_HOUSE, 2, PalletTown_RivalsHouse_EventScript_AlreadyReceivedTownMap
	end

PalletTown_RivalsHouse_EventScript_MoveDaisyToTable::
	setobjectmovementtype LOCALID_DAISY, MOVEMENT_TYPE_FACE_RIGHT
	setobjectxyperm LOCALID_DAISY, 5, 4
	return

PalletTown_RivalsHouse_EventScript_AlreadyReceivedTownMap::
	setvar RECEIVED_TOWN_MAP, TRUE
	return

PalletTown_RivalsHouse_EventScript_Daisy::
	lock
	faceplayer
	famechecker FAMECHECKER_DAISY, FCPICKSTATE_COLORED, UpdatePickStateFromSpecialVar8005
	goto_if_set FLAG_SYS_GAME_CLEAR, PalletTown_RivalsHouse_EventScript_GroomMon
	@ FireRed Isekai: after the TOWN MAP, she reads how much his POKéMON like him, and grooms them
	@ (so players can check friendship long before the post-game).
	goto_if_eq RECEIVED_TOWN_MAP, TRUE, PalletTown_RivalsHouse_EventScript_IsekaiDaisyRates
	goto_if_eq VAR_MAP_SCENE_PALLET_TOWN_RIVALS_HOUSE, 2, PalletTown_RivalsHouse_EventScript_IsekaiDaisyRates
	goto_if_eq VAR_MAP_SCENE_PALLET_TOWN_RIVALS_HOUSE, 1, PalletTown_RivalsHouse_EventScript_GiveTownMap
	goto_if_ge VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 1, PalletTown_RivalsHouse_EventScript_HeardBattledRival
	msgbox PalletTown_RivalsHouse_Text_HiBrothersAtLab
	closemessage
	applymovement LOCALID_DAISY, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

PalletTown_RivalsHouse_EventScript_IsekaiDaisyRates::
	@ v0.14.9: one topic per talk. First the introduction, then her request, then her services menu.
	goto_if_set FLAG_ISEKAI_DAISY_RATES, PalletTown_RivalsHouse_EventScript_IsekaiDaisyQuest
	setflag FLAG_ISEKAI_DAISY_RATES
	msgbox PalletTown_RivalsHouse_Text_IsekaiDaisyRates
	goto PalletTown_RivalsHouse_EventScript_IsekaiDaisyMenu

PalletTown_RivalsHouse_EventScript_IsekaiDaisyQuest::
	setvar VAR_0x8004, ROM_DAISY
	special IsekaiRomGetState
	goto_if_ne VAR_RESULT, ROM_STATE_NONE, PalletTown_RivalsHouse_EventScript_IsekaiDaisyHub
	switch VAR_ISEKAI_DAISY_QUEST
	case DAISY_QUEST_MESSAGE, PalletTown_RivalsHouse_EventScript_IsekaiDaisyWaiting
	case DAISY_QUEST_DELIVERED, PalletTown_RivalsHouse_EventScript_IsekaiDaisyHeCalled
	@ her request, on its own
	msgbox PalletTown_RivalsHouse_Text_IsekaiDaisyAsk
	setvar VAR_ISEKAI_DAISY_QUEST, DAISY_QUEST_MESSAGE
	release
	end

PalletTown_RivalsHouse_EventScript_IsekaiDaisyWaiting::
	msgbox PalletTown_RivalsHouse_Text_IsekaiDaisyWaiting
	goto PalletTown_RivalsHouse_EventScript_IsekaiDaisyMenu

PalletTown_RivalsHouse_EventScript_IsekaiDaisyHeCalled::
	setvar VAR_ISEKAI_DAISY_QUEST, DAISY_QUEST_DONE
	special IsekaiRomQuestDone
	call IsekaiEventScript_RomHub
	release
	end

PalletTown_RivalsHouse_EventScript_IsekaiDaisyHub::
	call IsekaiEventScript_RomHub
	goto PalletTown_RivalsHouse_EventScript_IsekaiDaisyMenu

@ Her services: read the lead POKéMON's friendship, or groom one.
PalletTown_RivalsHouse_EventScript_GroomMon::
PalletTown_RivalsHouse_EventScript_IsekaiDaisyMenu::
	msgbox PalletTown_RivalsHouse_Text_IsekaiDaisyWhatNow
	dynmultichoice 0, 0, FALSE, 3, 0, DYN_MULTICHOICE_CB_NONE, PalletTown_RivalsHouse_Text_IsekaiOptRate, PalletTown_RivalsHouse_Text_IsekaiOptGroom, PalletTown_RivalsHouse_Text_IsekaiOptNothing
	switch VAR_RESULT
	case 0, PalletTown_RivalsHouse_EventScript_RateMonFriendship
	case 1, PalletTown_RivalsHouse_EventScript_IsekaiDaisyGroom
	msgbox PalletTown_RivalsHouse_Text_DontNeedAnyGrooming
	release
	end

PalletTown_RivalsHouse_EventScript_IsekaiDaisyGroom::
	@ dating DAISY: grooming any time
	setvar VAR_0x8004, ROM_DAISY
	special IsekaiRomCheckPerk
	goto_if_eq VAR_RESULT, TRUE, PalletTown_RivalsHouse_EventScript_IsekaiGroomAnyTime
	goto_if_lt VAR_MASSAGE_COOLDOWN_STEP_COUNTER, 500, PalletTown_RivalsHouse_EventScript_IsekaiGroomTooSoon
PalletTown_RivalsHouse_EventScript_IsekaiGroomAnyTime:
	msgbox PalletTown_RivalsHouse_Text_GroomWhichOne
	special ChoosePartyMon
	lock
	faceplayer
	goto_if_ge VAR_0x8004, PARTY_SIZE, PalletTown_RivalsHouse_EventScript_DeclineGrooming
	specialvar VAR_RESULT, ScriptGetPartyMonSpecies
	goto_if_eq VAR_RESULT, SPECIES_EGG, PalletTown_RivalsHouse_EventScript_CantGroomEgg
	msgbox PalletTown_RivalsHouse_Text_LookingNiceInNoTime
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special DaisyMassageServices
	fadescreen FADE_FROM_BLACK
	special BufferMonNickname
	msgbox PalletTown_RivalsHouse_Text_ThereYouGoAllDone
	release
	end

PalletTown_RivalsHouse_EventScript_IsekaiGroomTooSoon::
	msgbox PalletTown_RivalsHouse_Text_IsekaiGroomTooSoon
	release
	end

PalletTown_RivalsHouse_Text_IsekaiDaisyAsk::
	.string "DAISY: Can I ask you something?\p"
	.string "BLUE hasn't called since he left.\n"
	.string "Not once.\p"
	.string "Our parents work abroad, and Grandpa\n"
	.string "practically lives in his LAB.\p"
	.string "I've kept this house running since\n"
	.string "I was sixteen. I'm twenty-two now.\p"
	.string "If you run into him… tell him to\n"
	.string "call home?\p"
	.string "He only listens to people who beat\n"
	.string "him. He's like that.$"

PalletTown_RivalsHouse_Text_IsekaiDaisyWaiting::
	.string "DAISY: Any news from BLUE?\n"
	.string "…No?\p"
	.string "Beat him first. Then find the\n"
	.string "right words.$"

PalletTown_RivalsHouse_Text_IsekaiDaisyRates::
	.string "DAISY: Oh. It's you again.\p"
	.string "Well… since you're here.\n"
	.string "I'm good with POKéMON, you know.\p"
	.string "I can tell how much they like\n"
	.string "someone just by looking.\p"
	.string "And I groom them. For free.\n"
	.string "Grandpa's never once paid me.$"

PalletTown_RivalsHouse_Text_IsekaiDaisyWhatNow::
	.string "DAISY: So. What can I do for you?$"

PalletTown_RivalsHouse_Text_IsekaiOptRate:
	.string "How do they feel?$"

PalletTown_RivalsHouse_Text_IsekaiOptGroom:
	.string "Groom one$"

PalletTown_RivalsHouse_Text_IsekaiOptNothing:
	.string "Nothing$"

PalletTown_RivalsHouse_Text_IsekaiGroomTooSoon::
	.string "DAISY: I just did one.\n"
	.string "Give it a few hundred steps.$"

PalletTown_RivalsHouse_EventScript_HeardBattledRival::
	msgbox PalletTown_RivalsHouse_Text_HeardYouBattledRival
	release
	end

PalletTown_RivalsHouse_EventScript_CantGroomEgg::
	msgbox PalletTown_RivalsHouse_Text_CantGroomAnEgg
	release
	end

PalletTown_RivalsHouse_EventScript_DeclineGrooming::
	msgbox PalletTown_RivalsHouse_Text_DontNeedAnyGrooming
	release
	end

PalletTown_RivalsHouse_EventScript_RateMonFriendship::
	msgbox PalletTown_RivalsHouse_Text_MayISeeFirstMon
	specialvar VAR_RESULT, GetLeadMonFriendship
	switch VAR_RESULT
	case 0, PalletTown_RivalsHouse_EventScript_MonFriendshipLowest
	case 1, PalletTown_RivalsHouse_EventScript_MonFriendshipLower
	case 2, PalletTown_RivalsHouse_EventScript_MonFriendshipLow
	case 3, PalletTown_RivalsHouse_EventScript_MonFriendshipMid
	case 4, PalletTown_RivalsHouse_EventScript_MonFriendshipHigh
	case 5, PalletTown_RivalsHouse_EventScript_MonFriendshipHigher
	case 6, PalletTown_RivalsHouse_EventScript_MonFriendshipHighest
	end

PalletTown_RivalsHouse_EventScript_MonFriendshipLowest::
	msgbox PalletTown_RivalsHouse_Text_WhyWouldMonHateYouSoMuch
	release
	end

PalletTown_RivalsHouse_EventScript_MonFriendshipLower::
	msgbox PalletTown_RivalsHouse_Text_DontLikeWayItGlaresAtYou
	release
	end

PalletTown_RivalsHouse_EventScript_MonFriendshipLow::
	msgbox PalletTown_RivalsHouse_Text_NotFamiliarWithYouYet
	release
	end

PalletTown_RivalsHouse_EventScript_MonFriendshipMid::
	msgbox PalletTown_RivalsHouse_Text_ItsWarmingUpToYou
	release
	end

PalletTown_RivalsHouse_EventScript_MonFriendshipHigh::
	msgbox PalletTown_RivalsHouse_Text_ItsQuiteFriendly
	release
	end

PalletTown_RivalsHouse_EventScript_MonFriendshipHigher::
	msgbox PalletTown_RivalsHouse_Text_ItLooksVeryHappy
	release
	end

PalletTown_RivalsHouse_EventScript_MonFriendshipHighest::
	msgbox PalletTown_RivalsHouse_Text_CouldntLoveYouMore
	release
	end

PalletTown_RivalsHouse_EventScript_PleaseGiveMonsRest::
	msgbox PalletTown_RivalsHouse_Text_PleaseGiveMonsRest
	release
	end

PalletTown_RivalsHouse_EventScript_GiveTownMap::
	msgbox PalletTown_RivalsHouse_Text_ErrandForGrandpaThisWillHelp
	closemessage
	checkitemspace ITEM_TOWN_MAP
	goto_if_eq VAR_RESULT, FALSE, PalletTown_RivalsHouse_EventScript_NoRoomForTownMap
	applymovement LOCALID_DAISY, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	removeobject LOCALID_TOWN_MAP
	setvar VAR_MAP_SCENE_PALLET_TOWN_RIVALS_HOUSE, 2
	delay 15
	applymovement LOCALID_DAISY, Common_Movement_FacePlayer
	waitmovement 0
	delay 12
	giveitem_msg PalletTown_RivalsHouse_Text_ReceivedTownMapFromDaisy, ITEM_TOWN_MAP, 1, MUS_RG_OBTAIN_KEY_ITEM
	release
	end

PalletTown_RivalsHouse_EventScript_NoRoomForTownMap::
	msgbox PalletTown_RivalsHouse_Text_DontHaveSpaceForThis
	release
	end

PalletTown_RivalsHouse_EventScript_ExplainTownMap::
	msgbox PalletTown_RivalsHouse_Text_ExplainTownMap
	release
	end

PalletTown_RivalsHouse_EventScript_TownMap::
	msgbox PalletTown_RivalsHouse_Text_ItsBigMapOfKanto, MSGBOX_NPC
	end

PalletTown_RivalsHouse_EventScript_Bookshelf::
	msgbox PalletTown_RivalsHouse_Text_ShelvesCrammedFullOfBooks, MSGBOX_SIGN
	end

PalletTown_RivalsHouse_EventScript_Picture::
	msgbox PalletTown_RivalsHouse_Text_LovelyAndSweetClefairy, MSGBOX_SIGN
	end

@ FireRed Isekai: Daisy has never seen this man before.
PalletTown_RivalsHouse_Text_HiBrothersAtLab::
	.string "DAISY: Oh! Um… can I help you?\p"
	.string "If you're looking for my brother,\n"
	.string "{RIVAL}, he's at Grandpa's LAB.\p"
	.string "And if you're selling something,\n"
	.string "we're not interested.$"

PalletTown_RivalsHouse_Text_HeardYouBattledRival::
	.string "DAISY: Wait. You're the man from\n"
	.string "the LAB?\p"
	.string "{RIVAL} came home talking about\n"
	.string "“some old guy with a rental.”\p"
	.string "He hasn't stopped since.\n"
	.string "I wish I'd seen it!$"

PalletTown_RivalsHouse_Text_ErrandForGrandpaThisWillHelp::
	.string "Grandpa sent a total stranger to\n"
	.string "run his errands?\p"
	.string "Gee, that's lazy of him.\n"
	.string "Here. You look like you get lost.$"

PalletTown_RivalsHouse_Text_ReceivedTownMapFromDaisy::
	.string "{PLAYER} received a TOWN MAP\n"
	.string "from DAISY.$"

PalletTown_RivalsHouse_Text_DontHaveSpaceForThis::
	.string "You don't have space for this in\n"
	.string "your BAG.$"

PalletTown_RivalsHouse_Text_ExplainTownMap::
	.string "You can use the TOWN MAP to find\n"
	.string "out where you are, or check the\l"
	.string "names of places.$"

PalletTown_RivalsHouse_Text_PleaseGiveMonsRest::
	.string "DAISY: Just like people, POKéMON\n"
	.string "are living things.\p"
	.string "When they get tired, please give\n"
	.string "them a rest.$"

PalletTown_RivalsHouse_Text_ItsBigMapOfKanto::
	.string "It's a big map of the KANTO region.\n"
	.string "Now this would be useful!$"

PalletTown_RivalsHouse_Text_ShelvesCrammedFullOfBooks::
	.string "The shelves are crammed full of\n"
	.string "books on POKéMON.$"

PalletTown_RivalsHouse_Text_LikeMeToGroomMon::
	.string "DAISY: Hi, {PLAYER}!\n"
	.string "Good timing.\p"
	.string "I'm about to have some tea.\n"
	.string "Would you like to join me?\p"
	.string "Oh, but look.\n"
	.string "Your POKéMON are a little dirty.\p"
	.string "Would you like me to groom one?$"

PalletTown_RivalsHouse_Text_DontNeedAnyGrooming::
	.string "DAISY: Okay. Mind the cat on your\n"
	.string "way out.$"

PalletTown_RivalsHouse_Text_GroomWhichOne::
	.string "DAISY: Which one?$"

PalletTown_RivalsHouse_Text_LookingNiceInNoTime::
	.string "DAISY: Hold still. This won't take\n"
	.string "long.$"

PalletTown_RivalsHouse_Text_ThereYouGoAllDone::
	.string "{FONT_NORMAL}{STR_VAR_1} looks dreamily content…\p"
	.string "{FONT_FEMALE}DAISY: There. Much better.\n"
	.string "Try to keep it that way.$"

PalletTown_RivalsHouse_Text_CantGroomAnEgg::
	.string "DAISY: That's an EGG.\n"
	.string "I'm good, but not that good.$"

PalletTown_RivalsHouse_Text_MayISeeFirstMon::
	.string "DAISY: Let me see your first\n"
	.string "POKéMON.$"

PalletTown_RivalsHouse_Text_CouldntLoveYouMore::
	.string "It couldn't possibly love you\n"
	.string "any more than it does now.\p"
	.string "Your POKéMON is happy beyond\n"
	.string "words.$"

PalletTown_RivalsHouse_Text_ItLooksVeryHappy::
	.string "It looks very happy.\p"
	.string "I wish {RIVAL} could see this and\n"
	.string "learn something from it.$"

PalletTown_RivalsHouse_Text_ItsQuiteFriendly::
	.string "It's quite friendly with you.\n"
	.string "Keep being good to it!$"

PalletTown_RivalsHouse_Text_ItsWarmingUpToYou::
	.string "It's warming up to you.\n"
	.string "Trust must be growing between you.$"

PalletTown_RivalsHouse_Text_NotFamiliarWithYouYet::
	.string "It doesn't know you very well yet.\n"
	.string "Give it time.$"

PalletTown_RivalsHouse_Text_DontLikeWayItGlaresAtYou::
	.string "I don't like the way it glares at\n"
	.string "you. Be a little nicer to it.$"

PalletTown_RivalsHouse_Text_WhyWouldMonHateYouSoMuch::
	.string "…Um, it's not easy for me to say\n"
	.string "this, but…\p"
	.string "Is there some reason why your\n"
	.string "POKéMON would hate you so much?$"

PalletTown_RivalsHouse_Text_LovelyAndSweetClefairy::
	.string "“The lovely and sweet\n"
	.string "CLEFAIRY”$"

