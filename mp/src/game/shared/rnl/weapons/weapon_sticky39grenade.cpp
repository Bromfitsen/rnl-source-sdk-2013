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
#include "weapon_sticky39grenade.h"


#ifdef CLIENT_DLL
	
#else

	#include "rnl_player.h"
	#include "items.h"
	#include "rnl_basegrenade_projectile.h"
	#include "particle_smokegrenade.h"

#endif


#define GRENADE_TIMER	3.0f //Seconds

IMPLEMENT_NETWORKCLASS_ALIASED( Sticky39Grenade, DT_Sticky39Grenade )

BEGIN_NETWORK_TABLE(CSticky39Grenade, DT_Sticky39Grenade)
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CSticky39Grenade )
END_PREDICTION_DATA()
#endif

acttable_t CSticky39Grenade::m_acttable[] = 
{
	{ ACT_SPRINT,			ACT_DOD_SPRINT_AIM_GREN_STICK,			false },

	{ ACT_DOD_STAND_AIM,	ACT_DOD_STAND_AIM_GREN_STICK,			false },
	{ ACT_DOD_STAND_IDLE,	ACT_DOD_STAND_AIM_GREN_STICK,			false },
	
	{ ACT_DOD_CROUCH_AIM,	ACT_DOD_CROUCH_AIM_GREN_STICK,			false },
	{ ACT_DOD_CROUCH_IDLE,	ACT_DOD_CROUCH_AIM_GREN_STICK,			false },

	{ ACT_DOD_CROUCHWALK_AIM,	ACT_DOD_CROUCHWALK_AIM_GREN_STICK,	false },
	{ ACT_DOD_CROUCHWALK_IDLE,	ACT_DOD_CROUCHWALK_AIM_GREN_STICK,	false },
	
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_AIM_GREN_STICK,		false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONE_AIM_GREN_STICK,			false },
	
	{ ACT_DOD_RUN_AIM,		ACT_DOD_RUN_AIM_GREN_STICK,				false },
	{ ACT_DOD_RUN_IDLE,		ACT_DOD_RUN_AIM_GREN_STICK,			false },
	
	{ ACT_DOD_WALK_AIM,		ACT_DOD_WALK_AIM_GREN_STICK,			false },
	{ ACT_DOD_WALK_IDLE,	ACT_DOD_WALK_AIM_GREN_STICK,			false },

	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_GREN_STICK,	false },
	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PRIMARYATTACK_PRONE_GREN_STICK, false },
};

IMPLEMENT_ACTTABLE(CSticky39Grenade);

LINK_ENTITY_TO_CLASS( weapon_sticky39grenade, CSticky39Grenade );
PRECACHE_WEAPON_REGISTER( weapon_sticky39grenade );

bool CSticky39Grenade::CanHolster( void )
{
	return ((m_iWeaponAnimationState != WEAPON_ANIMATION_HOLSTER && m_iWeaponAnimationState != WEAPON_ANIMATION_DRAW) || IsSequenceFinished() ) && (m_iGrenadeState <= GRENADE_PREP);
}

#ifdef GAME_DLL

#define STICK39_GRENADE_MODEL "models/weapons/w_smoke_ger.mdl"

//this 
class CSticky39GrenadeProjectile : public CBaseGrenadeProjectile
{
public:
	DECLARE_CLASS( CSticky39GrenadeProjectile, CBaseGrenadeProjectile );
	DECLARE_DATADESC();

	// Overrides.
public:
	virtual void Spawn()
	{
		if( mp_melontastic.GetBool() )
			SetModel( MELON_MODEL );
		else
			SetModel( STICK39_GRENADE_MODEL );
		BaseClass::Spawn();
	}

	virtual void Precache()
	{
		PrecacheModel( MELON_MODEL );
		PrecacheModel( STICK39_GRENADE_MODEL );
		PrecacheScriptSound( "BaseSmokeEffect.Sound" );
		PrecacheScriptSound( "BaseSmokeEmitEffect.Sound" );
		BaseClass::Precache();
	}

	// Grenade stuff.
public:
	
	virtual void Detonate( void )
	{
		EmitSound( "BaseSmokeEffect.Sound" );

		SetThink( &CSticky39GrenadeProjectile::SmokeEmit );
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

		SetThink( &CBaseEntity::SUB_FadeOut );
		SetNextThink( gpGlobals->curtime + 60.0f );
	}

	static CSticky39GrenadeProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner, 
		float timer, bool roll )
	{
		CSticky39GrenadeProjectile *pGrenade = (CSticky39GrenadeProjectile*)CBaseEntity::Create( "Sticky39grenade_projectile", position, angles, pOwner );

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
		pGrenade->SetThink( &CSticky39GrenadeProjectile::DangerSoundThink );
		pGrenade->SetNextThink( gpGlobals->curtime );

		return pGrenade;
	}
};

LINK_ENTITY_TO_CLASS( sticky39grenade_projectile, CSticky39GrenadeProjectile );
PRECACHE_WEAPON_REGISTER( sticky39grenade_projectile );

BEGIN_DATADESC( CSticky39GrenadeProjectile )
	DEFINE_THINKFUNC( SmokeEmit ),
END_DATADESC()

BEGIN_DATADESC( CSticky39Grenade )
END_DATADESC()

void CSticky39Grenade::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, float flDelay, bool roll )
{
	// remove a grenade from the players third person
	//CRnLPlayer *pRnLPlayer = dynamic_cast< CRnLPlayer* >( pPlayer );
	//pRnLPlayer->RemoveEquipmentFromPlayer( EQUIPMENT_AXIS_SMOKEGRENADE );

	CSticky39GrenadeProjectile *pGrenade = CSticky39GrenadeProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, GRENADE_TIMER, roll );

	if( !pGrenade )
	{
		DevMsg( "Oh Noes Grenade projectile didn't get created.\n" );
	}
}
#endif

