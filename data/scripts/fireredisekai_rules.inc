@ FireRed Isekai: the core rules (v0.7-v0.10). C side: src/fireredisekai_rules.c

@ ---------------------------------------------------------------------------
@ A reset during a battle: the same battle starts again, harder.
@ ---------------------------------------------------------------------------
IsekaiEventScript_ResumeBattle::
	lockall
	special IsekaiCheckResumeValid
	goto_if_eq VAR_RESULT, FALSE, IsekaiEventScript_ResumeBattle_Release
	special IsekaiPrepareResume
	goto_if_eq VAR_0x8005, FALSE, IsekaiEventScript_ResumeBattle_NoHint
	msgbox IsekaiText_TryToReload
IsekaiEventScript_ResumeBattle_NoHint:
	goto_if_eq VAR_0x8004, 2, IsekaiEventScript_ResumeTrainer
	msgbox IsekaiText_WildOffended
	special IsekaiCreateResumeWildMon
	dowildbattle
	releaseall
	end

IsekaiEventScript_ResumeTrainer:
	goto_if_eq VAR_0x8006, TRUE, IsekaiEventScript_ResumeRival
	msgbox IsekaiText_SawYouFlicker
	special IsekaiResumeTrainerBattle
	end

IsekaiEventScript_ResumeRival:
	msgbox IsekaiText_RivalSawYouFlicker
	special IsekaiResumeTrainerBattle
	end

IsekaiEventScript_ResumeBattle_Release:
	releaseall
	end

IsekaiText_TryToReload:
	.string "You try to reload.\n"
	.string "Nothing happens.\p"
	.string "This isn't a game.$"

IsekaiText_WildOffended:
	.string "The wild {STR_VAR_1} is still here.\n"
	.string "It looks personally offended.$"

IsekaiText_RivalSawYouFlicker:
	.string "{RIVAL}: Whoa. Did you just…\n"
	.string "flicker?\p"
	.string "Gramps! He flickered!\n"
	.string "Doesn't matter. Round two!$"

IsekaiText_SawYouFlicker:
	.string "“Going somewhere?\n"
	.string "I saw you flicker.”$"

@ ---------------------------------------------------------------------------
@ After a battle: deaths first (quiet, never a joke), then new injuries.
@ ---------------------------------------------------------------------------
IsekaiEventScript_AfterBattleNews::
	lockall
	special IsekaiNextProtectedNote
	call_if_eq VAR_RESULT, 1, IsekaiEventScript_ProtectedNote_NoBadge
	call_if_eq VAR_RESULT, 2, IsekaiEventScript_ProtectedNote_Badge
IsekaiEventScript_AfterBattleNews_Deaths:
	special IsekaiNextDeathMessage
	goto_if_eq VAR_RESULT, FALSE, IsekaiEventScript_AfterBattleNews_Injuries
	msgbox IsekaiText_DidntGetBackUp
	goto IsekaiEventScript_AfterBattleNews_Deaths
IsekaiEventScript_AfterBattleNews_Injuries:
	special IsekaiNextInjuryMessage
	goto_if_eq VAR_RESULT, FALSE, IsekaiEventScript_AfterBattleNews_Done
	msgbox IsekaiText_BadlyInjured
	goto IsekaiEventScript_AfterBattleNews_Injuries
IsekaiEventScript_AfterBattleNews_Done:
	releaseall
	end

IsekaiEventScript_ProtectedNote_NoBadge:
	msgbox IsekaiText_ProtectedNote_NoBadge
	return

IsekaiEventScript_ProtectedNote_Badge:
	msgbox IsekaiText_ProtectedNote_Badge
	return

IsekaiText_ProtectedNote_NoBadge:
	.string "{STR_VAR_1} only fainted.\n"
	.string "No lasting harm.\p"
	.string "Young POKéMON bounce back. Anything\n"
	.string "under level 10 does, for now.\p"
	.string "That stops once you've earned your\n"
	.string "first BADGE.$"

IsekaiText_ProtectedNote_Badge:
	.string "{STR_VAR_1} only fainted.\n"
	.string "No lasting harm.\p"
	.string "Whatever knocked it down was too\n"
	.string "small to do real damage.\l"
	.string "Anything level 10 or up will.$"

IsekaiText_DidntGetBackUp:
	.string "{STR_VAR_1} didn't get back up.\p"
	.string "…$"

IsekaiText_BadlyInjured:
	.string "{STR_VAR_1} gets back up. Slowly.\n"
	.string "It's Badly Injured.\p"
	.string "One more knock like that could be its\n"
	.string "last. A POKéMON CENTER can help.$"

@ ---------------------------------------------------------------------------
@ Waking up after a whiteout (replaces FireRed's nurse scene).
@ ---------------------------------------------------------------------------
IsekaiEventScript_WakeUp::
	special IsekaiGetWakeUpScene
	switch VAR_RESULT
	case 0, IsekaiEventScript_WakeUp_WaitingRoom
	case 1, IsekaiEventScript_WakeUp_Ward
	case 2, IsekaiEventScript_GameOver
	msgbox IsekaiText_WakeUp_Forfeit
	return

IsekaiEventScript_WakeUp_WaitingRoom:
	goto_if_set FLAG_BADGE01_GET, IsekaiEventScript_WakeUp_WaitingRoomBadge
	msgbox IsekaiText_WakeUp_WaitingRoom
	return
IsekaiEventScript_WakeUp_WaitingRoomBadge:
	msgbox IsekaiText_WakeUp_WaitingRoomBadge
	return

IsekaiText_WakeUp_WaitingRoomBadge:
	.string "You come to on a bench in the\n"
	.string "waiting room.\p"
	.string "NURSE: They only fainted. Whatever\n"
	.string "you ran into was too small to do\l"
	.string "real damage.\p"
	.string "Anything level 10 or up won't be.$"

IsekaiEventScript_WakeUp_Ward:
	fadescreen FADE_TO_BLACK
	switch VAR_0x8004
	case 2, IsekaiEventScript_WakeUp_HowWild  @ ISEKAI_DEFEAT_WILD
	case 4, IsekaiEventScript_WakeUp_HowRocket  @ ISEKAI_DEFEAT_ROCKET
	case 5, IsekaiEventScript_WakeUp_HowGrass   @ ISEKAI_DEFEAT_GRASS
	msgbox IsekaiText_WakeUp_HowTrainer
	goto IsekaiEventScript_WakeUp_InBed
IsekaiEventScript_WakeUp_HowWild:
	msgbox IsekaiText_WakeUp_HowWild
	goto IsekaiEventScript_WakeUp_InBed
IsekaiEventScript_WakeUp_HowGrass:
	msgbox IsekaiText_WakeUp_HowGrass
	goto IsekaiEventScript_WakeUp_InBed
IsekaiEventScript_WakeUp_HowRocket:
	msgbox IsekaiText_WakeUp_HowRocket
IsekaiEventScript_WakeUp_InBed:
	textcolor NPC_TEXT_COLOR_FEMALE
	goto_if_set FLAG_ISEKAI_SEEN_WARD, IsekaiEventScript_WakeUp_Again
	setflag FLAG_ISEKAI_SEEN_WARD
	msgbox IsekaiText_WakeUp_FirstTime
	goto IsekaiEventScript_WakeUp_Ankle
IsekaiEventScript_WakeUp_Again:
	msgbox IsekaiText_WakeUp_Again
IsekaiEventScript_WakeUp_Ankle:
	call_if_set FLAG_ISEKAI_LIMPING, IsekaiEventScript_TreatAnkle
	fadescreen FADE_FROM_BLACK
	return

@ Before the first Center: Red's mum finds him.
IsekaiEventScript_WakeUpHome::
	special IsekaiGetWakeUpScene
	goto_if_eq VAR_RESULT, 2, IsekaiEventScript_GameOver
	goto_if_ne VAR_RESULT, 1, IsekaiEventScript_WakeUpHome_End
	fadescreen FADE_TO_BLACK
	msgbox IsekaiText_WakeUpHome
	fadescreen FADE_FROM_BLACK
IsekaiEventScript_WakeUpHome_End:
	return

IsekaiText_WakeUpHome:
	.string "You wake up in a bed.\n"
	.string "…Red's bed. Again.\p"
	.string "WOMAN: Someone carried you to the\n"
	.string "door. I'm not calling the police.\l"
	.string "This time.\p"
	.string "(People get hurt out there too.\n"
	.string "Lives left: {STR_VAR_1})$"

@ The window escape's sprained ankle: any Center nurse straps it up.
IsekaiEventScript_TreatAnkle::
	msgbox IsekaiText_TreatAnkle
	clearflag FLAG_ISEKAI_LIMPING
	return

IsekaiText_WakeUp_Forfeit:
	.string "You walked away.\n"
	.string "Your POKéMON are fine.\p"
	.string "Your wallet isn't.$"

IsekaiText_WakeUp_WaitingRoom:
	.string "You come to on a bench in the\n"
	.string "waiting room.\p"
	.string "NURSE: They only fainted. Nobody out\n"
	.string "there was really trying to hurt you.\p"
	.string "Young POKéMON bounce back. Under\n"
	.string "level 10, they always do.\p"
	.string "That stops once you've earned your\n"
	.string "first BADGE.$"

IsekaiText_WakeUp_HowWild:
	.string "With your team down, nothing stood\n"
	.string "between you and the wild {STR_VAR_2}.\p"
	.string "It turned its attention to you.$"

IsekaiText_WakeUp_HowTrainer:
	.string "Your last POKéMON was about to take\n"
	.string "the final hit.\p"
	.string "You don't remember deciding to move.$"

IsekaiText_WakeUp_HowGrass:
	.string "The last thing you remember is the\n"
	.string "grass moving the wrong way.$"

IsekaiText_WakeUp_HowRocket:
	.string "Your POKéMON were already down.\p"
	.string "The ROCKETS kept going anyway.\n"
	.string "That was the point.$"

IsekaiText_WakeUp_FirstTime:
	.string "You wake up in a bed. White sheets.\n"
	.string "A sign on the door: HUMAN WARD.\p"
	.string "NURSE: Easy. People get hurt out\n"
	.string "there too. We just don't put it on\l"
	.string "the posters.\p"
	.string "You can come through that door\n"
	.string "{STR_VAR_1} more times.\p"
	.string "After that, you won't walk back out\n"
	.string "of it. I mean that kindly.$"

IsekaiText_WakeUp_Again:
	.string "You wake up in a bed. Again.\p"
	.string "NURSE: Welcome back. Try not to make\n"
	.string "this a habit.\p"
	.string "(Lives left: {STR_VAR_1})$"

IsekaiText_TreatAnkle:
	.string "NURSE: And let me see that ankle.\p"
	.string "…Sprained. Not broken.\n"
	.string "Who climbs out of a window at your age?\p"
	.string "(Your ankle is strapped up.\n"
	.string "You can run again.)$"

@ ---------------------------------------------------------------------------
@ No lives left.
@ ---------------------------------------------------------------------------
IsekaiEventScript_GameOver::
	lockall
	fadescreen FADE_TO_BLACK
	textcolor NPC_TEXT_COLOR_NEUTRAL
	msgbox IsekaiText_GameOver
	special IsekaiGameOver
	waitstate
	end

IsekaiText_GameOver:
	.string "You wake up in a bed.\p"
	.string "The NURSE is sitting next to it.\n"
	.string "She doesn't say anything this time.\p"
	.string "…\p"
	.string "Somewhere far away, a truck horn.\p"
	.string "Your journey ends here.\p"
	.string "This story is over.\n"
	.string "Its save has been erased.$"

@ ---------------------------------------------------------------------------
@ At the Center counter, after healing: the ward for Badly Injured Pokémon.
@ ---------------------------------------------------------------------------
IsekaiEventScript_CenterWard::
	call_if_set FLAG_ISEKAI_LIMPING, IsekaiEventScript_TreatAnkle
	special IsekaiCollectHealedMons
	goto_if_eq VAR_RESULT, 0, IsekaiEventScript_CenterWard_StillResting
	playfanfare MUS_HEAL
	msgbox IsekaiText_WardHealed
	waitfanfare
	call_if_eq VAR_0x8004, TRUE, IsekaiEventScript_CenterWard_MankeyLeaves
IsekaiEventScript_CenterWard_StillResting:
	special IsekaiWardStatus
	call_if_ne VAR_RESULT, 0, IsekaiEventScript_CenterWard_ShowResting
	special IsekaiCountInjuredInParty
	goto_if_eq VAR_RESULT, 0, IsekaiEventScript_CenterWard_End
	msgbox IsekaiText_WardOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IsekaiEventScript_CenterWard_Declined
	special IsekaiWouldLeaveEmptyParty
	goto_if_eq VAR_RESULT, TRUE, IsekaiEventScript_CenterWard_LastOne
	special IsekaiLeaveInjuredAtCenter
	msgbox IsekaiText_WardTaken
IsekaiEventScript_CenterWard_End:
	return

IsekaiEventScript_CenterWard_ShowResting:
	msgbox IsekaiText_WardResting
	return

IsekaiEventScript_CenterWard_MankeyLeaves:
	msgbox IsekaiText_MankeyLeaves
	return

IsekaiEventScript_CenterWard_Declined:
	msgbox IsekaiText_WardDeclined
	return

IsekaiEventScript_CenterWard_LastOne:
	msgbox IsekaiText_WardLastOne, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IsekaiEventScript_CenterWard_Declined
	special IsekaiLeaveInjuredAtCenter
	special IsekaiGiveCenterLoaner
	playfanfare MUS_LEVEL_UP
	msgbox IsekaiText_GotMankey
	waitfanfare
	msgbox IsekaiText_WardTaken
	return

IsekaiText_WardHealed:
	.string "Oh, and {STR_VAR_1} is all better!\n"
	.string "Here you go.$"

IsekaiText_MankeyLeaves:
	.string "The MANKEY goes back to the ward.\n"
	.string "It doesn't look back.$"

IsekaiText_WardResting:
	.string "Your POKéMON in our ward is still\n"
	.string "resting. Fight {STR_VAR_2} more battle(s)\l"
	.string "and come back to any CENTER.$"

IsekaiText_WardOffer:
	.string "…{STR_VAR_1} is Badly Injured.\n"
	.string "The machine can't fix that.\p"
	.string "It needs real rest. Leave it with\n"
	.string "us while you fight {STR_VAR_2} battles?$"

IsekaiText_WardDeclined:
	.string "Then please be careful.\n"
	.string "Another knock could be the last.$"

IsekaiText_WardTaken:
	.string "We'll take good care of it.\p"
	.string "Come back to any POKéMON CENTER\n"
	.string "once you've fought {STR_VAR_2} battles.$"

IsekaiText_WardLastOne:
	.string "That would leave you with nobody.\p"
	.string "We do have a MANKEY nobody's come\n"
	.string "back for. It's… grumpy.\p"
	.string "Take it until yours is better?$"

IsekaiText_GotMankey:
	.string "{PLAYER} received MANKEY.\n"
	.string "It glares at you.$"

@ ---------------------------------------------------------------------------
@ The graveyard behind the Viridian Center.
@ ---------------------------------------------------------------------------
ViridianCity_EventScript_IsekaiGardenOfRest::
	lockall
	msgbox IsekaiText_GardenOfRest
	special IsekaiBufferGraveNames
	goto_if_eq VAR_RESULT, 0, IsekaiEventScript_Garden_Empty
	msgbox IsekaiText_GardenNames
	special IsekaiBufferGraveNames @ again: printing a message reuses gStringVar4
	msgbox gStringVar4
	releaseall
	end
IsekaiEventScript_Garden_Empty:
	msgbox IsekaiText_GardenEmpty
	releaseall
	end

IsekaiText_GardenOfRest:
	.string "VIRIDIAN POKéMON GARDEN OF REST\n"
	.string "Please keep your voice down.$"

IsekaiText_GardenEmpty:
	.string "The plot is empty.\n"
	.string "Keep it that way.$"

IsekaiText_GardenNames:
	.string "Names have been carved into the\n"
	.string "stone.$"

@ ---------------------------------------------------------------------------
@ Walking into tall grass with no POKéMON left.
@ ---------------------------------------------------------------------------
IsekaiEventScript_AttackedInGrass::
	lockall
	msgbox IsekaiText_AttackedInGrass
	special IsekaiAttackedWithNoTeam
	waitstate
	end

IsekaiText_AttackedInGrass:
	.string "Something in the grass lunges at you.\p"
	.string "There's no POKéMON left to stand\n"
	.string "between you and it.$"

@ ---------------------------------------------------------------------------
@ Autosave with a small "Saving..." box (entering important buildings, new zones).
@ ---------------------------------------------------------------------------
IsekaiEventScript_Saving::
	lockall
	special IsekaiShowSavingBox
	delay 2
	special IsekaiDoFieldAutosave
	releaseall
	end
