//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: Basic Timer!
//
//			Simplified Version of Valves <^^,>
//
//=============================================================================//

#ifndef RNL_BASETIMER_H
#define RNL_BASETIMER_H

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define CRnLBaseTimer C_RnLBaseTimer
#endif

class CRnLBaseTimer : public CBaseEntity
{
public:
	DECLARE_CLASS( CRnLBaseTimer, CBaseEntity );
	DECLARE_NETWORKCLASS();
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif
	CRnLBaseTimer();
	virtual ~CRnLBaseTimer();

	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void Activate( void );

	// Returns seconds to display.
	virtual float GetTimeRemaining( void );

#ifndef CLIENT_DLL
	virtual void SetTimeRemaining( int iTimerSeconds ); // Set the initial length of the timer
	virtual void AddTimerSeconds( int iSecondsToAdd, int iTeamResponsible = TEAM_UNASSIGNED ); // Add time to an already running ( or paused ) timer

	int UpdateTransmitState();

	void InputSetTime( inputdata_t &input );
	void InputAddTime( inputdata_t &input );
	void InputAddTeamTime( inputdata_t &input );
#endif

protected:
	CNetworkVar( float, m_flTimerEndTime );	
#ifndef CLIENT_DLL 
	float m_flTimerLength;
#endif 
};

#endif	//RNL_BASETIMER_H