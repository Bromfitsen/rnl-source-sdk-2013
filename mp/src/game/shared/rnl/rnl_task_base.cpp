#include "cbase.h"
#include "rnl_task_base.h"
#include "rnl_gamerules.h"

#include "tier0/memdbgon.h"

const char *gpCaptureStateNames[RNL_TASK_STATES_MAX] = {
	"Neutral",									//RNL_TASK_NEUTRAL
	
	"Invalid State",							//RNL_TASK_ALLIED_STATES_MIN
	"Allied Controlled",						//RNL_TASK_ALLIED_CONTROLLED
	"Allied Capturing From Neutral",			//RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL
	"Allied Capturing From Neutral Blocked",	//RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL_BLOCKED
	"Allied Capturing From Axis",				//RNL_TASK_ALLIED_CAPTURING_FROM_AXIS,
	"Allied Capturing From Axis Blocked",		//RNL_TASK_ALLIED_CAPTURING_FROM_AXIS_BLOCKED,
	"Invalid State",							//RNL_TASK_ALLIED_STATES_MAX,
	
	"Invalid State",							//RNL_TASK_AXIS_STATES_MIN
	"Axis Controlled",							//RNL_TASK_AXIS_CONTROLLED
	"Axis Capturing From Neutral",				//RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL
	"Axis Capturing From Neutral Blocked",		//RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL_BLOCKED
	"Axis Capturing From Allied",				//RNL_TASK_AXIS_CAPTURING_FROM_ALLIED,
	"Axis Capturing From Allied Blocked",		//RNL_TASK_AXIS_CAPTURING_FROM_ALLIED_BLOCKED,
	"Invalid State",							//RNL_TASK_AXIS_STATES_MAX,
};

const char *TaskStateToName( int state )
{
	if( state < RNL_TASK_NEUTRAL || state >= RNL_TASK_STATES_MAX )
		return "Unknown";
	return gpCaptureStateNames[state];
}

IMPLEMENT_NETWORKCLASS_ALIASED(RnLTaskBase, DT_RnLTaskBase)
BEGIN_NETWORK_TABLE( CRnLTaskBase, DT_RnLTaskBase )
#ifndef CLIENT_DLL
	SendPropEHandle( SENDINFO( m_eParentObjective ) ),
	SendPropBool( SENDINFO( m_bVisibleOnMap)),
	SendPropInt( SENDINFO( m_TaskState ) ),
	SendPropInt( SENDINFO( m_TaskPrevState ) ),
	SendPropInt( SENDINFO( m_iCapturePercent ) ),
	SendPropString( SENDINFO( m_szTaskName ) ),
	SendPropVector( SENDINFO(m_vTaskOrigin), -1, SPROP_COORD, 0.0f, HIGH_DEFAULT ),	// only sent once
	SendPropVector( SENDINFO(m_vTaskExtentMin), -1, SPROP_COORD, 0.0f, HIGH_DEFAULT ),	// only sent once
	SendPropVector( SENDINFO(m_vTaskExtentMax), -1, SPROP_COORD, 0.0f, HIGH_DEFAULT ),	// only sent once
#else
	RecvPropEHandle( RECVINFO( m_eParentObjective ) ),
	RecvPropBool( RECVINFO( m_bVisibleOnMap)),
	RecvPropInt( RECVINFO( m_TaskState ) ),
	RecvPropInt( RECVINFO( m_TaskPrevState ) ),
	RecvPropInt( RECVINFO( m_iCapturePercent ) ),
	RecvPropString( RECVINFO( m_szTaskName ) ),
	RecvPropVector( RECVINFO(m_vTaskOrigin) ),
	RecvPropVector( RECVINFO(m_vTaskExtentMin) ),
	RecvPropVector( RECVINFO(m_vTaskExtentMax) ),
#endif
END_NETWORK_TABLE()

#ifndef CLIENT_DLL
	BEGIN_DATADESC( CRnLTaskBase )
		DEFINE_KEYFIELD( m_iInitialControllingTeam, FIELD_INTEGER, "initialowners" ),
		DEFINE_KEYFIELD( m_iCapturableTeam, FIELD_INTEGER, "captureteam" ),

		DEFINE_OUTPUT( m_OnAxisCaptureStart,		"OnAxisCaptureStart" ),
		DEFINE_OUTPUT( m_OnAxisCaptureBlocked,		"OnAxisCaptureBlocked" ),
		DEFINE_OUTPUT( m_OnAxisCaptureFailed,		"OnAxisCaptureFailed" ),
		DEFINE_OUTPUT( m_OnAxisCaptured,			"OnAxisCaptured" ),
		DEFINE_OUTPUT( m_OnAxisLose,				"OnAxisLose" ),

		DEFINE_OUTPUT( m_OnAlliedCaptureStart,		"OnAlliesCaptureStart" ),
		DEFINE_OUTPUT( m_OnAlliedCaptureBlocked,	"OnAlliesCaptureBlocked" ),
		DEFINE_OUTPUT( m_OnAlliedCaptureFailed,		"OnAlliesCaptureFailed" ),
		DEFINE_OUTPUT( m_OnAlliedCaptured,			"OnAlliesCaptured" ),
		DEFINE_OUTPUT( m_OnAlliedLose,				"OnAlliesLose" ),

		DEFINE_OUTPUT( m_OnBecomeNeutral,			"OnBecomeNeutral" ),
	END_DATADESC()
#endif

#ifdef CLIENT_DLL
CUtlVector<CRnLTaskBase*> gvClientTaskList;

CUtlVector<CRnLTaskBase*>& GetClientTaskList( void )
{
	return gvClientTaskList;
}

CRnLTaskBase* FindClientTaskByName( char* pName )
{
	for( int i = 0; i < gvClientTaskList.Count(); i++ )
	{
		if( Q_stricmp( gvClientTaskList[i]->GetTaskName(), pName ) == 0 )
			return gvClientTaskList[i];
	}

	return NULL;
}
#endif

CRnLTaskBase::CRnLTaskBase()
{
	m_bVisibleOnMap = true;
	m_TaskPrevState = RNL_TASK_INVALID;
	m_TaskState = RNL_TASK_NEUTRAL;

#ifndef CLIENT_DLL
	m_iInitialControllingTeam = TEAM_NONE;
	m_iCapturableTeam = TEAM_NONE;
#else
	gvClientTaskList.AddToTail( this );
#endif
}

CRnLTaskBase::~CRnLTaskBase()
{
#ifdef CLIENT_DLL
	gvClientTaskList.FindAndRemove( this );
#endif
}

#ifndef CLIENT_DLL
	void CRnLTaskBase::Activate( void )
	{
		BaseClass::Activate();
	}
#endif

void CRnLTaskBase::Reset( void )
{
}

void CRnLTaskBase::Spawn( void )
{
	BaseClass::Spawn();

#ifndef CLIENT_DLL
	m_vTaskOrigin = GetAbsOrigin();

	if( m_iInitialControllingTeam == TEAM_AXIS )
		SetTaskState( RNL_TASK_AXIS_CONTROLLED );
	else if( m_iInitialControllingTeam == TEAM_ALLIES )
		SetTaskState( RNL_TASK_ALLIED_CONTROLLED );
	else
		SetTaskState( RNL_TASK_NEUTRAL );

	Q_strncpy( m_szTaskName.GetForModify(), STRING(GetEntityName()), RNL_MAX_TASK_NAME );
#endif
}

void CRnLTaskBase::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();
}

bool CRnLTaskBase::AreRequirementsMet( int iTeam )
{
	if( m_eParentObjective.Get() )
		return m_eParentObjective.Get()->AreRequirementsMet( iTeam );
	
	return true;
}

void CRnLTaskBase::GetTaskExtents( Vector& min, Vector& max)
{
	min = m_vTaskExtentMin;
	max = m_vTaskExtentMax;
}

void CRnLTaskBase::SetTaskExtents( Vector min, Vector max)
{
	m_vTaskExtentMin = min;
	m_vTaskExtentMax = max;
}

void CRnLTaskBase::SetTaskState( int iState )
{
	//DevMsg( "Setting Task State to %d\n", iState );
	if( m_TaskState == iState )
		return;

#ifndef CLIENT_DLL
	/////////////////////////////////////
	//
	// FIRE EVENTS FOR THE NEW STATE
	//
	/////////////////////////////////////
	//------------------------
	//Neutral Events
	//------------------------
	if( iState == RNL_TASK_NEUTRAL )
	{
		switch( GetTaskState() )
		{
		case RNL_TASK_AXIS_CONTROLLED:
			m_OnAxisLose.FireOutput( this, this );
			break;
		case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL:
		case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL_BLOCKED:
		case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED:
		case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED_BLOCKED:
			m_OnAxisCaptureFailed.FireOutput( this, this );
			break;
		case RNL_TASK_ALLIED_CONTROLLED:
			m_OnAlliedLose.FireOutput( this, this );
			break;
		case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL:
		case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL_BLOCKED:
		case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS:
		case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS_BLOCKED:
			m_OnAlliedCaptureFailed.FireOutput( this, this );
			break;
		};

		m_OnBecomeNeutral.FireOutput( this, this );
	}
	//------------------------
	//Allied Events
	//------------------------
	else if( iState > RNL_TASK_ALLIED_STATES_MIN && iState < RNL_TASK_ALLIED_STATES_MAX )
	{
		switch( GetTaskState() )
		{
		case RNL_TASK_AXIS_CONTROLLED:
			m_OnAxisLose.FireOutput( this, this );
			break;
		case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL:
		case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL_BLOCKED:
		case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED:
		case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED_BLOCKED:
			m_OnAxisCaptureFailed.FireOutput( this, this );
			break;
		};	

		switch( iState )
		{
		case RNL_TASK_ALLIED_CONTROLLED:
			m_OnAlliedCaptured.FireOutput( this, this );
			break;
		case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL:
			m_OnAlliedCaptureStart.FireOutput( this, this );
			break;
		case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL_BLOCKED:
			m_OnAlliedCaptureBlocked.FireOutput( this, this );
			break;
		case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS:
			m_OnAlliedCaptureStart.FireOutput( this, this );
			break;
		case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS_BLOCKED:
			m_OnAlliedCaptureBlocked.FireOutput( this, this );
			break;
		};
	}
	//------------------------
	//Axis Events
	//------------------------
	else if( iState > RNL_TASK_AXIS_STATES_MIN && iState < RNL_TASK_AXIS_STATES_MAX )
	{
		switch( GetTaskState() )
		{
		case RNL_TASK_ALLIED_CONTROLLED:
			m_OnAlliedLose.FireOutput( this, this );
			break;
		case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL:
		case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL_BLOCKED:
		case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS:
		case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS_BLOCKED:
			m_OnAlliedCaptureFailed.FireOutput( this, this );
			break;
		};

		switch( iState )
		{
		case RNL_TASK_AXIS_CONTROLLED:
			m_OnAxisCaptured.FireOutput( this, this );
			break;
		case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL:
			m_OnAxisCaptureStart.FireOutput( this, this );
			break;
		case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL_BLOCKED:
			m_OnAxisCaptureBlocked.FireOutput( this, this );
			break;
		case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED:
			m_OnAxisCaptureStart.FireOutput( this, this );
			break;
		case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED_BLOCKED:
			m_OnAxisCaptureBlocked.FireOutput( this, this );
			break;
		};

	}

	if( iState == RNL_TASK_ALLIED_CONTROLLED )
		ChangeTeam( TEAM_ALLIES );
	else if( iState == RNL_TASK_AXIS_CONTROLLED )
		ChangeTeam( TEAM_AXIS );
	else
		ChangeTeam( TEAM_NONE );
#endif
	PreStateChange( iState );

	m_TaskPrevState = m_TaskState;
	m_TaskState = iState;

	PostStateChange();

	if( m_eParentObjective.Get() )
	{
		m_eParentObjective.Get()->OnTaskStateChanged( this );
	}
}

void CRnLTaskBase::PreStateChange( int iNewState )
{
	if( GetTaskPrevState() == RNL_TASK_INVALID )
		return;

#ifndef CLIENT_DLL
	//send it to everyone
	IGameEvent *event = gameeventmanager->CreateEvent( "rnl_task_state_change" );
	if ( event )
	{
		event->SetString( "task_name", GetTaskName() );
		event->SetInt( "new_state", iNewState );
		event->SetInt( "current_state", GetTaskState() );
		event->SetInt( "previous_state", GetTaskPrevState() );
		gameeventmanager->FireEvent( event );
	}
#endif
}

void CRnLTaskBase::PostStateChange( void )
{
}