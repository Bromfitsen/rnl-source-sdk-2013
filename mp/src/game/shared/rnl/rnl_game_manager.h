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

	void Spawn( void ) OVERRIDE;
	void Precache( void ) OVERRIDE;
	void Activate( void ) OVERRIDE;

	//Shared Spawn Timer Stuff
	virtual float GetSpawnTimer( int iTeam ) const;
	virtual float GetAxisSpawnTimer( void ) const;
	virtual float GetAlliedSpawnTimer( void ) const;

	//Shared Ticket Stuff
	virtual int GetTicketsRemaining( int iTeam ) const;
	virtual int GetAxisTicketsRemaining( void ) const;
	virtual int GetAlliedTicketsRemaining( void ) const;

#ifndef CLIENT_DLL
	int UpdateTransmitState() OVERRIDE;
	void Update();
	
	virtual float GetHoldToWinTime( void );

	//Server side spawn timer handling
	virtual float GetSpawnDelay(int iTeam);
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

protected:
	float m_fHoldToWinDuration;

	float m_fAlliedSpawnDelay;
	float m_fAxisSpawnDelay;

	int m_iAlliedInitialTickets;
	int m_iAxisInitialTickets;
#endif 
};

#endif	//RNL_GAME_MANAGER_H