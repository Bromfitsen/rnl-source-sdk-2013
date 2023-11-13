#include "cbase.h"
#include "rnl_shareddefs.h"
#include "rnl_weapon_parse.h"

ConVar	sv_speed_walk("sv_speed_walk", "65", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_speed_run("sv_speed_run", "160", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_speed_sprint("sv_speed_sprint", "235", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_speed_crouchwalk("sv_speed_crouchwalk", "45", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_speed_crouchrun("sv_speed_crouchrun", "65", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_speed_crouchsprint("sv_speed_crouchsprint", "160", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_speed_pronewalk("sv_speed_pronewalk", "30", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_speed_pronerun("sv_speed_pronerun", "55", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_speed_recovery("sv_speed_recovery", "140", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_speed_para("sv_speed_para", "100", FCVAR_NOTIFY | FCVAR_REPLICATED);


ConVar	sv_stamina_recover("sv_stamina_recover", "10", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_stamina_recover_walk("sv_stamina_recover_walk", "8", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_stamina_recover_crouchwalk("sv_stamina_recover_crouchwalk", "8", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_stamina_recover_crouchrun("sv_stamina_recover_crouchrun", "4", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_stamina_recover_pronewalk("sv_stamina_recover_pronewalk", "6", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_stamina_recover_pronerun("sv_stamina_recover_pronerun", "4", FCVAR_NOTIFY | FCVAR_REPLICATED);

ConVar	sv_stamina_sprint("sv_stamina_sprint", "6", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_stamina_crouchsprint("sv_stamina_crouchsprint", "10", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_stamina_pronesprint("sv_stamina_pronesprint", "15", FCVAR_NOTIFY | FCVAR_REPLICATED);

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

BEGIN_NETWORK_TABLE_NOBASE(CRnLLoadoutModelBodyGroupInfo, DT_RnLLoadoutModelBodyGroupInfo)
	PropString(PROPINFO(groupName)),
	PropInt(PROPINFO(iVal)),
END_NETWORK_TABLE()

BEGIN_NETWORK_TABLE_NOBASE(CRnLLoadoutModelInfo, DT_RnLLoadoutModelInfo)
	PropString(PROPINFO(file)),
	PropInt(PROPINFO(iSkin)),
	PropUtlVectorDataTable(vecBodyGroups, 4, DT_RnLLoadoutModelBodyGroupInfo),
END_NETWORK_TABLE()

// Datatable
BEGIN_NETWORK_TABLE_NOBASE(RnLLoadoutKitInfo, DT_RnLLoadoutKitInfo)
	PropString(PROPINFO(title)),
	PropString(PROPINFO(name)),
	PropInt(PROPINFO(iClass)),
//	PropBool(PROPINFO(bSquadLeader)),
//	PropDataTable(PROPINFO_DT(model), 0, &REFERENCE_PROP_TABLE(DT_RnLLoadoutModelInfo)),
//	PropUtlVector(PROPINFO_UTLVECTOR(weapons), 5, PropInt(NULL, 0, 1)),
END_NETWORK_TABLE()

bool CUtlStringLessFunc(const CUtlString& lhs, const CUtlString& rhs)
{
	return CaselessStringLessThan(lhs.Get(), rhs.Get());
}

CRnLViewVectors::CRnLViewVectors()
{
}

CRnLViewVectors::CRnLViewVectors(
	Vector vView,
	Vector vHullMin,
	Vector vHullMax,
	Vector vDuckView,
	Vector vDuckWalkView,
	Vector vDuckHullMin,
	Vector vDuckHullMax,
	Vector vProneView,
	Vector vProneHullMin,
	Vector vProneHullMax,
	Vector vObsViewHeight,
	Vector vObsHullMin,
	Vector vObsHullMax)
	: CViewVectors(
		vView, vHullMin, vHullMax,
		vDuckHullMin, vDuckHullMax, vDuckView,
		vObsHullMin, vObsHullMax, vObsViewHeight
	)
	, m_vDuckWalkView(vDuckWalkView)
	, m_vProneView(vProneView)
	, m_vProneHullMin(vProneHullMin)
	, m_vProneHullMax(vProneHullMax)
{
}
