PalletTown_PlayersHouse_2F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PalletTown_PlayersHouse_2F_OnTransition
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, PalletTown_PlayersHouse_2F_OnWarp
	map_script MAP_SCRIPT_ON_FRAME_TABLE, PalletTown_PlayersHouse_2F_OnFrame
	.byte 0

@ FireRed Isekai: waking up in Red's bed
PalletTown_PlayersHouse_2F_OnFrame::
	map_script_2 VAR_ISEKAI_INTRO, 0, PalletTown_PlayersHouse_2F_EventScript_WakeUp
	.2byte 0

PalletTown_PlayersHouse_2F_EventScript_WakeUp::
	lockall
	delay 30
	msgbox PalletTown_PlayersHouse_2F_Text_WakeUp
	closemessage
	setvar VAR_ISEKAI_INTRO, 1
	releaseall
	end

@ FireRed Isekai: in the games you can't use windows. Here you can.
PalletTown_PlayersHouse_2F_EventScript_Window::
	lockall
	goto_if_ge VAR_ISEKAI_INTRO, 2, PalletTown_PlayersHouse_2F_EventScript_WindowAgain
	msgbox PalletTown_PlayersHouse_2F_Text_WindowRule
	msgbox PalletTown_PlayersHouse_2F_Text_ClimbOut, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PalletTown_PlayersHouse_2F_EventScript_WindowNo
	closemessage
	setflag FLAG_ISEKAI_WINDOW_ESCAPE
	setflag FLAG_ISEKAI_LIMPING
	setvar VAR_ISEKAI_INTRO, 3
	playse SE_FALL
	fadescreen FADE_TO_BLACK
	waitse
	playse SE_WALL_HIT
	waitse
	warp MAP_PALLET_TOWN, 4, 6
	waitstate
	releaseall
	end

PalletTown_PlayersHouse_2F_EventScript_WindowNo::
	msgbox PalletTown_PlayersHouse_2F_Text_StairsSafer
	releaseall
	end

PalletTown_PlayersHouse_2F_EventScript_WindowAgain::
	msgbox PalletTown_PlayersHouse_2F_Text_WindowAgain
	releaseall
	end

PalletTown_PlayersHouse_2F_OnTransition::
	call_if_eq VAR_MAP_SCENE_PALLET_TOWN_PLAYERS_HOUSE_2F, 0, PalletTown_PlayersHouse_2F_EventScript_SetRespawn
	end

PalletTown_PlayersHouse_2F_EventScript_SetRespawn::
	setrespawn HEAL_LOCATION_PALLET_TOWN
	return

PalletTown_PlayersHouse_2F_OnWarp::
	map_script_2 VAR_MAP_SCENE_PALLET_TOWN_PLAYERS_HOUSE_2F, 0, PalletTown_PlayersHouse_2F_FirstWarpIn
	.2byte 0

PalletTown_PlayersHouse_2F_FirstWarpIn::
	turnobject LOCALID_PLAYER, DIR_WEST
	setvar VAR_MAP_SCENE_PALLET_TOWN_PLAYERS_HOUSE_2F, 1
	end

PalletTown_PlayersHouse_2F_EventScript_NES::
	msgbox PalletTown_PlayersHouse_2F_Text_PlayedWithNES, MSGBOX_SIGN
	end

PalletTown_PlayersHouse_2F_EventScript_Sign::
	msgbox PalletTown_PlayersHouse_2F_Text_PressLRForHelp, MSGBOX_SIGN
	end

PalletTown_PlayersHouse_2F_Text_PlayedWithNES::
	.string "It's an SNES.\p"
	.string "You know exactly which games are\n"
	.string "on it. You could list them in order.\p"
	.string "…Not the time. Probably.$"

PalletTown_PlayersHouse_2F_Text_WakeUp::
	.string "…\p"
	.string "…Ow.\p"
	.string "You're lying in a bed that is much\n"
	.string "too small for you.\p"
	.string "…Wait.\p"
	.string "The SNES. The bookshelf.\n"
	.string "The little green rug.\p"
	.string "This is RED's room.\p"
	.string "You've seen it a thousand times.\n"
	.string "Just never from the inside.\p"
	.string "…So where's RED?$"

PalletTown_PlayersHouse_2F_Text_WindowRule::
	.string "A window.\p"
	.string "In every POKéMON game, windows are\n"
	.string "just decoration. You've never once\l"
	.string "opened one.\p"
	.string "You try anyway.\p"
	.string "…\p"
	.string "It opens.$"

PalletTown_PlayersHouse_2F_Text_ClimbOut::
	.string "Climb out of the window?$"

PalletTown_PlayersHouse_2F_Text_StairsSafer::
	.string "The stairs are probably safer.\n"
	.string "Probably.$"

PalletTown_PlayersHouse_2F_Text_WindowAgain::
	.string "Your ankle remembers last time.\n"
	.string "Your ankle says no.$"

PalletTown_PlayersHouse_2F_Text_PressLRForHelp::
	.string "It's a posted notice…\p"
	.string "If you're confused, ask for HELP!\n"
	.string "Press the L or R Button!$"

