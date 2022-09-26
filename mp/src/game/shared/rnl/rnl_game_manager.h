//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// LIES!!! Copyright no one in particular
//
//=============================================================================//

#ifndef RNL_GAME_MANAGER_H
#define RNL_GAME_MANAGER_H

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define CRnLGameManager C_RnLGameManager
#endif

class CRnLGameManager : public CBaseEntity
{
public:
	DECLARE_CLASS( CRnLGameManager, CBaseEntity );
	DECLARE_NETWORKCLASS();
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif
	CRnLGameManager();
	virtual ~CRnLGameManager();

	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void Activate( void );

	//Shared Spawn Timer Stuff
	virtual float GetSpawnTimer( int iTeam );
	virtual float GetAxisSpawnTimer( void );
	virtual float GetAlliedSpawnTimer( void );

	//Shared Ticket Stuff
	virtual int GetTicketsRemaining( int iTeam );
	virtual int GetAxisTicketsRemaining( void );
	virtual int GetAlliedTicketsRemaining( void );

#ifndef CLIENT_DLL
	int UpdateTransmitState();
	void Update();
	
	virtual float GetHoldToWinTime( void );

	//Server side spawn timer handling
	virtual void SetSpawnTimer( int iTeam, float flDuration );
	virtual void SetAxisSpawnTimer( float flDuration );
	virtual void SetAlliedSpawnTimer( float flDuration );

	//Server side ticket handling
	virtual int GetAxisInitialTickets( void );
	virtual int GetAlliedInitialTickets( void );
	virtual void SetTicketsRemaining( int iTeam, int iTickets );
	virtual void SetAxisTicketsRemaining( int iTickets );
	virtual void SetAlliedTicketsRemaining( int iTickets );

	virtual void AddTickets( int iTeam, int iTickets );
	virtual void AddAxisTickets( int iTickets );
	virtual void AddAlliedTickets( int iTickets );

	void InputSetTickets( inputdata_t &input );
	void InputSetAxisTickets( inputdata_t &input );
	void InputSetAlliedTickets( inputdata_t &input );
	void InputAddTickets( inputdata_t &input );
	void InputAddAxisTickets( inputdata_t &input );
	void InputAddAlliedTickets( inputdata_t &input );
#endif

protected:
	CNetworkVar( float, m_fAlliedSpawnTime );
	CNetworkVar( float, m_fAxisSpawnTime );

	CNetworkVar( int, m_iAlliedTickets );
	CNetworkVar( int, m_iAxisTickets );
#ifndef CLIENT_DLL
	float m_fHoldToWinDuration;

	float m_fAlliedSpawnDelay;
	float m_fAxisSpawnDelay;

	int m_iAlliedInitialTickets;
	int m_iAxisInitialTickets;
#endif 
};

#endif	//RNL_GAME_MANAGER_H