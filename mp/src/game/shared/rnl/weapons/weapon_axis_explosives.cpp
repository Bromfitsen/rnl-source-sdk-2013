//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_rnl_baseexplosive.h"

#ifdef CLIENT_DLL
	
#else

	#include "rnl_player.h"
	#include "items.h"
	#include "rnl_explosive.h"

#endif

#ifdef CLIENT_DLL
	#define CWeaponAxisExplosives C_WeaponAxisExplosives
#endif

//-----------------------------------------------------------------------------
// The American Explosives
//-----------------------------------------------------------------------------
class CWeaponAxisExplosives : public CWeaponRnLBaseExplosive
{
public:
	DECLARE_CLASS( CWeaponAxisExplosives, CWeaponRnLBaseExplosive );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CWeaponAxisExplosives() {}
	~CWeaponAxisExplosives() = default;

	RnLWeaponID GetWeaponID( void ) const OVERRIDE { return WEAPON_AXISEXPLOSIVES; }

#ifndef CLIENT_DLL
	void PlantExplosive() OVERRIDE;
#endif
private:

	CWeaponAxisExplosives( const CWeaponAxisExplosives & ) {}
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAxisExplosives, DT_WeaponAxisExplosives )

BEGIN_NETWORK_TABLE(CWeaponAxisExplosives, DT_WeaponAxisExplosives)
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponAxisExplosives )
END_PREDICTION_DATA()
#endif

acttable_t CWeaponAxisExplosives::m_acttable[] = 
{
	{ ACT_SPRINT,			ACT_DOD_SPRINT_IDLE_TNT,			false },

	{ ACT_DOD_STAND_AIM,	ACT_DOD_STAND_IDLE_TNT,			false },
	{ ACT_DOD_STAND_IDLE,	ACT_DOD_STAND_IDLE_TNT,			false },
	
	{ ACT_DOD_CROUCH_AIM,	ACT_DOD_CROUCH_IDLE_TNT,			false },
	{ ACT_DOD_CROUCH_IDLE,	ACT_DOD_CROUCH_IDLE_TNT,			false },

	{ ACT_DOD_CROUCHWALK_AIM,	ACT_DOD_CROUCHWALK_IDLE_TNT,	false },
	{ ACT_DOD_CROUCHWALK_IDLE,	ACT_DOD_CROUCHWALK_IDLE_TNT,	false },
	
	{ ACT_PRONE,			ACT_DOD_PRONEWALK_IDLE_TNT,		false },
	{ ACT_PRONEIDLE,		ACT_DOD_PRONEWALK_IDLE_TNT,			false },
	
	{ ACT_DOD_RUN_AIM,		ACT_DOD_RUN_IDLE_TNT,				false },
	{ ACT_DOD_RUN_IDLE,		ACT_DOD_RUN_IDLE_TNT,			false },
	
	{ ACT_DOD_WALK_AIM,		ACT_DOD_WALK_IDLE_TNT,			false },
	{ ACT_DOD_WALK_IDLE,	ACT_DOD_WALK_IDLE_TNT,			false },

	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PLANT_TNT,	false },
	{ ACT_DOD_PRIMARYATTACK_PRONE,	ACT_DOD_PLANT_TNT, false },
};

IMPLEMENT_ACTTABLE(CWeaponAxisExplosives);

LINK_ENTITY_TO_CLASS( weapon_axisexplosives, CWeaponAxisExplosives );
PRECACHE_WEAPON_REGISTER( weapon_axisexplosives );


#ifdef GAME_DLL
	//use this until we get our grenade models done - nuke
	#define GERMAN_EXPLOSIVE_MODEL "models/weapons/w_axis_exp.mdl"

	class CAxisExplosive : public CRnLBaseExplosive
	{
	public:
		DECLARE_CLASS( CAxisExplosive, CRnLBaseExplosive );

		// Overrides.
	public:
		virtual void Spawn()
		{
			if( mp_melontastic.GetBool() )
				SetModel( TNT_MELON_MODEL );
			else
				SetModel( GERMAN_EXPLOSIVE_MODEL );
			BaseClass::Spawn();
		}

		virtual void Precache()
		{
			PrecacheModel( GERMAN_EXPLOSIVE_MODEL );
			PrecacheModel( TNT_MELON_MODEL );
			BaseClass::Precache();
		}

	public:
		static CAxisExplosive* Create( const Vector &position, const QAngle &angles, CRnLPlayer *pOwner, float timer )
		{
			CAxisExplosive *pTnT = (CAxisExplosive*)CBaseEntity::Create( "axis_explosive", position, angles, pOwner );
			if( pTnT )
			{
				pTnT->SetDetonateTimerLength( timer );
				pTnT->SetAbsVelocity( vec3_origin );
				pTnT->SetThrower( pOwner ); 

				pTnT->m_flDamage = 200;
				pTnT->m_DmgRadius = 500;
				pTnT->ChangeTeam( pOwner->GetTeamNumber() );

				// make NPCs afaid of it while in the air
				pTnT->SetThink( &CAxisExplosive::DangerSoundThink );
				pTnT->SetNextThink( gpGlobals->curtime );
			}
			return pTnT;
		}
	};

	LINK_ENTITY_TO_CLASS( axis_explosive, CAxisExplosive );
	PRECACHE_WEAPON_REGISTER( axis_explosive );

	BEGIN_DATADESC( CWeaponAxisExplosives )
	END_DATADESC()

	void CWeaponAxisExplosives::PlantExplosive()
	{
		CRnLPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer || !(m_pPlantingTarget.Get()) )
			return;

		CAxisExplosive* pTnT = CAxisExplosive::Create( m_pPlantingTarget->GetAbsOrigin(), m_pPlantingTarget->GetAbsAngles(), pPlayer, 20.0f );
		if( !m_pPlantingTarget->AttachExplosive(pTnT) )
		{
			UTIL_Remove( pTnT );
		}
	}
#endif

