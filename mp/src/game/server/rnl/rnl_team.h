//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_TEAM_H
#define RNL_TEAM_H

#ifdef _WIN32
#pragma once
#endif


#include "utlvector.h"
#include "team.h"
#include "rnl_player.h"

//-----------------------------------------------------------------------------
// Purpose: Team Manager
//-----------------------------------------------------------------------------
class CRnLTeam : public CTeam
{
	DECLARE_CLASS( CRnLTeam, CTeam );
	DECLARE_SERVERCLASS();

public:

	// Initialization
	virtual void	Init( const char *pName, int iNumber, KeyValues* pKeyVals );
	virtual void	OnPlayerSpawn( CRnLPlayer* pPlayer ) {}
	//Can Has Cheezburger?
	virtual bool	CanHaveItem( CRnLPlayer* pPlayer, int iType, int iItem ) { return false; }
	virtual void	Update( void ) {}
};


extern CRnLTeam *GetGlobalRnLTeam( int iIndex );


#endif // RNL_TEAM_H
