//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
//#include "player.h"

#include "cbase.h"
#include "weapon_rnlbasemachinegun.h"
#include "rnl_fx_shared.h"


#if defined( CLIENT_DLL )

	#define CWeaponBrowning C_WeaponBrowning
	#include "c_rnl_player.h"
	#include "c_te_effect_dispatch.h"
	#include "fx.h"

#else

	#include "rnl_player.h"

#endif

ConVar browning_standoffset("browning_standoffset", "32.0", FCVAR_REPLICATED, "" );

ConVar browning_ironsightsx("browning_ironsightsx", "0.0", FCVAR_REPLICATED, "" );
ConVar browning_ironsightsy("browning_ironsightsy", "0.0", FCVAR_REPLICATED, "" );
ConVar browning_ironsightsz("browning_ironsightsz", "0.0", FCVAR_REPLICATED, "" );

ConVar browning_shoulderx("browning_shoulderx", "0.0", FCVAR_REPLICATED, "" );
ConVar browning_shouldery("browning_shouldery", "0.0", FCVAR_REPLICATED, "" );
ConVar browning_shoulderz("browning_shoulderz", "0.0", FCVAR_REPLICATED, "" );

class CWeaponBrowning : public CWeaponRnLBaseMachineGun
{
public:
	DECLARE_CLASS( CWeaponBrowning, CWeaponRnLBaseMachineGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponBrowning();
	void Precache();
	void ItemPostFrame();

	virtual float		GetStandOffset() { return browning_standoffset.GetFloat(); }
	
	virtual RnLWeaponID GetWeaponID( void ) const		{ return WEAPON_BROWNING; }

#ifdef CLIENT_DLL
	Vector	GetIronsightsOffset(){ return Vector( browning_ironsightsx.GetFloat(), browning_ironsightsy.GetFloat(), browning_ironsightsz.GetFloat() ); }
	Vector	GetShoulderOffset(){ return Vector( browning_shoulderx.GetFloat(), browning_shouldery.GetFloat(), browning_shoulderz.GetFloat() ); }
#endif

private:

	CWeaponBrowning( const CWeaponBrowning & );

	// Weapon art
	CNetworkVar( int, m_iBarrelHeat );
	CNetworkVar( float, m_flLastHeatCheck );
	CNetworkVar( bool, m_bBarrelVeryHot );
	CNetworkVar( float, m_flNextHeatSound );

#ifdef CLIENT_DLL
	float m_flNextHeatSmoke;
#endif

	void Fire( float flSpread );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBrowning, DT_WeaponBrowning )

BEGIN_NETWORK_TABLE( CWeaponBrowning, DT_WeaponBrowning )
#ifndef CLIENT_DLL	
	SendPropInt( SENDINFO( m_iBarrelHeat ) ),
	SendPropFloat( SENDINFO( m_flLastHeatCheck ) ),
	SendPropBool( SENDINFO ( m_bBarrelVeryHot ) ),
	SendPropFloat( SENDINFO( m_flNextHeatSound ) ),
#else
	RecvPropInt( RECVINFO( m_iBarrelHeat ) ),
	RecvPropFloat( RECVINFO( m_flLastHeatCheck ) ),
	RecvPropBool( RECVINFO( m_bBarrelVeryHot ) ),
	RecvPropFloat( RECVINFO( m_flNextHeatSound ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponBrowning )
#ifdef CLIENT_DLL
	DEFINE_PRED_FIELD( m_iBarrelHeat, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flLastHeatCheck, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bBarrelVeryHot, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flNextHeatSound, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
#endif
END_PREDICTION_DATA()

acttable_t CWeaponBrowning::m_acttable[] = 
{
	{ ACT_SPRINT,			ACT_DOD_SPRINT_IDLE_MG,			false },
	{ ACT_IDLE,				ACT_DOD_STAND_AIM_MG,			false },
	{ ACT_CROUCHIDLE,		ACT_DOD_CROUCH_AIM_MG,			false },
	{ ACT_RUN_CROUCH,		ACT_DOD_CROUCHWALK_AIM_MG,	false },
	{ ACT_WALK_CROUCH,		ACT_DOD_CROUCHWALK_AIM_MG,	false },
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_IDLE_MG,		false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_MG,			false },
	{ ACT_RUN,				ACT_DOD_RUN_AIM_MG,				false },
	{ ACT_WALK,				ACT_DOD_WALK_AIM_MG,			false },
	
	{ ACT_DOD_DEPLOYED,		ACT_DOD_DEPLOY_MG,			false },
	{ ACT_DOD_PRONE_DEPLOYED,		ACT_DOD_PRONE_DEPLOY_MG,			false },

	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_MG,	false },
	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PRIMARYATTACK_PRONE_MG, false },

	{ ACT_GESTURE_RELOAD,		ACT_DOD_RELOAD_DEPLOYED_MG,			false },
	{ ACT_DOD_RELOAD_CROUCH,	ACT_DOD_RELOAD_DEPLOYED_MG,	false },
	{ ACT_DOD_RELOAD_PRONE,		ACT_DOD_RELOAD_PRONE_DEPLOYED_MG,	false },
};

//
//acttable_t CWeaponBrowning::m_acttable[] = 
//{
//	{ ACT_SPRINT,			ACT_DOD_SPRINT_IDLE_MG,			false },
//
//	{ ACT_DOD_STAND_AIM,	ACT_DOD_STAND_AIM_MG,			false },
//	{ ACT_DOD_STAND_IDLE,	ACT_DOD_STAND_IDLE_MG,			false },
//	
//	{ ACT_DOD_CROUCH_AIM,	ACT_DOD_CROUCH_AIM_MG,			false },
//	{ ACT_DOD_CROUCH_IDLE,	ACT_DOD_CROUCH_IDLE_MG,			false },
//
//	{ ACT_DOD_CROUCHWALK_AIM,	ACT_DOD_CROUCHWALK_AIM_MG,	false },
//	{ ACT_DOD_CROUCHWALK_IDLE,	ACT_DOD_CROUCHWALK_IDLE_MG,	false },
//	
//	{ ACT_PRONE,			ACT_DOD_PRONEWALK_IDLE_MG,		false },
//	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_MG,			false },
//	
//	{ ACT_DOD_RUN_AIM,		ACT_DOD_RUN_AIM_MG,				false },
//	{ ACT_DOD_RUN_IDLE,		ACT_DOD_RUN_IDLE_MG,			false },
//	
//	{ ACT_DOD_WALK_AIM,		ACT_DOD_WALK_AIM_MG,			false },
//	{ ACT_DOD_WALK_IDLE,	ACT_DOD_WALK_IDLE_MG,			false },
//
//	{ ACT_DOD_DEPLOYED,		ACT_DOD_DEPLOY_MG,			false },
//	{ ACT_DOD_PRONE_DEPLOYED,		ACT_DOD_PRONE_DEPLOY_MG,			false },
//
//	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_MG,	false },
//	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PRIMARYATTACK_PRONE_MG, false },
//
//	{ ACT_GESTURE_RELOAD,		ACT_DOD_RELOAD_DEPLOYED_MG,			false },
//	{ ACT_DOD_RELOAD_CROUCH,	ACT_DOD_RELOAD_DEPLOYED_MG,	false },
//	{ ACT_DOD_RELOAD_PRONE,		ACT_DOD_RELOAD_PRONE_DEPLOYED_MG,	false },
//};

IMPLEMENT_ACTTABLE(CWeaponBrowning);

LINK_ENTITY_TO_CLASS( weapon_browning, CWeaponBrowning );
PRECACHE_WEAPON_REGISTER( weapon_browning );

CWeaponBrowning::CWeaponBrowning()
{
	m_iBarrelHeat = 0;
	m_flLastHeatCheck = gpGlobals->curtime;
	m_bBarrelVeryHot = false;
}

void CWeaponBrowning::Precache()
{
	BaseClass::Precache();
}

//======================================================================================// 
//																						//
//							CWeaponBrowning::ItempostFrame()							//
//	Purpose: Method called every frame. Check for buttons/keys pressed Checks reload	//
//							status and calls WeaponIdle()								//
//																						//
//======================================================================================//
void CWeaponBrowning::ItemPostFrame()
{
	// Do we have a valid owner holding the weapon? 
	CRnLPlayer *pPlayer = GetPlayerOwner(); 
	const CRnLWeaponInfo &pWeaponInfo = GetRnLWpnData();

	if ( !pPlayer ) return; 

	HandleFoVTransitions();
	HandleShotDelay();

	if( HandleReloadTransitions() )
	{
		return;
	}
	else if( !IsDeployed() && ShouldHandleWeaponLimitations() )
	{
		WeaponIdle();
		return;
	}
	else if( CheckUnDeploy() )
		return;

	bool canDoActivity = ( m_flNextPrimaryAttack <= gpGlobals->curtime ) ? true : false;
	bool bMagEmpty = ( Clip1() > 0 ) ? false : true;
	bool bUnderWater = ( pPlayer->GetWaterLevel() == 3 ) ? true : false;
 
	if ( pPlayer->m_nButtons & IN_ATTACK && IsDeployed() && canDoActivity && !m_bBarrelVeryHot )
	{
		PrimaryAttack();

		SetLastShotTime( gpGlobals->curtime, 1.0f );
		if ( !bMagEmpty && !bUnderWater )
			m_iBarrelHeat++;
#ifndef CLIENT_DLL
		//HandleSuppression();
#endif 
	}
	else if ( ((pPlayer->m_nButtons & IN_ATTACK2) || (pPlayer->m_nButtons & IN_IRONSIGHTS)) && canDoActivity )
	{ 
		ToggleDeploy();
	}
	else if (pPlayer->m_nButtons & IN_RELOAD && canDoActivity && IsDeployed() )
	{ 
		ReloadTransition( 1 );
	}

	WeaponIdle();

	if ( !bUnderWater )
	{
		// Check if barrel overheated
		if ( m_iBarrelHeat >= pWeaponInfo.m_iHeatIndex && !m_bBarrelVeryHot )
		{
			m_flNextHeatSound = gpGlobals->curtime;
			m_bBarrelVeryHot = true;
		}
		// Wait for cool down by 30%
		else if ( m_iBarrelHeat <= pWeaponInfo.m_iHeatIndex * 0.7 && m_bBarrelVeryHot )
		{
			m_bBarrelVeryHot = false;
		}
		// Emit heat sound
		else if( m_iBarrelHeat > pWeaponInfo.m_iHeatIndex * 0.4 )
		{
			if ( m_flNextHeatSound <= gpGlobals->curtime )
			{
				CPASAttenuationFilter filter( this );
				filter.UsePredictionRules();
			
				if ( m_bBarrelVeryHot )
					EmitSound( filter, entindex(), "Weapon_30cal.VeryHot" );
				else
					EmitSound( filter, entindex(), "Weapon_30cal.Hot" );
	
				m_flNextHeatSound = gpGlobals->curtime + 5.7;
			}

	#ifdef CLIENT_DLL 
			if ( IsDeployed() && m_flNextHeatSmoke <= gpGlobals->curtime )
			{
				// Emit smoke
				Vector vecDir;
				QAngle playerAngles = pPlayer->GetWeaponAngle();
				//playerAngles[PITCH] = 0;
				AngleVectors( playerAngles, &vecDir );

				Vector vOrigin = pPlayer->Weapon_ShootPosition() + vecDir * random->RandomInt(30,40);
				vOrigin.z -= 3;

				int iColor = random->RandomInt(190,220);
				float fScale = random->RandomFloat(0.005f, 0.01f);
				color32 color;
				color.r = iColor;
				color.g = iColor;
				color.b = iColor;
				color.a = iColor;
				QAngle angles( 0.0, 0.0, 1.0 );
				FX_Smoke( vOrigin, angles, m_iBarrelHeat * fScale, m_iBarrelHeat * 0.1, (unsigned char *)&color, 200 );

				m_flNextHeatSmoke = gpGlobals->curtime + 0.2;
			}
	#endif
		}
			
		// Barrel cools down 2 unit per second
		if ( m_flLastHeatCheck + 0.5 <= gpGlobals->curtime )
		{
			m_flLastHeatCheck = gpGlobals->curtime;
			//DevMsg(2, "Heat value: %d\n", m_iBarrelHeat);
			if (m_iBarrelHeat > 0) 
				m_iBarrelHeat--;
		}
	}
	else
	{
		// Barrel + Water = Cool
		m_iBarrelHeat = 0;
	}
}