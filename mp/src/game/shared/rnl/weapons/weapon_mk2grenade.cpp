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
#include "weapon_mk2grenade.h"


#ifdef CLIENT_DLL
	
#else

	#include "rnl_player.h"
	#include "items.h"
	#include "rnl_basegrenade_projectile.h"

#endif

//  TODO: Network this shit! - Stefan

IMPLEMENT_NETWORKCLASS_ALIASED( MK2Grenade, DT_MK2Grenade )

BEGIN_NETWORK_TABLE(CMK2Grenade, DT_MK2Grenade)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CMK2Grenade )
END_PREDICTION_DATA()

acttable_t CMK2Grenade::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(CMK2Grenade);

LINK_ENTITY_TO_CLASS( weapon_mk2grenade, CMK2Grenade );
PRECACHE_WEAPON_REGISTER( weapon_mk2grenade );


#ifdef GAME_DLL

#define US_MK2MODEL "models/weapons/w_frag.mdl"

//this 
class CMK2GrenadeProjectile : public CBaseGrenadeProjectile
{
public:
	DECLARE_CLASS( CMK2GrenadeProjectile, CBaseGrenadeProjectile );

	// Overrides.
public:
	virtual void Spawn()
	{
		if( mp_melontastic.GetBool() )
			SetModel( MELON_MODEL );
		else
			SetModel( US_MK2MODEL );
		BaseClass::Spawn();
	}

	virtual void Precache()
	{
		PrecacheModel( US_MK2MODEL );
		PrecacheModel( MELON_MODEL );
		BaseClass::Precache();
	}

public:

	static CMK2GrenadeProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner, 
		float timer, bool roll )
	{
		CMK2GrenadeProjectile *pGrenade = (CMK2GrenadeProjectile*)CBaseEntity::Create( "mk2grenade_projectile", position, angles, pOwner );

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
		pGrenade->SetThink( &CMK2GrenadeProjectile::DangerSoundThink );
		pGrenade->SetNextThink( gpGlobals->curtime );

		return pGrenade;
	}
};

LINK_ENTITY_TO_CLASS( mk2grenade_projectile, CMK2GrenadeProjectile );
PRECACHE_WEAPON_REGISTER( mk2grenade_projectile );

BEGIN_DATADESC( CMK2Grenade )
END_DATADESC()

void CMK2Grenade::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, float fDelay, bool roll )
{
	// remove a grenade from the players third person
	//CRnLPlayer *pRnLPlayer = dynamic_cast< CRnLPlayer* >( pPlayer );
	//pRnLPlayer->RemoveEquipmentFromPlayer( EQUIPMENT_ALLIES_FRAGGRENADES );

	CMK2GrenadeProjectile* pGrenade = CMK2GrenadeProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, fDelay, roll );

	if( !pGrenade )
	{
		DevMsg( "Oh Noes Grenade projectile didn't get created.\n" );
	}
}
	
#endif

