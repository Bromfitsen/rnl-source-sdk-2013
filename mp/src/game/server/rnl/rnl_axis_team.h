//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_AXIS_TEAM_H
#define RNL_AXIS_TEAM_H

#ifdef _WIN32
#pragma once
#endif


#include "utlvector.h"
#include "rnl_game_team.h"
#include "rnl_base_squad.h"

class CRnLAxisTeam;

//-----------------------------------------------------------------------------
// Puprose: Axis Squad
//-----------------------------------------------------------------------------
class CRnLAxisSquad : public CRnLBaseSquad
{
	DECLARE_CLASS( CRnLAxisSquad, CRnLBaseSquad );
	DECLARE_SERVERCLASS();

public:

	CRnLAxisSquad();

	virtual bool		Load( KeyValues* pKey );
	virtual bool		AddPlayer( CRnLPlayer* pPlayer, int iSlot );
	virtual void		Update();

	void			SetOwnerTeam( CRnLAxisTeam* pOwner );
	CRnLAxisTeam*	GetOwnerTeam( void );

private:
	CRnLAxisTeam*	m_pOwner;
};

//-----------------------------------------------------------------------------
// Purpose: Team Manager
//-----------------------------------------------------------------------------
class CRnLAxisTeam : public CRnLGameTeam
{
	DECLARE_CLASS( CRnLAxisTeam, CRnLGameTeam );
	DECLARE_SERVERCLASS();

public:

	CRnLAxisTeam();

	// Initialization
	virtual void Init( const char *pName, int iNumber, KeyValues* pKeyVals  );
};


#endif // RNL_AXIS_TEAM_H
