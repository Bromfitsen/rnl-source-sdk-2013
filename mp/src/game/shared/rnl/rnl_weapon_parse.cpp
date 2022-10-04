//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include <KeyValues.h>
#include "rnl_weapon_parse.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static bool g_bOverrideScriptedWeapons = false;
static const char* g_pszOverrideScriptedWeapon = NULL;

bool IsScriptedWeaponOverride( void )
{
	return g_bOverrideScriptedWeapons;
}

void SetScriptedWeaponOverride( bool state, const char* pszName )
{
	g_bOverrideScriptedWeapons = state;
	g_pszOverrideScriptedWeapon = pszName;
}

const char* GetScriptedWeaponOverride( void )
{
	return g_pszOverrideScriptedWeapon;
}

static CScriptedWeaponLinker* g_pHeadScriptLinker = NULL;
CScriptedWeaponLinker::CScriptedWeaponLinker(LinkerFn fn)
{
	m_Fn = fn;

	m_pNext = g_pHeadScriptLinker;
	g_pHeadScriptLinker = this;
}

void CScriptedWeaponLinker::Precache()
{
	for(CScriptedWeaponLinker *pCur=g_pHeadScriptLinker; pCur; pCur=pCur->m_pNext)
		pCur->m_Fn();
}

const char* szScriptedWeaponNames[] = 
{
	"invalid",
	"pistol",
	"rifle",
	"smg",
	"mg",
	"grenade",
	"smokegrenade",
};

int ScriptedNameToType( const char* name )
{
	for( int i = 0; i < SCRIPTED_WEAPON_MAX; i++ )
	{
		if( stricmp( name, szScriptedWeaponNames[i] ) == 0 )
			return i;
	}

	return SCRIPTED_WEAPON_INVALID;
}

extern bool CUtlStringLessFunc( const CUtlString &lhs, const CUtlString &rhs );

static CUtlMap< int, CUtlString > m_ScriptedWeaponTypeDatabase( DefLessFunc( int ) );
static CUtlMap< CUtlString, unsigned short > m_ScriptedWeaponTranslateDatabase( CUtlStringLessFunc );

void LinkScriptedWeaponToType( const char* pszWeaponName, int iType )
{
	Msg( "Linking Scripted Weapon: %s to type %s\n", pszWeaponName, szScriptedWeaponNames[iType] );
	m_ScriptedWeaponTypeDatabase.Insert( iType, pszWeaponName );
}


void LinkScriptedWeaponToWeapon( const char* pszScriptedName, int iType )
{
	if( m_ScriptedWeaponTypeDatabase.Find( iType ) != m_ScriptedWeaponTypeDatabase.InvalidIndex() )
	{
		unsigned short indx = m_ScriptedWeaponTypeDatabase.Find( iType );
		m_ScriptedWeaponTranslateDatabase.Insert( pszScriptedName, indx );
	}
}

const char* TranslateScriptedWeaponTypeToName( const char* pszScriptedName )
{
	unsigned short index = m_ScriptedWeaponTranslateDatabase.Find( pszScriptedName );
	if( index == m_ScriptedWeaponTranslateDatabase.InvalidIndex() )
		return NULL;

	Msg( "Translated Scripted weapon: %s\n", pszScriptedName );
	
	unsigned short val =  m_ScriptedWeaponTranslateDatabase[index];
	if( m_ScriptedWeaponTypeDatabase.IsValidIndex( val ) )
		return m_ScriptedWeaponTypeDatabase[val].Get();

	return NULL;
}

void PrecacheFileScriptedWeaponInfoDatabase( IFileSystem *filesystem, const unsigned char *pICEKey )
{
	char tempFilename[MAX_PATH];
#ifdef CLIENT_DLL
	char mapName[64];
	Q_FileBase( engine->GetLevelName(), mapName, sizeof(mapName) );
	Q_snprintf( tempFilename, sizeof( tempFilename ), "scripts/weapon_manifest_%s.res", mapName );
#else
	Q_snprintf( tempFilename, sizeof( tempFilename ), "scripts/weapon_manifest_%s.res", STRING( gpGlobals->mapname ) );
#endif
	
	Msg( "Loading Weapon Script: %s\n", tempFilename );

	KeyValues *manifest = new KeyValues( "weaponscripts" );
	if ( manifest->LoadFromFile( filesystem, tempFilename, "MOD" ) )
	{
		for ( KeyValues *sub = manifest->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
		{
			if ( !Q_stricmp( sub->GetName(), "file" ) )
			{
				char fileBase[512];
				Q_FileBase( sub->GetString(), fileBase, sizeof(fileBase) );
				WEAPON_FILE_INFO_HANDLE tmp;
				if ( ReadWeaponDataFromFileForSlot( filesystem, fileBase, &tmp, pICEKey ) )
				{
#ifdef CLIENT_DLL	
					gWR.LoadWeaponSprites( tmp );
#endif
					CRnLWeaponInfo* pInfo = (CRnLWeaponInfo*)GetFileWeaponInfoFromHandle( tmp );
					if( pInfo && pInfo->m_iScriptedWeaponType != SCRIPTED_WEAPON_INVALID )
					{
						Msg( "Added Scripted Weapon: %s\n", fileBase );
						LinkScriptedWeaponToWeapon( fileBase, pInfo->m_iScriptedWeaponType );
						CBaseEntity::PrecacheModel( pInfo->szViewModel );
						CBaseEntity::PrecacheModel( pInfo->szWorldModel );
					}
				}
			}
			else
			{
				Error( "Expecting 'file', got %s\n", sub->GetName() );
			}
		}
	}
	manifest->deleteThis();
}

FileWeaponInfo_t* CreateWeaponInfo()
{
	return new CRnLWeaponInfo;
}


CRnLWeaponInfo::CRnLWeaponInfo()
{
}


void CRnLWeaponInfo::Parse( KeyValues *pKeyValuesData, const char *szWeaponName )
{
	BaseClass::Parse( pKeyValuesData, szWeaponName );

	m_iDamage		= pKeyValuesData->GetInt( "Damage", 42 ); // Douglas Adams 1952 - 2001
	m_iBullets		= pKeyValuesData->GetInt( "Bullets", 1 );
	m_iClipSize		= pKeyValuesData->GetInt( "clip_size", 8 );
	m_flCycleTime	= pKeyValuesData->GetFloat( "CycleTime", 0.15 );
	m_flSoundCycleTime	= pKeyValuesData->GetFloat( "SoundCycleTime", 0.09 );
	m_flSpread		= pKeyValuesData->GetFloat( "Spread", 0.00873f );
	m_flMaxSpread	= pKeyValuesData->GetFloat( "MaxSpread", 0.00873f );
	m_iHeatIndex	= pKeyValuesData->GetInt( "HeatIndex", 0 );
	m_iScriptedWeaponType = ScriptedNameToType( pKeyValuesData->GetString( "scripted_type" ) );

	KeyValues* pSubKey = pKeyValuesData->FindKey( "FieldsOfView" );

	if( pSubKey )
	{
		m_flFieldOfView[WEAPON_POSTURE_HIP] = pSubKey->GetFloat( "Hip", 75.0f );
		m_flFieldOfView[WEAPON_POSTURE_SHOULDER] = pSubKey->GetFloat( "Shoulder", 75.0f );
		m_flFieldOfView[WEAPON_POSTURE_IRONSIGHTS] = pSubKey->GetFloat( "Sights", 50.0f );
		m_flFieldOfView[WEAPON_POSTURE_SUPERSIGHTS] = pSubKey->GetFloat( "SteadySights", 35.0f );
		m_flFieldOfView[WEAPON_POSTURE_THROWING_GRENADE] = pSubKey->GetFloat( "Throwing", 75.0f );
	}
	else
	{
		m_flFieldOfView[WEAPON_POSTURE_HIP] = 75.0f;
		m_flFieldOfView[WEAPON_POSTURE_SHOULDER] = 75.0f;
		m_flFieldOfView[WEAPON_POSTURE_IRONSIGHTS] = 50.0f;
		m_flFieldOfView[WEAPON_POSTURE_SUPERSIGHTS] = 35.0f;
		m_flFieldOfView[WEAPON_POSTURE_THROWING_GRENADE] = 75.0f;
	}

	pSubKey = pKeyValuesData->FindKey( "Recoil" );

	if( pSubKey )
	{
		m_flRecoil[RECOIL_NORMAL] = pSubKey->GetFloat( "Normal", 4.50f );
		m_flRecoil[RECOIL_CROUCHED] = pSubKey->GetFloat( "Crouched", 2.50f );
		m_flRecoil[RECOIL_PRONE] = pSubKey->GetFloat( "Prone", 0.50f );
		m_flRecoil[RECOIL_JUMPING] = pSubKey->GetFloat( "Jumping", 8.0f );
		m_flRecoil[RECOIL_SIGHTED] = pSubKey->GetFloat( "SightedFactor", 0.66 );
	}
	else
	{
		m_flRecoil[RECOIL_NORMAL] = 4.50f;
		m_flRecoil[RECOIL_CROUCHED] = 2.50f;
		m_flRecoil[RECOIL_PRONE] = 0.50f;
		m_flRecoil[RECOIL_JUMPING] = 8.0f;
		m_flRecoil[RECOIL_SIGHTED] = 0.66;
	}

	Q_strncpy( m_szAnimExtension, pKeyValuesData->GetString( "PlayerAnimationExtension", "M1" ), sizeof( m_szAnimExtension ) );
	Q_strncpy( m_szClipModel, pKeyValuesData->GetString( "ClipModel", "models/items/BoxMRounds.mdl" ), sizeof( m_szClipModel ) );

	const char *pTeamName = pKeyValuesData->GetString( "Team", "None" );
	m_iTeamAssociation = TEAM_INVALID;
	if( pTeamName != NULL )
	{
		if( Q_stricmp( pTeamName, "allies" ) == 0 )
		{
			m_iTeamAssociation = TEAM_ALLIES;
		}
		else if( Q_stricmp( pTeamName, "axis" ) == 0 )
		{
			m_iTeamAssociation = TEAM_AXIS;
		}
	}
}
