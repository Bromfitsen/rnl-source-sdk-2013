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
#include "weapon_sticky24grenade.h"


#ifdef CLIENT_DLL
	
#else

	#include "rnl_player.h"
	#include "items.h"
	#include "rnl_basegrenade_projectile.h"

#endif


#define GRENADE_TIMER	3.0f //Seconds

IMPLEMENT_NETWORKCLASS_ALIASED( Sticky24Grenade, DT_Sticky24Grenade )

BEGIN_NETWORK_TABLE(CSticky24Grenade, DT_Sticky24Grenade)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CSticky24Grenade )
END_PREDICTION_DATA()

acttable_t CSticky24Grenade::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(CSticky24Grenade);

LINK_ENTITY_TO_CLASS( weapon_sticky24grenade, CSticky24Grenade );
PRECACHE_WEAPON_REGISTER( weapon_sticky24grenade );

bool CSticky24Grenade::CanHolster( void )
{
	return ((m_iWeaponAnimationState != WEAPON_ANIMATION_HOLSTER && m_iWeaponAnimationState != WEAPON_ANIMATION_DRAW) || IsSequenceFinished() ) && (m_iGrenadeState <= GRENADE_PREP);
}

#ifdef GAME_DLL

#define GER_STICK24MODEL "models/weapons/w_stick.mdl"

//this 
class CSticky24GrenadeProjectile : public CBaseGrenadeProjectile
{
public:
	DECLARE_CLASS( CSticky24GrenadeProjectile, CBaseGrenadeProjectile );


	// Overrides.
public:
	virtual void Spawn()
	{
		if( mp_melontastic.GetBool() )
			SetModel( MELON_MODEL );
		else
			SetModel( GER_STICK24MODEL );
		BaseClass::Spawn();
	}

	virtual void Precache()
	{
		PrecacheModel( MELON_MODEL );
		PrecacheModel( GER_STICK24MODEL );
		BaseClass::Precache();
	}

	// Grenade stuff.
public:

	static CSticky24GrenadeProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner, 
		float timer, bool roll )
	{
		CSticky24GrenadeProjectile *pGrenade = (CSticky24GrenadeProjectile*)CBaseEntity::Create( "Sticky24grenade_projectile", position, angles, pOwner );

		// Set the timer for 1 second less than requested. We're going to issue a SOUND_DANGER
		// one second before detonation.

		pGrenade->SetDetonateTimerLength( timer );
		pGrenade->SetAbsVelocity( velocity );
		pGrenade->SetupInitialTransmittedGrenadeVelocity( velocity );
		pGrenade->SetThrower( pOwner ); 

		if( !roll )
		{
			pGrenade->SetGravity( BaseClass::GetGrenadeGravity() );
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
		pGrenade->SetThink( &CSticky24GrenadeProjectile::DangerSoundThink );
		pGrenade->SetNextThink( gpGlobals->curtime );

		return pGrenade;
	}
};

LINK_ENTITY_TO_CLASS( Sticky24grenade_projectile, CSticky24GrenadeProjectile );
PRECACHE_WEAPON_REGISTER( Sticky24grenade_projectile );

BEGIN_DATADESC( CSticky24Grenade )
END_DATADESC()

void CSticky24Grenade::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, float flDelay, bool roll )
{
	// remove a grenade from the players third person
	//CRnLPlayer *pRnLPlayer = dynamic_cast< CRnLPlayer* >( pPlayer );
	//pRnLPlayer->RemoveEquipmentFromPlayer( EQUIPMENT_AXIS_STIELHANDGRANATE );

	CSticky24GrenadeProjectile* pGrenade = CSticky24GrenadeProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, flDelay, roll );

	if( !pGrenade )
	{
		DevMsg( "Oh Noes Grenade projectile didn't get created.\n" );
	}
}
	
#endif
