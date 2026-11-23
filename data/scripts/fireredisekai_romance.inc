@ ===========================================================================
@ FireRed Isekai (v0.13): the romance system. Shared flows; VAR_0x8004 = partner.
@ A partner's own script handles her quest, then calls IsekaiEventScript_RomHub.
@ ===========================================================================

.macro romline line:req
	setvar VAR_0x8005, \line
	special IsekaiRomBufferLine
	msgbox gStringVar4
.endm

@ Everything after her quest. Returns.
IsekaiEventScript_RomHub::
	special IsekaiRomGetState
	switch VAR_RESULT
	case ROM_STATE_INTEREST, IsekaiEventScript_RomOffer
	case ROM_STATE_FRIEND, IsekaiEventScript_RomHub_Friend
	case ROM_STATE_ON_HOLD, IsekaiEventScript_RomHub_OnHold
	case ROM_STATE_DATING, IsekaiEventScript_RomDating
	case ROM_STATE_EX_CLEAN, IsekaiEventScript_RomHub_Ex
	case ROM_STATE_EX_SORRY, IsekaiEventScript_RomHub_Ex
	case ROM_STATE_EX_MESSY, IsekaiEventScript_RomHub_ExMessy
	return

IsekaiEventScript_RomHub_Friend:
	msgbox IsekaiText_RomBringItUp, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, IsekaiEventScript_RomOffer
	return

IsekaiEventScript_RomHub_OnHold:
	special IsekaiRomOfferCheck
	goto_if_eq VAR_RESULT, 0, IsekaiEventScript_RomOffer
	romline ROMLINE_HOLD
	return

IsekaiEventScript_RomHub_Ex:
	romline ROMLINE_EX
	return

IsekaiEventScript_RomHub_ExMessy:
	romline ROMLINE_EX_MESSY
	msgbox IsekaiText_RomApologise, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, IsekaiEventScript_RomApology
	return

@ She's interested: accept or decline (and "Are you seeing anyone?").
IsekaiEventScript_RomOffer::
	special IsekaiRomOfferCheck
	copyvar VAR_0x8007, VAR_RESULT
	goto_if_eq VAR_0x8007, 2, IsekaiEventScript_RomOffer_Heard
	romline ROMLINE_INTEREST
	msgbox IsekaiText_RomGoOut, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IsekaiEventScript_RomOffer_Decline
	goto_if_eq VAR_0x8007, 1, IsekaiEventScript_RomOffer_Ask
	setvar VAR_0x8005, FALSE
	special IsekaiRomAccept
	romline ROMLINE_ACCEPT
	call IsekaiEventScript_RomFirstTimeHint
	return

IsekaiEventScript_RomOffer_Ask:
	romline ROMLINE_ASK
	dynmultichoice 0, 0, TRUE, 2, 0, DYN_MULTICHOICE_CB_NONE, IsekaiText_RomOptHonest, IsekaiText_RomOptLie
	goto_if_eq VAR_RESULT, 1, IsekaiEventScript_RomOffer_Lie
	special IsekaiRomHold
	romline ROMLINE_HOLD
	return

IsekaiEventScript_RomOffer_Lie:
	msgbox IsekaiText_RomYouLie
	setvar VAR_0x8005, TRUE
	special IsekaiRomAccept
	romline ROMLINE_ACCEPT
	return

IsekaiEventScript_RomOffer_Decline:
	special IsekaiRomDecline
	romline ROMLINE_DECLINE
	return

IsekaiEventScript_RomOffer_Heard:
	romline ROMLINE_HEARD
	return

IsekaiEventScript_RomFirstTimeHint:
	goto_if_set FLAG_ISEKAI_ROM_CALLED_ONCE, IsekaiEventScript_RomFirstTimeHint_End
	msgbox IsekaiText_RomHint
IsekaiEventScript_RomFirstTimeHint_End:
	return

@ Dating: confrontation if she caught him; otherwise a chat and the option to break up.
IsekaiEventScript_RomDating::
	special IsekaiRomTalk
	goto_if_eq VAR_RESULT, ROM_TALK_CAUGHT, IsekaiEventScript_RomConfront
	romline ROMLINE_DATING
	dynmultichoice 0, 0, FALSE, 3, 0, DYN_MULTICHOICE_CB_NONE, IsekaiText_RomOptAppreciate, IsekaiText_RomOptBreakUp, IsekaiText_RomOptJustVisiting
	switch VAR_RESULT
	case 0, IsekaiEventScript_RomAppreciate
	case 1, IsekaiEventScript_RomBreakUpMenu
	return

IsekaiEventScript_RomAppreciate:
	special IsekaiRomAppreciate
	romline ROMLINE_THANKS
	return

IsekaiEventScript_RomBreakUpMenu:
	msgbox IsekaiText_RomWhatDoYouSay
	dynmultichoice 0, 0, FALSE, 3, 2, DYN_MULTICHOICE_CB_NONE, IsekaiText_RomOptHonestBreakUp, IsekaiText_RomOptBlame, IsekaiText_RomOptNever
	switch VAR_RESULT
	case 0, IsekaiEventScript_RomBreakUpClean
	case 1, IsekaiEventScript_RomBreakUpMessy
	msgbox IsekaiText_RomNeverMind
	return

IsekaiEventScript_RomBreakUpClean:
	setvar VAR_0x8005, 0
	special IsekaiRomBreakUp
	goto_if_eq VAR_RESULT, TRUE, IsekaiEventScript_RomBreakUp_SaidMessy
	romline ROMLINE_BREAK_CLEAN
	return

IsekaiEventScript_RomBreakUpMessy:
	setvar VAR_0x8005, 1
	special IsekaiRomBreakUp
IsekaiEventScript_RomBreakUp_SaidMessy:
	romline ROMLINE_BREAK_MESSY
	return

IsekaiEventScript_RomConfront:
	romline ROMLINE_CONFRONT
	dynmultichoice 0, 0, TRUE, 3, 0, DYN_MULTICHOICE_CB_NONE, IsekaiText_RomOptHonestBreakUp, IsekaiText_RomOptEndOthers, IsekaiText_RomOptDeny
	switch VAR_RESULT
	case 0, IsekaiEventScript_RomBreakUpClean
	case 1, IsekaiEventScript_RomConfront_Promise
	goto IsekaiEventScript_RomBreakUpMessy

IsekaiEventScript_RomConfront_Promise:
	special IsekaiRomPromise
	msgbox IsekaiText_RomPromised
	romline ROMLINE_PROMISE
	return

@ The apology quest: a lot of humble pie. Two answers; one wrong one and it's "come back later".
IsekaiEventScript_RomApology:
	msgbox IsekaiText_RomSorry1
	dynmultichoice 0, 0, TRUE, 3, 0, DYN_MULTICHOICE_CB_NONE, IsekaiText_RomSorryA1, IsekaiText_RomSorryB1, IsekaiText_RomSorryC1
	goto_if_ne VAR_RESULT, 1, IsekaiEventScript_RomApology_Fail
	msgbox IsekaiText_RomSorry2
	dynmultichoice 0, 0, TRUE, 3, 0, DYN_MULTICHOICE_CB_NONE, IsekaiText_RomSorryA2, IsekaiText_RomSorryB2, IsekaiText_RomSorryC2
	goto_if_ne VAR_RESULT, 2, IsekaiEventScript_RomApology_Fail
	special IsekaiRomApologised
	romline ROMLINE_SORRY_DONE
	return

IsekaiEventScript_RomApology_Fail:
	msgbox IsekaiText_RomSorryFail
	return

@ A partner calls the CENTER (from IsekaiFieldIdle). gStringVar4 = her line.
IsekaiEventScript_RomCall::
	lockall
	playse SE_POKENAV_CALL
	waitse
	playse SE_POKENAV_CALL
	msgbox IsekaiText_RomBzzz
	textcolor NPC_TEXT_COLOR_FEMALE
	copyvar VAR_0x8005, VAR_0x8007
	special IsekaiRomBufferLine
	msgbox gStringVar4
	call EventScript_RestorePrevTextColor
	goto_if_eq VAR_0x8006, FALSE, IsekaiEventScript_RomCall_End
	msgbox IsekaiText_RomGift
IsekaiEventScript_RomCall_End:
	releaseall
	end

@ Ghosted: he hasn't visited her in two BADGES.
IsekaiEventScript_RomGhosted::
	lockall
	msgbox IsekaiText_RomGhosted
	releaseall
	end

IsekaiText_RomBringItUp:
	.string "Bring up the two of you again?$"

IsekaiText_RomGoOut:
	.string "Go out with her?$"

IsekaiText_RomOptHonest:
	.string "Be honest$"

IsekaiText_RomOptLie:
	.string "Lie$"

IsekaiText_RomYouLie:
	.string "YOU: …No. Nobody.$"

IsekaiText_RomHint:
	.string "(You're seeing someone now.\n"
	.string "She might call you at a CENTER.\p"
	.string "Visit her now and then. Go two\n"
	.string "BADGES without stopping by, and\l"
	.string "she'll stop waiting.\p"
	.string "Dating more than one person is\n"
	.string "possible. Getting away with it is\l"
	.string "another matter.)$"

IsekaiText_RomOptAppreciate:
	.string "Show appreciation$"

IsekaiText_RomOptBreakUp:
	.string "Talk about breaking up$"

IsekaiText_RomOptJustVisiting:
	.string "Just visiting$"

IsekaiText_RomWhatDoYouSay:
	.string "What do you say?$"

IsekaiText_RomOptHonestBreakUp:
	.string "End it, honestly$"

IsekaiText_RomOptBlame:
	.string "Blame her$"

IsekaiText_RomOptNever:
	.string "Never mind$"

IsekaiText_RomNeverMind:
	.string "YOU: …It's nothing.$"

IsekaiText_RomOptEndOthers:
	.string "End it with the others$"

IsekaiText_RomOptDeny:
	.string "Deny everything$"

IsekaiText_RomPromised:
	.string "(You send the others a message\n"
	.string "on the PC.\p"
	.string "It's a terrible way to end things.\n"
	.string "They'll remember it.)$"

IsekaiText_RomApologise:
	.string "Apologise?$"

IsekaiText_RomSorry1:
	.string "She waits. Arms folded.$"

IsekaiText_RomSorryA1:
	.string "You overreacted.$"

IsekaiText_RomSorryB1:
	.string "I was wrong. No excuses.$"

IsekaiText_RomSorryC1:
	.string "It's complicated.$"

IsekaiText_RomSorry2:
	.string "…Go on.$"

IsekaiText_RomSorryA2:
	.string "Can we start over?$"

IsekaiText_RomSorryB2:
	.string "Everyone does it.$"

IsekaiText_RomSorryC2:
	.string "You deserved better.$"

IsekaiText_RomSorryFail:
	.string "…Come back when you mean it.$"

IsekaiText_RomBzzz:
	.string "Bzzz. The CENTER's phone.\n"
	.string "The nurse waves you over. It's for you.$"

IsekaiText_RomGift:
	.string "(She left something for you in\n"
	.string "the PC: {STR_VAR_2}.)$"

IsekaiText_RomGhosted:
	.string "A message on the PC from\n"
	.string "{STR_VAR_1}:\p"
	.string "“Don't bother.”\p"
	.string "…You haven't visited her in a\n"
	.string "long time.$"

@ v0.14: the ward nurse's perk and penalty when he wakes in a ward.
IsekaiEventScript_WakeUp_NursePerk::
	msgbox IsekaiText_WakeUp_NursePerk
	return

IsekaiEventScript_WakeUp_NursePenalty::
	msgbox IsekaiText_WakeUp_NursePenalty
	return

IsekaiText_WakeUp_NursePerk:
	.string "A note on the bedside table:\p"
	.string "“Patched up your team too. No charge.\n"
	.string "Don't make a habit of this.”$"

IsekaiText_WakeUp_NursePenalty:
	.string "A different nurse. A clipboard.\p"
	.string "“Your file has a note on it.\n"
	.string "'Charge him. Lecture him.'”\p"
	.string "(It cost ¥{STR_VAR_1}. The lecture\n"
	.string "was free, and long.)$"

@ Walking the tall grass for the ward nurse's trial.
IsekaiEventScript_FoundHerb::
	lockall
	playse SE_RG_BAG_POCKET
	msgbox IsekaiText_FoundHerb
	releaseall
	end

IsekaiText_FoundHerb:
	.string "Something bitter-smelling grows\n"
	.string "between the grass stalks.\p"
	.string "You pick a herb from {STR_VAR_1}.\n"
	.string "(Herbs: {STR_VAR_2} of 3)$"

@ BROCK's Rematch Pass: a flying taxi back to the last loss.
IsekaiEventScript_RematchPass::
	lockall
	special IsekaiRematchPassCheck
	goto_if_eq VAR_RESULT, FALSE, IsekaiEventScript_RematchPass_None
	msgbox IsekaiText_RematchPassAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IsekaiEventScript_RematchPass_End
	msgbox IsekaiText_RematchPassGo
	closemessage
	playse SE_M_FLY
	waitse
	special IsekaiRematchPassSetWarp
	waitstate
	end
IsekaiEventScript_RematchPass_None:
	msgbox IsekaiText_RematchPassNone
IsekaiEventScript_RematchPass_End:
	releaseall
	end

IsekaiText_RematchPassAsk:
	.string "REMATCH PASS: one flying taxi to the\n"
	.string "spot of your last loss. Book it?$"

IsekaiText_RematchPassGo:
	.string "A PIDGEOT with a taxi light on its\n"
	.string "head lands next to you.$"

IsekaiText_RematchPassNone:
	.string "REMATCH PASS: no loss on record.\p"
	.string "…Enjoy it while it lasts.$"

@ ---------------------------------------------------------------------------
@ v0.14: BROCK's licence speech, the Rematch Pass, and the loaner grows up.
@ ---------------------------------------------------------------------------
IsekaiEventScript_BrockLicence::
	setvar VAR_ISEKAI_BLAISIA, 1
	msgbox IsekaiText_BrockLicence
	giveitem ITEM_REMATCH_PASS
	msgbox IsekaiText_BrockRematchPass
	special IsekaiLoanerInParty
	goto_if_eq VAR_RESULT, FALSE, IsekaiEventScript_BrockLicence_End
	setflag FLAG_ISEKAI_LOANER_GREW_UP
	bufferspeciesname STR_VAR_1, VAR_ISEKAI_LOANER_SPECIES
	goto_if_eq VAR_ISEKAI_LOANER_SPECIES, SPECIES_SLOWPOKE, IsekaiEventScript_BrockLicence_Slowpoke
	msgbox IsekaiText_LoanerGrowsUpPsyduck
	return
IsekaiEventScript_BrockLicence_Slowpoke:
	msgbox IsekaiText_LoanerGrowsUpSlowpoke
IsekaiEventScript_BrockLicence_End:
	return

IsekaiText_BrockLicence:
	.string "BROCK: This BADGE isn't just a\n"
	.string "trophy. It's a licence.\p"
	.string "Once you have it, you're a real\n"
	.string "TRAINER.\p"
	.string "From here on, nobody goes easy on\n"
	.string "you. Not TRAINERS. Not the wild.\l"
	.string "Take it seriously.\p"
	.string "{FONT_NORMAL}(From now on, losing to a level 10+\n"
	.string "opponent has real consequences.)\p"
	.string "{FONT_MALE}BROCK: Licensed TRAINERS get this too.$"

IsekaiText_BrockRematchPass:
	.string "BROCK: If you lose somewhere, it'll\n"
	.string "fly you straight back there once\l"
	.string "you're ready.\p"
	.string "Trust me. You'll lose somewhere.\p"
	.string "YOU: …A checkpoint. Finally.\p"
	.string "BROCK: A what?$"

IsekaiText_LoanerGrowsUpPsyduck:
	.string "{STR_VAR_1} looks at the BADGE.\n"
	.string "For once, it isn't holding its head.\p"
	.string "({STR_VAR_1} grew confident!\n"
	.string "It won't freeze up anymore.)$"

IsekaiText_LoanerGrowsUpSlowpoke:
	.string "{STR_VAR_1} looks at the BADGE…\n"
	.string "and reacts straight away.\p"
	.string "({STR_VAR_1} is paying attention now!\n"
	.string "Its moves won't lag anymore.)$"

@ ---------------------------------------------------------------------------
@ v0.14: DAISY's message for her brother (after beating BLUE in CERULEAN or on the S.S. ANNE).
@ ---------------------------------------------------------------------------
IsekaiEventScript_DaisyMessageToBlue::
	goto_if_ne VAR_ISEKAI_DAISY_QUEST, DAISY_QUEST_MESSAGE, IsekaiEventScript_DaisyMessageToBlue_End
	msgbox IsekaiText_DaisyMessageNow
	dynmultichoice 0, 0, TRUE, 3, 0, DYN_MULTICHOICE_CB_NONE, IsekaiText_DaisyOptMisses, IsekaiText_DaisyOptBrat, IsekaiText_DaisyOptGrandpa
	switch VAR_RESULT
	case 0, IsekaiEventScript_DaisyMessageToBlue_Right
	case 1, IsekaiEventScript_DaisyMessageToBlue_Lecture
	msgbox IsekaiText_DaisyBlueGrandpa
	return
IsekaiEventScript_DaisyMessageToBlue_Lecture:
	msgbox IsekaiText_DaisyBlueLecture
	return
IsekaiEventScript_DaisyMessageToBlue_Right:
	msgbox IsekaiText_DaisyBlueRight
	setvar VAR_ISEKAI_DAISY_QUEST, DAISY_QUEST_DELIVERED
IsekaiEventScript_DaisyMessageToBlue_End:
	return

IsekaiText_DaisyMessageNow:
	.string "(DAISY's message.\n"
	.string "He'll only listen to a winner.)$"

IsekaiText_DaisyOptMisses:
	.string "She misses you. Call her.$"

IsekaiText_DaisyOptBrat:
	.string "Call your sister, you brat.$"

IsekaiText_DaisyOptGrandpa:
	.string "Your grandpa says hi.$"

IsekaiText_DaisyBlueRight:
	.string "BLUE: …She asked YOU?\p"
	.string "…Fine. FINE.\n"
	.string "Tell her I'm eating properly.\p"
	.string "(He isn't.)$"

IsekaiText_DaisyBlueLecture:
	.string "BLUE: Don't lecture me, old man!\p"
	.string "(He storms off. Maybe next time,\n"
	.string "find better words.)$"

IsekaiText_DaisyBlueGrandpa:
	.string "BLUE: Tell him I don't care.\p"
	.string "(Wrong words. Maybe next time.)$"

@ ---------------------------------------------------------------------------
@ v0.14.7: a ward nurse in every CENTER (the VIRIDIAN one, in lavender, is the romance).
@ ---------------------------------------------------------------------------
IsekaiEventScript_WardNurseGeneric::
	lock
	faceplayer
	textcolor NPC_TEXT_COLOR_FEMALE
	special IsekaiBufferLives
	msgbox IsekaiText_WardNurseGeneric
	setvar VAR_0x8004, ROM_WARD_NURSE
	special IsekaiRomCheckPerk
	goto_if_eq VAR_RESULT, FALSE, IsekaiEventScript_WardNurseGeneric_End
	msgbox IsekaiText_WardNurseGenericDating
IsekaiEventScript_WardNurseGeneric_End:
	call IsekaiEventScript_CenterWard
	release
	end

IsekaiText_WardNurseGeneric:
	.string "WARD NURSE: This is the ward.\p"
	.string "Badly Injured POKéMON rest here while\n"
	.string "you fight a few TRAINERS. Ask the\l"
	.string "nurse at the counter.\p"
	.string "And if you're ever carried in\n"
	.string "yourself… you can come through that\l"
	.string "door {STR_VAR_1} more times.\p"
	.string "Let's not find out.$"

IsekaiText_WardNurseGenericDating:
	.string "…Wait. You're the one VIRIDIAN's ward\n"
	.string "nurse won't stop talking about?\p"
	.string "Huh. I pictured you taller.$"
