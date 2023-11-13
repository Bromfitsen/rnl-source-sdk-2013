//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_GAME_TEAM_H
#define RNL_GAME_TEAM_H

#ifdef _WIN32
#pragma once
#endif


#include "utlvector.h"
#include "rnl_team.h"
#include "rnl_spawn_area.h"
#include "rnl_shareddefs.h"
#include "rnl_player.h"
#include "rnl_squad.h"

//-----------------------------------------------------------------------------
// Purpose: Team Manager
//-----------------------------------------------------------------------------
class CRnLGameTeam : public CRnLTeam
{
	DECLARE_CLASS( CRnLGameTeam, CRnLTeam );
	DECLARE_SERVERCLASS();

public:

	CRnLGameTeam();
	~CRnLGameTeam();

	bool	IsGameTeam() const override { return true; }

	// Initialization
	virtual void Init( const char *pName, int iNumber, KeyValues* pKeyVals  );
	virtual void Update( void );

	virtual void AddPlayer(CBasePlayer* pPlayer);
	virtual void RemovePlayer(CBasePlayer* pPlayer);
	virtual bool JoinSquad(CRnLPlayer* pPlayer, int iSquad, int iKit);

	int					GetNumberOfSquads( void ) const;
	int					GetNextAvailableSquad( void ) const;
	const CRnLSquad*	GetSquad( int idx ) const;

	void				SetWaveSpawnTimer(float fNextWave);
	float				GetWaveSpawnTimer(void) const;
	void				SetSpawnTickets(int iTickets);
	int					GetSpawnTickets(void) const;

	void				SetBaseSpawn( CRnLSpawnArea* pArea );

	virtual int						LookupKitDescription( const char* pName ) const;
	virtual bool					IsKitDescriptionValid( int iIndex ) const;
	virtual const RnLLoadoutKitInfo& GetKitDescription( int iIndex ) const;
	virtual int						GetKitDescriptionCount( void ) const;

	virtual void					OnPlayerSpawn( CRnLPlayer* pPlayer );

public:
	CUtlVector<RnLLoadoutKitInfo>		m_aClassDescriptions;
	CUtlVector<CHandle<CRnLSquad>>		m_aSquads;
	CNetworkHandle(CRnLSpawnArea,		m_hBaseSpawnArea);
	CNetworkVar(float,					m_fWaveSpawnTime);
	CNetworkVar(int,					m_iSpawnTickets);

protected:
	virtual bool					LoadClassDescriptions(KeyValues* pKey);
	virtual bool					LoadSquadDescriptions(KeyValues* pKey);
};

CRnLGameTeam* GetRnLGameTeam(int iTeamNumber);

#endif // RNL_GAME_TEAM_H
