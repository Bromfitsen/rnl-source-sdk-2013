//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ========
//
// Purpose: Mine brush entity
//
//=============================================================================
#include "cbase.h"
#include "rnl_mine_entity.h"
#include "triggers.h"

#define SF_HOSTILE2AXIS		0x0001
#define SF_HOSTILE2ALLIES	0x0002

class CRnLMineBrush: public CBaseTrigger
{
public:
	DECLARE_CLASS( CRnLMineBrush , CBaseTrigger );
	DECLARE_DATADESC();

	void Precache() OVERRIDE;
	void Spawn( void ) OVERRIDE;

	void InputToggleMine( inputdata_t &inputdata );
	void LaunchMine( void );

	// custom filter, only accepts players
	bool PassesTriggerFilters( CBaseEntity *pOther ) OVERRIDE;
private:
	
	int m_iMineDamage;
	bool m_bMineFieldEnabled;
	string_t m_sMineModel;
};

LINK_ENTITY_TO_CLASS( rnl_mine_field , CRnLMineBrush );

// Start of our data description for the class
BEGIN_DATADESC( CRnLMineBrush  )
	DEFINE_KEYFIELD( m_iMineDamage, FIELD_INTEGER, "mineDamage" ),
	DEFINE_KEYFIELD( m_sMineModel,	FIELD_STRING, "mineModel" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "ToggleMine", InputToggleMine ),
	DEFINE_THINKFUNC( LaunchMine )
END_DATADESC()

void CRnLMineBrush::Precache()
{
	PrecacheModel( STRING(m_sMineModel) );
}

void CRnLMineBrush::Spawn( void )
{
	Precache();

	// We want to capture touches from other entities
	BaseClass::Spawn();

	InitTrigger();
	
	SetThink( &CRnLMineBrush::LaunchMine );
	SetNextThink( gpGlobals->curtime + 3.0f );

	m_bMineFieldEnabled = true;
}

void CRnLMineBrush::InputToggleMine( inputdata_t &inputdata )
{
	m_bMineFieldEnabled = !m_bMineFieldEnabled;
}

// only let enemy players touch this entity
bool CRnLMineBrush::PassesTriggerFilters( CBaseEntity *pOther )
{
	if( pOther->IsPlayer() && pOther->IsAlive() )
	{
		return true;
	}
	else
		return false;
}

void CRnLMineBrush::LaunchMine( void )
{	
	// check if any players are touching
	if( m_hTouchingEntities.Count() > 0 )
	{
		int randomPerson = random->RandomInt( 0, (m_hTouchingEntities.Count() - 1 ) );

		CBasePlayer* pPlayer = ToBasePlayer( m_hTouchingEntities[randomPerson] );

		if( pPlayer )
		{
			Vector forward;
			pPlayer->EyeVectors( &forward );
			forward[PITCH] = 0.0f;
			// if there are... >:D
			CRnLMineEntity *pMine = dynamic_cast<CRnLMineEntity*>( CreateEntityByName("rnl_mine_entity") );
			
			pMine->SetDamage( m_iMineDamage );
			pMine->SetDamageRadius( 250.0f );

			pMine->CreateMine( STRING(m_sMineModel) );

			forward = pPlayer->GetAbsOrigin() + (forward * ( 11.0f + random->RandomFloat( 32.0, 54.0 ) ) );
			forward.z += 24.0f;

			// set it at their feet
			pMine->SetAbsOrigin( forward );
			pMine->SetAbsVelocity( Vector( 0, 0, 400 ) );

			pMine->SetThink(&CRnLMineEntity::ExplodeThink);
			pMine->SetNextThink( gpGlobals->curtime + 0.95f );
		}
	}

	SetNextThink( gpGlobals->curtime + random->RandomFloat( 5.0, 8.0 ) );
}