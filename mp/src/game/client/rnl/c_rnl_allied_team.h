//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_RNL_ALLIED_TEAM_H
#define C_RNL_ALLIED_TEAM_H

#ifdef _WIN32
#pragma once
#endif


#include "utlvector.h"
#include "c_rnl_game_team.h"
#include "c_rnl_base_squad.h"

class C_RnLAlliedTeam;

//-----------------------------------------------------------------------------
// Puprose: Allied Squad
//-----------------------------------------------------------------------------
class C_RnLAlliedSquad : public C_RnLBaseSquad
{
	DECLARE_CLASS( C_RnLAlliedSquad, C_RnLBaseSquad );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_RnLAlliedSquad();

	virtual bool		Load( KeyValues* pKey );

	void				SetOwnerTeam( C_RnLAlliedTeam* pOwner );
	C_RnLAlliedTeam*	GetOwnerTeam( void );

private:
	C_RnLAlliedTeam*	m_pOwner;
};

//-----------------------------------------------------------------------------
// Purpose: Team Manager
//-----------------------------------------------------------------------------
class C_RnLAlliedTeam : public C_RnLGameTeam
{
	DECLARE_CLASS( C_RnLAlliedTeam, C_RnLGameTeam );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_RnLAlliedTeam();

	// Initialization
	virtual void	InitSquads( KeyValues* pKeyVals  );
	virtual void	PostDataUpdate( DataUpdateType_t updateType );
};


#endif // C_RNL_ALLIED_TEAM_H
