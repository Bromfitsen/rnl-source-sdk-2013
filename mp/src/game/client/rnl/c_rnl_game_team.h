//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTFTeam class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_RNL_GAME_TEAM_H
#define C_RNL_GAME_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_rnl_team.h"
#include "c_rnl_spawn_area.h"
#include "rnl_shareddefs.h"
#include "rnl_squad.h"

#define CRnLGameTeam C_RnLGameTeam

//-----------------------------------------------------------------------------
// Purpose: TF's Team manager
//-----------------------------------------------------------------------------
class C_RnLGameTeam : public C_RnLTeam
{
	DECLARE_CLASS( C_RnLGameTeam, C_RnLTeam );

public:
	DECLARE_CLIENTCLASS();

					C_RnLGameTeam();
	virtual			~C_RnLGameTeam();

	bool				IsGameTeam() const OVERRIDE { return true; }
	int					GetNumberOfSquads( void ) const;
	const CRnLSquad*	GetSquad(int idx) const;

	float				GetWaveSpawnTimer(void) const;
	int					GetSpawnTickets(void) const;

	virtual int						LookupKitDescription(const char* pName) const;
	virtual bool					IsKitDescriptionValid(int iIndex) const;
	virtual const RnLLoadoutKitInfo& GetKitDescription(int iIndex) const;
	virtual int						GetKitDescriptionCount(void) const;

public:
	CUtlVector<RnLLoadoutKitInfo>	m_aClassDescriptions;
	CUtlVector<CHandle<CRnLSquad>>	m_aSquads;
	CNetworkHandle(C_RnLSpawnArea,	m_hBaseSpawnArea);
	CNetworkVar(float, m_fWaveSpawnTime);
	CNetworkVar(int, m_iSpawnTickets);
};

C_RnLGameTeam* GetRnLGameTeam(int iTeamNumber);


#endif // C_RNL_GAME_TEAM_H
