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
#include "rnl_squad.h"
#include "rnl_spawn_area.h"


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
	IRnLSquad*		GetSquad( int idx );

	void			SetBaseSpawn( CRnLSpawnArea* pArea );

	virtual int						LookupKitDescription( const char* pName );
	virtual bool					IsKitDescriptionValid( int iIndex );
	virtual RnLKitDescription&		GetKitDescription( int iIndex );
	virtual int						GetKitDescriptionCount( void );

	virtual bool					LoadClassDescriptions( KeyValues* pKey );

	virtual void					OnPlayerSpawn( CRnLPlayer* pPlayer );

public:
	CUtlVector<RnLKitDescription>	m_aClassDescriptions;
	CUtlMap<CUtlString, int>		m_aClassIndexLookUp;
	CUtlVector< IRnLSquad * >		m_aSquads;
	CRnLSpawnArea*					m_pBaseSpawnArea;
};


#endif // RNL_GAME_TEAM_H
