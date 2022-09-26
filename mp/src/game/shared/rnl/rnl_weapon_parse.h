//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef RNL_WEAPON_PARSE_H
#define RNL_WEAPON_PARSE_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_parse.h"
#include "networkvar.h"
#include "rnl_shareddefs.h"

enum eScriptedWeaponTypes
{
	SCRIPTED_WEAPON_INVALID = 0,
	SCRIPTED_WEAPON_PISTOL,
	SCRIPTED_WEAPON_RIFLE,
	SCRIPTED_WEAPON_SUBMACHINEGUN,
	SCRIPTED_WEAPON_MACHINEGUN,
	SCRIPTED_WEAPON_GRENADE,
	SCRITPED_WEAPON_SMOKEGRENADE,
	SCRIPTED_WEAPON_MAX,
};

int ScriptedNameToType( const char* name );

class CScriptedWeaponLinker
{
public:
	
	typedef void (*LinkerFn)(void);	// Prototype for a custom precache function.

	CScriptedWeaponLinker(LinkerFn fn);

	LinkerFn				m_Fn;	
	CScriptedWeaponLinker	*m_pNext;

	static void			Precache();						// Calls everything that has registered to precache.
};

bool IsScriptedWeaponOverride( void );
void SetScriptedWeaponOverride( bool state, const char* pszName = NULL );
const char* GetScriptedWeaponOverride( void );

void LinkScriptedWeaponToType( const char* pszWeaponName, int iType );
const char* TranslateScriptedWeaponTypeToName( const char* pszScriptedName );
void PrecacheFileScriptedWeaponInfoDatabase( IFileSystem *filesystem, const unsigned char *pICEKey );

#define LINK_SCRIPTED_WEAPON_TO_TYPE(className,type)		\
	void ScriptedWeaponLinker_##className( void ) {	\
		LinkScriptedWeaponToType( #className, type );		\
	}														\
	static CScriptedWeaponLinker scripted_weapon_link_##className( ScriptedWeaponLinker_##className ); \
	

//--------------------------------------------------------------------------------------------------------
class CRnLWeaponInfo : public FileWeaponInfo_t
{
public:
	DECLARE_CLASS_GAMEROOT( CRnLWeaponInfo, FileWeaponInfo_t );
	
	CRnLWeaponInfo();
	
	virtual void Parse( ::KeyValues *pKeyValuesData, const char *szWeaponName );

	int		m_iScriptedWeaponType;

	char m_szAnimExtension[16];				// string used to generate player animations with this weapon
	char m_szClipModel[MAX_WEAPON_STRING];		// string used to store weapon clip model

	// Parameters for FX_FireBullets:
	int		m_iDamage;
	int		m_iBullets;
	int		m_iClipSize;
	float	m_flCycleTime;
	float	m_flSoundCycleTime;
	//RnL : Andrew : BULLET SPREAD OH YEA!!!
	float	m_flSpread;
	float	m_flMaxSpread;
	int		m_iHeatIndex;

	int		m_iTeamAssociation;

	float	m_flFieldOfView[WEAPON_POSTURE_MAX];
	float	m_flRecoil[RECOIL_MAX];
};


#endif // RNL_WEAPON_PARSE_H
