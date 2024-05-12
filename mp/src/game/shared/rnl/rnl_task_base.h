#ifndef RNL_TASK_BASE_H
#define RNL_TASK_BASE_H

#ifdef _WIN32
#pragma once
#endif

#include "rnl_itask.h"
#include "rnl_iobjective.h"
#include "rnl_objective_base.h"

#ifdef CLIENT_DLL
	#define CRnLTaskBase C_RnLTaskBase
#endif

#define RNL_MAX_TASK_NAME 64

class CRnLTaskBase : public CBaseAnimating, public IRnLTask
{
public:
	//obligatory declare class
	DECLARE_CLASS( CRnLTaskBase, CBaseEntity);
	DECLARE_NETWORKCLASS() ;
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CRnLTaskBase();
	~CRnLTaskBase() override;
	
	virtual void Spawn( void );
	virtual void UpdateOnRemove();
#ifndef CLIENT_DLL
	virtual int UpdateTransmitState() { return SetTransmitState( FL_EDICT_ALWAYS ); }
	virtual void Activate( void );
#endif

	//IRnLTask
public:
	virtual bool			AreRequirementsMet( int iTeam );
	virtual int				GetTaskState()							{ return m_TaskState; }
	virtual int				GetTaskPrevState()						{ return m_TaskPrevState; }
	virtual RnLTaskType		GetTaskType()							{ return RNL_TASK_TYPE_UNKNOWN; }
	virtual Vector			GetTaskOrigin()							{ return m_vTaskOrigin; }
	virtual bool			IsTaskVisibleOnMap()					{ return m_bVisibleOnMap; } 
	virtual void			Reset( void );
	virtual void			OnStateChange( int iState, int iTeam )	{}
public:
	virtual void SetVisibleOnMap( bool bState )	{ m_bVisibleOnMap = bState; }
	virtual void SetTaskState( int iState );

	virtual void			GetTaskExtents(Vector& min, Vector& max);
	virtual void			SetTaskExtents( Vector min, Vector max);

	const char*				GetTaskName( void )						{ return m_szTaskName.Get(); }

	virtual IRnLObjective*	GetParentObjective( void )							{ return m_eParentObjective.Get(); }
	virtual void			SetParentObjective( CRnLObjectiveBase* pParent )	{ m_eParentObjective = pParent; }

	virtual int				GetCapturePercent( void )			{ return m_iCapturePercent; }
	virtual void			SetCapturePercent( int iVal )		{ m_iCapturePercent = iVal; }

protected:

	virtual void			PreStateChange( int iNewState );
	virtual void			PostStateChange( void );

#ifndef CLIENT_DLL
	bool	m_bInitialMapVisibility;
	int		m_iInitialControllingTeam;
	int		m_iCapturableTeam;

	string_t	m_iszAxisOrderFilterName;
	string_t	m_iszAlliesOrderFilterName;

	COutputEvent m_OnAxisCaptureStart;
	COutputEvent m_OnAxisCaptureBlocked;
	COutputEvent m_OnAxisCaptureFailed;
	COutputEvent m_OnAxisCaptured;
	COutputEvent m_OnAxisLose;

	COutputEvent m_OnAlliedCaptureStart;
	COutputEvent m_OnAlliedCaptureBlocked;
	COutputEvent m_OnAlliedCaptureFailed;
	COutputEvent m_OnAlliedCaptured;
	COutputEvent m_OnAlliedLose;

	COutputEvent m_OnBecomeNeutral;
#endif

	
	CNetworkVar( int, m_iCapturePercent );
	CNetworkVar( CHandle<CRnLObjectiveBase>, m_eParentObjective );
	CNetworkVar( bool, m_bVisibleOnMap );
	CNetworkVar( int, m_TaskState );
	CNetworkVar( int, m_TaskPrevState );
	CNetworkVar( Vector, m_vTaskOrigin );
	CNetworkVar( Vector, m_vTaskExtentMin );
	CNetworkVar( Vector, m_vTaskExtentMax );
	CNetworkString( m_szTaskName, RNL_MAX_TASK_NAME );
};

#ifdef CLIENT_DLL
CUtlVector<CRnLTaskBase*>& GetClientTaskList( void );
CRnLTaskBase* FindClientTaskByName( char* pName );
#endif

#endif //RNL_TASK_BASE_H