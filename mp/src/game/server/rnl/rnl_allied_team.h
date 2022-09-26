//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_ALLIED_TEAM_H
#define RNL_ALLIED_TEAM_H

#ifdef _WIN32
#pragma once
#endif


#include "utlvector.h"
#include "rnl_game_team.h"
#include "rnl_base_squad.h"

class CRnLAlliedTeam;

//-----------------------------------------------------------------------------
// Puprose: Allied Squad
//-----------------------------------------------------------------------------
class CRnLAlliedSquad : public CRnLBaseSquad
{
	DECLARE_CLASS( CRnLAlliedSquad, CRnLBaseSquad );
	DECLARE_SERVERCLASS();

public:

	CRnLAlliedSquad();

	virtual bool		Load( KeyValues* pKey );
	virtual bool		AddPlayer( CRnLPlayer* pPlayer, int iSlot );
	virtual void		Update();

	void			SetOwnerTeam( CRnLAlliedTeam* pOwner );
	CRnLAlliedTeam*	GetOwnerTeam( void );

private:
	CRnLAlliedTeam*	m_pOwner;
};

//-----------------------------------------------------------------------------
// Purpose: Team Manager
//-----------------------------------------------------------------------------
class CRnLAlliedTeam : public CRnLGameTeam
{
	DECLARE_CLASS( CRnLAlliedTeam, CRnLGameTeam );
	DECLARE_SERVERCLASS();

public:

	CRnLAlliedTeam();

	// Initialization
	virtual void Init( const char *pName, int iNumber, KeyValues* pKeyVals  );
};


#endif // RNL_ALLIED_TEAM_H
