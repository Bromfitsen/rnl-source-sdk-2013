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
	#define CWeaponAlliedExplosives C_WeaponAlliedExplosives
#endif

//-----------------------------------------------------------------------------
// The American Explosives
//-----------------------------------------------------------------------------
class CWeaponAlliedExplosives : public CWeaponRnLBaseExplosive
{
public:
	DECLARE_CLASS( CWeaponAlliedExplosives, CWeaponRnLBaseExplosive );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CWeaponAlliedExplosives() {}
	~CWeaponAlliedExplosives() = default;

	RnLWeaponID GetWeaponID( void ) const OVERRIDE { return WEAPON_ALLIEDEXPLOSIVES; }

#ifndef CLIENT_DLL
	void PlantExplosive() OVERRIDE;
#endif
private:

	CWeaponAlliedExplosives( const CWeaponAlliedExplosives & ) {}
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAlliedExplosives, DT_WeaponAlliedExplosives )

BEGIN_NETWORK_TABLE(CWeaponAlliedExplosives, DT_WeaponAlliedExplosives)
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponAlliedExplosives )
END_PREDICTION_DATA()
#endif

acttable_t CWeaponAlliedExplosives::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(CWeaponAlliedExplosives);

LINK_ENTITY_TO_CLASS( weapon_alliedexplosives, CWeaponAlliedExplosives );
PRECACHE_WEAPON_REGISTER( weapon_alliedexplosives );


#ifdef GAME_DLL
	//use this until we get our grenade models done - nuke
	#define US_EXPLOSIVE_MODEL "models/weapons/w_allied_exp.mdl"

	class CAlliedExplosive : public CRnLBaseExplosive
	{
	public:
		DECLARE_CLASS( CAlliedExplosive, CRnLBaseExplosive );

		// Overrides.
	public:
		virtual void Spawn()
		{
			if( mp_melontastic.GetBool() )
				SetModel( TNT_MELON_MODEL );
			else
				SetModel( US_EXPLOSIVE_MODEL );
			BaseClass::Spawn();
		}

		virtual void Precache()
		{
			PrecacheModel( US_EXPLOSIVE_MODEL );
			PrecacheModel( TNT_MELON_MODEL );
			BaseClass::Precache();
		}

	public:
		static CAlliedExplosive* Create( const Vector &position, const QAngle &angles, CRnLPlayer *pOwner, float timer )
		{
			CAlliedExplosive *pTnT = (CAlliedExplosive*)CBaseEntity::Create( "allied_explosive", position, angles, pOwner );
			if( pTnT )
			{
				pTnT->SetDetonateTimerLength( timer );
				pTnT->SetAbsVelocity( vec3_origin );
				pTnT->SetThrower( pOwner ); 

				pTnT->m_flDamage = 200;
				pTnT->m_DmgRadius = 500;
				pTnT->ChangeTeam( pOwner->GetTeamNumber() );

				// make NPCs afaid of it while in the air
				pTnT->SetThink( &CAlliedExplosive::DangerSoundThink );
				pTnT->SetNextThink( gpGlobals->curtime );
			}
			return pTnT;
		}
	};

	LINK_ENTITY_TO_CLASS( allied_explosive, CAlliedExplosive );
	PRECACHE_WEAPON_REGISTER( allied_explosive );

	BEGIN_DATADESC( CWeaponAlliedExplosives )
	END_DATADESC()

	void CWeaponAlliedExplosives::PlantExplosive()
	{
		CRnLPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer || !(m_pPlantingTarget.Get()) )
			return;

		CAlliedExplosive* pTnT = CAlliedExplosive::Create( m_pPlantingTarget->GetAbsOrigin(), m_pPlantingTarget->GetAbsAngles(), pPlayer, 20.0f );
		if( !m_pPlantingTarget->AttachExplosive(pTnT) )
		{
			UTIL_Remove( pTnT );
		}
	}
#endif

