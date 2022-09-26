#ifndef RNL_OBJECTIVE_BASE_H
#define RNL_OBJECTIVE_BASE_H

#ifdef _WIN32
#pragma once
#endif

#include "rnl_iobjective.h"
#include "rnl_objective_order_filter.h"

#ifdef CLIENT_DLL
	#define CRnLObjectiveBase C_RnLObjectiveBase
	class C_RnLTaskBase;
	typedef CHandle<C_RnLTaskBase> TaskHandle;
#else
	class CRnLTaskBase;
	typedef CHandle<CRnLTaskBase> TaskHandle;
#endif

#define RNL_MAX_OBJECTIVE_NAME 64

#ifndef CLIENT_DLL
	enum eObjectiveBehaviours
	{
		RNL_BEHAVIOUR_NORMAL = 0,
		RNL_BEHAVIOUR_ALL_TO_SWITCH,
		RNL_BEHAVIOUR_MORE_THAN_OTHER_TO_SWITCH,
		RNL_BEHAVIOUR_MAJORITY_TO_SWITCH,
		
		RNL_BEHAVIOUR_MAX,
	};
#endif
class CRnLObjectiveBase : public CBaseEntity, public IRnLObjective
{
public:
	//obligatory declare class
	DECLARE_CLASS( CRnLObjectiveBase, CBaseEntity);
	DECLARE_NETWORKCLASS() ;
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CRnLObjectiveBase();
	
	virtual void Spawn( void );
	virtual void UpdateOnRemove();
#ifndef CLIENT_DLL
	virtual int UpdateTransmitState() { return SetTransmitState( FL_EDICT_ALWAYS ); }
	virtual void Activate( void );
	virtual bool KeyValue( const char *szKeyName, const char *szValue ); 
	void InputSetControllingTeam( inputdata_t &input );
	void InputSetVisibleOnMap( inputdata_t &input );
	void InputSetIconVisibleOnMap( inputdata_t &input );
	void InputSetSetPrimary( inputdata_t &input );
#else
	void SetDisplayPosition( Vector& vec ) { m_DisplayPos = vec; }
	Vector GetDisplayPosition( void ) { return m_DisplayPos; }
#endif

	//IRnLObjective
public:
	virtual int				GetTaskCount( void );
	virtual IRnLTask*		GetTask( int index );
	virtual void			OnTaskStateChanged( IRnLTask* pTask );

	virtual bool			IsPrimary( void )					{ return m_bPrimary; }
	virtual int				GetObjectiveState()					{ return m_iObjectiveState; }
	virtual int				GetObjectivePrevState()				{ return m_iObjectivePrevState; }
	virtual const char*		GetObjectiveName()					{ return m_szObjectiveName.Get(); }
	virtual bool			IsObjectiveVisibleOnMap()			{ return m_bVisibleOnMap; } 
	virtual bool			IsObjectiveIconVisibleOnMap()		{ return m_bIconVisibleOnMap; } 
	
	virtual void			Reset( void );
public:
	virtual int		CalculateState( void );
	virtual void	SetVisibleOnMap( bool bState )	{ m_bVisibleOnMap = bState; }
	virtual void	SetObjectiveState( int iState );

	virtual EHANDLE			GetTaskHandle( int index );
	virtual bool			AreRequirementsMet( int iTeam );

	virtual CRnLObjectiveOrderFilter* GetOrderFilter( int iTeam );

protected:

	virtual void			PreStateChange( int iNewState );
	virtual void			PostStateChange( void );

#ifndef CLIENT_DLL
	bool	m_bInitialMapVisibility;
	int		m_iInitialControllingTeam;

	CUtlVector<CUtlString> m_szPendingTasks;
	

	string_t	m_iszAxisOrderFilterName;
	string_t	m_iszAlliesOrderFilterName;

	COutputEvent m_OnNeutral;

	COutputEvent m_OnAxisControlled;
	COutputEvent m_OnAxisLose;

	COutputEvent m_OnAlliedControlled;
	COutputEvent m_OnAlliedLose;

	int m_iBehaviour;
#else
	Vector m_DisplayPos;
#endif

public:
	CUtlVector<TaskHandle> m_aTasks;
protected:
	CNetworkString( m_szObjectiveName, RNL_MAX_OBJECTIVE_NAME );
	CNetworkVar( bool, m_bVisibleOnMap );
	CNetworkVar( bool, m_bIconVisibleOnMap );
	CNetworkVar( bool, m_bPrimary );
	CNetworkVar( int, m_iObjectiveState );
	CNetworkVar( int, m_iObjectivePrevState );
	CNetworkVar( CHandle<CRnLObjectiveOrderFilter>, m_eAxisOrderFilter );
	CNetworkVar( CHandle<CRnLObjectiveOrderFilter>, m_eAlliesOrderFilter );
	
};

#endif //RNL_OBJECTIVE_BASE_H