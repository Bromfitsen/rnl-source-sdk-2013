#ifndef RNL_AIR_SUPPORT_H
#define RNL_AIR_SUPPORT_H

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
	#define CRnLAirSupport C_RnLAirSupport
	#define CRnLAirSupportVehicle C_RnLAirSupportVehicle
#endif

enum eAirSupportStates
{
	RNL_AIRSUPPORT_STATE_NONE = 0,
	RNL_AIRSUPPORT_BOMBING_INITIALISED,
	RNL_AIRSUPPORT_BOMBING_APPROACHING,
	RNL_AIRSUPPORT_BOMBING_IN_PROGRESS,
	RNL_AIRSUPPORT_BOMBING_DEPARTING,
};

class CRnLAirSupportVehicle;
class CRnLAirSupport : public CBaseEntity
{
public:
	//obligatory declare class
	DECLARE_CLASS( CRnLAirSupport, CBaseEntity );
	DECLARE_NETWORKCLASS();
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CRnLAirSupport();

	virtual void Precache( void );
	virtual void Spawn( void );
	//virtual void UpdateOnRemove();

#ifdef CLIENT_DLL

	//virtual bool ShouldDraw( void );
	//virtual void OnDataChanged( DataUpdateType_t type );

#else

	//virtual void Activate( void );
	virtual void BombingInitialise( Vector vecTarget );
	virtual void BombingRunThink( void );

	// Input function
	void InputBombTarget( inputdata_t &inputData );

#endif

protected:
#ifndef CLIENT_DLL
	COutputEvent m_OnBeginFlight;
	COutputEvent m_OnFinishFlight;

	COutputEvent m_OnBeginBombingRun;
	COutputEvent m_OnFinishBombingRun;


	CHandle<CPointEntity>	m_hEntryPoint;
	CUtlVector< CHandle< CRnLAirSupportVehicle > > m_hSupportVehicles;

	typedef struct tDelayedEvent
	{
		float time;
		Vector position;
	}RnLDelayedSupportEvent;
	CUtlVector<RnLDelayedSupportEvent> m_vDelayedEvents;
	
	Vector	m_vPositionTarget;
	Vector	m_vExitTarget;
	int m_iOwningTeam;
	float m_fYawOffset;

	float m_fSpeed;
	float m_fTravelTime;
	float m_fBombingRadius;
	int m_iSupportForceSize;
	int m_iBomber;
	float m_flNextBombTime;
	float m_flBombDelay;

	int m_iBombModelIndex;
#endif

	CNetworkVar( int, m_iSupportState );
};

class CRnLAirSupportVehicle : public CBaseAnimating
{
public:
	//obligatory declare class
	DECLARE_CLASS( CRnLAirSupportVehicle, CBaseAnimating );
	DECLARE_NETWORKCLASS();
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CRnLAirSupportVehicle();

	virtual void Precache( void );
	virtual void Spawn( void );

protected:
};

#endif //RNL_AIR_SUPPORT_H	
