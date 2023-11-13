//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that propagates general data needed by clients for every player.
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_PLAYER_RESOURCE_H
#define RNL_PLAYER_RESOURCE_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#include "c_playerresource.h"
#define CPlayerResource C_PlayerResource
#define CRnLPlayerResource C_RnLPlayerResource
#else
#include "player_resource.h"
#endif

class CRnLPlayerResource : public CPlayerResource
{
public:
	DECLARE_CLASS(CRnLPlayerResource, CPlayerResource);
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

#ifndef CLIENT_DLL
	virtual void Spawn( void );
	virtual void UpdatePlayerData( void );

	virtual void	ChangeSquad(int iPlayer, int iSquad);
	virtual void	ChangeKit(int iPlayer, int iSquad);
#endif

	int GetSquadMemberCount(int iTeam, int iSquad) const;
	int GetSquadMemberIndex(int iTeam, int iSquad, int iMemberIndex) const;
	int GetKitMemberCount(int iTeam, int iSquad, int iKit) const;
	int GetKitMemberIndex(int iTeam, int iSquad, int iKit, int iMemberIndex) const;

	FORCEINLINE int GetTeam(int PlayerIndex) const {
#ifdef CLIENT_DLL
		return m_iTeam[PlayerIndex];
#else
		return m_iTeam.Get(PlayerIndex);
#endif
	}

	FORCEINLINE int GetSquad(int PlayerIndex) const {
#ifdef CLIENT_DLL
		return m_iSquad[PlayerIndex];
#else
		return m_iSquad.Get(PlayerIndex);
#endif
	}

	FORCEINLINE int GetKit(int PlayerIndex) const {
#ifdef CLIENT_DLL
		return m_iKit[PlayerIndex];
#else
		return m_iKit.Get(PlayerIndex);
#endif
	}

protected:
	// Data for each player that's propagated to all clients
	// Stored in individual arrays so they can be sent down via datatables
	CNetworkArray( int, m_iSquad, MAX_PLAYERS+1 );
	CNetworkArray( int, m_iKit, MAX_PLAYERS+1 );
};

extern CRnLPlayerResource* GetRnLPlayerResource(void);

#endif // RNL_PLAYER_RESOURCE_H
