//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		The NCO map
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "ammodef.h"
#include "in_buttons.h"
#include "vstdlib/random.h"
#include "npcevent.h"

#if defined( CLIENT_DLL )
	#include "c_rnl_player.h"
	#include "iinput.h"	
	#include <igameresources.h>
	#include "rnl_ncomap.h"
#else
	#include "rnl_player.h"
	#include "viewport_panel_names.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// The list of official maps
typedef enum
{
	MAP_NONE = -1,
	MAP_STEMARIEDUMONT = 0,
	MAP_STCOMEDUMONT,
	MAP_EGLISE,
	MAP_CARENTAN_CAUSEWAY,
	MAP_CARENTAN,
	MAP_VIERVILLE,
	MAP_INOFFICIAL
};

#ifdef CLIENT_DLL
#define CWeaponUSMap C_WeaponUSMap
#define CWeaponGermanMap C_WeaponGermanMap
#define CWeaponNCOMap C_WeaponNCOMap
#define CWeaponGruppenFuhrerMap C_WeaponGruppenFuhrerMap
#endif

//-----------------------------------------------------------------------------
// CWeaponNCOMap
//-----------------------------------------------------------------------------

class CWeaponUSMap : public CWeaponRnLBase
{
public:
	DECLARE_CLASS( CWeaponUSMap, CWeaponRnLBase );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponUSMap();

	virtual void Spawn();
	void		SetSkin( int skinNum );
	bool		Deploy();
	bool		Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	void		ItemPostFrame( void );
	void		GetControlPanelInfo(int nPanelIndex, const char *&pPanelName);
	bool		AllowsAutoSwitchTo() { return false; }
	virtual RnLWeaponID GetWeaponID( void ) const { return WEAPON_NCOMAP; }

private:

	//CNetworkVar( int, m_iSkin );
	CWeaponUSMap( const CWeaponUSMap & );		
};

//-----------------------------------------------------------------------------
// CWeaponUSMap
//-----------------------------------------------------------------------------

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponUSMap, DT_WeaponUSMap )

BEGIN_NETWORK_TABLE( CWeaponUSMap, DT_WeaponUSMap )
//#ifndef CLIENT_DLL
//	SendPropInt( SENDINFO( m_iSkin ) ),
//#else
//	RecvPropInt( RECVINFO( m_iSkin ) ),
//#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponUSMap )
END_PREDICTION_DATA()

acttable_t CWeaponUSMap::m_acttable[] = 
{
	{ ACT_SPRINT,			ACT_DOD_SPRINT_AIM_SPADE,			false },

	{ ACT_DOD_STAND_AIM,	ACT_DOD_STAND_AIM_SPADE,			false },
	{ ACT_DOD_STAND_IDLE,	ACT_DOD_STAND_AIM_SPADE,			false },
	
	{ ACT_DOD_CROUCH_AIM,	ACT_DOD_CROUCH_AIM_SPADE,			false },
	{ ACT_DOD_CROUCH_IDLE,	ACT_DOD_CROUCH_AIM_SPADE,			false },

	{ ACT_DOD_CROUCHWALK_AIM,	ACT_DOD_CROUCHWALK_AIM_SPADE,	false },
	{ ACT_DOD_CROUCHWALK_IDLE,	ACT_DOD_CROUCHWALK_AIM_SPADE,	false },
	
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_AIM_SPADE,		false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_SPADE,			false },
	
	{ ACT_DOD_RUN_AIM,		ACT_DOD_RUN_AIM_SPADE,				false },
	{ ACT_DOD_RUN_IDLE,		ACT_DOD_RUN_AIM_SPADE,			false },
	
	{ ACT_DOD_WALK_AIM,		ACT_DOD_WALK_AIM_SPADE,			false },
	{ ACT_DOD_WALK_IDLE,	ACT_DOD_WALK_AIM_SPADE,			false },

	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_SPADE,	false },
	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PRIMARYATTACK_PRONE_SPADE, false },
};

IMPLEMENT_ACTTABLE(CWeaponUSMap);

LINK_ENTITY_TO_CLASS( weapon_usmap, CWeaponUSMap );
PRECACHE_WEAPON_REGISTER( weapon_usmap );

//-----------------------------------------------------------------------------
// CWeaponGermanMap
//-----------------------------------------------------------------------------
class CWeaponGermanMap : public CWeaponUSMap
{
public:
	DECLARE_CLASS( CWeaponGermanMap, CWeaponUSMap );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponGermanMap() {}

private:
	CWeaponGermanMap( const CWeaponGermanMap & );	
};

//-----------------------------------------------------------------------------
// CWeaponGermanMap
//-----------------------------------------------------------------------------

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGermanMap, DT_WeaponGermanMap )

BEGIN_NETWORK_TABLE( CWeaponGermanMap, DT_WeaponGermanMap )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGermanMap )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_germanmap, CWeaponGermanMap );
PRECACHE_WEAPON_REGISTER( weapon_germanmap );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponUSMap::CWeaponUSMap( void )
{
	//m_iSkin = MAP_NONE;
}

void CWeaponUSMap::Spawn( void )
{
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Deploy the map model fire the deploy anim and show the map when that's done
//-----------------------------------------------------------------------------
bool CWeaponUSMap::Deploy()
{
#ifndef CLIENT_DLL
	char cMapName[MAX_MAP_NAME];
	Q_FileBase( STRING(gpGlobals->mapname), cMapName, sizeof(cMapName) );

	// Now compare the map name
	if( !strcmp( cMapName, "rnl_eglise" ) == true)
		SetSkin( MAP_EGLISE );
	else if( !strcmp( cMapName, "rnl_stcomedumont" ) == true)
		SetSkin( MAP_STCOMEDUMONT );
	else if( !strcmp( cMapName, "rnl_stemariedumont" ) == true)
		SetSkin( MAP_STEMARIEDUMONT );
	else if( !strcmp( cMapName, "rnl_carentan_causeway" ) == true)
		SetSkin( MAP_CARENTAN_CAUSEWAY );
	else if( !strcmp( cMapName, "rnl_carentan" ) == true)
		SetSkin( MAP_CARENTAN );
	else if( !strcmp( cMapName, "rnl_vierville" ) == true)
		SetSkin( MAP_VIERVILLE );
	else
		SetSkin( MAP_INOFFICIAL );
#endif
	SendWeaponAnim(ACT_VM_SHOULDERDRAW);
	
	CRnLPlayer *pPlayer = GetPlayerOwner();

	if( !pPlayer )
		return false;

#ifndef CLIENT_DLL
	//if( pPlayer->GetCurrentClass() == ALLIES_NCO || pPlayer->GetCurrentClass() == AXIS_OFFIZIER)
	{
		pPlayer->ShowViewPortPanel( PANEL_NCOMAP, true );
	}
#endif

	WeaponSound( SPECIAL1 );
	pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );

	//SetSkin( m_iSkin );

	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_DRAW );

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: Holster the map model fire the holster anim
// Send a "false" over to the client, so the map panel hides
//-----------------------------------------------------------------------------
bool CWeaponUSMap::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	WeaponSound( SPECIAL2 );
	SendWeaponAnim( ACT_VM_SHOULDERHOLSTER );

	CRnLPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer )
	{
		pPlayer->SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
#ifndef CLIENT_DLL
		pPlayer->ShowViewPortPanel( PANEL_NCOMAP, false );
#endif
	}

	return BaseClass::Holster();
}

//------------------------------------------------------------------------------
// Purpose : Update weapon
//------------------------------------------------------------------------------
void CWeaponUSMap::ItemPostFrame( void )
{
	if ( IsViewModelSequenceFinished() )
	{
		SendWeaponAnim( ACT_VM_SHOULDERIDLE );
	}

}

void CWeaponUSMap::GetControlPanelInfo(int nPanelIndex, const char *&pPanelName)
{
	pPanelName = NULL;//"vgui_map_screen";
}

//-----------------------------------------------------------------------------
// Purpose: Change the skin depending on the map
// Input  : skinNum - 
//-----------------------------------------------------------------------------
void CWeaponUSMap::SetSkin( int skinNum )
{
	// Now set the skin
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	if ( pViewModel == NULL )
		return;

	pViewModel->m_nSkin = skinNum;
}

//Appended here because including them back in caused some bad joo joo
#if defined( CLIENT_DLL )
	#define CWeaponPIRFists C_WeaponPIRFists
#endif

class CWeaponPIRFists : public CWeaponRnLBase
{
public:
	DECLARE_CLASS( CWeaponPIRFists, CWeaponRnLBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	
	CWeaponPIRFists(){}
	void NextFirePos(){}
	void PrevFirePos(){}
	void ItemPostFrame( void )
	{
		WeaponIdle();
	}
	virtual RnLWeaponID GetWeaponID( void ) const { return WEAPON_FISTS; }

private:
	CWeaponPIRFists( const CWeaponPIRFists & );
	void Fire( float flSpread );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPIRFists, DT_WeaponPIRFists )

BEGIN_NETWORK_TABLE(CWeaponPIRFists, DT_WeaponPIRFists)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponPIRFists )
END_PREDICTION_DATA()

acttable_t CWeaponPIRFists::m_acttable[] = 
{
	{ ACT_SPRINT,			ACT_DOD_SPRINT_AIM_SPADE,			false },

	{ ACT_DOD_STAND_AIM,	ACT_DOD_STAND_AIM_SPADE,			false },
	{ ACT_DOD_STAND_IDLE,	ACT_DOD_STAND_AIM_SPADE,			false },
	
	{ ACT_DOD_CROUCH_AIM,	ACT_DOD_CROUCH_AIM_SPADE,			false },
	{ ACT_DOD_CROUCH_IDLE,	ACT_DOD_CROUCH_AIM_SPADE,			false },

	{ ACT_DOD_CROUCHWALK_AIM,	ACT_DOD_CROUCHWALK_AIM_SPADE,	false },
	{ ACT_DOD_CROUCHWALK_IDLE,	ACT_DOD_CROUCHWALK_AIM_SPADE,	false },
	
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_AIM_SPADE,		false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_SPADE,			false },
	
	{ ACT_DOD_RUN_AIM,		ACT_DOD_RUN_AIM_SPADE,				false },
	{ ACT_DOD_RUN_IDLE,		ACT_DOD_RUN_AIM_SPADE,			false },
	
	{ ACT_DOD_WALK_AIM,		ACT_DOD_WALK_AIM_SPADE,			false },
	{ ACT_DOD_WALK_IDLE,	ACT_DOD_WALK_AIM_SPADE,			false },

	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_SPADE,	false },
	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PRIMARYATTACK_PRONE_SPADE, false },
};

IMPLEMENT_ACTTABLE(CWeaponPIRFists);

LINK_ENTITY_TO_CLASS( weapon_pirfists, CWeaponPIRFists );
PRECACHE_WEAPON_REGISTER( weapon_pirfists );