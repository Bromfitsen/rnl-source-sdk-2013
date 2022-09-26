/********************************************************************
 Created for the Resistance and Liberation Mod for the Source-Engine
 Purpose: The equipment prop, that is attached to the player to show
 his current equipment state

 Created 29/4/05 by NuclearFriend AKA Jonathan Murphy
********************************************************************/


#include "cbase.h"
#include "rnl_shareddefs.h"
#include "rnl_player.h"
#include "weapon_rnl_base.h"
#include "rnl_equipment_prop.h"
#include "triggers.h"
#include "shareddefs.h"

LINK_ENTITY_TO_CLASS( rnl_equipment_prop, CRnLEquipmentProp );

IMPLEMENT_SERVERCLASS_ST( CRnLEquipmentProp, DT_RnLEquipmentProp )
END_SEND_TABLE()

CRnLEquipmentProp::CRnLEquipmentProp()
{

}

CRnLEquipmentProp::~CRnLEquipmentProp()
{
}

class CParachuteTrigger : public CBaseTrigger
{
	DECLARE_CLASS( CParachuteTrigger, CBaseTrigger );

public:

	virtual void StartTouch( CBaseEntity *pOther );
	virtual void Spawn( void );

};

LINK_ENTITY_TO_CLASS( trigger_parachute, CParachuteTrigger );

void CParachuteTrigger::Spawn( void )
{
	BaseClass::Spawn();

	InitTrigger();
}

void CParachuteTrigger::StartTouch(CBaseEntity *pOther)
{
	BaseClass::StartTouch( pOther );

	if ( pOther->IsPlayer() )
	{
		CRnLPlayer* pPlayer = ToRnLPlayer( pOther );

		if( pPlayer )
		{
			pPlayer->OpenParachute();
		}
	}
}

LINK_ENTITY_TO_CLASS( rnl_equipment_parachute, CRnLEquipmentParachute );
PRECACHE_REGISTER(rnl_equipment_parachute);

#define EQ_PIR_PARACHUTE_MODEL "models/items/w_parachute1.mdl"
#define EQ_PIR_PARACHUTE_MODEL_OPEN "models/items/w_parachute1_open.mdl"


CRnLEquipmentParachute::CRnLEquipmentParachute()
{

}

CRnLEquipmentParachute::~CRnLEquipmentParachute()
{
}

void CRnLEquipmentParachute::Precache()
{
	PrecacheScriptSound( "Player.Parachute_Opening" );
	PrecacheModel( EQ_PIR_PARACHUTE_MODEL );
	PrecacheModel( EQ_PIR_PARACHUTE_MODEL_OPEN );
}

void CRnLEquipmentParachute::Spawn( void )
{
	m_bIsDeployed = false;

	Precache();

	m_takedamage = DAMAGE_NO;

	SetTransmitState( FL_EDICT_ALWAYS );

	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );
	SetSize( vec3_origin, vec3_origin );
	
	SetModel( EQ_PIR_PARACHUTE_MODEL );
}

void CRnLEquipmentParachute::Deploy( void )
{
	if( m_bIsDeployed == false )
	{
		EmitSound( "Player.Parachute_Opening" );
		m_bIsDeployed = true;
		SetModel( EQ_PIR_PARACHUTE_MODEL_OPEN );
	}
}

LINK_ENTITY_TO_CLASS( rnl_equipment_radio, CRnLEquipmentRadio );
PRECACHE_REGISTER(rnl_equipment_radio);

#define EQ_PIR_RADIO_MODEL "models/items/us_radio.mdl"


CRnLEquipmentRadio::CRnLEquipmentRadio()
{

}

CRnLEquipmentRadio::~CRnLEquipmentRadio()
{
}

void CRnLEquipmentRadio::Precache()
{
	PrecacheModel( EQ_PIR_RADIO_MODEL );
}

void CRnLEquipmentRadio::Spawn( void )
{
	Precache();

	m_takedamage = DAMAGE_NO;

	SetTransmitState( FL_EDICT_ALWAYS );

	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );
	SetSize( vec3_origin, vec3_origin );
	
	SetModel( EQ_PIR_RADIO_MODEL );
}