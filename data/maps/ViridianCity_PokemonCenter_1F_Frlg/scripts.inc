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
