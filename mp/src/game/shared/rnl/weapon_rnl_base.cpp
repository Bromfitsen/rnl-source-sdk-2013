//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "in_buttons.h"
#include "takedamageinfo.h"
#include "weapon_rnl_base.h"
#include "ammodef.h"


#if defined( CLIENT_DLL )

	#include "c_rnl_player.h"

#else

	#include "rnl_player.h"

#endif

// ----------------------------------------------------------------------------- //
// CWeaponRnLBase tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRnLBase, DT_WeaponRnLBase )

BEGIN_NETWORK_TABLE( CWeaponRnLBase, DT_WeaponRnLBase )
#ifdef CLIENT_DLL
  RecvPropInt( RECVINFO( m_iWeaponAnimationState ) ),
#else
	// world weapon models have no animations
  	SendPropExclude( "DT_AnimTimeMustBeFirst", "m_flAnimTime" ),
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseAnimating", "m_nNewSequenceParity" ),
	SendPropExclude( "DT_BaseAnimating", "m_nResetEventsParity" ),
	SendPropExclude( "DT_BaseEntity", "m_flSimulationTime" ),

	SendPropInt( SENDINFO( m_iWeaponAnimationState ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponRnLBase )
	DEFINE_PRED_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_rnl_base, CWeaponRnLBase );


#ifdef GAME_DLL

	BEGIN_DATADESC( CWeaponRnLBase )

		// New weapon Think and Touch Functions go here..

	END_DATADESC()

#endif

#ifdef CLIENT_DLL
bool CWeaponRnLBase::ShouldPredict()
{
	if ( GetOwner() && GetOwner() == C_BasePlayer::GetLocalPlayer())
		return true;

	return BaseClass::ShouldPredict();
}
#endif
// ----------------------------------------------------------------------------- //
// CWeaponCSBase implementation. 
// ----------------------------------------------------------------------------- //
CWeaponRnLBase::CWeaponRnLBase()
{
	SetPredictionEligible( true );

	AddSolidFlags( FSOLID_TRIGGER ); // Nothing collides with these but it gets touches.
}

CWeaponRnLBase::~CWeaponRnLBase()
{
}

void CWeaponRnLBase::Precache( void )
{
	BaseClass::Precache();
	CBaseEntity::PrecacheModel( GetRnLWpnData().m_szClipModel );
}

const CRnLWeaponInfo &CWeaponRnLBase::GetRnLWpnData() const
{
	const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
	const CRnLWeaponInfo *pRnLInfo;

	#ifdef _DEBUG
		pRnLInfo = dynamic_cast< const CRnLWeaponInfo* >( pWeaponInfo );
		Assert( pRnLInfo );
	#else
		pRnLInfo = static_cast< const CRnLWeaponInfo* >( pWeaponInfo );
	#endif

	return *pRnLInfo;
}

bool CWeaponRnLBase::CanPickup( CRnLPlayer* pPlayer )
{
	if( pPlayer == NULL || GetRnLWpnData().m_iTeamAssociation == TEAM_NONE )
		return true;
	else
		return GetRnLWpnData().m_iTeamAssociation == pPlayer->GetTeamNumber();
}

bool CWeaponRnLBase::CanHolster( void )
{
	return BaseClass::CanHolster() && ((m_iWeaponAnimationState != WEAPON_ANIMATION_HOLSTER && m_iWeaponAnimationState != WEAPON_ANIMATION_DRAW) || IsSequenceFinished() );
}

bool CWeaponRnLBase::PlayEmptySound()
{
	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();

	EmitSound( filter, entindex(), "Default.ClipEmpty_Rifle" );
	
	return 0;
}

bool CWeaponRnLBase::PlayLastBulletSound()
{
	/*
	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();

	EmitSound( filter, entindex(), "Weapon_Garand.Ping" );
	*/
	return 0;
}

CRnLPlayer* CWeaponRnLBase::GetPlayerOwner() const
{
	return dynamic_cast< CRnLPlayer* >( GetOwner() );
}


bool CWeaponRnLBase::ShouldUseFreeAim()
{
	CRnLPlayer* pRnLPlayer = GetPlayerOwner();

	if( !pRnLPlayer )
		return false;

	return true;//(pRnLPlayer->GetWeaponPosture() != WEAPON_POSTURE_SUPERSIGHTS );
}

bool CWeaponRnLBase::ShouldHandleWeaponLimitations( void )
{
	CRnLPlayer* pRnLPlayer = GetPlayerOwner();

	if( !pRnLPlayer )
	{
		return false;
	}

	if( pRnLPlayer->GetMovementPosture() == MOVEMENT_POSTURE_CLIMBING && ( pRnLPlayer->GetClimbheight() >= LOWEST_CONTROLLED_CLIMB ) )
	{
		return true;
	}

	if( !(pRnLPlayer->GetFlags() & FL_ONGROUND) )
	{
		return true;
	}

	if ( pRnLPlayer->IsProne() )
	{
		float flSpeed = pRnLPlayer->GetLocalVelocity().Length2D();
		if( flSpeed > 0.5f && pRnLPlayer->IsProne() )
		{
			ReturnToDefaultPosture();

			if( GetAnimationState() != WEAPON_ANIMATION_PRONE_IDLE && GetAnimationState() != WEAPON_ANIMATION_PRONE_ENTER  
				&& GetAnimationState() != WEAPON_ANIMATION_PRONE_LEAVE )
			{
				HandleViewAnimation( WEAPON_ANIMATION_PRONE_ENTER );

				m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
			}
			else if( GetAnimationState() == WEAPON_ANIMATION_PRONE_IDLE )
			{
				m_flPlaybackRate = flSpeed / 25;
			}
			return true;
		}
		else if( GetAnimationState() == WEAPON_ANIMATION_PRONE_IDLE ) 
		{
			HandleViewAnimation( WEAPON_ANIMATION_PRONE_LEAVE );

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
			return true;
		}
	}

	if( pRnLPlayer->IsSprinting()  )
	{
		if( GetAnimationState() != WEAPON_ANIMATION_SPRINT_IDLE && GetAnimationState() != WEAPON_ANIMATION_SPRINT_ENTER && GetAnimationState() != WEAPON_ANIMATION_SPRINT_LEAVE )
		{
			HandleViewAnimation( WEAPON_ANIMATION_SPRINT_ENTER );

			m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
		}
		return true;
	}
	else if( GetAnimationState() == WEAPON_ANIMATION_SPRINT_IDLE )
	{
		HandleViewAnimation( WEAPON_ANIMATION_SPRINT_LEAVE );

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
		return true;
	}

	if(  !IsViewModelSequenceFinished() && (GetAnimationState() == WEAPON_ANIMATION_SPRINT_LEAVE || GetAnimationState() == WEAPON_ANIMATION_PRONE_LEAVE ) )
		return true;

	return false;
}
//-----------------------------------------------------------------------------
// Purpose: Drop/throw the weapon with the given velocity.
//-----------------------------------------------------------------------------
void CWeaponRnLBase::Drop( const Vector &vecVelocity )
{
#if !defined( CLIENT_DLL )

	CRnLPlayer *pPlayer = ToRnLPlayer( GetOwner() );
	if( pPlayer )
	{
		pPlayer->SetFOV( pPlayer, 0 );
	}	

	// Once somebody drops a gun, it's fair game for removal when/if
	// a game_weapon_manager does a cleanup on surplus weapons in the
	// world.
	SetRemoveable( true );

	//If it was dropped then there's no need to respawn it.
	AddSpawnFlags( SF_NORESPAWN );

	StopAnimation();
	StopFollowingEntity( );
	SetMoveType( MOVETYPE_FLYGRAVITY );
	// clear follow stuff, setup for collision
	SetGravity(1.0);
	m_iState = WEAPON_NOT_CARRIED;
	RemoveEffects( EF_NODRAW );
	FallInit();
	SetGroundEntity( NULL );
	// in RnL, remove weapons after such a time
	SetThink( &CWeaponRnLBase::SUB_FadeOut );
	SetNextThink( gpGlobals->curtime + 30.0f );
	SetTouch(NULL);

	IPhysicsObject *pObj = VPhysicsGetObject();
	if ( pObj != NULL )
	{
		AngularImpulse	angImp( 200, 200, 200 );
		pObj->AddVelocity( &vecVelocity, &angImp );
		// set a very low mass to see weapons behave realistically
		pObj->SetMass( 0.3f );
	}
	else
	{
		SetAbsVelocity( vecVelocity );
	}

	SetOwnerEntity( NULL );
	SetOwner( NULL );
#endif
}

#ifdef GAME_DLL

void CWeaponRnLBase::SendReloadEvents( bool bMidReload )
{
	CRnLPlayer *pPlayer = dynamic_cast< CRnLPlayer* >( GetOwner() );
	if ( !pPlayer )
		return;

	/*
	// Send a message to any clients that have this entity to play the reload.
	CPASFilter filter( pPlayer->GetAbsOrigin() );
	filter.RemoveRecipient( pPlayer );

	UserMessageBegin( filter, "ReloadEffect" );
		WRITE_SHORT( pPlayer->entindex() );
	MessageEnd();
	*/
	// play relevant animation
	if( bMidReload )
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_MIDRELOAD );
	else
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );
}

#endif