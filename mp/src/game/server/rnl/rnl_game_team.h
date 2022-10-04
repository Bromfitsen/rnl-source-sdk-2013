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

class CRnLSquad;

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

	// Initialization
	virtual void Init( const char *pName, int iNumber, KeyValues* pKeyVals  );
	virtual void	Update( void );

	int				GetNumberOfSquads( void );
	int				GetNextAvailableSquad( void );
	CRnLSquad*		GetSquad( int idx );

	void			SetBaseSpawn( CRnLSpawnArea* pArea );

	virtual int						LookupKitDescription( const char* pName );
	virtual bool					IsKitDescriptionValid( int iIndex );
	virtual CRnLLoadoutKitInfo&		GetKitDescription( int iIndex );
	virtual int						GetKitDescriptionCount( void );

	virtual void					OnPlayerSpawn( CRnLPlayer* pPlayer );

public:
	CUtlVector<CRnLLoadoutKitInfo>	m_aClassDescriptions;
	CUtlVector<CHandle<CRnLSquad>>	m_aSquads;
	CRnLSpawnArea*					m_pBaseSpawnArea;

protected:
	virtual bool					LoadClassDescriptions(KeyValues* pKey);
	virtual bool					LoadSquadDescriptions(KeyValues* pKey);
};


#endif // RNL_GAME_TEAM_H
