//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL1.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "rnl_player.h"
#include "rnl_gamerules.h"
#include "weapon_rnl_base.h"
#include "weapon_rnlbasemachinegun.h"
#include "weapon_rnl_basegrenade.h"
#include "predicted_viewmodel.h"
#include "iservervehicle.h"
#include "viewport_panel_names.h"
#include "rnl_game_team.h"
#include "rnl_ammodef.h"
#include "rnl_hitgroupdata.h"
#include "rnl_shareddefs.h"
#include "rnl_squad.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int gEvilImpulse101;


#define SHOW_TEAM_SELECTION "changeteam"
#define SHOW_SQUAD_SELECTION "changesquad"
#define SHOW_LOADOUT_SELECTION "changeloadout"

#define SELECT_TEAM "jointeam"
#define SELECT_SQUAD "joinsquad"
#define SELECT_CLASS "joinclass"
#define SELECT_LOADOUT "loadout"

#define VOTE_SQUAD_LEADER "votesquadleader"

// cjd @add
#define SHOUT_CMD "shout"
#define RADIO_CMD "radio"
// cjd end

extern ConVar mp_moraledistance;
ConVar mp_maxteamkills( "mp_maxteamkills", "4", FCVAR_NOTIFY|FCVAR_REPLICATED, "Number of Team Kills before player is announced as a team killer and punished." );
ConVar mp_banteamkillers( "mp_banteamkillers", "0", FCVAR_NOTIFY|FCVAR_REPLICATED, "Enables/Disables auto banning team killers once a limit is reached." );
ConVar mp_teamkillerbantime( "mp_teamkillerbantime", "0", FCVAR_NOTIFY|FCVAR_REPLICATED, "Number of minutes a team killer is banned for if mp_banteamkillers is set to 1. (0 for permenant)" );


// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class CTEPlayerAnimEvent : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEPlayerAnimEvent, CBaseTempEntity );
	DECLARE_SERVERCLASS();

					CTEPlayerAnimEvent( const char *name ) : CBaseTempEntity( name )
					{
					}

	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

#define THROWGRENADE_COUNTER_BITS 3

#define RNL_CLASS_BITS 4
#define RNL_SQUAD_BITS 4

IMPLEMENT_SERVERCLASS_ST_NOBASE( CTEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropInt( SENDINFO( m_iEvent ), Q_log2( PLAYERANIMEVENT_COUNT ) + 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nData ), 32 )
END_SEND_TABLE()

static CTEPlayerAnimEvent g_TEPlayerAnimEvent( "PlayerAnimEvent" );

void TE_PlayerAnimEvent( CBasePlayer *pPlayer, PlayerAnimEvent_t event, int nData )
{
	CPVSFilter filter( (const Vector&)pPlayer->EyePosition() );
	
	g_TEPlayerAnimEvent.m_hPlayer = pPlayer;
	g_TEPlayerAnimEvent.m_iEvent = event;
	g_TEPlayerAnimEvent.m_nData = nData;
	g_TEPlayerAnimEvent.Create( filter, 0 );
}

// -------------------------------------------------------------------------------- //
// Tables.
// -------------------------------------------------------------------------------- //

BEGIN_DATADESC( CRnLPlayer )
	DEFINE_THINKFUNC( FadeToBlackThink ),
	DEFINE_THINKFUNC( KnockDownThink ),
END_DATADESC()

extern void SendProxy_Origin( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );

LINK_ENTITY_TO_CLASS( player, CRnLPlayer );
PRECACHE_REGISTER(player);

BEGIN_SEND_TABLE_NOBASE( CRnLPlayer, DT_RnLLocalPlayerExclusive )
	SendPropInt( SENDINFO( m_iShotsFired ), 8, SPROP_UNSIGNED ),
	// send a hi-res origin to the local player for use in prediction
	SendPropVector	(SENDINFO(m_vecOrigin), -1,  SPROP_NOSCALE|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),

	SendPropInt( SENDINFO( m_iPreviousSquadNumber ) ),
	SendPropInt( SENDINFO( m_iPreviousKitNumber ) ),
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE( CRnLPlayer, DT_RnLNonLocalPlayerExclusive )
	// send a lo-res origin to other players
	SendPropVector	(SENDINFO(m_vecOrigin), -1,  SPROP_COORD_MP_LOWPRECISION|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),
	SendPropQAngles	(SENDINFO(m_angWeaponAngle), 13, SPROP_CHANGES_OFTEN ),
	//Tony; spawn interp.
	SendPropBool( SENDINFO( m_bSpawnInterpCounter) ),
END_SEND_TABLE()


//-----------------------------------------------------------------------------
// Purpose: Only send to non-local players
//-----------------------------------------------------------------------------
void* SendProxy_SendNonLocalPlayerDataTable(const SendProp* pProp, const void* pStruct, const void* pVarData, CSendProxyRecipients* pRecipients, int objectID)
{
	pRecipients->SetAllRecipients();

	CRnLPlayer* pPlayer = (CRnLPlayer*)pVarData;
	if (pPlayer)
	{
		pRecipients->ClearRecipient(pPlayer->GetClientIndex());
		return (void*)pVarData;
	}

	return NULL;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER(SendProxy_SendNonLocalPlayerDataTable);

IMPLEMENT_SERVERCLASS_ST( CRnLPlayer, DT_RnLPlayer )
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseAnimating", "m_flPlaybackRate" ),	
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
	SendPropExclude( "DT_BaseAnimatingOverlay", "overlay_vars" ),
	SendPropExclude( "DT_BaseEntity", "m_vecOrigin" ),
	
	// playeranimstate and clientside animation takes care of these on the client
	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),

	// Data that only gets sent to the local player.
	SendPropDataTable( "rnllocaldata", 0, &REFERENCE_SEND_TABLE(DT_RnLLocalPlayerExclusive), SendProxy_SendLocalDataTable ),
	SendPropDataTable( "rnlnonlocaldata", 0, &REFERENCE_SEND_TABLE(DT_RnLNonLocalPlayerExclusive), SendProxy_SendNonLocalPlayerDataTable),
	SendPropDataTable(SENDINFO_DT(m_RnLLocal), &REFERENCE_SEND_TABLE(DT_RnLLocal), SendProxy_SendLocalDataTable),

	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 11 ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 11 ),
	SendPropInt(SENDINFO(m_iThrowGrenadeCounter), THROWGRENADE_COUNTER_BITS, SPROP_UNSIGNED),
	SendPropEHandle( SENDINFO( m_hRagdoll ) ),
	SendPropEHandle( SENDINFO( m_hKnockDownRagdoll ) ),

	SendPropInt( SENDINFO( m_iSquadNumber ) ),
	SendPropInt( SENDINFO( m_iKitNumber ) ),

	SendPropInt( SENDINFO( m_iClimbheight ) ),
	SendPropInt( SENDINFO( m_nWeaponPosture ) ),
	SendPropVector( SENDINFO( m_vecLeanOffset ) ),
	SendPropFloat( SENDINFO( m_flBodyHeight ) ),
	SendPropFloat( SENDINFO( m_flDeathViewTime ) ),

	SendPropInt(SENDINFO(m_nMovementPosture)),
	SendPropInt(SENDINFO(m_nMovementPostureFrom)),
END_SEND_TABLE()

class CRnLRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CRnLRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};



LINK_ENTITY_TO_CLASS( rnl_ragdoll, CRnLRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CRnLRagdoll, DT_RnLRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) ),

	//SendPropInt( RECVINFO( m_iTeamNumber ) ),
	SendPropInt		(SENDINFO(m_nSkin)),
	SendPropInt		(SENDINFO(m_nBody)),
END_SEND_TABLE()


// -------------------------------------------------------------------------------- //

void cc_CreatePredictionError_f()
{
	CBaseEntity *pEnt = CBaseEntity::Instance( 1 );
	pEnt->SetAbsOrigin( pEnt->GetAbsOrigin() + Vector( 63, 0, 0 ) );
}

ConCommand cc_CreatePredictionError( "CreatePredictionError", cc_CreatePredictionError_f, "Create a prediction error", FCVAR_CHEAT );

CRnLPlayer::CRnLPlayer()
{
	m_PlayerAnimState = CreatePlayerAnimState( this, this, LEGANIM_9WAY, true );

	UseClientSideAnimation();
	m_angEyeAngles.Init();

	SetViewOffset( SDK_PLAYER_VIEW_OFFSET );

	m_iThrowGrenadeCounter = 0;

	m_iSquadNumber = RNL_SQUAD_INVALID;
	m_iKitNumber = RNL_KIT_INVALID;

	m_bSpawnInterpCounter = false;

	m_pRnLParachute = NULL;
	m_pRnLRadio = NULL;

	m_hRagdoll = NULL;
	m_hKnockDownRagdoll = NULL;

	m_pSwitchingToWeapon = NULL;
}


CRnLPlayer::~CRnLPlayer()
{
	m_PlayerAnimState->Release();
}


CRnLPlayer *CRnLPlayer::CreatePlayer( const char *className, edict_t *ed )
{
	CRnLPlayer::s_PlayerEdict = ed;
	return (CRnLPlayer*)CreateEntityByName( className );
}

void CRnLPlayer::LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles )
{
	BaseClass::LeaveVehicle( vecExitPoint, vecExitAngles );

	//teleport physics shadow too
	// Vector newPos = GetAbsOrigin();
	// QAngle newAng = GetAbsAngles();

	// Teleport( &newPos, &newAng, &vec3_origin );
}

void CRnLPlayer::PreThink(void)
{
	// Riding a vehicle?
	if ( IsInAVehicle() )	
	{
		// make sure we update the client, check for timed damage and update suit even if we are in a vehicle
		UpdateClientData();		
		CheckTimeBasedDamage();

		// Allow the suit to recharge when in the vehicle.
		CheckSuitUpdate();
		
		WaterMove();	
		return;
	}

	BaseClass::PreThink();
}


void CRnLPlayer::PostThink()
{
	BaseClass::PostThink();

	if( m_nButtons & IN_MAP && (GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS) )
	{
		if( GetActiveRnLWeapon() )
			GetActiveRnLWeapon()->ReturnToDefaultPosture();
	}

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );
	
	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();

    m_PlayerAnimState->Update( m_angEyeAngles[YAW], m_angEyeAngles[PITCH] );
}


void CRnLPlayer::Precache()
{
	PrecacheModel( RNL_DEFAULT_PLAYER_MODEL );
	PrecacheModel( "models/shells/spoon.mdl" );

	// Michael Lebson
	// Precache script sounds for game events.

	// Axis
	PrecacheScriptSound( "Game.Win_Axis" );
	PrecacheScriptSound( "Game.Lose_Axis" );
	PrecacheScriptSound( "Objective.Captured_GER" );
	PrecacheScriptSound( "Objective.Lost_GER" );
	PrecacheScriptSound( "Objective.Blocked_GER" );
	// Allies
	PrecacheScriptSound( "Game.Win_Allies" );
	PrecacheScriptSound( "Game.Lose_Allies" );
	PrecacheScriptSound( "Objective.Captured_US" );
	PrecacheScriptSound( "Objective.Lost_US" );
	PrecacheScriptSound( "Objective.Blocked_US" );
	// Misc
	PrecacheScriptSound( "Objective.Captured" );
	PrecacheScriptSound( "Objective.Lost" );
	PrecacheScriptSound( "Objective.Blocked" );

	// Player's wounded sounds
	PrecacheScriptSound( "Player.MoveWoundedPain" );
	PrecacheScriptSound( "Player.Knockdown" );
	PrecacheScriptSound( "Player.QuickPain" );

	// Player's low morale sound
	PrecacheScriptSound( "Player.LowMorale" );

	// Player's breathing sounds
	for( int i = 0; i < (sizeof(g_sSprintSoundInfo) / sizeof(g_sSprintSoundInfo[0])); i++ )
		PrecacheScriptSound( g_sSprintSoundInfo[i].szName );

	PrecacheModel("models/items/505th_pir_helm.mdl");
	PrecacheModel("models/items/w_eq_ger_helmet.mdl");
	// End: Michael Lebson

	BaseClass::Precache();
}

void CRnLPlayer::Spawn()
{
	BaseClass::Spawn();

	m_pSwitchingToWeapon = NULL;

	m_iPreviousSquadNumber = RNL_SQUAD_INVALID;
	m_iPreviousKitNumber = RNL_KIT_INVALID;
	m_vPlayerDamageInfo.Purge();

	if( GetTeamNumber() == TEAM_SPECTATOR )
	{
		SetMaxSpeed( 250.0f );
		return;
	}
	
	if( IsReadyToSpawn() )
	{
		StopObserverMode();

		SetMoveType( MOVETYPE_WALK );
		RemoveSolidFlags( FSOLID_NOT_SOLID );
		SetAbsVelocity( vec3_origin );
		SetBloodColor( BLOOD_COLOR_RED );

		m_flNextMoraleUpdate = 0.0f;
		SetMoraleLevel( 70 );
		SetStamina( 100.0f );

		SetWeaponPosture( WEAPON_POSTURE_SHOULDER );
		SetMovementPosture( MOVEMENT_POSTURE_STAND );

		RemoveEffects( EF_NOINTERP );

		StopSprinting();
		SetMaxSpeed( 250.0f );
		m_flNextMovingWoundedSound = 0;
		SetDamageBasedSpeedModifier( 0, 0.0f );
		SetDamageBasedSpeedModifier( 1, 0.0f );

		// Remove the player's faded screen.
		color32 color = {0, 0, 0, 0};
		UTIL_ScreenFade(this, color, 0.1f, 0.0f, FFADE_PURGE);

		CRnLTeam* pTeam = GetGlobalRnLTeam( GetTeamNumber() );
		if( pTeam )
		{
			pTeam->OnPlayerSpawn( this );
		}
	}
	else
	{
		SetMaxSpeed( 250.0f );
		StartObserverMode( m_iObserverLastMode );
	}

	// Remove Motion Blur effect
	CSingleUserRecipientFilter bluruser( this );
	bluruser.MakeReliable();

	UserMessageBegin( bluruser, "BlurVision" /*"BlurScreen"*/ );
		WRITE_FLOAT( 0 );
		WRITE_FLOAT( -1 );
		WRITE_FLOAT( 1.0 );
		WRITE_FLOAT( 1.0 );
	MessageEnd();

	//Tony; flip the spawn interp counter.
	m_bSpawnInterpCounter = !m_bSpawnInterpCounter;

	RegisterThinkContext("KnockDownThink");
}

void CRnLPlayer::InitialSpawn( void )
{
	BaseClass::InitialSpawn();

	const ConVar *hostname = cvar->FindVar( "hostname" );
	const char *title = (hostname) ? hostname->GetString() : "MESSAGE OF THE DAY";

	// open info panel on client showing MOTD:
	KeyValues::AutoDelete data("data");
	data->SetString( "title", title );		// info panel title
	data->SetString( "type", "1" );			// show userdata from stringtable entry
	data->SetString( "msg",	"motd" );		// use this stringtable entry
	data->SetInt( "cmd", TEXTWINDOW_CMD_CHANGETEAM);// exec this command if panel closed

	ShowViewPortPanel( PANEL_INFO, true, data );

	m_RnLLocal.m_bUseAutobolt = Q_atoi( engine->GetClientConVarValue( entindex(), "cl_autobolt" ) ) != 0;

	SetModel( RNL_DEFAULT_PLAYER_MODEL );
	RemoveAllItems( true );
	ChangeTeam( TEAM_SPECTATOR );
}

extern ConVar mp_legshotimpedance;
extern ConVar mp_armshotimpedance;

int CRnLPlayer::OnTakeDamage( const CTakeDamageInfo &info )
{
	CTakeDamageInfo newDamageInfo = info;
	if( IsAlive() && newDamageInfo.GetDamage() && g_pGameRules->FPlayerCanTakeDamage(this, newDamageInfo.GetAttacker(), info) )
	{
		if( newDamageInfo.GetDamageType() & DMG_BULLET )
		{
			CRnLHitGroupDetails& details = GetRnLHitGroupDetails()[LastHitGroup()];
			newDamageInfo.ScaleDamage( details.GetScale() );

			if( details.IsInstantKill() && newDamageInfo.GetDamage() < GetHealth() )
				newDamageInfo.SetDamage( GetHealth() * details.GetScale() );

			EmitSound( "Player.QuickPain" );
		}

		if( (newDamageInfo.GetDamageType() & DMG_BLAST) && ( random->RandomInt( 0, 10 ) <= 4 ) )
		{
			// When a player gets hit by explosive damage, remove their helmet 70% of the time
			int iBodyGroup = FindBodygroupByName("helmet");

			// Only remove their helmet if they have one
			if( iBodyGroup >= 0 && GetBodygroup( iBodyGroup ) > 0 )
			{
				SetBodygroup( iBodyGroup, 0 );

				// Then spawn one near the player
				Vector origin = GetAbsOrigin();
				origin.y += 5.0f;
				origin.z += 65.0f;

				CBaseEntity *pHelmet = CBaseEntity::CreateNoSpawn( "prop_physics", origin, GetAbsAngles() );

				if( pHelmet )
				{
					// And send it flying
					Vector vecThrow = BodyDirection2D() * newDamageInfo.GetDamageForce();

					// Set the model to a generic Allied model or Axis model.
					if( GetTeamNumber() == TEAM_ALLIES )
						pHelmet->SetModelName( AllocPooledString( "models/items/505th_pir_helm.mdl" ) );
					else
						pHelmet->SetModelName( AllocPooledString( "models/items/w_eq_ger_helmet.mdl" ) );
					
					pHelmet->SetModel( STRING(pHelmet->GetModelName()) );
					pHelmet->ChangeTeam( GetTeamNumber() );
					pHelmet->m_takedamage = DAMAGE_NO;

					// Make sure we can't get stuck on the helmet
					pHelmet->SetCollisionGroup( COLLISION_GROUP_DEBRIS );

					DispatchSpawn( pHelmet );

					// Remove the helmet model after 1 minute
					pHelmet->SetThink( &CBaseEntity::SUB_Remove );
					pHelmet->SetNextThink( gpGlobals->curtime + 60.0f );

					// Send the helmet flying with the calculated velocity
					pHelmet->ApplyAbsVelocityImpulse( vecThrow );
				}
			}
		}

		if( LastHitGroup() == HITGROUP_RNL_HEAD || LastHitGroup() == HITGROUP_HEAD || LastHitGroup() == HITGROUP_RNL_NECK )
		{
			Vector dir = newDamageInfo.GetDamageForce();
			dir.NormalizeInPlace();

			UTIL_BloodSpray( newDamageInfo.GetDamagePosition(), dir, BLOOD_COLOR_RED, 6, FX_BLOODSPRAY_ALL );
		}
		// If they get hit in the arm
		else if( LastHitGroup() == HITGROUP_LEFTARM || LastHitGroup() == HITGROUP_RIGHTARM || LastHitGroup() == HITGROUP_RNL_UPPER_ARM || LastHitGroup() == HITGROUP_RNL_LOWER_ARM || LastHitGroup() == HITGROUP_RNL_HAND )
		{
			AddDamageBasedSpeedModifier( 0, mp_armshotimpedance.GetFloat() );

			// Drop the weapon sometimes
			if(	( GetMovementPosture() != MOVEMENT_POSTURE_CLIMBING && GetMovementPosture() != MOVEMENT_POSTURE_CLIMBING_TO_CROUCH ) && random->RandomInt( 0, 10 ) < 3 )
			{
				if( GetActiveRnLWeapon() )
					DropActiveWeapon();
			}
		}
		// If they get hit in the leg
		else if( LastHitGroup() == HITGROUP_LEFTLEG || LastHitGroup() == HITGROUP_RIGHTLEG || LastHitGroup() == HITGROUP_RNL_UPPER_LEG || LastHitGroup() == HITGROUP_RNL_LOWER_LEG || LastHitGroup() == HITGROUP_RNL_FOOT)
		{
			AddDamageBasedSpeedModifier( 1, mp_legshotimpedance.GetFloat() );

			// Make sure their current stamina
			// is under the restricted maximum
			// due to the leg damage.
			float flMaxStamina = GetMaxStamina();

			if( GetStamina() > flMaxStamina )
				SetStamina( flMaxStamina );
		}

		bool bKnockedDown = IsAlive();

		if( bKnockedDown )
		{
			if( /*IsBot() ||*/ IsDeployed() || IsProne() )
				bKnockedDown = false;
			else if( newDamageInfo.GetDamageType() & DMG_BLAST && ( newDamageInfo.GetDamage() > 65 ) )
				bKnockedDown = true;
			else if( newDamageInfo.GetDamageType() & DMG_CLUB && ( newDamageInfo.GetDamage() > 55 || ( (GetHealth() - newDamageInfo.GetDamage()) <= 15 ) ) )
				bKnockedDown = true;
			else if( newDamageInfo.GetDamageType() & DMG_BULLET && ( newDamageInfo.GetDamage() > 60 || ( (GetHealth() - newDamageInfo.GetDamage()) <= 15 ) ) )
				bKnockedDown = true;
			else if( newDamageInfo.GetDamageType() & DMG_FALL && ( newDamageInfo.GetDamage() > 30 ) )
				bKnockedDown = true;
			else
				bKnockedDown = false;
		}

		// Drop to the ground sometimes
		if( bKnockedDown && ( random->RandomInt( 0, 10 ) <= 5 ) )
			KnockDown();
		// If they aren't knocked down, do the normal blur.
		else if( newDamageInfo.GetDamage() )
		{
			CSingleUserRecipientFilter bluruser( this );
			bluruser.MakeReliable();
			UserMessageBegin( bluruser, "BlurVision" /*"BlurScreen"*/ );
				WRITE_FLOAT( 0.04f ); // bluriness
				WRITE_FLOAT( 2.0f ); // duration
				WRITE_FLOAT( newDamageInfo.GetDamage() / 62.5f ); // scale
				WRITE_FLOAT( 1.0f ); // speed
			MessageEnd();
		}

		/*Michael Lebson
		Info for the damage report
		Taken from A4 code*/
		player_damage_info ammoInfo;
		CWeaponRnLBase *pRnLWeapon = NULL;

		if( newDamageInfo.GetWeapon() )
			pRnLWeapon = dynamic_cast<CWeaponRnLBase *>(newDamageInfo.GetWeapon());

		if( newDamageInfo.GetAttacker() && newDamageInfo.GetAttacker()->IsPlayer() )
		{
			CRnLPlayer *pAttackerPlayer = ToRnLPlayer( newDamageInfo.GetAttacker() );

			if( pAttackerPlayer )
			{
				char info[128];

				if( pAttackerPlayer == this )
					Q_snprintf( info, sizeof( info ), "inflicted by your own damn self." );
				else if( pAttackerPlayer->GetTeamNumber() == GetTeamNumber() )
					Q_snprintf( info, sizeof( info ), "inflicted by %s ( %s ). !-!-FRIENDLY FIRE-!-!", pAttackerPlayer->GetPlayerName(), pAttackerPlayer->GetNetworkIDString() );
				else
					Q_snprintf( info, sizeof( info ), "inflicted by %s", pAttackerPlayer->GetPlayerName() );

				ammoInfo.strInfo.Set( info );
			}
		}

		ammoInfo.iDamageAmount = newDamageInfo.GetDamage();

		if( pRnLWeapon )
			ammoInfo.iDamageType = pRnLWeapon->GetRnLWpnData().iAmmoType;
		else
		{
			if( newDamageInfo.GetDamageType() & DMG_BLAST )
				ammoInfo.iDamageType = -1;
			else
				ammoInfo.iDamageType = -2;
		}

		m_vPlayerDamageInfo.AddToTail( ammoInfo );
		// End Lebson
	}

	return BaseClass::OnTakeDamage( newDamageInfo );
}

// knock player down when wounded (start)
void CRnLPlayer::KnockDown()
{
	if( !IsAlive() || IsObserver() || GetTeamNumber() == TEAM_SPECTATOR || GetTeamNumber() == TEAM_UNASSIGNED )
		return;

	if( GetMovementPosture() == MOVEMENT_POSTURE_KNOCKDOWN )
		return;

	CWeaponRnLBase *pWeap = GetActiveRnLWeapon();
	if( pWeap )
	{
		pWeap->Holster(NULL);
		pWeap->m_flNextPrimaryAttack.Set(gpGlobals->curtime + 5.1f);
#ifdef CLIENT_DLL
		pWeap->m_flPrevPrimaryAttack = m_flNextPrimaryAttack;
#endif
	}

	CreateRagdollEntity(false);
	
	SetMovementPosture( MOVEMENT_POSTURE_KNOCKDOWN );
	SetViewOffset( VEC_PRONE_VIEW );

	color32 black = {0, 0, 0, 230};
	UTIL_ScreenFade(this, black, 0.5f, 1.0f, FFADE_OUT | FFADE_STAYOUT);

	// strong blur
	CSingleUserRecipientFilter bluruser( this );
	bluruser.MakeReliable();
	UserMessageBegin( bluruser, "BlurVision" );
		WRITE_FLOAT( 2.0f ); // bluriness
		WRITE_FLOAT( 5.0f ); // duration
		WRITE_FLOAT( 1.0f ); // scale
		WRITE_FLOAT( 0.2f ); // speed
	MessageEnd();

	SetMaxSpeed( 0.0f );
	SetAbsVelocity( Vector(0, 0, 0) );

	SetContextThink( &CRnLPlayer::KnockDownThink, gpGlobals->curtime + 5.0f, "KnockDownThink" );
}

// knock player down when wounded (end)
void CRnLPlayer::KnockDownThink()
{
	SetMovementPosture( MOVEMENT_POSTURE_PRONE );

	CRnLRagdoll *pRagdoll = dynamic_cast<CRnLRagdoll*>( m_hKnockDownRagdoll.Get() );	
	if( pRagdoll )
	{
		UTIL_Remove( pRagdoll );
	}

	if( GetActiveRnLWeapon() )
	{
		GetActiveRnLWeapon()->SetWeaponVisible(true);
		GetActiveRnLWeapon()->HandleViewAnimation( WEAPON_ANIMATION_DRAW );
	}

	color32 black = {0, 0, 0, 230};
	UTIL_ScreenFade(this, black, 3.0f, 0.0f, FFADE_PURGE | FFADE_IN);

	SetMaxSpeed( 250.0f );

	SetNextThink(0, "KnockDownThink");
}

extern ConVar sv_fadetoblack;
extern ConVar sv_deathtime;

void CRnLPlayer::Event_Killed( const CTakeDamageInfo &info )
{
	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.

	float flFadeTime = sv_deathtime.GetFloat();
	m_flDeathViewTime = gpGlobals->curtime + flFadeTime;

	if( sv_fadetoblack.GetBool() )
	{
		if( flFadeTime > 1 )
			flFadeTime -= 1;
		
		SetContextThink( &CRnLPlayer::FadeToBlackThink, gpGlobals->curtime + flFadeTime, "FadeToBlack" );
	}

	// Michael Lebson
	// Override the basecombatcharacter's weapon drop to
	// allow for cooking grenades to explode on death.
	// Without this override, the player would drop the nade
	// and it would do nothing.
	if( GetActiveRnLWeapon() )
	{
		// If this is a grenade and it isn't just being held
		// that means that the pin is already pulled.
		if( GetActiveRnLWeapon()->IsGrenade() )
		{
			CBaseRnLGrenade *pGren = static_cast<CBaseRnLGrenade*>(GetActiveWeapon());

			if( pGren && (pGren->GetGrenadeState() > GRENADE_DRAWN) )
				pGren->DropGrenade();
		}
		// Without this override, the grenade weapon would drop
		// in addition to the exploding grenade.
		else
			Weapon_Drop( GetActiveRnLWeapon(), NULL, NULL );
	}

	BaseClass::Event_Killed( info );

	// now look for nearby players and decrease their morale
	Vector absOrigin = GetAbsOrigin();
	for( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CRnLPlayer *pPlayer = ToRnLPlayer( UTIL_PlayerByIndex( i ) );
		if( pPlayer && pPlayer != this && pPlayer->IsAlive() )
		{
			if( pPlayer->GetAbsOrigin().DistTo( absOrigin ) < mp_moraledistance.GetFloat() )
			{
				if( pPlayer->GetTeamNumber() == GetTeamNumber() )
					pPlayer->SetMoraleLevel( ( pPlayer->GetMoraleLevel() - 15 ) );
				else
					pPlayer->SetMoraleLevel( ( pPlayer->GetMoraleLevel() + 10 ) );
			}
		}
	}

	// Michael Lebson
	// Remove any equipment they had when they died.
	RemoveEquipment(RNL_EQUIPMENT_ANY);

	// Decrement the team tickets remaining by one.
	if( RnLGameRules() && RnLGameRules()->GetGameManager() )
	{
		CRnLGameManager *pGame = RnLGameRules()->GetGameManager();
		if( pGame )
		{
			int iTickets = pGame->GetTicketsRemaining( GetTeamNumber() );

			if( iTickets > 0 )
				pGame->SetTicketsRemaining( GetTeamNumber(), iTickets - 1 );
		}
	}
	//End Lebson

	CreateRagdollEntity();

	//Damage report + teamkill kick/ban

	ClientPrint( this, HUD_PRINTCONSOLE, "----------------\nDamage Report\n----------------\n" );
	char output[128];
	if( info.GetAttacker() && info.GetAttacker() != this && info.GetAttacker()->IsPlayer() && info.GetAttacker()->GetTeamNumber() == GetTeamNumber() )
	{
		CRnLPlayer* pOtherPlayer = ToRnLPlayer( info.GetAttacker() );
		if( pOtherPlayer )
		{
			Q_snprintf( output, sizeof( output ), "Killed Through Friendly Fire by %s ( %s )", pOtherPlayer->GetPlayerName(), pOtherPlayer->GetNetworkIDString() );
			ClientPrint( this, HUD_PRINTCONSOLE, output );
			pOtherPlayer->m_iTeamKills++;
			if( mp_maxteamkills.GetInt() > 0 && pOtherPlayer->m_iTeamKills >= mp_maxteamkills.GetInt() )
			{
				char chCommand[64];
				UTIL_ClientPrintAll( HUD_PRINTCENTER, "#RnL_TeamKiller", pOtherPlayer->GetPlayerName() );

				if( mp_banteamkillers.GetBool() )
				{
					Q_snprintf( chCommand, sizeof( chCommand ), "banid %d %s\n", mp_teamkillerbantime.GetInt(), pOtherPlayer->GetNetworkIDString() );
					engine->ServerCommand( chCommand );
				}

				Q_snprintf( chCommand, sizeof( chCommand ), "kickid %d\n", pOtherPlayer->GetUserID() );
				engine->ServerCommand( chCommand );
			}
		}
	}
	ClientPrint( this, HUD_PRINTCONSOLE, "----------------\n" );
	for( int i = 0; i < m_vPlayerDamageInfo.Size(); i++ )
	{
		if( m_vPlayerDamageInfo[i].iDamageType > -1 )
		{
			Ammo_t* pAmmo = GetAmmoDef()->GetAmmoOfIndex( m_vPlayerDamageInfo[i].iDamageType );
			if( pAmmo )
				Q_snprintf( output, sizeof( output ), "   %s for %d %s", pAmmo->pName, m_vPlayerDamageInfo[i].iDamageAmount, m_vPlayerDamageInfo[i].strInfo.Get() );
		}
		else if( m_vPlayerDamageInfo[i].iDamageType == -1 )
		{
			Q_snprintf( output, sizeof( output ), "   Explosion damage for %d %s", m_vPlayerDamageInfo[i].iDamageAmount, m_vPlayerDamageInfo[i].strInfo.Get() );
		}
		else
		{
			Q_snprintf( output, sizeof( output ), "   Unnamed or non ammunition based damage for %d %s", m_vPlayerDamageInfo[i].iDamageAmount, m_vPlayerDamageInfo[i].strInfo.Get() );
		}
		ClientPrint( this, HUD_PRINTCONSOLE, output );
	}

	ClientPrint( this, HUD_PRINTCONSOLE, "----------------\n" );
	m_vPlayerDamageInfo.Purge();

	//End damage report + teamkill kick/ban
}

void CRnLPlayer::FadeToBlackThink()
{
	SetNextThink( 0, "FadeToBlack" );

	color32 black = {0, 0, 0, 255};
	UTIL_ScreenFade(this, black, 0.5f, 1.0f, FFADE_OUT | FFADE_STAYOUT);
}

void CRnLPlayer::CreateRagdollEntity(bool bDead)
{
	CRnLRagdoll *pRagdoll = NULL;

	// If we already have a ragdoll, don't make another one.
	if (bDead) 
	{
		if( m_hKnockDownRagdoll.Get() )
			UTIL_Remove( m_hKnockDownRagdoll.Get() );
		
		pRagdoll = dynamic_cast< CRnLRagdoll* >( m_hRagdoll.Get() );
	}
	else // leave the dead ragdoll alone
		pRagdoll = dynamic_cast< CRnLRagdoll* >( m_hKnockDownRagdoll.Get() );

	if ( pRagdoll )
		UTIL_Remove( pRagdoll );

	// create a new one
	pRagdoll = dynamic_cast< CRnLRagdoll* >( CreateEntityByName( "rnl_ragdoll" ) );

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = Vector(0,0,0);
		pRagdoll->ChangeTeam( GetTeamNumber() );
		pRagdoll->m_nBody = m_nBody;
		pRagdoll->m_nSkin = m_nSkin;
	}

	// ragdolls will be removed on round restart automatically
	if (bDead == true)
		m_hRagdoll = pRagdoll;
	else
		m_hKnockDownRagdoll = pRagdoll;

}

void CRnLPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	if ( event == PLAYERANIMEVENT_THROW_GRENADE )
	{
		// Grenade throwing has to synchronize exactly with the player's grenade weapon going away,
		// and events get delayed a bit, so we let CCSPlayerAnimState pickup the change to this
		// variable.
		m_iThrowGrenadeCounter = (m_iThrowGrenadeCounter+1) % (1<<THROWGRENADE_COUNTER_BITS);
	}
	else
	{
		m_PlayerAnimState->DoAnimationEvent( event, nData );
		TE_PlayerAnimEvent( this, event, nData );	// Send to any clients who can see this guy.
	}
}

CWeaponRnLBase* CRnLPlayer::GetActiveRnLWeapon() const
{
	return dynamic_cast< CWeaponRnLBase* >( GetActiveWeapon() );
}

//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CRnLPlayer::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CWeaponRnLBase* pRnLWeapon = dynamic_cast<CWeaponRnLBase*>(pWeapon);
	if( pRnLWeapon != NULL )
	{
		if( pRnLWeapon->CanPickup( this ) == false )
			return false;
	}

	CBaseCombatWeapon *pSlot = Weapon_GetSlot( pWeapon->GetSlot() );

	if( pSlot != NULL )
	{
		if( pSlot->GetPosition() == pWeapon->GetPosition() )
			return false;
	}
	
	return BaseClass::BumpWeapon( pWeapon );
}

void CRnLPlayer::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "predicted_viewmodel" );
	if ( vm )
	{
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
	}
}

void CRnLPlayer::CheatImpulseCommands( int iImpulse )
{
	if ( !sv_cheats->GetBool() )
	{
		return;
	}

	if ( iImpulse != 101 )
	{
		BaseClass::CheatImpulseCommands( iImpulse );
		return ;
	}
	gEvilImpulse101 = true;

	EquipSuit();

	if( GetTeamNumber() == TEAM_ALLIES )
	{
		GiveNamedItem( "weapon_alliedfists" );
		GiveNamedItem( "weapon_garand" );
		GiveNamedItem( "weapon_colt" );

		GiveAmmo( 56, AMMO_GARAND );
		GiveAmmo( 270, AMMO_THOMPSON );
		GiveAmmo( 185, AMMO_BAR );
		GiveAmmo( 150, AMMO_BROWNING );
		GiveAmmo( 90, AMMO_CARBINE );
		GiveAmmo( 14, AMMO_COLT );
	}
	else if( GetTeamNumber() == TEAM_AXIS )
	{
		GiveNamedItem( "weapon_axisfists" );
		GiveNamedItem( "weapon_k98k" );
		GiveNamedItem( "weapon_walther" );

		GiveAmmo( 45, AMMO_K98K );
		GiveAmmo( 288, AMMO_MP40 );
		GiveAmmo( 250, AMMO_MG42 );
		GiveAmmo( 80, AMMO_G43 );
		GiveAmmo( 16, AMMO_WALTHER );
	}

	if ( GetHealth() < 100 )
	{
		TakeHealth( 25, DMG_GENERIC );
	}

	gEvilImpulse101 = false;
}

//==============================================
//-----------------------------------------------------------------------------
// Purpose: Create and give the named item to the player. Then return it.
//-----------------------------------------------------------------------------
CBaseEntity *CRnLPlayer::GiveNamedItem( const char *pszName, int iSubType )
{
	if( strcmp( pszName, "parachute" ) == 0 )
	{
		AddEquipment(RNL_EQUIPMENT_PARACHUTE);
		return m_pRnLParachute;
	}
	else if( strcmp( pszName, "radio" ) == 0 )
	{
		AddEquipment(RNL_EQUIPMENT_RADIO);
		return m_pRnLRadio;
	}
	else
	{
		return BaseClass::GiveNamedItem( pszName, iSubType );
	}
}

void CRnLPlayer::AddEquipment(RnLEquipmentTypes_t iType )
{
	if( iType == RNL_EQUIPMENT_PARACHUTE)
	{
		if( m_pRnLParachute == NULL )
		{
			m_pRnLParachute = dynamic_cast< CRnLEquipmentParachute* >(CreateEntityByName( "rnl_equipment_parachute" ));

			if( m_pRnLParachute )
			{
				m_pRnLParachute->Spawn();
				m_pRnLParachute->SetAbsOrigin( GetAbsOrigin() );
				m_pRnLParachute->FollowEntity( this );
			}
		}
	}
	/*else if( iType == RNL_EQUIPMENT_RADIO )
	{
		if( m_pRnLRadio == NULL )
		{
			m_pRnLRadio = dynamic_cast< CRnLEquipmentRadio* >(CreateEntityByName( "rnl_equipment_radio" ));

			if( m_pRnLRadio )
			{
				m_pRnLRadio->Spawn();
				m_pRnLRadio->SetAbsOrigin( GetAbsOrigin() );
				m_pRnLRadio->FollowEntity( this );
			}
		}
	}*/
}

void CRnLPlayer::RemoveEquipment(RnLEquipmentTypes_t iType )
{
	if( iType == RNL_EQUIPMENT_PARACHUTE || iType == RNL_EQUIPMENT_ANY)
	{
		if( m_pRnLParachute )
		{
			m_pRnLParachute->StopFollowingEntity();
			UTIL_Remove( m_pRnLParachute );
			SetMovementPosture( MOVEMENT_POSTURE_STAND );
			DoAnimationEvent( PLAYERANIMEVENT_ENDPARACHUTING );
			m_pRnLParachute = NULL;
		}
	}

	if( iType == RNL_EQUIPMENT_RADIO || iType == RNL_EQUIPMENT_ANY)
	{
		if( m_pRnLRadio )
		{
			m_pRnLRadio->StopFollowingEntity();
			UTIL_Remove( m_pRnLRadio );
			m_pRnLRadio = NULL;
		}
	}
}

void CRnLPlayer::OpenParachute( void )
{
	if( m_pRnLParachute )
	{
		m_pRnLParachute->Deploy();
		SetMovementPosture( MOVEMENT_POSTURE_PARACHUTING );
		ViewPunch( QAngle( 180, 200, 140 ) );
		DoAnimationEvent( PLAYERANIMEVENT_STARTPARACHUTING );
	}
}

extern ConVar mp_limitteams;
extern ConVar* sv_cheats;

bool CRnLPlayer::ClientCommand( const CCommand &args )
{
	const char *pcmd = args[0];

	if( stricmp( pcmd, SHOW_TEAM_SELECTION ) == 0 )
	{
		ShowViewPortPanel( PANEL_TEAM, true );
		ShowViewPortPanel( PANEL_SQUAD_ALLIES, false );
		ShowViewPortPanel( PANEL_SQUAD_AXIS, false );
		return true;
	}
	else if( stricmp( pcmd, SHOW_SQUAD_SELECTION ) == 0 )
	{
		ShowViewPortPanel( PANEL_TEAM, false );
		if( GetTeamNumber() == TEAM_ALLIES )
		{
			ShowViewPortPanel( PANEL_SQUAD_ALLIES, true );
			ShowViewPortPanel( PANEL_SQUAD_AXIS, false );
		}
		else if( GetTeamNumber() == TEAM_AXIS )
		{
			ShowViewPortPanel( PANEL_SQUAD_ALLIES, false );
			ShowViewPortPanel( PANEL_SQUAD_AXIS, true );
		}

		return true;
	}
	/*
	else if( stricmp( pcmd, "dev_custom_model" ) == 0 )
	{
		if( args.ArgC() > 1 )
			SetCustomModel( atoi( args.Arg( 1 ) ) );
		return true;
	}
	else if( stricmp( pcmd, SHOW_LOADOUT_SELECTION ) == 0 )
	{
		if( args.ArgC() < 2 )
			ShowViewPortPanel( PANEL_LOADOUT, true );
		else
			ChangeLoadout( args );
		return true;
	}
	*/
	else if( stricmp( pcmd, SELECT_TEAM ) == 0 )
	{
		if( args.ArgC() > 1 )
		{
			int iTeam = atoi( args.Arg( 1 ) );

			if( iTeam != GetTeamNumber() && (iTeam > TEAM_INVALID && iTeam < TEAMS_COUNT) )
			{
				CRnLTeam *pAllies = GetGlobalRnLTeam( TEAM_ALLIES );
				CRnLTeam *pAxis = GetGlobalRnLTeam( TEAM_AXIS );

				if ( pAllies == NULL || pAxis == NULL )
					ChangeTeam( random->RandomInt( TEAM_ALLIES, TEAM_AXIS ) );
				else if( iTeam > 0 )
				{
					if( iTeam == TEAM_SPECTATOR ||
						((iTeam == TEAM_ALLIES) && (pAllies->GetNumPlayers() <= (pAxis->GetNumPlayers()	+ mp_limitteams.GetInt()))) ||
						((iTeam == TEAM_AXIS)	&& (pAxis->GetNumPlayers()   <= (pAllies->GetNumPlayers() + mp_limitteams.GetInt()))) )
					{
						ChangeTeam( iTeam );
					}
					else
					{
						ShowViewPortPanel( PANEL_TEAM );
						ClientPrint( this, HUD_PRINTCENTER, "#RnL_TeamFull" );
					}
				}
				else
				{
					if ( pAllies->GetNumPlayers() > pAxis->GetNumPlayers() )
					{
						ChangeTeam( TEAM_AXIS );
					}
					else if ( pAllies->GetNumPlayers() < pAxis->GetNumPlayers() )
					{
						ChangeTeam( TEAM_ALLIES );
					}
					else
					{
						ChangeTeam( random->RandomInt( TEAM_ALLIES, TEAM_AXIS ) );
					}
				}
			}
		}
		return true;
	}
	else if( stricmp( pcmd, SELECT_SQUAD ) == 0 )
	{
		if( args.ArgC() > 2 )
		{
			int iSquad = atoi( args.Arg( 1 ) );
			int iSlot = atoi( args.Arg( 2 ) );

			if( !ChangeSquad( iSquad, iSlot ) )
			{
				if( GetTeamNumber() == TEAM_ALLIES )
					ShowViewPortPanel( PANEL_SQUAD_ALLIES, true );
				else if( GetTeamNumber() == TEAM_AXIS )
					ShowViewPortPanel( PANEL_SQUAD_AXIS, true );
			}
		}
		return true;
	}
	else if( stricmp( pcmd, SELECT_CLASS ) == 0 )
	{
	}
	else if( stricmp( pcmd, VOTE_SQUAD_LEADER ) == 0 )
	{
		if( GetTeamNumber() == TEAM_AXIS || GetTeamNumber() == TEAM_ALLIES )
		{
			if( args.ArgC() > 1 )
			{
				m_iSquadLeaderVote = atoi( args[1] );

				DevMsg("%s voted for player #%i\n", GetPlayerName(), m_iSquadLeaderVote);

				if( m_iSquadLeaderVote < 1 || m_iSquadLeaderVote > gpGlobals->maxClients )
					m_iSquadLeaderVote = 0;
				else
				{
					CHandle<CRnLPlayer> votedPlayer = m_iSquadLeaderVote;
					if( votedPlayer.Get() != NULL )
					{
						if( votedPlayer->GetTeamNumber() != GetTeamNumber() ||
							votedPlayer->GetSquadNumber() != GetSquadNumber() )
							m_iSquadLeaderVote = 0;
					}
					else
					{
						m_iSquadLeaderVote = 0;
					}
				}
			}
		}
		return true;
	}
	// cjd @add
	else if( stricmp( pcmd, SHOUT_CMD ) == 0 )
	{
		// Get the vocalization string
		const char *pVocString = args.ArgV()[1];
		PlayVocalization( pVocString );
		return true;
	}
	else if( stricmp( pcmd, RADIO_CMD ) == 0 )
	{
		const char *pVocString = args.ArgV()[1];
		PlayVocalization( pVocString, true );
		return true;
	}
	// cjd end
	else if( FStrEq( pcmd, "owie_owie_owie" ) && sv_cheats->GetBool() )
	{
		trace_t tr;
		Vector forward;
		EyeVectors( &forward );

		UTIL_TraceLine( EyePosition() * (forward * 32), GetAbsOrigin() + Vector( 0, 0, 15 ), MASK_SHOT, NULL, COLLISION_GROUP_NONE, &tr );
		if( tr.m_pEnt != this )
			tr.m_pEnt = this;

		forward.Negate();

		CTakeDamageInfo info( this, this, 10, DMG_BULLET );
		CalculateBulletDamageForce( &info, GetAmmoDef()->Index( AMMO_GARAND ), forward, tr.endpos );
		DispatchTraceAttack( info, forward, &tr );
		TraceAttackToTriggers( info, tr.startpos, tr.endpos, forward );
		ApplyMultiDamage();
		return true;
	}
	else if ( FStrEq( pcmd, "moralehit" ) && sv_cheats->GetBool() )
	{
		if( GetMoraleLevel() > 10 )
		{
			DevMsg("Morale changed from %i to ", GetMoraleLevel());
			SetMoraleLevel( GetMoraleLevel() - 10 );
			DevMsg("%i\n", GetMoraleLevel());
		}

		return true;
	}
	else if ( FStrEq( pcmd, "stamhit" ) && sv_cheats->GetBool() )
	{
		if( GetStamina() > 10.0f )
		{
			DevMsg("Morale changed from %i to ", GetStamina());
			SetStamina( GetStamina() - 10.0f );
			DevMsg("%i\n", GetStamina());
		}

		return true;
	}
	else if ( FStrEq( pcmd, "knockdown" ) && sv_cheats->GetBool() )
	{
		KnockDown();
		return true;
	}

	return BaseClass::ClientCommand( args );
}

// Team Handling
void CRnLPlayer::ChangeTeam( int iTeamNum )
{
	int iOldTeam = GetTeamNumber();

	BaseClass::ChangeTeam( iTeamNum );

	if( iOldTeam != GetTeamNumber() )
	{
		if( m_iSquadNumber > -1 )
		{
			CRnLGameTeam* pOldTeam = (CRnLGameTeam*)GetGlobalRnLTeam( iOldTeam );
			if( pOldTeam )
			{
				CRnLSquad* pOldSquad = pOldTeam->GetSquad( m_iSquadNumber );
				if( pOldSquad )
				{
					pOldSquad->RemovePlayer( this );
				}
			}
		}

		m_iPreviousSquadNumber = m_iSquadNumber;
		m_iPreviousKitNumber = m_iKitNumber;

		m_iSquadNumber = RNL_SQUAD_INVALID;
		m_iKitNumber = RNL_KIT_INVALID;

		if( IsAlive() && (iOldTeam == TEAM_AXIS || iOldTeam == TEAM_ALLIES) )
			CommitSuicide( false, true );

		if( GetTeamNumber() == TEAM_ALLIES )
			ShowViewPortPanel( PANEL_SQUAD_ALLIES, true );
		else if( GetTeamNumber() == TEAM_AXIS )
			ShowViewPortPanel( PANEL_SQUAD_AXIS, true );
		else if( GetTeamNumber() == TEAM_SPECTATOR )
			StartObserverMode( m_iObserverLastMode );
	}
}

bool CRnLPlayer::ChangeSquad( int iSquad, int iSlot )
{
	if ( !GetGlobalRnLTeam( GetTeamNumber() ) )
		return false;

	CRnLGameTeam* pTeam = dynamic_cast<CRnLGameTeam*>(GetGlobalRnLTeam( GetTeamNumber() ));

	if( !pTeam )
		return false;

	if( iSquad < 0 || iSquad >= pTeam->GetNumberOfSquads() )
		return false;

	CRnLSquad* pSquad = pTeam->GetSquad( iSquad );
	if( !pSquad )
		return false;

	if( !pSquad->IsKitAvailable( iSlot ) )
		return false;

	int oldVote = m_iSquadLeaderVote;
	m_iSquadLeaderVote = 0;

	if( m_iSquadNumber > RNL_SQUAD_INVALID )
	{
		CRnLGameTeam* pOldTeam = (CRnLGameTeam*)GetGlobalRnLTeam( GetTeamNumber() );
		if( pOldTeam )
		{
			CRnLSquad* pOldSquad = pOldTeam->GetSquad( m_iSquadNumber );
			if( pOldSquad )
			{
				if( pOldSquad == pSquad )
					m_iSquadLeaderVote = oldVote;

				pOldSquad->RemovePlayer( this );
			}
		}
	}

	// Immediately tell all clients that he's changing team. This has to be done
	// first, so that all user messages that follow as a result of the team change
	// come after this one, allowing the client to be prepared for them.
	IGameEvent * event = gameeventmanager->CreateEvent( "player_squad" );
	if ( event )
	{
		event->SetInt("userid", GetUserID());
		event->SetInt("squad", iSquad);
		event->SetInt("slot", iSlot);
		event->SetBool("disconnect", IsDisconnecting());

		gameeventmanager->FireEvent( event );
	}

	if( pSquad->AddPlayer( this, iSlot ) )
		m_iSquadNumber = iSquad;
	else
		m_iSquadNumber = -1;
	
	return (m_iSquadNumber != -1);
}

bool CRnLPlayer::IsReadyToSpawn( void )
{
	if( m_flDeathViewTime > gpGlobals->curtime )
		return false;

	if( GetTeamNumber() == TEAM_AXIS || GetTeamNumber() == TEAM_ALLIES )
	{
		if( m_iSquadNumber > RNL_SQUAD_INVALID && m_iKitNumber > RNL_KIT_INVALID )
			return true;
		else
			return false;
	}
	return true;
}

void CRnLPlayer::ResetPerRoundStats( void )
{
	return;
}

// cjd @add
void CRnLPlayer::PlayVocalization( const char *pszVocalizationName, bool bRadio /* = false */ )
{
	if( m_flNextVocalizationTime <= gpGlobals->curtime )
		m_flNextVocalizationTime = gpGlobals->curtime + 1.5f;
	else
		return;

	if( bRadio )
	{
		CBroadcastRecipientFilter bcFilter;
		bcFilter.AddRecipientsByTeam( GetTeam() );
		UserMessageBegin( bcFilter, "Vocalization" );
			WRITE_BYTE( 1 );
			WRITE_BYTE( entindex() );
			WRITE_STRING( pszVocalizationName );
		MessageEnd();
	}
	else
	{
		CPASFilter pasFilter( GetAbsOrigin() );
		UserMessageBegin( pasFilter, "Vocalization" );
			WRITE_BYTE( 0 );
			WRITE_BYTE( entindex() );
			WRITE_STRING( pszVocalizationName );
		MessageEnd();
	}
}
// cjd end

//-----------------------------------------------------------------------------
// Purpose: Cheat to create a jeep in front of the player
//-----------------------------------------------------------------------------
static void MakeZeeBroomBroom( CBasePlayer *pPlayer )
{
	Vector vecForward;
	AngleVectors( pPlayer->EyeAngles(), &vecForward );
	CBaseEntity *pJeep = (CBaseEntity *)CreateEntityByName( "prop_vehicle_jeep" );
	if ( pJeep )
	{
		Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 256 + Vector(0,0,64);
		QAngle vecAngles( 0, pPlayer->GetAbsAngles().y - 90, 0 );
		pJeep->SetAbsOrigin( vecOrigin );
		pJeep->SetAbsAngles( vecAngles );
		if ( pPlayer->GetTeamNumber() == TEAM_AXIS  )
		{
			pJeep->KeyValue( "model", "models/opel_blitz_driveable.mdl" );
			pJeep->KeyValue( "vehiclescript", "scripts/vehicles/opel_blitz.txt" );
		}
		else
		{
			pJeep->KeyValue( "model", "models/ze_broom_broom.mdl" );
			pJeep->KeyValue( "vehiclescript", "scripts/vehicles/jeep_test.txt" );
		}
		pJeep->KeyValue( "solid", "6" );
		pJeep->KeyValue( "targetname", "ze_broom_broom" );
		pJeep->Spawn();
		pJeep->Activate();
		pJeep->Teleport( &vecOrigin, &vecAngles, NULL );

		pPlayer->StopSound( "Weapon_Garand.Special" );
		pPlayer->EmitAmbientSound( pPlayer->entindex(), pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset(), "Weapon_Garand.Special" );
	}
}


void CC_MakeZeeBroomBroom( void )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;
	MakeZeeBroomBroom( pPlayer );
}

static ConCommand make_ze_broom_broom("make_ze_broom_broom", CC_MakeZeeBroomBroom, "Make ze Broom Broom in front of the player.", FCVAR_CHEAT);

		

// Michael Lebson

//----------------------------------------------------------------------------- 
// Purpose: Drops player's current weapon
//-----------------------------------------------------------------------------
void CRnLPlayer::DropActiveWeapon( void )
{
	if( IsAlive() && ( GetTeamNumber() == TEAM_ALLIES || GetTeamNumber() == TEAM_AXIS ) )
	{
		if( GetMovementPosture() != MOVEMENT_POSTURE_CLIMBING && 
			GetMovementPosture() != MOVEMENT_POSTURE_CLIMBING_TO_CROUCH && 
			GetMovementPosture() != MOVEMENT_POSTURE_PARACHUTING && 
			GetMoveType() != MOVETYPE_LADDER &&
			IsDeployed() == false )
		{
			CWeaponRnLBase* pRnLWep = GetActiveRnLWeapon();

			if( pRnLWep )
			{
				if( pRnLWep->CanHolster() && pRnLWep->GetWeaponID() != WEAPON_NCOMAP && pRnLWep->GetWeaponID() != WEAPON_AXISFISTS && pRnLWep->GetWeaponID() != WEAPON_ALLIEDFISTS )
				{
					Vector vecThrow;
					float throwForce;

					if ( pRnLWep->IsMachineGun() )
						throwForce = IsPlayer() ? 5000.0f : random->RandomInt( 64, 128 );
					else
						throwForce = IsPlayer() ? 400.0f : random->RandomInt( 64, 128 );

					if ( pRnLWep->IsGrenade() )
						SetAmmoCount( 0, pRnLWep->GetPrimaryAmmoType() );

					vecThrow = BodyDirection2D() * throwForce;
					Weapon_Drop( pRnLWep, NULL, &vecThrow );
				}
			}
		}
	}
}

//----------------------------------------------------------------------------- 
// Purpose: Drops player's current weapon
//-----------------------------------------------------------------------------
void CC_Player_Drop( void )
{
	CRnLPlayer *pPlayer = ToRnLPlayer( UTIL_GetCommandClient() );

	if( !pPlayer )
		return;

	pPlayer->DropActiveWeapon();
}

static ConCommand drop("drop", CC_Player_Drop, "drop: Drops the player's current weapon.");

//----------------------------------------------------------------------------- 
// Purpose: Toggles duck
//-----------------------------------------------------------------------------
void CC_Player_DuckToggle( void )
{
	CRnLPlayer *pPlayer = ToRnLPlayer( UTIL_GetCommandClient() );

	if( !pPlayer )
		return;

	pPlayer->ToggleDuck();
}

static ConCommand ducktoggle("ducktoggle", CC_Player_DuckToggle, "ducktoggle: Toggles whether the player is ducking or not.");

//----------------------------------------------------------------------------- 
// Purpose: Holster player's current weapon
//-----------------------------------------------------------------------------
void CC_Player_Holster( void )
{
	CRnLPlayer *pPlayer = ToRnLPlayer( UTIL_GetCommandClient() );

	if( !pPlayer )
		return;

	if( pPlayer->IsAlive() && (pPlayer->GetTeamNumber() == TEAM_ALLIES || pPlayer->GetTeamNumber() == TEAM_AXIS) )
	{
		if( pPlayer->GetMovementPosture() != MOVEMENT_POSTURE_CLIMBING && 
			pPlayer->GetMovementPosture() != MOVEMENT_POSTURE_CLIMBING_TO_CROUCH && 
			pPlayer->GetMovementPosture() != MOVEMENT_POSTURE_PARACHUTING && 
			pPlayer->GetMoveType() != MOVETYPE_LADDER &&
			pPlayer->IsDeployed() == false )
		{
			CWeaponRnLBase* pRnLFists = static_cast<CWeaponRnLBase*>(pPlayer->Weapon_GetSlot(20));
			
			if( pRnLFists && (pRnLFists->GetWeaponID() == WEAPON_ALLIEDFISTS || pRnLFists->GetWeaponID() == WEAPON_AXISFISTS) )
				pPlayer->SelectItem( pRnLFists->GetClassname(), pRnLFists->GetSubType() );
		}
	}
}

static ConCommand holster("holster", CC_Player_Holster, "holster: Holsters the player's active weapon.");

