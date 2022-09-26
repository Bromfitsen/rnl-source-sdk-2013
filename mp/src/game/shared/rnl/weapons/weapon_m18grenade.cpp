//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_rnl_base.h"
#include "gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "weapon_m18grenade.h"


#ifdef CLIENT_DLL
	
#else

	#include "rnl_player.h"
	#include "items.h"
	#include "rnl_basegrenade_projectile.h"
	#include "particle_smokegrenade.h"

#endif


#define GRENADE_TIMER	3.0f //Seconds

IMPLEMENT_NETWORKCLASS_ALIASED( M18Grenade, DT_M18Grenade )

BEGIN_NETWORK_TABLE(CM18Grenade, DT_M18Grenade)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CM18Grenade )
END_PREDICTION_DATA()

acttable_t CM18Grenade::m_acttable[] = 
{
	{ ACT_SPRINT,			ACT_DOD_SPRINT_AIM_GREN_FRAG,			false },

	{ ACT_DOD_STAND_AIM,	ACT_DOD_STAND_AIM_GREN_FRAG,			false },
	{ ACT_DOD_STAND_IDLE,	ACT_DOD_STAND_AIM_GREN_FRAG,			false },
	
	{ ACT_DOD_CROUCH_AIM,	ACT_DOD_CROUCH_AIM_GREN_FRAG,			false },
	{ ACT_DOD_CROUCH_IDLE,	ACT_DOD_CROUCH_AIM_GREN_FRAG,			false },

	{ ACT_DOD_CROUCHWALK_AIM,	ACT_DOD_CROUCHWALK_AIM_GREN_FRAG,	false },
	{ ACT_DOD_CROUCHWALK_IDLE,	ACT_DOD_CROUCHWALK_AIM_GREN_FRAG,	false },
	
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_AIM_GREN_FRAG,		false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_GREN_FRAG,			false },
	
	{ ACT_DOD_RUN_AIM,		ACT_DOD_RUN_AIM_GREN_FRAG,				false },
	{ ACT_DOD_RUN_IDLE,		ACT_DOD_RUN_AIM_GREN_FRAG,			false },
	
	{ ACT_DOD_WALK_AIM,		ACT_DOD_WALK_AIM_GREN_FRAG,			false },
	{ ACT_DOD_WALK_IDLE,	ACT_DOD_WALK_AIM_GREN_FRAG,			false },

	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_GREN_FRAG,	false },
	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PRIMARYATTACK_PRONE_GREN_FRAG, false },
};

IMPLEMENT_ACTTABLE(CM18Grenade);

LINK_ENTITY_TO_CLASS( weapon_m18grenade, CM18Grenade );
PRECACHE_WEAPON_REGISTER( weapon_m18grenade );


#ifdef GAME_DLL

#define M18GRENADE_MODEL "models/weapons/w_m18_thrown.mdl"

//this 
class CM18GrenadeProjectile : public CBaseGrenadeProjectile
{
public:
	DECLARE_CLASS( CM18GrenadeProjectile, CBaseGrenadeProjectile );
	DECLARE_DATADESC();

	// Overrides.
public:
	virtual void Spawn()
	{
		if( mp_melontastic.GetBool() )
			SetModel( MELON_MODEL );
		else
			SetModel( M18GRENADE_MODEL );
		BaseClass::Spawn();
	}

	virtual void Precache()
	{
		PrecacheModel( M18GRENADE_MODEL );
		PrecacheModel( MELON_MODEL );
		PrecacheScriptSound( "BaseSmokeEffect.Sound" );
		PrecacheScriptSound( "BaseSmokeEmitEffect.Sound" );
		BaseClass::Precache();
	}

	// Grenade stuff.
public:

	virtual void Detonate( void )
	{
		EmitSound( "BaseSmokeEffect.Sound" );

		SetThink( &CM18GrenadeProjectile::SmokeEmit );
		SetNextThink( gpGlobals->curtime + 1.0f );
	}

	virtual void SmokeEmit()
	{
		EmitSound( "BaseSmokeEmitEffect.Sound" );

		Vector		vecSpot;
		vecSpot = GetAbsOrigin() + Vector ( 0 , 0 , 8 );

		ParticleSmokeGrenade *pSmoke = dynamic_cast<ParticleSmokeGrenade*>( CreateEntityByName(PARTICLESMOKEGRENADE_ENTITYNAME) );
		if ( pSmoke )
		{
			Vector vForward;
			AngleVectors( GetLocalAngles(), &vForward );
			vForward.z = 0;
			VectorNormalize( vForward );

			if( GetThrower() )
			{
				pSmoke->ChangeTeam( GetThrower()->GetTeamNumber() );
			}

			pSmoke->SetLocalOrigin( vecSpot );
			pSmoke->SetFadeTime(25, 30);	// Fade out between 25 seconds and 30 seconds.
			pSmoke->Activate();
			pSmoke->SetLifetime(30);
			pSmoke->FillVolume();
		}

		SetThink( &CBaseEntity::SUB_Remove );
		SetNextThink( gpGlobals->curtime + 45.0f );
	}

	static CM18GrenadeProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner, 
		float timer, bool roll )
	{
		CM18GrenadeProjectile *pGrenade = (CM18GrenadeProjectile*)CBaseEntity::Create( "m18grenade_projectile", position, angles, pOwner );

		// Set the timer for 1 second less than requested. We're going to issue a SOUND_DANGER
		// one second before detonation.

		pGrenade->SetDetonateTimerLength( timer );
		pGrenade->SetAbsVelocity( velocity );
		pGrenade->SetupInitialTransmittedGrenadeVelocity( velocity );
		pGrenade->SetThrower( pOwner ); 

		if( !roll )
		{
			pGrenade->SetGravity( GetGrenadeGravity() );
			pGrenade->SetFriction( BaseClass::GetGrenadeFriction() );
			pGrenade->SetElasticity( BaseClass::GetGrenadeElasticity() );
		}
		else
		{
			pGrenade->SetGravity( BaseClass::GetRollGravity() );
			pGrenade->SetFriction( BaseClass::GetRollFriction() );
			pGrenade->SetElasticity( BaseClass::GetRollElasticity() );
		}

		pGrenade->m_flDamage = 200;
		pGrenade->m_DmgRadius = 500;
		pGrenade->ChangeTeam( pOwner->GetTeamNumber() );
		pGrenade->ApplyLocalAngularVelocityImpulse( angVelocity );	

		// make NPCs afaid of it while in the air
		pGrenade->SetThink( &CM18GrenadeProjectile::DangerSoundThink );
		pGrenade->SetNextThink( gpGlobals->curtime );

		return pGrenade;
	}
};

LINK_ENTITY_TO_CLASS( m18grenade_projectile, CM18GrenadeProjectile );
PRECACHE_WEAPON_REGISTER( m18grenade_projectile );

BEGIN_DATADESC( CM18GrenadeProjectile )
	DEFINE_THINKFUNC( SmokeEmit ),
END_DATADESC()

BEGIN_DATADESC( CM18Grenade )
END_DATADESC()

void CM18Grenade::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, float flDelay, bool roll )
{
	// remove a grenade from the players third person
	//CRnLPlayer *pRnLPlayer = dynamic_cast< CRnLPlayer* >( pPlayer );
	//pRnLPlayer->RemoveEquipmentFromPlayer( EQUIPMENT_ALLIES_SMOKEGRENADE );

	CM18GrenadeProjectile* pGrenade = CM18GrenadeProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, GRENADE_TIMER, roll );

	if( !pGrenade )
	{
		DevMsg( "Oh Noes Grenade projectile didn't get created.\n" );
	}
}

#endif

