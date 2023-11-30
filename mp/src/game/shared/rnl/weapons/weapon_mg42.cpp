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

	#define CWeaponMG42 C_WeaponMG42
	#include "c_rnl_player.h"
	#include "c_te_effect_dispatch.h"
	#include "fx.h"

#else

	#include "rnl_player.h"

#endif

ConVar mg42_standoffset("mg42_standoffset", "32.0", FCVAR_REPLICATED, "" );

ConVar mg42_ironsightsx("mg42_ironsightsx", "0.0", FCVAR_REPLICATED, "" );
ConVar mg42_ironsightsy("mg42_ironsightsy", "0.0", FCVAR_REPLICATED, "" );
ConVar mg42_ironsightsz("mg42_ironsightsz", "0.0", FCVAR_REPLICATED, "" );

ConVar mg42_shoulderx("mg42_shoulderx", "0.0", FCVAR_REPLICATED, "" );
ConVar mg42_shouldery("mg42_shouldery", "0.0", FCVAR_REPLICATED, "" );
ConVar mg42_shoulderz("mg42_shoulderz", "0.0", FCVAR_REPLICATED, "" );


class CWeaponMG42 : public CWeaponRnLBaseMachineGun
{
public:
	DECLARE_CLASS( CWeaponMG42, CWeaponRnLBaseMachineGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponMG42();
	~CWeaponMG42() = default;

	void Precache() OVERRIDE;
	void ItemPostFrame() OVERRIDE;
	const char	*GetWorldModel( void ) const OVERRIDE;

	float		GetStandOffset() OVERRIDE { return mg42_standoffset.GetFloat(); }
	
	RnLWeaponID GetWeaponID( void ) const OVERRIDE { return WEAPON_MG42; }

#ifdef CLIENT_DLL
	int	GetWorldModelIndex(void) OVERRIDE;

	Vector	GetIronsightsOffset() OVERRIDE { return Vector( mg42_ironsightsx.GetFloat(), mg42_ironsightsy.GetFloat(), mg42_ironsightsz.GetFloat() ); }
	Vector	GetShoulderOffset() OVERRIDE { return Vector( mg42_shoulderx.GetFloat(), mg42_shouldery.GetFloat(), mg42_shoulderz.GetFloat() ); }
#endif

private:

	CWeaponMG42( const CWeaponMG42 & );

	// Weapon art
	CNetworkVar( int, m_iDeployedModelIndex );
	CNetworkVar( int, m_iBarrelHeat );
	CNetworkVar( float, m_flLastHeatCheck );
	CNetworkVar( bool, m_bBarrelVeryHot );
	CNetworkVar( float, m_flNextHeatSound );

#ifdef CLIENT_DLL
	float m_flNextHeatSmoke;
#endif
};

static char* pDeployedMG42Model = "models/weapons/w_mg42_deployed.mdl";

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMG42, DT_WeaponMG42 )

BEGIN_NETWORK_TABLE( CWeaponMG42, DT_WeaponMG42 )
	PropModelIndex( PROPINFO(m_iDeployedModelIndex) ),
	PropInt(PROPINFO( m_iBarrelHeat ) ),
	PropTime(PROPINFO( m_flLastHeatCheck ) ),
	PropBool(PROPINFO( m_bBarrelVeryHot ) ),
	PropTime(PROPINFO( m_flNextHeatSound ) ),
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponMG42 )
	DEFINE_PRED_FIELD( m_iDeployedModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX ),
	DEFINE_PRED_FIELD( m_iBarrelHeat, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD_TOL(m_flLastHeatCheck, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
	DEFINE_PRED_FIELD( m_bBarrelVeryHot, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD_TOL(m_flNextHeatSound, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
END_PREDICTION_DATA()
#endif

acttable_t CWeaponMG42::m_acttable[] = 
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

//acttable_t CWeaponMG42::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(CWeaponMG42);

LINK_ENTITY_TO_CLASS( weapon_mg42, CWeaponMG42 );
PRECACHE_WEAPON_REGISTER( weapon_mg42 );

CWeaponMG42::CWeaponMG42()
{
	m_iBarrelHeat = 0;
	m_flLastHeatCheck = gpGlobals->curtime;
	m_bBarrelVeryHot = false;
}

void CWeaponMG42::Precache()
{
	BaseClass::Precache();

	m_iDeployedModelIndex	= CBaseEntity::PrecacheModel( pDeployedMG42Model );
}

#if defined(CLIENT_DLL)
int	CWeaponMG42::GetWorldModelIndex( void )
{
	if( IsDeployed() )
	{
		return m_iDeployedModelIndex;
	}
	else
	{
		return m_iWorldModelIndex;
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CWeaponMG42::GetWorldModel( void ) const
{
	if( IsDeployed() )
	{
		return pDeployedMG42Model;
	}
	else
	{
		return BaseClass::GetWorldModel();
	}
}

//======================================================================================// 
//																						//
//							CWeaponMG42::ItempostFrame()							//
//	Purpose: Method called every frame. Check for buttons/keys pressed Checks reload	//
//							status and calls WeaponIdle()								//
//																						//
//======================================================================================//
void CWeaponMG42::ItemPostFrame()
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

		SetLastShotTime( gpGlobals->curtime, 0.75f );
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
					EmitSound( filter, entindex(), "Weapon_Mg42.VeryHot" );	
				else
					EmitSound( filter, entindex(), "Weapon_Mg42.Hot" );
	
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
			
		// Barrel cools down 4 unit per second
		if ( m_flLastHeatCheck + 0.25 <= gpGlobals->curtime )
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