PalletTown_PlayersHouse_1F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, PalletTown_PlayersHouse_1F_OnFrame
	.byte 0

@ FireRed Isekai: coming down the stairs into a very alarmed woman
PalletTown_PlayersHouse_1F_OnFrame::
	map_script_2 VAR_ISEKAI_INTRO, 1, PalletTown_PlayersHouse_1F_EventScript_FryingPan
	.2byte 0

PalletTown_PlayersHouse_1F_EventScript_FryingPan::
	lockall
	delay 20
	turnobject LOCALID_PLAYERS_HOUSE_1F_MOM, DIR_EAST
	playse SE_PIN
	applymovement LOCALID_PLAYERS_HOUSE_1F_MOM, Common_Movement_ExclamationMark
	waitmovement 0
	msgbox PalletTown_PlayersHouse_1F_Text_FryingPan
	closemessage
	setflag FLAG_ISEKAI_MET_REDS_MUM
	setvar VAR_ISEKAI_INTRO, 2
	releaseall
	end

PalletTown_PlayersHouse_1F_EventScript_Mom::
	lock
	faceplayer
	goto_if_set FLAG_BEAT_RIVAL_IN_OAKS_LAB, PalletTown_PlayersHouse_1F_EventScript_MomHeal
	call_if_set FLAG_ISEKAI_WINDOW_ESCAPE, PalletTown_PlayersHouse_1F_EventScript_MomWindowMan
	call_if_unset FLAG_ISEKAI_WINDOW_ESCAPE, PalletTown_PlayersHouse_1F_EventScript_MomStillHasPan
	closemessage
	applymovement LOCALID_PLAYERS_HOUSE_1F_MOM, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

PalletTown_PlayersHouse_1F_EventScript_MomStillHasPan::
	msgbox PalletTown_PlayersHouse_1F_Text_MomStillHasPan
	return

PalletTown_PlayersHouse_1F_EventScript_MomWindowMan::
	msgbox PalletTown_PlayersHouse_1F_Text_MomWindowMan
	return

PalletTown_PlayersHouse_1F_EventScript_MomOakLookingForYouMale::
	msgbox PalletTown_PlayersHouse_1F_Text_AllBoysLeaveOakLookingForYou
	return

PalletTown_PlayersHouse_1F_EventScript_MomOakLookingForYouFemale::
	msgbox PalletTown_PlayersHouse_1F_Text_AllGirlsLeaveOakLookingForYou
	return

PalletTown_PlayersHouse_1F_EventScript_MomHeal::
	msgbox PalletTown_PlayersHouse_1F_Text_YouShouldTakeQuickRest
	closemessage
	call Common_EventScript_OutOfCenterPartyHeal
	msgbox PalletTown_PlayersHouse_1F_Text_LookingGreatTakeCare
	release
	end

@ Displays special text if interacted with from side or back (which are normally inaccessible)
PalletTown_PlayersHouse_1F_EventScript_TV::
	lockall
	goto_if_eq VAR_FACING, DIR_NORTH, PalletTown_PlayersHouse_1F_EventScript_TVScreen
	msgbox PalletTown_PlayersHouse_1F_Text_OopsWrongSide
	releaseall
	end

PalletTown_PlayersHouse_1F_EventScript_TVScreen::
	checkplayergender
	call_if_eq VAR_RESULT, MALE, PalletTown_PlayersHouse_1F_EventScript_TVScreenMale
	call_if_eq VAR_RESULT, FEMALE, PalletTown_PlayersHouse_1F_EventScript_TVScreenFemale
	releaseall
	end

PalletTown_PlayersHouse_1F_EventScript_TVScreenMale::
	msgbox PalletTown_PlayersHouse_1F_Text_MovieOnTVFourBoysOnRailroad
	return

PalletTown_PlayersHouse_1F_EventScript_TVScreenFemale::
	msgbox PalletTown_PlayersHouse_1F_Text_MovieOnTVGirlOnBrickRoad
	return

PalletTown_PlayersHouse_1F_Text_AllBoysLeaveOakLookingForYou::
	.string "MOM: …Right.\n"
	.string "All boys leave home someday.\l"
	.string "It said so on TV.\p"
	.string "Oh, yes. PROF. OAK, next door, was\n"
	.string "looking for you.$"

PalletTown_PlayersHouse_1F_Text_AllGirlsLeaveOakLookingForYou::
	.string "MOM: …Right.\n"
	.string "All girls dream of traveling.\l"
	.string "It said so on TV.\p"
	.string "Oh, yes. PROF. OAK, next door, was\n"
	.string "looking for you.$"

PalletTown_PlayersHouse_1F_Text_YouShouldTakeQuickRest::
	.string "MOM: {PLAYER}!\n"
	.string "You should take a quick rest.$"

PalletTown_PlayersHouse_1F_Text_LookingGreatTakeCare::
	.string "MOM: Oh, good! You and your\n"
	.string "POKéMON are looking great.\l"
	.string "Take care now!$"

PalletTown_PlayersHouse_1F_Text_MovieOnTVFourBoysOnRailroad::
	.string "There's a movie on TV.\n"
	.string "Four boys are walking on railroad\l"
	.string "tracks.\p"
	.string "…I better go, too.$"

PalletTown_PlayersHouse_1F_Text_MovieOnTVGirlOnBrickRoad::
	.string "There's a movie on TV.\n"
	.string "A girl with her hair in pigtails is\l"
	.string "walking up a brick road.\p"
	.string "…I better go, too.$"

PalletTown_PlayersHouse_1F_Text_OopsWrongSide::
	.string "Oops, wrong side…$"

PalletTown_PlayersHouse_1F_Text_FryingPan::
	.string "WOMAN: STAY RIGHT THERE.\p"
	.string "I have a frying pan, and I am not\n"
	.string "afraid to use it.\p"
	.string "You came out of my son's room.\p"
	.string "My son is ten years old.\n"
	.string "You are not ten years old.\p"
	.string "…\p"
	.string "I have called PROF. OAK.\n"
	.string "And the police.\p"
	.string "OAK picked up first.\p"
	.string "He's waiting for you in his lab,\n"
	.string "next door. Go. Now.\p"
	.string "Before I change my mind.$"

PalletTown_PlayersHouse_1F_Text_MomStillHasPan::
	.string "She hasn't put the frying pan down.\p"
	.string "RED'S MOM: PROF. OAK. Next door.\n"
	.string "I'd hurry, if I were you.$"

PalletTown_PlayersHouse_1F_Text_MomWindowMan::
	.string "RED'S MOM: …You.\p"
	.string "The police are looking for a man\n"
	.string "who climbed out of my son's window.\p"
	.string "Tall. Tired-looking.\n"
	.string "Strangely familiar with the house.\p"
	.string "…Go and see PROF. OAK before\n"
	.string "I find the frying pan.$"
