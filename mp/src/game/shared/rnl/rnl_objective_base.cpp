#include "cbase.h"
#include "rnl_task_base.h"
#include "rnl_gamerules.h"
#include "rnl_objective_base.h"
#include "rnl_objective_order_filter.h"

#ifndef CLIENT_DLL
#include "dt_utlvector_send.h"
#else
#include "dt_utlvector_recv.h"
#endif

#include "tier0/memdbgon.h"

#define MAX_TASKS 8

const char *gpObjectiveStateNames[RNL_OBJECTIVE_STATES_MAX] = {
	"Neutral",									//RNL_OBJECTIVE_NEUTRAL
	"Allied Controlled",						//RNL_OBJECTIVE_ALLIED_CONTROLLED
	"Axis Controlled"							//RNL_OBJECTIVE_AXIS_CONTROLLED
};

const char *ObjectiveStateToName( int state )
{
	if( state < RNL_OBJECTIVE_NEUTRAL || state >= RNL_OBJECTIVE_STATES_MAX )
		return "Unknown";
	return gpObjectiveStateNames[state];
}


IMPLEMENT_NETWORKCLASS_ALIASED(RnLObjectiveBase, DT_RnLObjectiveBase)
BEGIN_NETWORK_TABLE( CRnLObjectiveBase, DT_RnLObjectiveBase )
#ifndef CLIENT_DLL
	SendPropString( SENDINFO( m_szObjectiveName ) ),
	SendPropInt(	SENDINFO( m_iObjectiveState ) ),
	SendPropInt(	SENDINFO( m_iObjectivePrevState ) ),
	SendPropBool(	SENDINFO( m_bVisibleOnMap)),
	SendPropBool(	SENDINFO( m_bIconVisibleOnMap)),
	SendPropBool(	SENDINFO( m_bPrimary)),
	SendPropEHandle( SENDINFO( m_eAxisOrderFilter ) ),
	SendPropEHandle( SENDINFO( m_eAlliesOrderFilter ) ),

	SendPropUtlVector(
		SENDINFO_UTLVECTOR( m_aTasks ),
		MAX_TASKS, // max elements
		SendPropEHandle( NULL, 0 ) ),
	
#else
	RecvPropString( RECVINFO( m_szObjectiveName ) ),
	RecvPropInt(	RECVINFO( m_iObjectiveState ) ),
	RecvPropInt(	RECVINFO( m_iObjectivePrevState ) ),
	RecvPropBool(	RECVINFO( m_bVisibleOnMap)),
	RecvPropBool(	RECVINFO( m_bIconVisibleOnMap)),
	RecvPropBool(	RECVINFO( m_bPrimary )),
	RecvPropEHandle( RECVINFO( m_eAxisOrderFilter ) ),
	RecvPropEHandle( RECVINFO( m_eAlliesOrderFilter ) ),
	
	RecvPropUtlVector( 
		RECVINFO_UTLVECTOR( m_aTasks ), 
		MAX_TASKS,
		RecvPropEHandle(NULL, 0, 0)),

#endif
END_NETWORK_TABLE()

#ifndef CLIENT_DLL
	BEGIN_DATADESC( CRnLObjectiveBase )
		DEFINE_KEYFIELD( m_iInitialControllingTeam, FIELD_INTEGER, "initialowners" ),
		DEFINE_KEYFIELD( m_bPrimary, FIELD_BOOLEAN, "primary" ),
		DEFINE_KEYFIELD( m_iBehaviour, FIELD_INTEGER, "behaviour" ),
		DEFINE_KEYFIELD( m_iszAxisOrderFilterName,		FIELD_STRING, "axisorderfilter" ),
		DEFINE_KEYFIELD( m_iszAlliesOrderFilterName,	FIELD_STRING, "alliesorderfilter" ),

		DEFINE_INPUTFUNC( FIELD_INTEGER,	"SetControllingTeam",	InputSetControllingTeam ),
		DEFINE_INPUTFUNC( FIELD_BOOLEAN,	"SetVisibleOnMap",		InputSetVisibleOnMap ),
		DEFINE_INPUTFUNC( FIELD_BOOLEAN,	"SetIconVisibleOnMap",	InputSetIconVisibleOnMap ),
		DEFINE_INPUTFUNC( FIELD_BOOLEAN,	"SetPrimary",			InputSetSetPrimary ),

		DEFINE_OUTPUT( m_OnNeutral,			"OnNeutral" ),
		DEFINE_OUTPUT( m_OnAxisControlled,	"OnAxisCaptured" ),
		DEFINE_OUTPUT( m_OnAxisLose,		"OnAxisLose" ),
		DEFINE_OUTPUT( m_OnAlliedControlled,"OnAlliedCaptured" ),
		DEFINE_OUTPUT( m_OnAlliedLose,		"OnAlliedLose" ),
	END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( rnl_objective, CRnLObjectiveBase );


CRnLObjectiveBase::CRnLObjectiveBase()
{
	m_bVisibleOnMap = true;
	m_bIconVisibleOnMap = true;
	m_bPrimary = true;

	m_iObjectivePrevState = RNL_OBJECTIVE_INVALID;
	m_iObjectiveState = RNL_OBJECTIVE_NEUTRAL;
#ifndef CLIENT_DLL
	m_iInitialControllingTeam = TEAM_NONE;
#endif
}

#ifndef CLIENT_DLL
	void CRnLObjectiveBase::Activate( void )
	{
		BaseClass::Activate();

		m_eAxisOrderFilter = (CRnLObjectiveOrderFilter*)gEntList.FindEntityByName( NULL, m_iszAxisOrderFilterName );
		m_eAlliesOrderFilter = (CRnLObjectiveOrderFilter*)gEntList.FindEntityByName( NULL, m_iszAlliesOrderFilterName );

		if( m_szPendingTasks.Count() > 0 )
		{
			TaskHandle handle = NULL;
			for( int i = 0; i < m_szPendingTasks.Count(); i++ )
			{
				handle = (CRnLTaskBase*)gEntList.FindEntityByName( NULL, m_szPendingTasks[i] );
				if( handle )
				{
					DevMsg( "Pending Task added to objective: %s\n", m_szPendingTasks[i] );
					handle->SetParentObjective( this );
					m_aTasks.AddToTail( handle );
				}
				else
				{
					DevMsg( "Pending Task failed to add to objective: %s\n", m_szPendingTasks[i] );
				}
			}
		}
	}
	
	bool CRnLObjectiveBase::KeyValue( const char *szKeyName, const char *szValue )
	{
		if( Q_strnicmp( szKeyName, "task", 4 ) == 0 )
		{
			DevMsg( "Objective adding task: %s\n", szValue );
			m_szPendingTasks.AddToTail( szValue );
			return true;
		}

		return BaseClass::KeyValue( szKeyName, szValue );
	}

	void CRnLObjectiveBase::InputSetControllingTeam( inputdata_t &input )
	{
		int iState = RNL_OBJECTIVE_NEUTRAL;

		switch( input.value.Int() )
		{
			case TEAM_ALLIES:
				iState = RNL_OBJECTIVE_ALLIED_CONTROLLED;
				break;
			case TEAM_AXIS:
				iState = RNL_OBJECTIVE_AXIS_CONTROLLED;
				break;
			default:
				break;
		}

		for( int i = 0; i < m_aTasks.Count(); i++ )
				m_aTasks[i]->SetTaskState( iState );

			SetObjectiveState( iState );
	}

	void CRnLObjectiveBase::InputSetVisibleOnMap( inputdata_t &input )
	{
		m_bVisibleOnMap = input.value.Bool();

		for( int i = 0; i < m_aTasks.Count(); i++ )
			m_aTasks[i]->SetVisibleOnMap( m_bVisibleOnMap );
	}

	void CRnLObjectiveBase::InputSetIconVisibleOnMap( inputdata_t &input )
	{
		m_bIconVisibleOnMap = input.value.Bool();
	}

	void CRnLObjectiveBase::InputSetSetPrimary( inputdata_t &input )
	{
		m_bPrimary = input.value.Bool();
	}

#endif

void CRnLObjectiveBase::Reset( void )
{
}

void CRnLObjectiveBase::Spawn( void )
{
	BaseClass::Spawn();

#ifndef CLIENT_DLL
	if( m_iInitialControllingTeam == TEAM_AXIS )
		m_iObjectiveState = RNL_OBJECTIVE_AXIS_CONTROLLED;
	else if( m_iInitialControllingTeam == TEAM_ALLIES )
		m_iObjectiveState = RNL_OBJECTIVE_ALLIED_CONTROLLED;
	else
		m_iObjectiveState = RNL_OBJECTIVE_NEUTRAL;

	Q_strncpy( m_szObjectiveName.GetForModify(), STRING(GetEntityName()), RNL_MAX_OBJECTIVE_NAME );
#endif

	if( RnLGameRules() )
		RnLGameRules()->RegisterObjective( this );
}

void CRnLObjectiveBase::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();

	if( RnLGameRules() )
		RnLGameRules()->RemoveObjective( this );
}

bool CRnLObjectiveBase::AreRequirementsMet( int iTeam )
{
	CRnLObjectiveOrderFilter* pFilter = NULL;
	if( iTeam == TEAM_AXIS )
	{
		pFilter = m_eAxisOrderFilter.Get();
		if( pFilter )
			return pFilter->AreRequirementsMet();
	}
	else if( iTeam == TEAM_ALLIES )
	{
		pFilter = m_eAlliesOrderFilter.Get();
		if( pFilter )
			return pFilter->AreRequirementsMet();
	}
	
	return true;
}

CRnLObjectiveOrderFilter* CRnLObjectiveBase::GetOrderFilter( int iTeam )
{
	if( iTeam == TEAM_AXIS )
		return m_eAxisOrderFilter.Get();
	else if( iTeam == TEAM_ALLIES )
		return m_eAlliesOrderFilter.Get();

	return NULL;
}

void CRnLObjectiveBase::SetObjectiveState( int iState )
{
#ifndef CLIENT_DLL
	/////////////////////////////////////
	//
	// FIRE EVENTS FOR THE NEW STATE
	//
	/////////////////////////////////////
	switch( iState )
	{
	//------------------------
	//Neutral Events
	//------------------------
	case RNL_OBJECTIVE_NEUTRAL:
		{
			switch( GetObjectiveState() )
			{
			case RNL_OBJECTIVE_AXIS_CONTROLLED:
				m_OnAxisLose.FireOutput( this, this );
				break;
			case RNL_OBJECTIVE_ALLIED_CONTROLLED:
				m_OnAlliedLose.FireOutput( this, this );
				break;
			};
		}
		ChangeTeam( TEAM_NONE );
		break;
	//------------------------
	//Allied Events
	//------------------------
	case RNL_OBJECTIVE_ALLIED_CONTROLLED:
		{
			switch( GetObjectiveState() )
			{
			case RNL_OBJECTIVE_AXIS_CONTROLLED:
				m_OnAxisLose.FireOutput( this, this );
				break;
			};
		}
		ChangeTeam( TEAM_ALLIES );
		m_OnAlliedControlled.FireOutput( this, this);
		break;
	//------------------------
	//Axis Events
	//------------------------
	case RNL_OBJECTIVE_AXIS_CONTROLLED:
		{
			switch( GetObjectiveState() )
			{
			case RNL_OBJECTIVE_ALLIED_CONTROLLED:
				m_OnAlliedLose.FireOutput( this, this );
				break;
			};
		}
		ChangeTeam( TEAM_AXIS );
		m_OnAxisControlled.FireOutput( this, this);
		break;
	};
#endif
	PreStateChange( iState );

	m_iObjectivePrevState = m_iObjectiveState;
	m_iObjectiveState = iState;

	PostStateChange();

	if( RnLGameRules() )
	{
		RnLGameRules()->ObjectiveStateChange( this );
	}
}

void CRnLObjectiveBase::PreStateChange( int iNewState )
{
	if( GetObjectivePrevState() == RNL_OBJECTIVE_INVALID )
		return;

#ifndef CLIENT_DLL
	//send it to everyone
	IGameEvent *event = gameeventmanager->CreateEvent( "rnl_objective_state_change" );
	if ( event )
	{
		event->SetString( "objective_name", GetObjectiveName() );
		event->SetInt( "new_state", iNewState );
		event->SetInt( "current_state", GetObjectiveState() );
		event->SetInt( "previous_state", GetObjectivePrevState() );
		gameeventmanager->FireEvent( event );
	}
#endif
}

void CRnLObjectiveBase::PostStateChange( void )
{
}

int CRnLObjectiveBase::GetTaskCount( void )
{
	return m_aTasks.Count();
}

IRnLTask* CRnLObjectiveBase::GetTask( int index )
{
	return (m_aTasks[index]);
}

int CRnLObjectiveBase::CalculateState( void )
{
	int AlliesControl = 0;
	int AxisControl = 0;

	for( int i = 0; i < m_aTasks.Count(); i++ )
	{
		switch( m_aTasks[i]->GetTaskState() )
		{
		case RNL_TASK_AXIS_CONTROLLED:
		case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS:
		case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS_BLOCKED:
			AxisControl++;
			break;
		case RNL_TASK_ALLIED_CONTROLLED:
		case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED:
		case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED_BLOCKED:
			AlliesControl++;
			break;
		}
	}

	if( AxisControl == m_aTasks.Count() )
		return RNL_OBJECTIVE_AXIS_CONTROLLED;
	else if( AlliesControl == m_aTasks.Count() )
		return RNL_OBJECTIVE_ALLIED_CONTROLLED;
	
#ifndef CLIENT_DLL
	if( m_iBehaviour == RNL_BEHAVIOUR_ALL_TO_SWITCH )
	{
		if( AxisControl > 0 && m_iObjectiveState == RNL_OBJECTIVE_AXIS_CONTROLLED )
			return RNL_OBJECTIVE_AXIS_CONTROLLED;
		else if( AlliesControl > 0 && m_iObjectiveState == RNL_OBJECTIVE_ALLIED_CONTROLLED )
			return RNL_OBJECTIVE_ALLIED_CONTROLLED;
	}
	else if( m_iBehaviour == RNL_BEHAVIOUR_MORE_THAN_OTHER_TO_SWITCH )
	{
		if( AlliesControl > AxisControl )
			return RNL_OBJECTIVE_ALLIED_CONTROLLED;
		else if( AxisControl > AlliesControl  )
			return RNL_OBJECTIVE_AXIS_CONTROLLED;
		else if( AlliesControl > 0 && AxisControl > 0 )
			return GetObjectiveState();
	}
	else if( m_iBehaviour == RNL_BEHAVIOUR_MAJORITY_TO_SWITCH )
	{
		if( AlliesControl > (m_aTasks.Count() / 2.0f) )
			return RNL_OBJECTIVE_ALLIED_CONTROLLED;
		else if( AxisControl > (m_aTasks.Count() / 2.0f) )
			return RNL_OBJECTIVE_AXIS_CONTROLLED;
	}
#endif
	return RNL_OBJECTIVE_NEUTRAL;
}

void CRnLObjectiveBase::OnTaskStateChanged( IRnLTask* pTask )
{
	for( int i = 0; i < m_aTasks.Count(); i++ )
	{
		if( m_aTasks[i].Get() == pTask )
		{
			SetObjectiveState( CalculateState() );
			return;
		}
	}
}

EHANDLE CRnLObjectiveBase::GetTaskHandle( int index )
{
	return m_aTasks[index];
}