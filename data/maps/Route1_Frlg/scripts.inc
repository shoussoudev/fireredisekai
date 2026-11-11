Route1_Frlg_MapScripts::
	.byte 0

Route1_EventScript_MartClerk::
	lock
	faceplayer
	goto_if_set FLAG_GOT_POTION_ON_ROUTE_1, Route1_EventScript_AlreadyGotPotion
	msgbox Route1_Text_WorkAtPokeMartTakeSample
	textcolor NPC_TEXT_COLOR_NEUTRAL
	checkitemspace ITEM_POTION
	goto_if_eq VAR_RESULT, FALSE, EventScript_BagIsFull
	bufferitemname STR_VAR_2, ITEM_POTION
	playfanfare MUS_LEVEL_UP
	message gText_ObtainedTheItem
	waitmessage
	waitfanfare
	additem ITEM_POTION
	msgbox Route1_Text_PutPotionAway
	call EventScript_RestorePrevTextColor
	setflag FLAG_GOT_POTION_ON_ROUTE_1
	release
	end

Route1_EventScript_AlreadyGotPotion::
	msgbox Route1_Text_ComeSeeUsIfYouNeedPokeBalls
	release
	end

Route1_EventScript_Boy::
	goto_if_set FLAG_ISEKAI_LIMPING, Route1_EventScript_IsekaiBoyLimp
	msgbox Route1_Text_CanJumpFromLedges, MSGBOX_NPC
	end

@ FireRed Isekai: he climbed out of a window this morning.
Route1_EventScript_IsekaiBoyLimp::
	msgbox Route1_Text_IsekaiLimping, MSGBOX_NPC
	end

Route1_Text_IsekaiLimping::
	.string "Are you… limping?\n"
	.string "Through the grass?\p"
	.string "Bold.$"

Route1_EventScript_RouteSign::
	msgbox Route1_Text_RouteSign, MSGBOX_SIGN
	end

Route1_Text_WorkAtPokeMartTakeSample::
	.string "Hi!\n"
	.string "I work at a POKéMON MART.\p"
	.string "It's part of a convenient chain\n"
	.string "selling all sorts of items.\p"
	.string "Please, visit us in VIRIDIAN CITY.\p"
	.string "I know, I'll give you a sample.\n"
	.string "Here you go!$"

Route1_Text_ComeSeeUsIfYouNeedPokeBalls::
	.string "Please come see us if you need\n"
	.string "POKé BALLS for catching POKéMON.$"

Route1_Text_PutPotionAway::
	.string "{PLAYER} put the POTION away in\n"
	.string "the BAG's ITEMS POCKET.$"

Route1_Text_CanJumpFromLedges::
	.string "See those ledges along the road?\p"
	.string "It's a bit scary, but you can jump\n"
	.string "from them.\p"
	.string "You can get back to PALLET TOWN\n"
	.string "quicker that way.$"

Route1_Text_RouteSign::
	.string "ROUTE 1\n"
	.string "PALLET TOWN - VIRIDIAN CITY$"


@ FireRed Isekai: the POKéMON RANGER, counting PIDGEY.
Route1_EventScript_IsekaiRanger::
	lock
	goto_if_set FLAG_ISEKAI_MET_RANGER, Route1_EventScript_IsekaiRangerAgain
	msgbox Route1_Text_IsekaiRangerCount
	faceplayer
	msgbox Route1_Text_IsekaiRanger
	setflag FLAG_ISEKAI_MET_RANGER
	release
	end

Route1_EventScript_IsekaiRangerAgain::
	msgbox Route1_Text_IsekaiRangerAgain
	release
	end

Route1_Text_IsekaiRangerCount:
	.string "RANGER: Shh. Forty-one.\n"
	.string "…Forty-two.$"

Route1_Text_IsekaiRanger:
	.string "RANGER: PIDGEY. I count them every\n"
	.string "spring.\p"
	.string "Half as many as when the LEAGUE\n"
	.string "opened.\p"
	.string "Everyone wants to catch them all.\n"
	.string "Nobody asks where “all” goes.\p"
	.string "…Sorry. Good luck out there.\n"
	.string "Catch what you need.$"

Route1_Text_IsekaiRangerAgain:
	.string "RANGER: …Forty-three.\n"
	.string "Shh.$"
