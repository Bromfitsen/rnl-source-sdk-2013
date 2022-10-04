//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//=============================================================================

// No spaces in event names, max length 32
// All strings are case sensitive
//
// valid data key types are:
//   string : a zero terminated string
//   bool   : unsigned int, 1 bit
//   byte   : unsigned int, 8 bit
//   short  : signed int, 16 bit
//   long   : signed int, 32 bit
//   float  : float, 32 bit
//   local  : any data, but not networked to clients
//
// following key names are reserved:
//   local      : if set to 1, event is not networked to clients
//   unreliable : networked, but unreliable
//   suppress   : never fire this event
//   time	: firing server time
//   eventid	: holds the event ID

"rnlevents"
{
	"player_spawn"				// player has spawned
	{
		"userid"		"short"		// user ID on server
	}

	"player_death"				// player has died
	{
		"userid"		"short"		// victim's user ID on server
		"attacker"		"short"		// attacker's user ID on server
		"customkill"	"byte"		// 
		"weapon"		"string" 	// name of weapon that killed player
	}
	
	"player_squad"				// player changed his squad
	{
		"userid"		"short"		// user ID on server
		"squad"			"byte"		// squad id
		"slot"			"byte"		// slot id
		"disconnect"	"bool"		// player is disconnecting
	}

	// Server-side player events
    "player_hurt"				// player was shot
    {
        "userid"		"local"		// user ID who was hurt
        "attacker"		"local"		// user ID who attacked
        "weapon"		"local"		// weapon name attacker used
        "health"		"local"		// health remaining
        "damage"		"local"		// how much damage in this attack
        "hitgroup"		"local"		// what hitgroup was hit
    }

    "player_shoot"				// player shot his weapon
    {
        "userid"		"local"		// user ID on server
        "weapon"		"local"		// weapon name
        "mode"			"local"		// weapon mode 0 = hip, 1 = shoulder, 2 = ironsights, 3 = supersights, 4 = throwing grenade
    }

	"teamplay_round_start"			// round start
	{
		"full_reset"	"bool"		// is this a full reset of the map
	}
	
	"teamplay_round_active"			// called when round is active, players can move
	{
		// nothing for now
	}

	"teamplay_restart_round"		// round restart
	{
		// nothing for now
	}
	
	"teamplay_round_win"			// the round is won
	{
		"team"				"short"		// team number who won the round
	}

	"rnl_task_state_change"			// a task has changed states
	{ 
		"task_name"			"string"	// task name
		"new_state"			"short"		// numeric value of new state, -1 = invalid, 0 = neutral, 1 = allied control, 2 = axis conrol
		"current_state" 	"short"		// numeric value of state prior to change
		"previous_state"	"short"		// numeric value of state prior to the current state
	}
	
	"rnl_objective_state_change"	// an objective has changed states
	{ 
		"objective_name"	"string"	// objective name
		"new_state"			"short"		// numeric value of new state, -1 = invalid, 0 = neutral, 1 = allied control, 2 = axis conrol
		"current_state" 	"short"		// numeric value of state prior to change
		"previous_state"	"short"		// numeric value of state prior to the current state
	}
}
