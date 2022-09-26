#ifndef C_RNL_SPAWN_AREA
#define C_RNL_SPAWN_AREA

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

class C_RnLSpawnArea : public C_BaseEntity
{
public:
	//obligatory declare class
	DECLARE_CLASS( C_RnLSpawnArea, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	void UpdateOnRemove( void );

	virtual void OnDataChanged( DataUpdateType_t type );

private:
	CNetworkHandle( C_RnLSpawnArea, m_hNextSpawnAllies );
	CNetworkHandle( C_RnLSpawnArea, m_hNextSpawnAxis );

	
	CNetworkHandle( C_RnLSpawnArea, m_hFallbackSpawnAllies);
	CNetworkHandle( C_RnLSpawnArea, m_hFallbackSpawnAxis);

	CNetworkVector( m_vecCenter );
	CNetworkVar( bool, m_bFallback );
};
#endif //C_RNL_SPAWN_AREA
