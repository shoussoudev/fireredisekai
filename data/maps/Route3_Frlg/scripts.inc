Route3_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route3_OnTransition
	.byte 0

@ FireRed Isekai: BLAISIA is only there after BROCK, until he's met her.
Route3_OnTransition::
	call_if_ne VAR_ISEKAI_BLAISIA, 1, Route3_EventScript_IsekaiHideBlaisia
	call_if_eq VAR_ISEKAI_BLAISIA, 1, Route3_EventScript_IsekaiShowBlaisia
	end

Route3_EventScript_IsekaiHideBlaisia::
	setflag FLAG_HIDE_ISEKAI_BLAISIA_1
	return

Route3_EventScript_IsekaiShowBlaisia::
	clearflag FLAG_HIDE_ISEKAI_BLAISIA_1
	return

Route3_EventScript_Youngster::
	msgbox Route3_Text_TunnelFromCeruleanTiring, MSGBOX_NPC
	end

Route3_EventScript_RouteSign::
	msgbox Route3_Text_RouteSign, MSGBOX_SIGN
	end

Route3_Text_TunnelFromCeruleanTiring::
	.string "Whew… I better take a rest…\n"
	.string "Groan…\p"
	.string "That tunnel from CERULEAN takes a\n"
	.string "lot out of you!$"

Route3_Text_ColtonIntro::
	.string "Hey!\n"
	.string "I saw you in VIRIDIAN FOREST!$"

Route3_Text_ColtonDefeat::
	.string "You beat me again!$"

Route3_Text_ColtonPostBattle::
	.string "There are other kinds of POKéMON\n"
	.string "than the ones you find in forests.$"

Route3_Text_BenIntro::
	.string "Hi!\n"
	.string "I like shorts!\p"
	.string "They're delightfully comfy and\n"
	.string "easy to wear!$"

Route3_Text_BenDefeat::
	.string "I don't believe it!$"

Route3_Text_BenPostBattle::
	.string "Are you using a POKéMON CENTER's\n"
	.string "PC for storing your POKéMON?\p"
	.string "Each BOX can hold up to\n"
	.string "30 POKéMON.$"

Route3_Text_JaniceIntro::
	.string "Excuse me!\n"
	.string "You looked at me, didn't you?$"

Route3_Text_JaniceDefeat::
	.string "You're mean!$"

Route3_Text_JanicePostBattle::
	.string "You shouldn't be staring if you\n"
	.string "don't want to battle!$"

Route3_Text_GregIntro::
	.string "Are you a TRAINER?\n"
	.string "Let's get with it right away!$"

Route3_Text_GregDefeat::
	.string "If I had new POKéMON, I would've\n"
	.string "won!$"

Route3_Text_GregPostBattle::
	.string "If a POKéMON BOX on the PC gets\n"
	.string "full, just switch to another BOX.$"

Route3_Text_SallyIntro::
	.string "That look you gave me…\n"
	.string "It's so intriguing!$"

Route3_Text_SallyDefeat::
	.string "Be nice!$"

Route3_Text_SallyPostBattle::
	.string "You can avoid battles by not\n"
	.string "letting TRAINERS see you.$"

Route3_Text_CalvinIntro::
	.string "Hey! You're not wearing shorts!\n"
	.string "What's wrong with you?$"

Route3_Text_CalvinDefeat::
	.string "Lost!\n"
	.string "Lost! Lost!$"

Route3_Text_CalvinPostBattle::
	.string "I always wear shorts, even in\n"
	.string "winter. That's my policy.$"

Route3_Text_JamesIntro::
	.string "I'll battle you with the POKéMON\n"
	.string "I just caught.$"

Route3_Text_JamesDefeat::
	.string "Done like dinner!$"

Route3_Text_JamesPostBattle::
	.string "Trained POKéMON are stronger than\n"
	.string "the wild ones.$"

Route3_Text_RobinIntro::
	.string "Eek!\n"
	.string "Did you touch me?$"

Route3_Text_RobinDefeat::
	.string "That's it?$"

Route3_Text_RobinPostBattle::
	.string "ROUTE 4 is at the foot of\n"
	.string "MT. MOON.$"

Route3_Text_RouteSign::
	.string "ROUTE 3\n"
	.string "MT. MOON AHEAD$"


@ FireRed Isekai (v0.14): BLAISIA, first meeting. She won't let him pass until he answers.
Route3_EventScript_IsekaiBlaisiaTrigger::
	lockall
	applymovement LOCALID_ROUTE3_BLAISIA, Common_Movement_ExclamationMark
	waitmovement 0
	goto Route3_EventScript_IsekaiBlaisiaTalk

Route3_EventScript_IsekaiBlaisia::
	lockall
	faceplayer
Route3_EventScript_IsekaiBlaisiaTalk::
	textcolor NPC_TEXT_COLOR_FEMALE
	msgbox Route3_Text_IsekaiBlaisia1
	msgbox Route3_Text_IsekaiBlaisia2
	closemessage
	trainerbattle_no_intro TRAINER_ISEKAI_BLAISIA_1, Route3_Text_IsekaiBlaisiaDefeat
	msgbox Route3_Text_IsekaiBlaisiaAfter
	closemessage
	setvar VAR_ISEKAI_BLAISIA, 2
	setflag FLAG_HIDE_ISEKAI_BLAISIA_1
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_ROUTE3_BLAISIA
	fadescreen FADE_FROM_BLACK
	releaseall
	end

Route3_Text_IsekaiBlaisia1:
	.string "A woman about your mum's age stands\n"
	.string "in the path, holding a RATTATA like\l"
	.string "a newborn.\p"
	.string "WOMAN: Excuse me, young man. You\n"
	.string "look like you know what you're doing.\p"
	.string "I'm BLAISIA. I have questions.\n"
	.string "Three. You can't pass until then.\p"
	.string "Question one. How do you make it do\n"
	.string "the electricity?\p"
	.string "YOU: …That's a RATTATA.\n"
	.string "It doesn't do electricity.\p"
	.string "BLAISIA: Oh. Well, I've named it\n"
	.string "Little One, so it can do what\l"
	.string "it likes.$"

Route3_Text_IsekaiBlaisia2:
	.string "Question two. Where does it go when\n"
	.string "it's in the ball? Is it comfortable?\l"
	.string "Is it cold in there?\p"
	.string "(You don't actually know.)\p"
	.string "Question three. Would you battle me?\n"
	.string "I've never done one. My son made it\l"
	.string "look easy.$"

Route3_Text_IsekaiBlaisiaDefeat:
	.string "Oh! Sorry! Sorry, little one!\n"
	.string "Not you, Little One. The other one.$"

Route3_Text_IsekaiBlaisiaAfter:
	.string "BLAISIA: That was wonderful.\n"
	.string "I didn't understand a single thing.\p"
	.string "I'll see you around!\n"
	.string "I'll have more questions!\p"
	.string "(She will.)$"
