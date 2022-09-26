#include "cbase.h"
#include "rnl_shareddefs.h"
#include "rnl_weapon_parse.h"

// NOTE: the indices here must match TEAM_TERRORIST, TEAM_CT, TEAM_SPECTATOR, etc.
char *sRnLTeamNames[] =
{
	"Unassigned",
	"Spectator",
	"Allies",
	"Axis"
};

const char *TeamNumberToName( int id )
{
	if( id < TEAM_UNASSIGNED || id >= TEAMS_COUNT )
		return NULL;

	return sRnLTeamNames[id];
}

int TeamNameToNumber( const char *name )
{
	if (name)
	{
		for( int i=0; sRnLTeamNames[i] != NULL; ++i )
			if (!Q_stricmp( sRnLTeamNames[i], name ))
				return i;
	}

	return TEAM_INVALID;
}

// ----------------------------------------------------------------------------- //
// Global functions.
// ----------------------------------------------------------------------------- //

//--------------------------------------------------------------------------------------------------------
static const char * s_WeaponAliasInfo[] = 
{
	"none",		// WEAPON_NONE
	"bar",
	"browning",
	"colt",
	"greasegun",
	"carbine",
	"garand",
	"springfield",
	"thompson",
	"m3knife",
	"mk2grenade",
	"m18grenade",
	"sticky24grenade",
	"sticky39grenade",
	"stg44",
	"walther",
	"k98k",
	"mp40",
	"mg34",
	"mg42",
	"trenchknife",
	"fg42",
	"gewehr43",
	"k98scoped",
	"ncomap",
	"devcam",
	"alliedfists",
	"axisfists",
	"alliedexplosives",
	"axisexplosives",

	//Scripted Weapons
	"scripted_weapon",

	NULL,
};

//--------------------------------------------------------------------------------------------------------
//
// Given an alias, return the associated weapon ID
//
int AliasToWeaponID( const char *alias )
{
	if (alias)
	{
		for( int i=0; s_WeaponAliasInfo[i] != NULL; ++i )
			if (!Q_stricmp( s_WeaponAliasInfo[i], alias ))
				return i;
	}

	char szFullTitle[64];
	Q_snprintf( szFullTitle, sizeof( szFullTitle ), "weapon_%s", alias );
	int ScriptedValue = ScriptedNameToType(szFullTitle);
	if( ScriptedValue != SCRIPTED_WEAPON_INVALID )
		return WEAPON_SCRIPTED;

	return WEAPON_NONE;
}

//--------------------------------------------------------------------------------------------------------
//
// Given a weapon ID, return its alias
//
const char *WeaponIDToAlias( int id )
{
	if ( (id >= WEAPON_MAX) || (id < 0) )
		return NULL;

	return s_WeaponAliasInfo[id];
}
