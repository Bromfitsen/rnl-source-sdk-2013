#include "cbase.h"
#include "rnl_task_destructable.h"
#include "rnl_gamerules.h"

#ifndef CLIENT_DLL
	#include "rnl_explosive.h"
#endif

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(RnLTaskDestructable, DT_RnLTaskDestructable)
BEGIN_NETWORK_TABLE( CRnLTaskDestructable, DT_RnLTaskDestructable )
#ifndef CLIENT_DLL
	SendPropBool( SENDINFO( m_bEnabled ) ),
	SendPropBool( SENDINFO( m_bDestroyed ) ),
	SendPropBool( SENDINFO( m_bPlanted ) ),
#else
	RecvPropBool( RECVINFO( m_bEnabled ) ),
	RecvPropBool( RECVINFO( m_bDestroyed ) ),
	RecvPropBool( RECVINFO( m_bPlanted ) ),
#endif
END_NETWORK_TABLE()

#ifndef CLIENT_DLL
	BEGIN_DATADESC( CRnLTaskDestructable )
		DEFINE_KEYFIELD( m_bEnabled, FIELD_BOOLEAN, "enabled" ),

		DEFINE_OUTPUT(	m_OnDestroyed,	"OnDestroyed" ),
		
		DEFINE_OUTPUT(	m_OnAxisDestroyed,	"OnAxisDestroyed" ),
		DEFINE_OUTPUT(	m_OnAxisChargePlaced,	"OnAxisChargePlaced" ),

		DEFINE_OUTPUT(	m_OnAlliedDestroyed,	"OnAlliedDestroyed" ),
		DEFINE_OUTPUT(	m_OnAlliedChargePlaced,	"OnAlliedChargePlaced" ),
	END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( rnl_task_destructable, CRnLTaskDestructable );

#define TASK_DESTRUCTABLE_MODEL_ALLIES "models/weapons/w_allied_exp_fake.mdl"
#define TASK_DESTRUCTABLE_MODEL_AXIS "models/weapons/w_axis_exp_fake.mdl"

CRnLTaskDestructable::CRnLTaskDestructable()
{
}

#ifdef CLIENT_DLL
	bool CRnLTaskDestructable::ShouldDraw( void )
	{
		if( !m_bEnabled || m_bDestroyed )
		{
			if( IsTaskVisibleOnMap() )
				SetVisibleOnMap(false);
			return false;
		}

		if( m_bPlanted )
			return false;

		return BaseClass::ShouldDraw();
	}

	void CRnLTaskDestructable::OnDataChanged( DataUpdateType_t type )
	{
		BaseClass::OnDataChanged( type );

		UpdateVisibility();
	}
#endif

void CRnLTaskDestructable::Precache()
{
	PrecacheModel( TASK_DESTRUCTABLE_MODEL_ALLIES );
	PrecacheModel( TASK_DESTRUCTABLE_MODEL_AXIS );
}

void CRnLTaskDestructable::Spawn( void )
{
	Precache();

	m_bDestroyed = false;
	m_bPlanted = false;
#ifndef CLIENT_DLL
	m_pAttachedTnT = NULL;
	UTIL_SetSize( this, Vector( -2, -2, -6 ), Vector( 2, 2, 6 ) );
	SetSolid( SOLID_NONE );
	if( m_iInitialControllingTeam == TEAM_ALLIES )
	{
		SetModel( TASK_DESTRUCTABLE_MODEL_AXIS );
		SetTaskState( RNL_TASK_ALLIED_CONTROLLED );
		ChangeTeam( TEAM_ALLIES );
	}
	else if( m_iInitialControllingTeam == TEAM_AXIS )
	{
		SetModel( TASK_DESTRUCTABLE_MODEL_ALLIES );
		SetTaskState( RNL_TASK_AXIS_CONTROLLED );
		ChangeTeam( TEAM_AXIS );
	}
	else
	{
		SetModel( TASK_DESTRUCTABLE_MODEL_ALLIES );
		SetTaskState( RNL_TASK_NEUTRAL );
		ChangeTeam( TEAM_UNASSIGNED );
	}
	m_takedamage = DAMAGE_YES;
#endif
	BaseClass::Spawn();
}

void CRnLTaskDestructable::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();
}

#ifndef CLIENT_DLL
	void CRnLTaskDestructable::Activate( void )
	{
		m_bDestroyed = false;
		m_bPlanted = false;
		BaseClass::Activate();
	}

	bool CRnLTaskDestructable::CanAttachExplosive( CRnLPlayer* pPlayer )
	{
		if( m_bDestroyed || !m_bEnabled || m_pAttachedTnT != NULL || m_bPlanted )
			return false;

		if( !pPlayer )
			return false;

		if( m_eParentObjective.Get() != NULL )
			if(  !(m_eParentObjective.Get()->AreRequirementsMet( pPlayer->GetTeamNumber() )) )
				return false;

		return pPlayer->GetTeamNumber() != GetTeamNumber();
	}

	bool CRnLTaskDestructable::AttachExplosive( CRnLBaseExplosive* pTnT  )
	{
		if( m_bDestroyed || !m_bEnabled || m_bPlanted || m_pAttachedTnT != NULL || pTnT == NULL )
			return false;

		m_pAttachedTnT = pTnT;
		m_bPlanted = true;
		return true;
	}

	int CRnLTaskDestructable::OnTakeDamage( const CTakeDamageInfo &info )
	{
		if( !m_bEnabled || m_bDestroyed || !m_bPlanted )
			return 0;

		if( !( info.GetDamageType() & DMG_RNL_TNT ) )
			return 0;

		if( !info.GetInflictor() || info.GetInflictor() != m_pAttachedTnT )
			return 0;

		if( !info.GetAttacker() )
			return 0;
		
		switch( info.GetAttacker()->GetTeamNumber() )
		{
		case TEAM_AXIS:
			SetTaskState( RNL_TASK_AXIS_CONTROLLED );
			m_bDestroyed = true;
			m_OnAxisDestroyed.FireOutput( info.GetInflictor(), this );
			m_OnDestroyed.FireOutput( info.GetInflictor(), this );
			m_pAttachedTnT = NULL;
			break;
		case TEAM_ALLIES:
			SetTaskState( RNL_TASK_ALLIED_CONTROLLED );
			m_bDestroyed = true;
			m_OnAlliedDestroyed.FireOutput( info.GetInflictor(), this );
			m_OnDestroyed.FireOutput( info.GetInflictor(), this );
			m_pAttachedTnT = NULL;
			break;
		}
		return 0;
	}
#endif