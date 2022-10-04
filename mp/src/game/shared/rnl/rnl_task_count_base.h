#ifndef RNL_TASK_COUNT_BASE_H
#define RNL_TASK_COUNT_BASE_H

#ifdef _WIN32
#pragma once
#endif

#include "rnl_task_base.h"
#include "rnl_shareddefs.h"

#ifdef CLIENT_DLL
	#define CRnLTaskCountBase C_RnLTaskCountBase
#endif

class CRnLTaskCountBase : public CRnLTaskBase
{
public:
	//obligatory declare class
	DECLARE_CLASS( CRnLTaskCountBase, CRnLTaskBase );
	DECLARE_NETWORKCLASS() ;
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CRnLTaskCountBase();
	
	virtual void Spawn( void );
	virtual void UpdateOnRemove();
#ifndef CLIENT_DLL
	virtual void Activate( void );
#endif
	
public:
	virtual int GetTeamCount( int iTeam );
	virtual int GetAxisCount( void )					{ return GetTeamCount( TEAM_AXIS ); }
	virtual int GetAlliesCount( void )					{ return GetTeamCount( TEAM_ALLIES ); }

	virtual int GetRequiredCount( int iTeam );
	virtual int GetRequiredAxisCount( void )			{ return GetRequiredCount( TEAM_AXIS ); }
	virtual int GetRequiredAlliesCount( void )			{ return GetRequiredCount( TEAM_ALLIES ); }

	virtual void SetCount( int iTeam, int iCount );
	virtual void SetAxisCount( int iCount )				{ SetCount( TEAM_AXIS, iCount ); }
	virtual void SetAlliesCount( int iCount )			{ SetCount( TEAM_ALLIES, iCount ); }

	virtual void IncrementAxisCount( void )				{ SetCount( TEAM_AXIS, m_iAxisCount+1 ); }
	virtual void DecrementAxisCount( void )				{ SetCount( TEAM_AXIS, m_iAxisCount-1 ); }
	virtual void IncrementAlliesCount( void )			{ SetCount( TEAM_ALLIES, m_iAlliesCount+1 ); }
	virtual void DecrementAlliesCount( void )			{ SetCount( TEAM_ALLIES, m_iAlliesCount-1 ); }

	virtual void SetRequiredCount( int iTeam, int iCount );
	virtual void SetRequiredAxisCount( int iCount )			{ SetRequiredCount( TEAM_AXIS, iCount ); }
	virtual void SetRequiredAlliesCount( int iCount )		{ SetRequiredCount( TEAM_ALLIES, iCount ); }

public:

	virtual void OnCountChanged( int iTeam, int iNewCount );

	virtual void OnRequiredCountAcheived( int iTeam )	{};
	virtual void OnRequiredCountLost( int iTeam )		{};

protected:
#ifndef CLIENT_DLL
	int			m_iInitialAxisCountRequired;
	int			m_iInitialAlliedCountRequired;

	COutputEvent m_OnAxisCountChanged;
	COutputEvent m_OnAxisCountIncrease;
	COutputEvent m_OnAxisCountDecrease;

	COutputEvent m_OnAlliedCountChanged;
	COutputEvent m_OnAlliedCountIncrease;
	COutputEvent m_OnAlliedCountDecrease;
#endif

	CNetworkVar( int, m_iAlliesCountRequired );
	CNetworkVar( int, m_iAlliesCount );
	CNetworkVar( int, m_iAxisCountRequired );
	CNetworkVar( int, m_iAxisCount );
};

#endif //RNL_TASK_COUNT_BASE_H	