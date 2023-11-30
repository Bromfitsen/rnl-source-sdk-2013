// this is a "group" of spawn points, all can be enabled/disabled easily
//  the spawn points must be parented to this entity, and the capture point it is tied to provided

#ifndef RNL_SPAWN_AREA_H
#define RNL_SPAWN_AREA_H

#include "cbase.h"
#include "rnl_player.h"

class CRnLSpawnArea : public CBaseEntity
{
public:
	DECLARE_CLASS( CRnLSpawnArea, CBaseEntity );
	DECLARE_SERVERCLASS();
	//Andrew : I need this, because it's quite important
	DECLARE_DATADESC();
	
	CRnLSpawnArea();
	~CRnLSpawnArea() {};

	void InputEnableFallback( inputdata_t &inputdata );
	void InputDisableFallback( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
	void InputEnable( inputdata_t &inputdata );
	void InputEnableAllies( inputdata_t &inputdata );
	void InputEnableAxis( inputdata_t &inputdata );

	
	bool OnPlayerSpawn( CBasePlayer* pPlayer );

	void Activate() OVERRIDE;
	void Spawn() OVERRIDE;
	void Think() OVERRIDE;

	int UpdateTransmitState() OVERRIDE { return SetTransmitState( FL_EDICT_ALWAYS ); }
	//Andrew : Let the mappers specify which direction for the player to
	//         be facing when they spawn.
	QAngle GetPlayerDirection( void ){ return m_angPlayerFaceDir; }
	void Reset( void ) { ChangeTeam( m_iIntitialOwner ); }

private:

	// used on the client to check owners status
	CNetworkHandle( CRnLSpawnArea, m_hNextSpawnAllies );
	CNetworkHandle( CRnLSpawnArea, m_hNextSpawnAxis );

	
	CNetworkHandle( CRnLSpawnArea, m_hFallbackSpawnAllies );
	CNetworkHandle( CRnLSpawnArea, m_hFallbackSpawnAxis );

	// this is passed to the player and then to the client map
	CNetworkVector( m_vecCenter );
	CNetworkVar( bool, m_bFallback );

	int m_iIntitialOwner;
	bool m_bBaseSpawn;

	//This is what we use to decide a players facing direction
	QAngle m_angPlayerFaceDir;

	string_t m_NextSpawnArea[2];
	string_t m_FallbackSpawnArea[2];

};

#endif