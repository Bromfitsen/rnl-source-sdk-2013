//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
//#include "player.h"
#include "cbase.h"
#include "rnl_fx_shared.h"
#include "weapon_rnlbaserifle.h"

#define M1GARAND_ROF 0.05f // semi automatic, very small ROF

#if defined( CLIENT_DLL )
	#define CWeaponM1Garand C_WeaponM1Garand
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
	#include "te_effect_dispatch.h"
	#include "smoke_trail.h"
	#include "explode.h"
#endif

ConVar garand_ironsightsx("garand_ironsightsx", "0.0", FCVAR_REPLICATED, "" );
ConVar garand_ironsightsy("garand_ironsightsy", "0.0", FCVAR_REPLICATED, "" );
ConVar garand_ironsightsz("garand_ironsightsz", "0.0", FCVAR_REPLICATED, "" );

ConVar garand_shoulderx("garand_shoulderx", "0.0", FCVAR_REPLICATED, "" );
ConVar garand_shouldery("garand_shouldery", "0.0", FCVAR_REPLICATED, "" );
ConVar garand_shoulderz("garand_shoulderz", "0.0", FCVAR_REPLICATED, "" );

class CWeaponM1Garand : public CWeaponRnLBaseRifle
{
public:
	DECLARE_CLASS( CWeaponM1Garand, CWeaponRnLBaseRifle );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif
	
	CWeaponM1Garand();
	~CWeaponM1Garand() override = default;

	void Precache( void ) override;
	
	bool PlayLastBulletSound( void ) override;
	void ItemPostFrame( void ) override;

	RnLWeaponID GetWeaponID( void ) const override { return WEAPON_M1GARAND; }

	const char* GetWorldModel(void) const override;

#ifdef CLIENT_DLL
	int	GetWorldModelIndex(void) override;

	Vector	GetIronsightsOffset() override { return Vector( garand_ironsightsx.GetFloat(), garand_ironsightsy.GetFloat(), garand_ironsightsz.GetFloat() ); }
	Vector	GetShoulderOffset() override { return Vector( garand_shoulderx.GetFloat(), garand_shouldery.GetFloat(), garand_shoulderz.GetFloat() ); }
#endif

private:

	CWeaponM1Garand( const CWeaponM1Garand & );
	CNetworkVar( int, m_iDeployedModelIndex );
};

static char* pGarandWithBayonetModel = "models/weapons/w_garand_bayonet.mdl";

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM1Garand, DT_WeaponM1Garand )

BEGIN_NETWORK_TABLE( CWeaponM1Garand, DT_WeaponM1Garand )
	PropModelIndex( PROPINFO(m_iDeployedModelIndex) ),
END_NETWORK_TABLE()

#ifdef GAME_DLL
	BEGIN_DATADESC( CWeaponM1Garand )
	END_DATADESC()
#elif defined(CLIENT_DLL)
	BEGIN_PREDICTION_DATA( CWeaponM1Garand )
		DEFINE_PRED_FIELD( m_iDeployedModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX ),
	END_PREDICTION_DATA()
#endif

acttable_t CWeaponM1Garand::m_acttable[] = 
{
	{ ACT_SPRINT,			ACT_DOD_SPRINT_IDLE_RIFLE,			false },

	{ ACT_DOD_STAND_AIM,	ACT_DOD_STAND_AIM_RIFLE,			false },
	{ ACT_DOD_STAND_IDLE,	ACT_DOD_STAND_IDLE_RIFLE,			false },
	
	{ ACT_DOD_CROUCH_AIM,	ACT_DOD_CROUCH_AIM_RIFLE,			false },
	{ ACT_DOD_CROUCH_IDLE,	ACT_DOD_CROUCH_IDLE_RIFLE,			false },

	{ ACT_DOD_CROUCHWALK_AIM,	ACT_DOD_CROUCHWALK_AIM_RIFLE,	false },
	{ ACT_DOD_CROUCHWALK_IDLE,	ACT_DOD_CROUCHWALK_IDLE_RIFLE,	false },
	
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_IDLE_RIFLE,		false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_RIFLE,			false },
	
	{ ACT_DOD_RUN_AIM,		ACT_DOD_RUN_AIM_RIFLE,				false },
	{ ACT_DOD_RUN_IDLE,		ACT_DOD_RUN_IDLE_RIFLE,			false },
	
	{ ACT_DOD_WALK_AIM,		ACT_DOD_WALK_AIM_RIFLE,			false },
	{ ACT_DOD_WALK_IDLE,	ACT_DOD_WALK_IDLE_RIFLE,			false },

	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_RIFLE,	false },
	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PRIMARYATTACK_PRONE_RIFLE, false },

	{ ACT_GESTURE_RELOAD,		ACT_DOD_RELOAD_RIFLE,			false },
	{ ACT_DOD_RELOAD_CROUCH,	ACT_DOD_RELOAD_CROUCH_RIFLE,	false },
	{ ACT_DOD_RELOAD_PRONE,		ACT_DOD_RELOAD_PRONE_RIFLE,	false },
};

IMPLEMENT_ACTTABLE(CWeaponM1Garand);

LINK_ENTITY_TO_CLASS( weapon_garand, CWeaponM1Garand );

PRECACHE_WEAPON_REGISTER( weapon_garand );

CWeaponM1Garand::CWeaponM1Garand()
{
}

void CWeaponM1Garand::Precache()
{
	BaseClass::Precache();

	m_iDeployedModelIndex	= CBaseEntity::PrecacheModel( pGarandWithBayonetModel );
}

#if defined(CLIENT_DLL)
int	CWeaponM1Garand::GetWorldModelIndex( void )
{
	if( IsBayonetDeployed() )
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
const char *CWeaponM1Garand::GetWorldModel( void ) const
{
	if( IsBayonetDeployed() )
	{
		return pGarandWithBayonetModel;
	}
	else
	{
		return BaseClass::GetWorldModel();
	}
}

bool CWeaponM1Garand::PlayLastBulletSound()
{
	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();

	EmitSound( filter, entindex(), "Weapon_Garand.Ping" );

	return 0;
}

//=====================================================================================// 
// CWeaponM1Garand::ItempostFrame()
// Purpose: Method called every frame. Check for buttons/keys pressed Checks reload 
// status and calls WeaponIdle()
//
//=====================================================================================//
 
void CWeaponM1Garand::ItemPostFrame()
{
    // Do we have a valid owner holding the weapon? 
    CRnLPlayer *pPlayer = GetPlayerOwner(); 
	if ( !pPlayer ) return; 

	HandleFoVTransitions();

	if( HandleReloadTransitions() )
	{
		return;
	}
	else if( ShouldHandleWeaponLimitations() )
	{
		WeaponIdle();
		return;
	}
	else if( HandleBayonetTransition() )
	{
		return;
	}
	else if( HandlePostureTransitions() )
	{
		WeaponIdle();
		return;
	}
	// if it's not, check what else has been requested
	else
	{
		//----------------------------------------------//
		// Did the player press the primary attack button in this frame?
		//----------------------------------------------//
		if ( pPlayer->m_afButtonPressed & IN_ATTACK )
		{
			if ( gpGlobals->curtime >= m_flNextPrimaryAttack )
			{
				PrimaryAttack(); 
			}
		}
		// Attach the bayonett for now - Stefan
		if ( pPlayer->m_nButtons & IN_ATTACK2 )
		{
			if ( gpGlobals->curtime >= m_flNextSecondaryAttack )
			{
				SecondaryAttack( true );
			}
		}

		// Attach the bayonett
		if (pPlayer->m_nButtons & IN_ATTACK3)
		{
			if (gpGlobals->curtime >= m_flNextPrimaryAttack &&
				gpGlobals->curtime >= m_flNextSecondaryAttack )
			{
				BayonetTransition(1);
			}
		}

		// Reload
		if (pPlayer->m_nButtons & IN_RELOAD  )
		{
			if( gpGlobals->curtime >= m_flNextPrimaryAttack )
			{
				ReloadTransition( 1 );
			}
		}


		WeaponIdle();
	}
}