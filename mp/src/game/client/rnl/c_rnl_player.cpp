//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_rnl_player.h"
#include "weapon_rnl_base.h"
#include "c_basetempentity.h"
#include "iclientvehicle.h"
#include "prediction.h"
#include "view.h"
#include "iviewrender.h"
#include "ivieweffects.h"
#include "filesystem.h" // cjd @add
#include "c_playerresource.h"	// cjd @add
#include <vgui/ILocalize.h> // cjd @add
#include "hud_basechat.h"	// cjd @add
#include "in_buttons.h"
#include "iinput.h"
#include "weapon_rnlbasemachinegun.h"

class CHudChat;		// cjd @add

//
#include "view_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if defined( CRnLPlayer )
	#undef CRnLPlayer
#endif

// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class C_TEPlayerAnimEvent : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEPlayerAnimEvent, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	virtual void PostDataUpdate( DataUpdateType_t updateType )
	{
		// Create the effect.
		C_RnLPlayer *pPlayer = dynamic_cast< C_RnLPlayer* >( m_hPlayer.Get() );
		if ( pPlayer && !pPlayer->IsDormant() )
		{
			pPlayer->DoAnimationEvent( (PlayerAnimEvent_t)m_iEvent.Get(), m_nData );
		}	
	}

public:
	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_CLIENTCLASS_EVENT( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent, CTEPlayerAnimEvent );

BEGIN_RECV_TABLE_NOBASE( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_iEvent ) ),
	RecvPropInt( RECVINFO( m_nData ) )
END_RECV_TABLE()

BEGIN_RECV_TABLE_NOBASE( C_RnLPlayer, DT_RnLLocalPlayerExclusive )
	RecvPropInt( RECVINFO( m_iShotsFired ) ),
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),

	RecvPropInt( RECVINFO( m_iPreviousSquadNumber ) ),
	RecvPropInt( RECVINFO( m_iPreviousKitNumber ) ),
END_RECV_TABLE()

BEGIN_RECV_TABLE_NOBASE( C_RnLPlayer, DT_RnLNonLocalPlayerExclusive )
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),
	RecvPropQAngles( RECVINFO( m_angWeaponAngle ) ),
	//Tony; spawn interp.
	RecvPropBool( RECVINFO( m_bSpawnInterpCounter) ),
END_RECV_TABLE()

IMPLEMENT_CLIENTCLASS_DT( C_RnLPlayer, DT_RnLPlayer, CRnLPlayer )
	RecvPropDataTable( "rnllocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_RnLLocalPlayerExclusive) ),
	RecvPropDataTable( "rnlnonlocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_RnLNonLocalPlayerExclusive) ),

	RecvPropDataTable( RECVINFO_DT(m_RnLLocal),0, &REFERENCE_RECV_TABLE(DT_RnLLocal) ),

	RecvPropFloat( RECVINFO( m_angEyeAngles[0] ) ),
	RecvPropFloat( RECVINFO( m_angEyeAngles[1] ) ),
	RecvPropInt( RECVINFO( m_iThrowGrenadeCounter ) ),
	RecvPropEHandle( RECVINFO( m_hRagdoll ) ),
	RecvPropEHandle( RECVINFO( m_hKnockDownRagdoll ) ),

	RecvPropInt( RECVINFO( m_iSquadNumber ) ),
	RecvPropInt( RECVINFO( m_iKitNumber ) ),

	RecvPropInt( RECVINFO( m_iClimbheight ) ),
	RecvPropInt( RECVINFO( m_nWeaponPosture ) ),
	RecvPropVector( RECVINFO( m_vecLeanOffset ) ),
	RecvPropFloat( RECVINFO( m_flBodyHeight ) ),
	RecvPropFloat( RECVINFO( m_flDeathViewTime ) ),

	RecvPropInt(RECVINFO(m_nMovementPosture)),
	RecvPropInt(RECVINFO(m_nMovementPostureFrom)),

END_RECV_TABLE()

BEGIN_PREDICTION_DATA(C_RnLPlayer)
	DEFINE_PRED_TYPEDESCRIPTION(m_RnLLocal, C_RnLPlayerLocalData),
	DEFINE_PRED_FIELD(m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK),
	DEFINE_PRED_FIELD(m_iShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_nWeaponPosture, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_vecLeanOffset, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),

	DEFINE_PRED_FIELD(m_nMovementPosture, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_nMovementPostureFrom, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()

class C_RnLRagdoll : public C_BaseAnimatingOverlay
{
public:
	DECLARE_CLASS( C_RnLRagdoll, C_BaseAnimatingOverlay );
	DECLARE_CLIENTCLASS();

	C_RnLRagdoll();
	~C_RnLRagdoll();

	virtual void OnDataChanged( DataUpdateType_t type );

	IRagdoll* GetIRagdoll() const;

	void ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName );
	void UpdateOnRemove(void);

private:

	C_RnLRagdoll( const C_RnLRagdoll & ) {}

	void Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity );

	void CreateRagdoll();

private:

	EHANDLE	m_hPlayer;
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

C_RnLRagdoll* g_pLocalRagdoll = NULL;

IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_RnLRagdoll, DT_RnLRagdoll, CRnLRagdoll )
	RecvPropVector( RECVINFO(m_vecRagdollOrigin) ),
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_nModelIndex ) ),
	RecvPropInt( RECVINFO(m_nForceBone) ),
	RecvPropVector( RECVINFO(m_vecForce) ),
	RecvPropVector( RECVINFO( m_vecRagdollVelocity ) ),

	//RecvPropInt( RECVINFO( m_iTeamNumber ) ),
	RecvPropInt		(RECVINFO(m_nSkin)),
	RecvPropInt		(RECVINFO(m_nBody)),
END_RECV_TABLE()

C_RnLRagdoll::C_RnLRagdoll()
{
}

C_RnLRagdoll::~C_RnLRagdoll()
{
	if( g_pLocalRagdoll == this )
		g_pLocalRagdoll = NULL;

	PhysCleanupFrictionSounds( this );

	if (m_hPlayer)
	{
		m_hPlayer->CreateModelInstance();
	}
}

void C_RnLRagdoll::Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity )
{
	if ( !pSourceEntity )
		return;
	
	VarMapping_t *pSrc = pSourceEntity->GetVarMapping();
	VarMapping_t *pDest = GetVarMapping();
    	
	// Find all the VarMapEntry_t's that represent the same variable.
	for ( int i = 0; i < pDest->m_Entries.Count(); i++ )
	{
		VarMapEntry_t *pDestEntry = &pDest->m_Entries[i];
		for ( int j=0; j < pSrc->m_Entries.Count(); j++ )
		{
			VarMapEntry_t *pSrcEntry = &pSrc->m_Entries[j];
			if ( !Q_strcmp( pSrcEntry->watcher->GetDebugName(),
				pDestEntry->watcher->GetDebugName() ) )
			{
				pDestEntry->watcher->Copy( pSrcEntry->watcher );
				break;
			}
		}
	}
}

void C_RnLRagdoll::ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

	if( !pPhysicsObject )
		return;

	Vector dir = pTrace->endpos - pTrace->startpos;

	if ( iDamageType == DMG_BLAST )
	{
		dir *= 200;  // adjust impact strenght

		// apply force at object mass center
		pPhysicsObject->ApplyForceCenter( dir );
	}
	else
	{
		Vector hitpos;  

		VectorMA( pTrace->startpos, pTrace->fraction, dir, hitpos );
		VectorNormalize( dir );

		dir *= 200;  // adjust impact strenght

		// apply force where we hit it
		pPhysicsObject->ApplyForceOffset( dir, hitpos );

		//Tony; throw in some bleeds! - just use a generic value for damage.
		TraceBleed( 90, dir, pTrace, iDamageType );
	}

	m_pRagdoll->ResetRagdollSleepAfterTime();
}

void C_RnLRagdoll::CreateRagdoll()
{
	// First, initialize all our data. If we have the player's entity on our client,
	// then we can make ourselves start out exactly where the player is.
	C_RnLPlayer *pPlayer = dynamic_cast< C_RnLPlayer* >( m_hPlayer.Get() );

	if ( pPlayer && !pPlayer->IsDormant() )
	{
		// move my current model instance to the ragdoll's so decals are preserved.
		pPlayer->SnatchModelInstance( this );

		VarMapping_t *varMap = GetVarMapping();

		// Copy all the interpolated vars from the player entity.
		// The entity uses the interpolated history to get bone velocity.
		bool bRemotePlayer = (pPlayer != C_BasePlayer::GetLocalPlayer());			
		if ( bRemotePlayer )
		{
			Interp_Copy( pPlayer );

			SetAbsAngles( pPlayer->GetRenderAngles() );
			GetRotationInterpolator().Reset();

			m_flAnimTime = pPlayer->m_flAnimTime;
			SetSequence( pPlayer->GetSequence() );
			m_flPlaybackRate = pPlayer->GetPlaybackRate();
		}
		else
		{
			g_pLocalRagdoll = this;

			// This is the local player, so set them in a default
			// pose and slam their velocity, angles and origin
			SetAbsOrigin( m_vecRagdollOrigin );

			SetAbsAngles( pPlayer->GetRenderAngles() );

			SetAbsVelocity( m_vecRagdollVelocity );

			int iSeq = LookupSequence( "walk_lower" );
			if ( iSeq == -1 )
			{
				Assert( false );	// missing walk_lower?
				iSeq = 0;
			}

			SetSequence( iSeq );	// walk_lower, basic pose
			SetCycle( 0.0 );

			Interp_Reset( varMap );
		}
	}
	else
	{
		// overwrite network origin so later interpolation will
		// use this position
		SetNetworkOrigin( m_vecRagdollOrigin );

		SetAbsOrigin( m_vecRagdollOrigin );
		SetAbsVelocity( m_vecRagdollVelocity );

		Interp_Reset( GetVarMapping() );

	}

	SetModelIndex( m_nModelIndex );

	// Make us a ragdoll..
	m_nRenderFX = kRenderFxRagdoll;

	matrix3x4_t boneDelta0[MAXSTUDIOBONES];
	matrix3x4_t boneDelta1[MAXSTUDIOBONES];
	matrix3x4_t currentBones[MAXSTUDIOBONES];
	const float boneDt = 0.05f;

	if ( pPlayer && !pPlayer->IsDormant() )
	{
		pPlayer->GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );
	}
	else
	{
		GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );
	}

	InitAsClientRagdoll( boneDelta0, boneDelta1, currentBones, boneDt );
}

void C_RnLRagdoll::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		CreateRagdoll();
	}
}

IRagdoll* C_RnLRagdoll::GetIRagdoll() const
{
	return m_pRagdoll;
}

void C_RnLRagdoll::UpdateOnRemove(void)
{
	VPhysicsSetObject(NULL);

	BaseClass::UpdateOnRemove();
}

C_BaseAnimating * C_RnLPlayer::BecomeRagdollOnClient()
{
	// Let the C_CSRagdoll entity do this.
	// m_builtRagdoll = true;
	return NULL;
}

IRagdoll* C_RnLPlayer::GetRepresentativeRagdoll() const
{
	if ( m_hRagdoll.Get() )
	{
		C_RnLRagdoll *pRagdoll = (C_RnLRagdoll*)m_hRagdoll.Get();

		return pRagdoll->GetIRagdoll();
	}
	else
	{
		return NULL;
	}
}

C_RnLPlayer::C_RnLPlayer() : 
	m_iv_angEyeAngles( "C_RnLPlayer::m_iv_angEyeAngles" )
{
	m_PlayerAnimState = CreatePlayerAnimState( this, this, LEGANIM_9WAY, true );

	m_angEyeAngles.Init();
	AddVar( &m_angEyeAngles, &m_iv_angEyeAngles, LATCH_SIMULATION_VAR );

	m_bInFreelook = false;
	m_angWeaponAngle.Init();
	m_angFreeLookAngle.Init();
	m_flPreviousMouseUpdateTime = 0;
	m_flFreeLookResyncTime = 0;

	m_flDeathViewTime = 0.0f;

	m_bSpawnInterpCounter = m_bSpawnInterpCounterCache = false;

	m_pSwitchingToWeapon = NULL;

	fRagdollViewTime = 0.0f;
	bLastRagdollView = false;

}

C_RnLPlayer::~C_RnLPlayer()
{
	m_PlayerAnimState->Release();
}

extern ConVar sensitivity;

ConVar sensitivity_hip( "sensitivity_hip", "6.0", FCVAR_ARCHIVE, "Mouse sensitivity for hip firing.", true, 0.0001f, false, 10000000 );
ConVar sensitivity_is( "sensitivity_is", "3.0", FCVAR_ARCHIVE, "Mouse sensitivity for IronSight firing.", true, 0.0001f, false, 10000000 );
ConVar sensitivity_ss( "sensitivity_ss", "2.0", FCVAR_ARCHIVE, "Mouse sensitivity for SuperSight firing.", true, 0.0001f, false, 10000000 );

void C_RnLPlayer::Spawn( void )
{
	// Set the player's sensitivity to their default when they first join a game.
	if( sensitivity.GetFloat() != sensitivity_hip.GetFloat() )
		sensitivity.SetValue(sensitivity_hip.GetFloat());

	BaseClass::Spawn();
}

ShadowType_t C_RnLPlayer::ShadowCastType( void ) 
{
	if ( !IsVisible() )
		 return SHADOWS_NONE;

	return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;
} 

C_RnLPlayer* C_RnLPlayer::GetLocalRnLPlayer()
{
	return ToRnLPlayer( C_BasePlayer::GetLocalPlayer() );
}

const QAngle& C_RnLPlayer::GetRenderAngles()
{
	if ( IsRagdoll() )
	{
		return vec3_angle;
	}
	else
	{
		return m_PlayerAnimState->GetRenderAngles();
	}
}

Vector gvLocalOrigin;
const Vector& C_RnLPlayer::GetRenderOrigin( void )
{
	if( GetLocalPlayer() == this && IsAlive() && !OverrideThirdPersonCamera() )
	{
		QAngle worldAng( 0, GetRenderAngles()[YAW], 0 );
		gvLocalOrigin = BaseClass::GetRenderOrigin();
		Vector vecOffset;
		AngleVectors( worldAng, &vecOffset );
		gvLocalOrigin -= (vecOffset * 17);
		return gvLocalOrigin;
	}
	else
	{
		return BaseClass::GetRenderOrigin();
	}
}

void C_RnLPlayer::UpdateClientSideAnimation()
{
	// Update the animation data. It does the local check here so this works when using
	// a third-person camera (and we don't have valid player angles).
	if ( this == C_RnLPlayer::GetLocalRnLPlayer() )
		m_PlayerAnimState->Update( EyeAngles()[YAW], m_angEyeAngles[PITCH] );
	else
		m_PlayerAnimState->Update( m_angEyeAngles[YAW], m_angEyeAngles[PITCH] );

	BaseClass::UpdateClientSideAnimation();
}

void C_RnLPlayer::PostDataUpdate( DataUpdateType_t updateType )
{
	if ( m_bSpawnInterpCounter != m_bSpawnInterpCounterCache )
	{
		ClientRespawn();
		m_bSpawnInterpCounterCache = m_bSpawnInterpCounter;
	}

	// C_BaseEntity assumes we're networking the entity's angles, so pretend that it
	// networked the same value we already have.
	SetNetworkAngles( GetLocalAngles() );
	
	BaseClass::PostDataUpdate( updateType );
}

void C_RnLPlayer::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	UpdateVisibility();
}

void C_RnLPlayer::ResetPlayerView( const QAngle& angle )
{
	m_bInFreelook = false;
	m_bInFreeAim = false;
	m_flFreeLookResyncTime = 0.0f;
	m_angWeaponAngle = angle;
	m_angFreeLookAngle = angle;
	m_angEyeAngles = angle;
	m_angWeaponSway.Init();
	
	if( this == GetLocalPlayer() )
		engine->SetViewAngles( m_angWeaponAngle );
	
	SetLocalViewAngles( m_angWeaponAngle );
}

static float flParaSound = 0.0f;
static float flKnockDownSound = 0.0f;

void C_RnLPlayer::ClientThink( void )
{
	BaseClass::ClientThink();

	if( this == GetLocalRnLPlayer() && IsAlive() )
	{
		/* Michael Lebson
		Play the parachute wind sound as long as we are parachuting.
		From EP1 code.
		*/
		if( GetMovementPosture() == MOVEMENT_POSTURE_PARACHUTING )
		{
			if (flParaSound < gpGlobals->curtime )
			{
				float flDuration = 0.0f;
				EmitSound( "Player.Parachute_Wind", 0.0f, &flDuration );
				flParaSound = gpGlobals->curtime + flDuration;
			}
		}
		else if( flParaSound > gpGlobals->curtime )
			StopSound( "Player.Parachute_Wind" );

		/*
		Play the knockdown sound while we are knocked down, but not
		so everyone can hear us.
		*/
		if( GetMovementPosture() == MOVEMENT_POSTURE_KNOCKDOWN )
		{
			if( flKnockDownSound < gpGlobals->curtime )
			{
				float flDuration = 0.0f;
				EmitSound( "Player.Knockdown", 0.0f, &flDuration );
				flKnockDownSound = gpGlobals->curtime + flDuration;
			}
		}
		else
			StopSound("Player.Knockdown");

		float flMorale = GetMoraleLevel();

		if( GetDamageBasedSpeedModifier( 0 ) > 0.0f || flMorale < 65  )
		{
			if( m_flNextMoraleEffect <= gpGlobals->curtime )
			{
				m_flNextMoraleEffect = gpGlobals->curtime + 0.05 + ( 0.25 * ( flMorale / 65.0f ) );
				float flSway = 0.0f;
				
				if( GetDamageBasedSpeedModifier( 0 ) > 0.0f )
					flSway = ( GetDamageBasedSpeedModifier( 0 ) / 100.0f );

				if( flMorale < 65 )
				{
					flSway += (1.0f - ( flMorale / 65.0f ));

					if( flMorale < 35 )
						flSway *= random->RandomFloat( 2.0, 5.20 );
				}

				if( GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS  )
				{
					if( flMorale < 45 )
						flSway *= random->RandomFloat( -15.0, 15.0 );
					else
						flSway *= random->RandomFloat( -5.0, 5.0 );
				}
				else if( flSway > 0.10 )
					flSway *= random->RandomFloat( -25.0, 25.0 );

				if( flMorale < 40 )
				{
					if( GetNextSprintSoundTime() <= gpGlobals->curtime )
					{
						float flDur = 0;
						EmitSound( "Player.LowMorale", 0.0f, &flDur );
						//DevMsg( "Playing LowMorale for %f seconds\n", flDur );
						SetNextSprintSoundTime( gpGlobals->curtime + flDur );
					}
				}

				if( flSway != 0.0f )
				{
					m_angMoraleEffect = QAngle( flSway, flSway, 0.0f );

					if( flMorale < 45 )
						m_angMoraleEffect *= 5;
				}
				else
					m_angMoraleEffect.Init();
			}

			if( m_angMoraleEffect[0] != 0.0f && m_angMoraleEffect[1] != 0.0f )
				ViewPunch( QAngle( m_angMoraleEffect[0] * gpGlobals->frametime, m_angMoraleEffect[1] * gpGlobals->frametime, 0.0f ) );
		}

		Vector vForward;
		Vector vEyePos = EyePosition();
		EyeVectors( &vForward );

		trace_t tr;
		UTIL_TraceLine( vEyePos, vEyePos + (vForward * 400), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

		if( tr.DidHitNonWorldEntity() && tr.m_pEnt && tr.m_pEnt->IsPlayer() && tr.m_pEnt->IsAlive() && tr.m_pEnt->GetTeamNumber() == GetTeamNumber() )
			m_iPlayerTargetID = tr.m_pEnt->entindex();
		else
			m_iPlayerTargetID = 0;

		switch( GetWeaponPosture() )
		{
			case WEAPON_POSTURE_SUPERSIGHTS:
				if( sensitivity.GetFloat() != sensitivity_ss.GetFloat())
					sensitivity.SetValue(sensitivity_ss.GetFloat());
				break;
			case WEAPON_POSTURE_IRONSIGHTS:
				if( sensitivity.GetFloat() != sensitivity_is.GetFloat())
					sensitivity.SetValue(sensitivity_is.GetFloat());
				break;
			default:
				if( sensitivity.GetFloat() != sensitivity_hip.GetFloat())
					sensitivity.SetValue(sensitivity_hip.GetFloat());
				break;
		}
	}
	/* Michael Lebson
	If this is the local player but they are not alive,
	stop the parachuting sound and the low morale sound.*/
	else if( this == GetLocalRnLPlayer() )
	{
		StopSound( "Player.Parachute_Wind" );
		StopSound( "Player.LowMorale" );
		StopSound( "Player.Knockdown" );

		// Set the player's sensitivity to their default when they die.
		if( sensitivity.GetFloat() != sensitivity_hip.GetFloat() )
			sensitivity.SetValue(sensitivity_hip.GetFloat());
	}
}

//Tony; spawn interp stuff.
void C_RnLPlayer::ClientRespawn()
{
	m_pSwitchingToWeapon = NULL;

	m_flDeathViewTime = 0.0f;
	m_flWeaponPostureTime = 0.0f;

	ResetPlayerView( EyeAngles() );

	//Tony; all we need to do right now..
	MoveToLastReceivedPosition( true );
	ResetLatched();
	RemoveAllDecals(); // nuke all decals off the player model
}

void C_RnLPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	if ( event == PLAYERANIMEVENT_THROW_GRENADE )
	{
		// Let the server handle this event. It will update m_iThrowGrenadeCounter and the client will
		// pick up the event in CCSPlayerAnimState.
	}
	else	
	{
		m_PlayerAnimState->DoAnimationEvent( event, nData );
	}
}

bool C_RnLPlayer::ShouldDrawViewModel( void )
{
	if( OverrideCamera() )
		return false;

	return ShouldDraw();
}

bool C_RnLPlayer::ShouldDraw( void )
{
	// If we're dead, our ragdoll will be drawn for us instead.
	if ( !IsAlive() )
		return false;

	// if knocked down, do not show self (clipping problems)
	if ( GetMovementPosture() == MOVEMENT_POSTURE_KNOCKDOWN )
		return false;

	if( GetTeamNumber() == TEAM_SPECTATOR )
		return false;

	if( IsLocalPlayer() && IsRagdoll() )
		return true;

	return true;
//	return BaseClass::ShouldDraw();
}

int C_RnLPlayer::InternalDrawModel( int flags )
{
	if( !input->CAM_IsThirdPerson() && !OverrideThirdPersonCamera() )
	{
		bool isLocalSpectating = false;

		C_RnLPlayer * player = C_RnLPlayer::GetLocalRnLPlayer();
		if( player && (player != this) )
		{
			if ( player && player->IsObserver() )
			{
				if ( player->GetObserverMode() == OBS_MODE_IN_EYE &&
					player->GetObserverTarget() == this )
				{
					isLocalSpectating = true;
				}
			}
		}

		if( isLocalSpectating || player == this )
		{
			int helmet = 0;
			int bodygroup = FindBodygroupByName( "upperbody" );
			if( bodygroup > -1 && !OverrideFirstPersonCamera())
				SetBodygroup( bodygroup, 1 );

			bodygroup = FindBodygroupByName( "helmet" );
			if( bodygroup > -1 )
			{
				helmet = GetBodygroup( bodygroup );
				SetBodygroup( bodygroup, 0 );
			}

			//modelrender->ForcedMaterialOverride( materials->FindMaterial( "models/shadertest/shader3.vmt", TEXTURE_GROUP_OTHER, true ) );
			int iRet = BaseClass::InternalDrawModel( flags );
			//modelrender->ForcedMaterialOverride( NULL );

			bodygroup = FindBodygroupByName( "upperbody" );
			if( bodygroup > -1 )
				SetBodygroup( bodygroup, 0 );

			bodygroup = FindBodygroupByName( "helmet" );
			if( bodygroup > -1 )
				SetBodygroup( bodygroup, helmet );

			return iRet;
		}
	}

	return BaseClass::InternalDrawModel( flags );
}

CWeaponRnLBase* C_RnLPlayer::GetActiveRnLWeapon() const
{
	return dynamic_cast< CWeaponRnLBase* >( GetActiveWeapon() );
}

//Andrew; Freeaim stuff
void C_RnLPlayer::PreMouseMove( QAngle& inAngles )
{
	if( IsAlive() )
	{
		CWeaponRnLBase* pWeapon = GetActiveRnLWeapon();

		if( !OverrideCamera() && !input->CAM_IsThirdPerson())
		{
			if( pWeapon && pWeapon->ShouldUseFreeAim() && !IsSprinting() )
			{
				m_bInFreeAim = true;
			}
			else if( m_bInFreeAim )
			{
				m_bInFreeAim = false;
				inAngles = m_angWeaponAngle;
			}
		}

		/* Michael Lebson
		If we are parachuting, act as if the third person camera
		is overridden and set the camera to free look.
		*/
		if( ((m_nButtons & IN_ATTACK3) && (IsDeployed() && pWeapon && pWeapon->GetAnimationState() != WEAPON_ANIMATION_RELOAD))
			|| OverrideThirdPersonCamera()
			|| GetMovementPosture() == MOVEMENT_POSTURE_PARACHUTING )
		{
			if( !m_bInFreelook )
				m_angFreeLookAngle = EyeAngles();
			
			m_bInFreelook = true;
			m_flFreeLookResyncTime = 0.0f;
			inAngles = m_angFreeLookAngle;
		}
		else if( m_bInFreeAim )
		{
			if( m_bInFreelook )
			{
				m_bInFreelook = false;
				m_flFreeLookResyncTime = gpGlobals->curtime + 0.5f;
			}
			inAngles = m_angWeaponAngle;
		}  
		else
		{
			if( m_bInFreelook )
			{
				m_bInFreelook = false;
				m_flFreeLookResyncTime = gpGlobals->curtime + 0.5f;
			}
			m_angWeaponAngle = inAngles;
		}
	}
}

ConVar cl_freeaimstiffness("cl_freeaimstiffness", "4.5", FCVAR_ARCHIVE, "Camera stiffness", true, 0.0f, true, 10.0f );
ConVar cl_freeaimdeadzone("cl_freeaimdeadzone", "1.0f", FCVAR_ARCHIVE, "Camera stiffness", true, 0.5f, true, 1.5f );
#define SLOW_AIM_MOVEMENT 50.0f

void C_RnLPlayer::PostMouseMove( QAngle& outAngles )
{
	if( IsAlive() )
	{
		if( m_bInFreelook )
		{
			m_angFreeLookAngle = outAngles;
			outAngles = EyeAngles();

			/* Michael Lebson
			If we are parachuting, lock our weapon in the middle
			of the screen.
			*/
			if( GetMovementPosture() == MOVEMENT_POSTURE_PARACHUTING )
			{
				outAngles = GetRenderAngles();
				outAngles[PITCH] = 0;
			}

			ClampAngles( m_angFreeLookAngle, outAngles, true, true, Vector2D( 45.0f, 45.0f ), AIM_LOCK_LIMIT );	
		}
		else if( m_bInFreeAim )
		{
			Vector2D freeAimExtents( 0, 0 );
			Vector2D freeAimDeadZone( 0, 0 );
			int freeAimLockExtents = AIM_LOCK_NONE;
			float flMouseUpdateDelta = gpGlobals->curtime - m_flPreviousMouseUpdateTime;

			CWeaponRnLBase* pWeapon = GetActiveRnLWeapon();
			if( pWeapon )
				pWeapon->GetFreeAimBounds( freeAimExtents, freeAimDeadZone, freeAimLockExtents );

			freeAimDeadZone *= cl_freeaimdeadzone.GetFloat();
			m_angWeaponAngle = outAngles;
			if( IsDeployed() )
				outAngles = GetMovementPostureAngle();
			else
				outAngles = EyeAngles();

			m_angWeaponAngle[ROLL] = outAngles[ROLL];

			ClampAngles( m_angWeaponAngle, outAngles, true, true, freeAimExtents, freeAimLockExtents, flMouseUpdateDelta );

			if( freeAimLockExtents == AIM_LOCK_NONE )
			{
				float kSpringConstant = cl_freeaimstiffness.GetFloat();
				//Spring
				float pitchDiff = m_angWeaponAngle[PITCH] - outAngles[PITCH];
				float yawDiff = UTIL_AngleDiff( m_angWeaponAngle[YAW], outAngles[YAW] );

				if( fabs( pitchDiff ) > freeAimDeadZone.y )
					outAngles[PITCH] += (pitchDiff * kSpringConstant) * flMouseUpdateDelta;

				if( fabs(yawDiff) > freeAimDeadZone.x )
					outAngles[YAW] += (yawDiff * kSpringConstant) * flMouseUpdateDelta;
			}
		}
		else
		{
			m_angWeaponAngle = outAngles;
		}
	}
	else
	{
		m_bInFreelook = false;
		m_bInFreeAim = false;
		m_flFreeLookResyncTime = 0.0f;
		m_angWeaponAngle = outAngles;
		m_angFreeLookAngle = outAngles;
	}

	m_flPreviousMouseUpdateTime = gpGlobals->curtime;
}

void C_RnLPlayer::AdjustYaw( float& speed, QAngle& viewangles )
{
}

void C_RnLPlayer::AdjustPitch( float& speed, QAngle& viewangles )
{
}

//-----------------------------------------------------------------------------
// Called when not in tactical mode. Allows view to be overriden for things like driving a tank.
//-----------------------------------------------------------------------------
void C_RnLPlayer::OverrideView( CViewSetup *pSetup )
{
	if( OverrideFirstPersonCamera() )
	{	
		GetFirstPersonOverride( pSetup );	
	}
	else if( OverrideThirdPersonCamera() )
	{
		GetThirdPersonOverride( pSetup );
	}
	else if( IsAlive() )
	{
		
		if( m_bInFreelook )
		{
			pSetup->angles = m_angFreeLookAngle;
		}
		else if( m_flFreeLookResyncTime > gpGlobals->curtime )
		{
			float timedelta = m_flFreeLookResyncTime - gpGlobals->curtime;
			float pitchDiff = m_angFreeLookAngle[PITCH] - pSetup->angles[PITCH];
			float yawDiff = UTIL_AngleDiff( m_angFreeLookAngle[YAW], pSetup->angles[YAW] );

			pSetup->angles[PITCH] += pitchDiff * timedelta;
			pSetup->angles[YAW] += yawDiff * timedelta;
		}
		else if( IsProne() && m_bInFreeAim && GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
		{
			pSetup->angles = m_angWeaponAngle;
		}
	}

	pSetup->fovViewmodel = pSetup->fov;
}

bool C_RnLPlayer::OverrideCamera( void )
{
	return (OverrideThirdPersonCamera() || OverrideFirstPersonCamera());
}

bool C_RnLPlayer::OverrideThirdPersonCamera( void )
{
	if( IsAlive() )
	{
		switch( GetMovementPosture() )
		{
		case MOVEMENT_POSTURE_MORTAL_WOUND:
			{
				return true;
			}
			break;
		};
	}

	return false;
}

bool C_RnLPlayer::GetThirdPersonOverride( CViewSetup *pSetup )
{
	pSetup->origin = m_vThirdPersonOverrideOffset;
	return true;
}

bool C_RnLPlayer::GetThirdPersonOverride( Vector& pos, QAngle& angle, Vector2D& limits, int& lockToExtents )
{
	static Vector idealAngles( 20, 0, 128 );
	Vector camOffset;
	QAngle viewangles;

	// Obtain engine view angles and if they popped while the camera was static,
	// fix the camera angles as well
	engine->GetViewAngles( viewangles );
	static QAngle s_oldAngles = viewangles;

	camOffset[ YAW ] = idealAngles[YAW] + viewangles[ YAW ];
	camOffset[ PITCH ] = idealAngles[PITCH] + viewangles[ PITCH ];
	camOffset[ 2 ] = idealAngles[2];

	// move the camera closer to the player if it hit something
	trace_t trace;
	
	Vector camForward;

	// find our player's origin, and from there, the eye position
	Vector origin = EyePosition();

	// get the forward vector
	AngleVectors( QAngle(camOffset[ PITCH ], camOffset[ YAW ], 0), &camForward, NULL, NULL );

	// use our previously #defined hull to collision trace
	CTraceFilterSimple traceFilter( this, COLLISION_GROUP_NONE );
	UTIL_TraceHull( origin, origin - (camForward * camOffset[ 2 ]),
		Vector( -5, -5, -5), Vector( 5, 5, 5 ),
		MASK_SOLID, &traceFilter, &trace );

	// move the camera closer if it hit something
	if( trace.fraction < 1.0 )
	{
		camOffset[ 2 ] *= trace.fraction;
	}

	// For now, I'd rather see the insade of a player model than punch the camera through a wall
	// might try the fade out trick at some point
	//if( camOffset[ DIST ] < CAM_MIN_DIST )
	//    camOffset[ DIST ] = CAM_MIN_DIST; // clamp up to minimum

	m_vThirdPersonOverrideOffset[ PITCH ] = camOffset[ PITCH ];
	m_vThirdPersonOverrideOffset[ YAW ]   = camOffset[ YAW ];
	m_vThirdPersonOverrideOffset[ 2 ]  = camOffset[ 2 ];
	return true;
}

bool C_RnLPlayer::OverrideFirstPersonCamera( void )
{
	if( !IsAlive() )
	{
		if( m_flDeathViewTime > gpGlobals->curtime && g_pLocalRagdoll != NULL )	
			return true;

		//Bail out anyway since we're not alive
		return false;
	}
	else
	{
		switch( GetMovementPosture() )
		{
		case MOVEMENT_POSTURE_CLIMBING:
		case MOVEMENT_POSTURE_CLIMBING_TO_CROUCH:
			{
				return true;
			}
			break;
		};
	}

	return false;
}

bool C_RnLPlayer::GetFirstPersonOverride( CViewSetup *pSetup )
{
	if( !IsAlive() )
	{
		if( m_flDeathViewTime > gpGlobals->curtime && g_pLocalRagdoll != NULL )
		{
			g_pLocalRagdoll->GetAttachment( g_pLocalRagdoll->LookupAttachment( "eyes" ), pSetup->origin, pSetup->angles );
			
			// move up to not clip through the ground
			pSetup->origin.z += 7.5f;
			return true;
		}
	}
	else
	{
		switch( GetMovementPosture() )
		{
		case MOVEMENT_POSTURE_PARACHUTING:
		case MOVEMENT_POSTURE_CLIMBING:
		case MOVEMENT_POSTURE_CLIMBING_TO_CROUCH:
			{
				GetAttachment( "eyes", pSetup->origin, pSetup->angles );
			}
			return true;
			break;
		};
	}

	return false;
}

bool C_RnLPlayer::GetFirstPersonOverride( Vector& pos, QAngle& angle, Vector2D& limits, int& lockToExtents )
{
	if( m_flDeathViewTime > gpGlobals->curtime && g_pLocalRagdoll != NULL )
	{
		limits.x = 0.0f;
		limits.y = 0.0f;
		lockToExtents = AIM_LOCK_LIMIT;
		g_pLocalRagdoll->GetAttachment( g_pLocalRagdoll->LookupAttachment( "eyes" ), pos, angle );
		
		// move up to not clip through the ground
		pos.z += 7.5f;
		return true;
	}
	
	
	switch( GetMovementPosture() )
	{
	case MOVEMENT_POSTURE_PARACHUTING:
	case MOVEMENT_POSTURE_CLIMBING:
	case MOVEMENT_POSTURE_CLIMBING_TO_CROUCH:
		{
			limits.x = 35.0f;
			limits.y = 75.0f;
			lockToExtents = AIM_LOCK_LIMIT;
			GetAttachment( LookupAttachment( "eyes" ), pos, angle );
			angle = EyeAngles();
		}
		return true;
		break;
	};

	return false;
}

//Andrew; End Freeaim stuff
void C_RnLPlayer::CalcViewRoll( QAngle& eyeAngles )
{
	BaseClass::CalcViewRoll( eyeAngles );

	if( !IsAlive() )
	{
		m_RnLLocal.m_flViewRollOffset = 0.0f;
		m_vecLeanOffset.Init();
		eyeAngles[ROLL] = 0.0f;
		return;
	}

	//RnL Andrew : Leaning Yo!
	if( m_RnLLocal.m_flViewRollOffset != 0 )
	{
		eyeAngles[ROLL] += m_RnLLocal.m_flViewRollOffset;
	}

	if( GetMovementPosture() == MOVEMENT_POSTURE_PRONE_ROLL_LEFT )
	{
		eyeAngles[ROLL] += (-360.0f * GetMovementPostureDuration());
	}
	else if( GetMovementPosture() == MOVEMENT_POSTURE_PRONE_ROLL_RIGHT )
	{
		eyeAngles[ROLL] += (360.0f * GetMovementPostureDuration());
	}
}

ConVar cl_viewbobscale_x( "cl_viewbobscale_x", "1.0f", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Scale the left-right view bob", true, 0.0f, true, 2.0f );
ConVar cl_viewbobscale_y( "cl_viewbobscale_y", "1.0f", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Scale the up-down view bob", true, 0.0f, true, 2.0f );

void C_RnLPlayer::CalcPlayerView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
	BaseClass::CalcPlayerView( eyeOrigin, eyeAngles, fov );

	// Knockdown view code
	if ( GetMovementPosture() == MOVEMENT_POSTURE_KNOCKDOWN )
	{
		C_RnLRagdoll *pRagdoll = (C_RnLRagdoll*)m_hKnockDownRagdoll.Get();
		if (pRagdoll)
		{
			pRagdoll->SetRenderMode(kRenderNone);

			if (bLastRagdollView == false)
				fRagdollViewTime = gpGlobals->curtime + 5.0f;

			// Gets it's origin and angles
			Vector vRagdollOrigin;
			QAngle aRagdollAngles;
			pRagdoll->GetAttachment( pRagdoll->LookupAttachment( "eyes" ), vRagdollOrigin, aRagdollAngles );
			Vector vRagdollForward;
			AngleVectors(aRagdollAngles, &vRagdollForward);

			// Move up to not clip
			vRagdollOrigin.z += 7.5f;

			// Scale to never look down
			vRagdollForward.z = (vRagdollForward.z + 1.0f) * 0.75f - 0.25f;

			float fViewFrac = fRagdollViewTime - gpGlobals->curtime;
			fViewFrac = clamp(fViewFrac / 2.0f, 0.0f, 1.0f);

			fViewFrac = SimpleSpline(fViewFrac);

			eyeOrigin = eyeOrigin * (1 - fViewFrac) + vRagdollOrigin * fViewFrac;

			Vector vEyeForward;
			AngleVectors(eyeAngles, &vEyeForward);

			vEyeForward = vEyeForward * (1 - fViewFrac) + vRagdollForward * fViewFrac;

			VectorAngles(vEyeForward, eyeAngles);

			bLastRagdollView = true;
			return;
		}
	}
	bLastRagdollView = false;

	// BEGIN VIEWBOBING CODE HERE - Jon
	/////////////////////////////////////
	//if( doViewbob )
	{
		Vector vecRight, vecUp;
		// roll view to simulate movement
		// first get right vector
		AngleVectors( eyeAngles, NULL, &vecRight, &vecUp );

		// normalize it
		vecRight.NormalizeInPlace();
		vecUp.NormalizeInPlace();

		float flCurrentSpeed = GetAbsVelocity().Length();

		if( IsProne() || GetMoveType() == MOVETYPE_LADDER )
		{
			if( flCurrentSpeed > 60 )
			{
				flCurrentSpeed = 75;
			}
			else if( flCurrentSpeed < 25 )
			{
				flCurrentSpeed = 0;
			}

			if( flCurrentSpeed > 0 )
			{
				// get a percentage of current speed, 240 is max speed (sprinting)
				float flMovementPercent = flCurrentSpeed / 75;

				//The player has taken leg damage so modify the movement to match
				if( m_RnLLocal.m_flDamageBasedSpeedModifierLegs > 0 && flCurrentSpeed > 0 )
					flMovementPercent += ( m_RnLLocal.m_flDamageBasedSpeedModifierLegs / 100.0f) / 1.25;

				vecRight *= flMovementPercent * 5; // max horizontal movement amount in units 
				vecUp *= flMovementPercent * 2; // max vertical movement amount in units

				vecRight *= sin(gpGlobals->curtime * 6 ); // last number effects period of sin wave
				// faster period, as the view bobs up twice as fast as it moves left/right
				vecUp *= sin(gpGlobals->curtime * 12 );
			}
		}
		else
		{
			float speedX = 4;
			float speedY = 8;

			if(flCurrentSpeed > 220 )
			{
				speedX = 8;
				speedY = 12;
				flCurrentSpeed = 220;
			}
			else if(flCurrentSpeed > 100 )
				flCurrentSpeed = 110;
			else if(flCurrentSpeed > 50)
				flCurrentSpeed = 30;
			else
				flCurrentSpeed = 0;

			// get a percentage of current speed, 240 is max speed (sprinting)
			float flMovementPercent = flCurrentSpeed / 240;

			//The player has taken leg damage so modify the movement to match
			if( m_RnLLocal.m_flDamageBasedSpeedModifierLegs > 0 && flCurrentSpeed > 0 )
			{
				flMovementPercent += ( m_RnLLocal.m_flDamageBasedSpeedModifierLegs / 100.0f) / 1.25;
				eyeOrigin.z -= (1.25 * ( sin(gpGlobals->curtime*(flMovementPercent*6)) ));
			}

			if( flMovementPercent > 0 )
			{
				vecRight *= flMovementPercent * 8; // max horizontal movement amount in units 
				vecUp *= flMovementPercent * 2; // max vertical movement amount in units

				vecRight *= sin( gpGlobals->curtime * speedX ); // last number effects period of sin wave
				// faster period, as the view bobs up twice as fast as it moves left/right
				vecUp *= sin( gpGlobals->curtime * speedY ); 
			}
		}

		vecUp *= cl_viewbobscale_y.GetFloat();
		vecRight *= cl_viewbobscale_x.GetFloat();

		// modify the eye origin
		eyeOrigin += vecRight + vecUp;

		//DevMsg("Current speed: %f, Movement percent: %f, sin of time: %f\n", flCurrentSpeed, flMovementPercent, sin(gpGlobals->curtime) );
	}
	// END VIEWBOBING CODE HERE - Jon
	/////////////////////////////////////
}

void C_RnLPlayer::CalcInEyeCamView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov)
{
	if( GetObserverTarget() == NULL )
	{
		BaseClass::CalcInEyeCamView( eyeOrigin, eyeAngles, fov );
		return;
	}

	C_RnLPlayer *pPlayerTarget = ToRnLPlayer( GetObserverTarget() );
	if( !pPlayerTarget )
	{
		BaseClass::CalcInEyeCamView( eyeOrigin, eyeAngles, fov );
		return;
	}

	if ( !pPlayerTarget->IsAlive() )
	{
		// if dead, show from 3rd person
		CalcChaseCamView( eyeOrigin, eyeAngles, fov );
		return;
	}

	fov = pPlayerTarget->GetFOV();

	m_flObserverChaseDistance = 0.0;

	eyeAngles = pPlayerTarget->m_angEyeAngles;
	eyeOrigin = pPlayerTarget->EyePosition();

	// Apply punch angle
	VectorAdd( eyeAngles, GetPunchAngle(), eyeAngles );

	engine->SetViewAngles( eyeAngles );
}

void C_RnLPlayer::CalcViewModelView( const Vector& eyeOrigin, const QAngle& eyeAngles)
{
	if( OverrideFirstPersonCamera() )
	{
		Vector pos;
		QAngle loc;
		Vector2D bogus1;
		int bogus2;
		GetFirstPersonOverride( pos, loc, bogus1, bogus2 );
		BaseClass::CalcViewModelView( pos, loc );
		return;
	}
	QAngle weaponAngle = GetWeaponAngle();
	weaponAngle[ROLL] = eyeAngles[ROLL];

	if( IsObserver() && (GetObserverMode() == OBS_MODE_IN_EYE) && GetObserverTarget() != NULL )
	{
		C_RnLPlayer *pPlayerTarget = ToRnLPlayer( GetObserverTarget() );
		if( pPlayerTarget )
		{
			weaponAngle = pPlayerTarget->GetWeaponAngle();
		}
	}

	BaseClass::CalcViewModelView( eyeOrigin, weaponAngle );
}

//-----------------------------------------------------------------------------
// Purpose: Non-Caching CalcVehicleView for Scratch RnL + Multiplayer
// TODO: fix the normal CalcVehicleView so that Caching can work in multiplayer.
//-----------------------------------------------------------------------------
void C_RnLPlayer::CalcVehicleView(IClientVehicle *pVehicle,	Vector& eyeOrigin, QAngle& eyeAngles,	float& zNear, float& zFar, float& fov )
{
	Assert( pVehicle );

	// Start with our base origin and angles
	int nRole = pVehicle->GetPassengerRole( this );

	// Get our view for this frame
	pVehicle->GetVehicleViewPosition( nRole, &eyeOrigin, &eyeAngles, &fov );

	// Allows the vehicle to change the clip planes
	pVehicle->GetVehicleClipPlanes( zNear, zFar );

	// Snack off the origin before bob + water offset are applied
	Vector vecBaseEyePosition = eyeOrigin;

	CalcViewRoll( eyeAngles );

	// Apply punch angle
	VectorAdd( eyeAngles, m_Local.m_vecPunchAngle, eyeAngles );

	if ( !prediction->InPrediction() )
	{
		// Shake it up baby!
		vieweffects->CalcShake();
		vieweffects->ApplyShake( eyeOrigin, eyeAngles, 1.0 );
	}
}

//RnL; Andrew
// Helper Function To Ease Angle Clamping
void ClampAngles( QAngle& angCur, QAngle& angBase, bool bClampPitch, bool bClampYaw, Vector2D& vExtents, int iClampType, float flDelta )
{
	if( bClampPitch ){
		if( angCur[PITCH] >= angBase[PITCH] + vExtents.y ) {
			if( iClampType == AIM_LOCK_LIMIT )
				angCur[PITCH] = angBase[PITCH] + vExtents.y;
			else if( iClampType == AIM_LOCK_SLOWDOWN ){
				angBase[PITCH] += (SLOW_AIM_MOVEMENT * flDelta);
				angCur[PITCH] = angBase[PITCH] + vExtents.y;
			} else {
				angBase[PITCH] = angCur[PITCH] - vExtents.y;
			}
		} else if( angCur[PITCH] <= angBase[PITCH] - vExtents.y ) {
			if( iClampType == AIM_LOCK_LIMIT )
				angCur[PITCH] = angBase[PITCH] - vExtents.y;
			else if( iClampType == AIM_LOCK_SLOWDOWN ){
				angBase[PITCH] -= (SLOW_AIM_MOVEMENT * flDelta);
				angCur[PITCH] = angBase[PITCH] - vExtents.y;
			} else {
				angBase[PITCH] = angCur[PITCH] + vExtents.y;
			}
		}
	}

	if( bClampYaw ) {
		float yawDiff = UTIL_AngleDiff( angCur[YAW], angBase[YAW] );
		if( yawDiff >= vExtents.x ){
			if( iClampType == AIM_LOCK_LIMIT ){
				angCur[YAW] = angBase[YAW] + vExtents.x;
			} else if( iClampType == AIM_LOCK_SLOWDOWN ){
				angBase[YAW] += (SLOW_AIM_MOVEMENT * flDelta);
				angCur[YAW] = angBase[YAW] + vExtents.x;
			} else {
				angBase[YAW] = angCur[YAW] - vExtents.x;
			}
		} else if( yawDiff <= -vExtents.x ) {
			if( iClampType == AIM_LOCK_LIMIT ){
				angCur[YAW] = angBase[YAW] - vExtents.x;
			} else if( iClampType == AIM_LOCK_SLOWDOWN ){
				angBase[YAW] -= (SLOW_AIM_MOVEMENT * flDelta);
				angCur[YAW] = angBase[YAW] - vExtents.x;
			} else {
				angBase[YAW] = angCur[YAW] + vExtents.x;
			}
		}
	}
}

/* Michael Lebson
Client-side equipment class defs and recieve table.
*/
class C_RnLEquipmentProp : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_RnLEquipmentProp, C_BaseAnimating );
	DECLARE_CLIENTCLASS();
	
	C_RnLEquipmentProp(){}
	~C_RnLEquipmentProp(){}

	virtual bool ShouldDraw( void )
	{
		if( GetMoveParent() )
		{
			C_RnLPlayer* pRnLPlayer = ToRnLPlayer( GetMoveParent() );
			
			if( pRnLPlayer && /*( pRnLPlayer->m_hRagdoll ||*/( pRnLPlayer->IsObserver() || !pRnLPlayer->IsAlive() ) )
				return false;
		}

		return BaseClass::ShouldDraw();
	}
};

IMPLEMENT_CLIENTCLASS_DT( C_RnLEquipmentProp, DT_RnLEquipmentProp, CRnLEquipmentProp )
END_RECV_TABLE()