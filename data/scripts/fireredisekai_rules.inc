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

@ The same news, from inside another script (the wake-up). Returns.
IsekaiEventScript_BattleNewsInline::
	special IsekaiNextDeathMessage
	goto_if_eq VAR_RESULT, FALSE, IsekaiEventScript_BattleNewsInline_Injuries
	msgbox IsekaiText_DidntGetBackUp
	goto IsekaiEventScript_BattleNewsInline
IsekaiEventScript_BattleNewsInline_Injuries:
	special IsekaiNextInjuryMessage
	goto_if_eq VAR_RESULT, FALSE, IsekaiEventScript_BattleNewsInline_Done
	msgbox IsekaiText_BadlyInjured
	goto IsekaiEventScript_BattleNewsInline_Injuries
IsekaiEventScript_BattleNewsInline_Done:
	return

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
	.string "…\p"
	.string "{STR_VAR_1} is gone. It was taken to\n"
	.string "the GARDEN OF REST in VIRIDIAN.\p"
	.string "It is no longer in your team.$"

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
	call IsekaiEventScript_BattleNewsInline
	msgbox IsekaiText_WakeUp_Forfeit
	call IsekaiEventScript_AfterWhiteOutTreated
	return

IsekaiEventScript_WakeUp_WaitingRoom:
	call IsekaiEventScript_BattleNewsInline
	goto_if_set FLAG_BADGE01_GET, IsekaiEventScript_WakeUp_WaitingRoomBadge
	msgbox IsekaiText_WakeUp_WaitingRoom
	call IsekaiEventScript_AfterWhiteOutTreated
	return
IsekaiEventScript_WakeUp_WaitingRoomBadge:
	msgbox IsekaiText_WakeUp_WaitingRoomBadge
	call IsekaiEventScript_AfterWhiteOutTreated
	return

IsekaiText_WakeUp_WaitingRoomBadge:
	.string "You come to on a bench in the\n"
	.string "waiting room.\p"
	.string "NURSE: They only fainted. Whatever\n"
	.string "you ran into was too small to do\l"
	.string "real damage.\p"
	.string "Anything level 10 or up won't be.$"

@ v0.14.11: the blackout and the night in the ward are told on the black screen (C side).
@ He wakes up standing in front of the ward nurse (VAR_LAST_TALKED), and she does the talking.
IsekaiEventScript_WakeUp_Ward:
	textcolor NPC_TEXT_COLOR_FEMALE
	special IsekaiWardWakeLine @ v0.14.12: if the ward nurse is (or was) his partner, it's personal
	goto_if_eq VAR_RESULT, FALSE, IsekaiEventScript_WakeUp_Usual
	msgbox gStringVar4
	goto_if_set FLAG_ISEKAI_SEEN_WARD, IsekaiEventScript_WakeUp_LivesLeft
	setflag FLAG_ISEKAI_SEEN_WARD
	msgbox IsekaiText_WakeUp_Rules
	goto IsekaiEventScript_WakeUp_Ankle
IsekaiEventScript_WakeUp_LivesLeft:
	msgbox IsekaiText_WakeUp_LivesLeft
	goto IsekaiEventScript_WakeUp_Ankle
IsekaiEventScript_WakeUp_Usual:
	goto_if_set FLAG_ISEKAI_SEEN_WARD, IsekaiEventScript_WakeUp_Again
	setflag FLAG_ISEKAI_SEEN_WARD
	msgbox IsekaiText_WakeUp_FirstTime
	goto IsekaiEventScript_WakeUp_Ankle
IsekaiEventScript_WakeUp_Again:
	msgbox IsekaiText_WakeUp_Again
IsekaiEventScript_WakeUp_Ankle:
	call_if_set FLAG_ISEKAI_LIMPING, IsekaiEventScript_TreatAnkle
	call IsekaiEventScript_WakeUp_Deaths
	special IsekaiWardWakeRomance
	call_if_eq VAR_RESULT, 1, IsekaiEventScript_WakeUp_NursePerk
	call_if_eq VAR_RESULT, 2, IsekaiEventScript_WakeUp_NursePenalty
	special IsekaiPartyCount
	call_if_ne VAR_RESULT, 0, IsekaiEventScript_WakeUp_TeamTreated
	call IsekaiEventScript_CenterWard @ Badly Injured, healed ones coming back, or nobody left at all
	return

IsekaiEventScript_WakeUp_TeamTreated:
	msgbox IsekaiText_WakeUp_TeamTreated
	return

@ The nurse tells him who didn't make it. The Badly Injured are left to the ward offer.
IsekaiEventScript_WakeUp_Deaths:
	special IsekaiNextDeathMessage
	goto_if_eq VAR_RESULT, FALSE, IsekaiEventScript_WakeUp_DrainInjuries
	msgbox IsekaiText_WakeUp_Died
	goto IsekaiEventScript_WakeUp_Deaths
IsekaiEventScript_WakeUp_DrainInjuries:
	special IsekaiNextInjuryMessage
	goto_if_eq VAR_RESULT, TRUE, IsekaiEventScript_WakeUp_DrainInjuries
	return

IsekaiText_WakeUp_Died:
	.string "NURSE: …There's something else.\n"
	.string "Your {STR_VAR_1}…\p"
	.string "It didn't make it. I'm so sorry.\p"
	.string "It was taken to the GARDEN OF REST\n"
	.string "in VIRIDIAN. It's no longer in your\l"
	.string "team.$"

IsekaiText_WakeUp_TeamTreated:
	.string "NURSE: The counter treated your\n"
	.string "POKéMON while you were out.$"

@ Before the first Center: Red's mum finds him.
IsekaiEventScript_WakeUpHome::
	call IsekaiEventScript_BattleNewsInline
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

IsekaiText_WakeUp_FirstTime:
	.string "NURSE: Easy. You're up. You're all\n"
	.string "right.\p"
	.string "People get hurt out there too.\n"
	.string "We just don't put it on the posters.\p"
	.string "You can come through that door\n"
	.string "{STR_VAR_1} more times.\p"
	.string "After that, you won't walk back out\n"
	.string "of it. I mean that kindly.$"

IsekaiText_WakeUp_Rules:
	.string "NURSE: And listen. You can come\n"
	.string "through that door {STR_VAR_1} more times.\p"
	.string "After that, you won't walk back out\n"
	.string "of it. So don't.$"

IsekaiText_WakeUp_LivesLeft:
	.string "(Lives left: {STR_VAR_1})$"

IsekaiText_WakeUp_Again:
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
	setvar VAR_0x8008, 0 @ set to 1 if the ward had anything to say
	call_if_set FLAG_ISEKAI_LIMPING, IsekaiEventScript_TreatAnkle
	special IsekaiCollectHealedMons
	goto_if_eq VAR_RESULT, 0, IsekaiEventScript_CenterWard_StillResting
	setvar VAR_0x8008, 1
	playfanfare MUS_HEAL
	call_if_eq VAR_RESULT, 1, IsekaiEventScript_CenterWard_HealedOne
	call_if_gt VAR_RESULT, 1, IsekaiEventScript_CenterWard_HealedMany
	waitfanfare
IsekaiEventScript_CenterWard_StillResting:
	call_if_eq VAR_0x8004, TRUE, IsekaiEventScript_CenterWard_MankeyLeaves
	special IsekaiWardStatus
	call_if_eq VAR_RESULT, 1, IsekaiEventScript_CenterWard_ShowResting
	call_if_gt VAR_RESULT, 1, IsekaiEventScript_CenterWard_ShowRestingMany
	special IsekaiPartyCount
	goto_if_eq VAR_RESULT, 0, IsekaiEventScript_CenterWard_NoTeam
	special IsekaiCountInjuredInParty
	goto_if_eq VAR_RESULT, 0, IsekaiEventScript_CenterWard_End
	copyvar VAR_0x8009, VAR_RESULT @ v0.14.12: how many, so she says "it" or "them"
	setvar VAR_0x8008, 1
	goto_if_gt VAR_0x8009, 1, IsekaiEventScript_CenterWard_OfferMany
	msgbox IsekaiText_WardOffer, MSGBOX_YESNO
IsekaiEventScript_CenterWard_Offered:
	goto_if_eq VAR_RESULT, NO, IsekaiEventScript_CenterWard_Declined
	special IsekaiWardQuote
	goto_if_eq VAR_RESULT, 2, IsekaiEventScript_CenterWard_CantPay
	goto_if_eq VAR_RESULT, 0, IsekaiEventScript_CenterWard_Paid
	showmoneybox 18, 0
	special IsekaiWardQuote @ again: drawing the money box reuses STR_VAR_1
	msgbox IsekaiText_WardFee, MSGBOX_YESNO
	hidemoneybox
	goto_if_eq VAR_RESULT, NO, IsekaiEventScript_CenterWard_Declined
	special IsekaiPayWard
	playse SE_SHOP
IsekaiEventScript_CenterWard_Paid:
	special IsekaiWouldLeaveEmptyParty
	goto_if_eq VAR_RESULT, TRUE, IsekaiEventScript_CenterWard_LastOne
	special IsekaiLeaveInjuredAtCenter
	call IsekaiEventScript_CenterWard_SayTaken
IsekaiEventScript_CenterWard_End:
	return

@ v0.14.11: the whole team is gone. Never leave him with nothing to send out.
IsekaiEventScript_CenterWard_NoTeam:
	setvar VAR_0x8008, 1
	special IsekaiHasMonsInPC
	goto_if_eq VAR_RESULT, TRUE, IsekaiEventScript_CenterWard_NoTeamPC
	msgbox IsekaiText_NoTeamMankey, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IsekaiEventScript_CenterWard_NoTeamDeclined
	special IsekaiGiveCenterLoaner
	playfanfare MUS_LEVEL_UP
	msgbox IsekaiText_GotMankey
	waitfanfare
	msgbox IsekaiText_NoTeamMankeyTerms
	return

IsekaiEventScript_CenterWard_NoTeamPC:
	msgbox IsekaiText_NoTeamPC
	return

IsekaiEventScript_CenterWard_NoTeamDeclined:
	msgbox IsekaiText_NoTeamDeclined
	return

IsekaiText_NoTeamMankey:
	.string "You don't have a single POKéMON with\n"
	.string "you. You can't go out like that.\p"
	.string "We do have a MANKEY nobody's come\n"
	.string "back for. It's… grumpy.\p"
	.string "Take it until you've caught one of\n"
	.string "your own?$"

IsekaiText_NoTeamMankeyTerms:
	.string "Bring it back to any CENTER once\n"
	.string "you've caught a POKéMON of your own.\p"
	.string "It'll find its own way home from\n"
	.string "there. It always does.$"

IsekaiText_NoTeamPC:
	.string "You don't have a single POKéMON with\n"
	.string "you. You can't go out like that.\p"
	.string "There are some in your PC box.\n"
	.string "Take one out before you leave.\l"
	.string "Please.$"

IsekaiText_NoTeamDeclined:
	.string "Then stay out of the tall grass.\n"
	.string "Please. Ask me again any time.$"

IsekaiEventScript_CenterWard_CantPay:
	msgbox IsekaiText_WardCantPay
	return

IsekaiEventScript_CenterWard_ShowResting:
	setvar VAR_0x8008, 1
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
	call IsekaiEventScript_CenterWard_SayTaken
	return

IsekaiEventScript_CenterWard_OfferMany:
	msgbox IsekaiText_WardOfferMany, MSGBOX_YESNO
	goto IsekaiEventScript_CenterWard_Offered

IsekaiEventScript_CenterWard_SayTaken:
	goto_if_gt VAR_0x8009, 1, IsekaiEventScript_CenterWard_SayTakenMany
	msgbox IsekaiText_WardTaken
	return
IsekaiEventScript_CenterWard_SayTakenMany:
	msgbox IsekaiText_WardTakenMany
	return

IsekaiEventScript_CenterWard_HealedOne:
	msgbox IsekaiText_WardHealed
	return
IsekaiEventScript_CenterWard_HealedMany:
	msgbox IsekaiText_WardHealedMany
	return

IsekaiEventScript_CenterWard_ShowRestingMany:
	setvar VAR_0x8008, 1
	msgbox IsekaiText_WardRestingMany
	return

IsekaiText_WardOfferMany:
	.string "…{STR_VAR_3} of your POKéMON are\n"
	.string "Badly Injured. The machine can't\l"
	.string "fix that.\p"
	.string "They need real rest. Leave them with\n"
	.string "us while you fight {STR_VAR_2} TRAINERS?$"

IsekaiText_WardTakenMany:
	.string "We'll take good care of them.\p"
	.string "Come back to any POKéMON CENTER\n"
	.string "once you've fought {STR_VAR_2} TRAINERS.$"

IsekaiText_WardHealedMany:
	.string "Oh, and your {STR_VAR_3} POKéMON from\n"
	.string "the ward are all better! Here.$"

IsekaiText_WardRestingMany:
	.string "Your POKéMON in our ward are still\n"
	.string "resting. Fight {STR_VAR_2} more TRAINER(S)\l"
	.string "and come back to any CENTER.$"

IsekaiText_WardFee:
	.string "Ward care is ¥{STR_VAR_1}.\n"
	.string "Is that all right?$"

IsekaiText_WardCantPay:
	.string "Ward care is ¥{STR_VAR_1}.\n"
	.string "…You don't have it.\p"
	.string "I'm sorry. Please be careful out\n"
	.string "there.$"

IsekaiText_WardHealed:
	.string "Oh, and {STR_VAR_1} is all better!\n"
	.string "Here you go.$"

IsekaiText_MankeyLeaves:
	.string "The MANKEY goes back to the ward.\n"
	.string "It doesn't look back.$"

IsekaiText_WardResting:
	.string "Your POKéMON in our ward is still\n"
	.string "resting. Fight {STR_VAR_2} more TRAINER(S)\l"
	.string "and come back to any CENTER.$"

IsekaiText_WardOffer:
	.string "…{STR_VAR_1} is Badly Injured.\n"
	.string "The machine can't fix that.\p"
	.string "It needs real rest. Leave it with\n"
	.string "us while you fight {STR_VAR_2} TRAINERS?$"

IsekaiText_WardDeclined:
	.string "Then please be careful.\n"
	.string "Another knock could be the last.$"

IsekaiText_WardTaken:
	.string "We'll take good care of it.\p"
	.string "Come back to any POKéMON CENTER\n"
	.string "once you've fought {STR_VAR_2} TRAINERS.$"

IsekaiText_WardLastOne:
	.string "That would leave you with nobody.\p"
	.string "We do have a MANKEY nobody's come\n"
	.string "back for. It's… grumpy.\p"
	.string "Take it until your team is better?$"

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

@ ---------------------------------------------------------------------------
@ Route 1: the first step into tall grass.
@ ---------------------------------------------------------------------------
IsekaiEventScript_FirstGrass::
	lockall
	msgbox IsekaiText_FirstGrass
	releaseall
	end

IsekaiText_FirstGrass:
	.string "It's just one step.\p"
	.string "…It's never just one step.$"

@ ---------------------------------------------------------------------------
@ Oak's phone calls about the loan. VAR_0x8004: 1 first catch, 2 overdue.
@ ---------------------------------------------------------------------------
IsekaiEventScript_OakCall::
	lockall
	playse SE_POKENAV_CALL
	waitse
	playse SE_POKENAV_CALL
	msgbox IsekaiText_PhoneRings
	textcolor NPC_TEXT_COLOR_MALE
	goto_if_eq VAR_0x8004, 2, IsekaiEventScript_OakCall_Overdue
	goto_if_eq VAR_0x8004, 3, IsekaiEventScript_OakCall_TooLate
	msgbox IsekaiText_OakCall_Caught
	releaseall
	end
IsekaiEventScript_OakCall_Overdue:
	msgbox IsekaiText_OakCall_Overdue
	releaseall
	end

IsekaiEventScript_OakCall_TooLate:
	msgbox IsekaiText_OakCall_TooLate
	releaseall
	end

IsekaiText_OakCall_TooLate:
	.string "OAK: I hear you won a BADGE.\n"
	.string "With MY POKéMON, no doubt.\p"
	.string "Don't bother bringing it back now.\p"
	.string "And don't bother coming to the lab.$"

IsekaiText_PhoneRings:
	.string "The cheap phone OAK gave you\n"
	.string "is ringing.$"

IsekaiText_OakCall_Caught:
	.string "OAK: I hear you caught something.\n"
	.string "Good.\p"
	.string "Now bring my POKéMON back.\p"
	.string "…\p"
	.string "…You ARE bringing it back,\n"
	.string "aren't you?$"

IsekaiText_OakCall_Overdue:
	.string "OAK: That's it.\p"
	.string "I've called JENNY.$"

@ ---------------------------------------------------------------------------
@ While wanted: Centers and Marts gently remind him. VAR_0x8004 picks a line.
@ ---------------------------------------------------------------------------
IsekaiEventScript_WantedReminder::
	lockall
	switch VAR_0x8004
	case 0, IsekaiEventScript_Wanted_Poster
	case 1, IsekaiEventScript_Wanted_Clerk
	case 3, IsekaiEventScript_Wanted_Fence
	case 4, IsekaiEventScript_Wanted_Kid
	case 5, IsekaiEventScript_Wanted_Door
	msgbox IsekaiText_Wanted_Whisper
	releaseall
	end
IsekaiEventScript_Wanted_Poster:
	msgbox IsekaiText_Wanted_Poster
	releaseall
	end
IsekaiEventScript_Wanted_Clerk:
	msgbox IsekaiText_Wanted_Clerk
	releaseall
	end

IsekaiEventScript_Wanted_Fence:
	msgbox IsekaiText_Wanted_Fence
	releaseall
	end
IsekaiEventScript_Wanted_Kid:
	msgbox IsekaiText_Wanted_Kid
	releaseall
	end
IsekaiEventScript_Wanted_Door:
	msgbox IsekaiText_Wanted_Door
	releaseall
	end

IsekaiText_Wanted_Fence:
	.string "Someone has pinned a WANTED poster\n"
	.string "to a fence post.\p"
	.string "It's you. They got the tired eyes\n"
	.string "right.$"

IsekaiText_Wanted_Kid:
	.string "A kid points at you.\p"
	.string "“MUM! It's the POKéMON thief!”\p"
	.string "His mum pulls him inside.$"

IsekaiText_Wanted_Door:
	.string "Somewhere nearby, a door shuts.\n"
	.string "Then a lock clicks.$"

IsekaiText_Wanted_Poster:
	.string "A poster by the door:\p"
	.string "WANTED: tall, tired-looking man.\n"
	.string "Last seen with PROF. OAK's POKéMON.\p"
	.string "…The drawing is generous.$"

IsekaiText_Wanted_Clerk:
	.string "Someone behind the counter looks at\n"
	.string "you, then at the poster by the door,\l"
	.string "then back at you.$"

IsekaiText_Wanted_Whisper:
	.string "“…that thief from PALLET…”\p"
	.string "The whispering stops when you\n"
	.string "turn around.$"

@ The late-return fine, taken from battle winnings until it's paid.
IsekaiEventScript_FineFromWinnings::
	lockall
	msgbox IsekaiText_FineFromWinnings
	call_if_eq VAR_0x8004, TRUE, IsekaiEventScript_FinePaidOff
	call_if_eq VAR_0x8004, FALSE, IsekaiEventScript_FineStillOwed
	releaseall
	end

IsekaiEventScript_FinePaidOff:
	msgbox IsekaiText_FinePaidOff
	return

IsekaiEventScript_FineStillOwed:
	msgbox IsekaiText_FineStillOwed
	return

IsekaiText_FineFromWinnings:
	.string "¥{STR_VAR_1} of your winnings went\n"
	.string "straight to the police.$"

IsekaiText_FinePaidOff:
	.string "The fine is paid off.\n"
	.string "JENNY will be thrilled.$"

IsekaiText_FineStillOwed:
	.string "Still owed: ¥{STR_VAR_2}.$"

@ Oak's parcel, opened from the bag. Oak will notice.
IsekaiEventScript_OpenParcel::
	lockall
	goto_if_set FLAG_ISEKAI_PARCEL_OPENED, IsekaiEventScript_ParcelAlreadyOpen
	msgbox IsekaiText_OpenParcel, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IsekaiEventScript_ParcelDone
	setflag FLAG_ISEKAI_PARCEL_OPENED
	msgbox IsekaiText_ParcelInside
IsekaiEventScript_ParcelDone:
	releaseall
	end

IsekaiEventScript_ParcelAlreadyOpen:
	msgbox IsekaiText_ParcelAlreadyOpen
	releaseall
	end

IsekaiText_OpenParcel:
	.string "It's addressed to PROF. OAK.\n"
	.string "Open it anyway?$"

IsekaiText_ParcelInside:
	.string "{PLAYER} picks at the tape…\p"
	.string "Five POKé BALLS, packed in straw.\n"
	.string "That's all.\p"
	.string "He tapes it back up.\n"
	.string "It looks… mostly fine.$"

IsekaiText_ParcelAlreadyOpen:
	.string "The tape is a mess already.\n"
	.string "Best not to make it worse.$"

@ ---------------------------------------------------------------------------
@ FireRed Isekai: the POKé CARE PLAN lapses; SHARKEY's associate
@ ---------------------------------------------------------------------------
IsekaiEventScript_CarePlanLapsed::
	lockall
	msgbox IsekaiText_CarePlanLapsed
	releaseall
	end

IsekaiText_CarePlanLapsed:
	.string "The POKé CARE PLAN couldn't take\n"
	.string "its ¥{STR_VAR_1} payment.\p"
	.string "Your plan has ended.$"

IsekaiEventScript_SharkGoon::
	lockall
	playse SE_PIN
	msgbox IsekaiText_SharkGoon
	closemessage
	trainerbattle_no_intro TRAINER_ISEKAI_SHARK_GOON, IsekaiText_SharkGoonDefeat
	cleartrainerflag TRAINER_ISEKAI_SHARK_GOON
	msgbox IsekaiText_SharkGoonAfter
	releaseall
	end

IsekaiText_SharkGoon:
	.string "A man in a very nice suit steps\n"
	.string "out of the grass.\p"
	.string "TONY: SHARKEY says hi.\n"
	.string "And he wants his money.$"

IsekaiText_SharkGoonDefeat:
	.string "TONY: …Okay. Okay.$"

IsekaiText_SharkGoonAfter:
	.string "TONY: You win this time.\n"
	.string "But the interest doesn't stop.$"

@ ---------------------------------------------------------------------------
@ FireRed Isekai: SHARKEY part two. TONY's "interest", the warning call, the auction.
@ ---------------------------------------------------------------------------
IsekaiEventScript_TonyTakes::
	lockall
	special IsekaiTonyDone
	msgbox IsekaiText_TonyArrives
	special IsekaiSharkBuildList
	goto_if_eq VAR_RESULT, 0, IsekaiEventScript_TonyNothing
	goto_if_eq VAR_RESULT, 0xFF, IsekaiEventScript_TonyNothing
	msgbox IsekaiText_TonyPick
	dynmultistack 0, 0, TRUE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	copyvar VAR_0x8004, VAR_RESULT
	setvar VAR_0x8005, 2
	special IsekaiSharkTake
	msgbox IsekaiText_TonyTook
	releaseall
	end

IsekaiEventScript_TonyNothing:
	msgbox IsekaiText_TonyNothing
	releaseall
	end

IsekaiText_TonyArrives:
	.string "A man in a very nice suit is\n"
	.string "waiting for you.\p"
	.string "TONY: SHARKEY still wants his money.\n"
	.string "So. Interest.$"

IsekaiText_TonyPick:
	.string "TONY: Not the junk. One of the ones\n"
	.string "that actually fight.\p"
	.string "The rest? Worthless.\n"
	.string "You pick.$"

IsekaiText_TonyTook:
	.string "TONY took {STR_VAR_1}.\p"
	.string "TONY: Call it interest.\n"
	.string "SHARKEY gives it back for ¥{STR_VAR_2}.\p"
	.string "For now.$"

IsekaiText_TonyNothing:
	.string "TONY: …That's it?\n"
	.string "Worthless. All of it.$"

IsekaiEventScript_SharkCall::
	lockall
	playse SE_POKENAV_CALL
	waitse
	playse SE_POKENAV_CALL
	msgbox IsekaiText_SharkPhone
	goto_if_eq VAR_0x8004, 2, IsekaiEventScript_SharkCall_Sold
	msgbox IsekaiText_SharkWarning
	releaseall
	end
IsekaiEventScript_SharkCall_Sold:
	msgbox IsekaiText_SharkSold
	releaseall
	end

IsekaiText_SharkPhone:
	.string "The cheap phone is ringing.\n"
	.string "Unknown number.$"

IsekaiText_SharkWarning:
	.string "SHARKEY: How did I get this number?\n"
	.string "I have everyone's number.\p"
	.string "Your {STR_VAR_1}'s looking lonely.\p"
	.string "1,000 more steps.\n"
	.string "Then it goes to auction.$"

IsekaiText_SharkSold:
	.string "SHARKEY: Going once…\n"
	.string "going twice…\p"
	.string "Your {STR_VAR_1} found a new home.\n"
	.string "Nothing personal.$"

@ ---------------------------------------------------------------------------
@ FireRed Isekai: the phone call after a death. VAR_0x8004: 0 Oak (first time, explains),
@ 1 Oak (later), 2 Oak (his own POKéMON), 3 the ward nurse (Oak's POKéMON, Oak won't call),
@ 4 the ward nurse (first time, Oak won't call). STR_VAR_1 = its name.
@ ---------------------------------------------------------------------------
IsekaiEventScript_DeathCall::
	lockall
	playse SE_POKENAV_CALL
	waitse
	playse SE_POKENAV_CALL
	msgbox IsekaiText_PhoneRings
	switch VAR_0x8004
	case 1, IsekaiEventScript_DeathCall_OakLater
	case 2, IsekaiEventScript_DeathCall_OakLoaner
	case 3, IsekaiEventScript_DeathCall_NurseLoaner
	case 4, IsekaiEventScript_DeathCall_Nurse
	textcolor NPC_TEXT_COLOR_MALE
	msgbox IsekaiText_DeathCall_OakFirst
	releaseall
	end
IsekaiEventScript_DeathCall_OakLater:
	textcolor NPC_TEXT_COLOR_MALE
	msgbox IsekaiText_DeathCall_OakLater
	releaseall
	end
IsekaiEventScript_DeathCall_OakLoaner:
	textcolor NPC_TEXT_COLOR_MALE
	msgbox IsekaiText_DeathCall_OakLoaner
	releaseall
	end
IsekaiEventScript_DeathCall_NurseLoaner:
	textcolor NPC_TEXT_COLOR_FEMALE
	msgbox IsekaiText_DeathCall_NurseLoaner
	releaseall
	end
IsekaiEventScript_DeathCall_Nurse:
	textcolor NPC_TEXT_COLOR_FEMALE
	msgbox IsekaiText_DeathCall_Nurse
	releaseall
	end

IsekaiText_DeathCall_OakFirst:
	.string "OAK: …I heard about {STR_VAR_1}.\p"
	.string "I'm sorry. Truly.\p"
	.string "Out there, they don't come back.\n"
	.string "Not ever.\p"
	.string "The CENTER takes them to the GARDEN\n"
	.string "OF REST in VIRIDIAN. Their names go\l"
	.string "on the stone.\p"
	.string "Visit, if you need to.\p"
	.string "…Be careful out there. Please.$"

IsekaiText_DeathCall_OakLater:
	.string "OAK: I heard about {STR_VAR_1}.\p"
	.string "…I'm sorry.\n"
	.string "Look after the others.$"

IsekaiText_DeathCall_OakLoaner:
	.string "OAK: …{STR_VAR_1}.\p"
	.string "…\p"
	.string "I raised it from an EGG, you know.\n"
	.string "Always so worried. About everything.\p"
	.string "It's not your fault. Out there,\n"
	.string "it's nobody's fault.\p"
	.string "They'll put its name on the stone\n"
	.string "in VIRIDIAN. I'll… go and see it.$"

IsekaiText_DeathCall_NurseLoaner:
	.string "Hello? This is the ward at the\n"
	.string "VIRIDIAN POKéMON CENTER.\p"
	.string "I'm calling about {STR_VAR_1}.\n"
	.string "I'm so sorry.\p"
	.string "Its name is on the stone in our\n"
	.string "GARDEN OF REST now.\p"
	.string "…PROF. OAK has been told. It was his.\n"
	.string "He asked me to call you.\p"
	.string "He didn't want to.$"

IsekaiText_DeathCall_Nurse:
	.string "Hello? This is the ward at the\n"
	.string "VIRIDIAN POKéMON CENTER.\p"
	.string "I'm calling about {STR_VAR_1}.\n"
	.string "I'm so sorry.\p"
	.string "Out there, when a POKéMON goes down\n"
	.string "for good, it doesn't get back up.\p"
	.string "We take them to the GARDEN OF REST,\n"
	.string "beside the CENTER. Their names go on\l"
	.string "the stone.\p"
	.string "…PROF. OAK asked me to call you.\n"
	.string "He didn't want to.$"

@ v0.14.8: after a whiteout, the counter nurse has already treated the team.
IsekaiEventScript_AfterWhiteOutTreated::
	special IsekaiPartyCount
	goto_if_eq VAR_RESULT, 0, IsekaiEventScript_CenterWard
	msgbox IsekaiText_TreatedWhileOut
	call EventScript_PkmnCenterNurse_TakeAndHealPkmn_Frlg
	call_if_set FLAG_ISEKAI_LIMPING, IsekaiEventScript_TreatAnkle
	special IsekaiCountInjuredInParty
	goto_if_eq VAR_RESULT, 0, IsekaiEventScript_AfterWhiteOutTreated_End
	msgbox IsekaiText_SeeTheWardNurse
IsekaiEventScript_AfterWhiteOutTreated_End:
	return

IsekaiText_TreatedWhileOut:
	.string "NURSE: We treated your POKéMON while\n"
	.string "you were out.$"

IsekaiText_SeeTheWardNurse:
	.string "The Badly Injured ones need more than\n"
	.string "the machine, though.\p"
	.string "Talk to our ward nurse when you're\n"
	.string "ready. She's just over there.$"
