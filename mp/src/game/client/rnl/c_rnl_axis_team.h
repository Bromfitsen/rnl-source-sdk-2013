//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_RNL_AXIS_TEAM_H
#define C_RNL_AXIS_TEAM_H

#ifdef _WIN32
#pragma once
#endif


#include "utlvector.h"
#include "c_rnl_game_team.h"
#include "c_rnl_base_squad.h"

class C_RnLAxisTeam;

//-----------------------------------------------------------------------------
// Puprose: Axis Squad
//-----------------------------------------------------------------------------
class C_RnLAxisSquad : public C_RnLBaseSquad
{
	DECLARE_CLASS( C_RnLAxisSquad, C_RnLBaseSquad );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_RnLAxisSquad();

	virtual bool		Load( KeyValues* pKey );

	void			SetOwnerTeam( C_RnLAxisTeam* pOwner );
	C_RnLAxisTeam*	GetOwnerTeam( void );

private:
	C_RnLAxisTeam*	m_pOwner;
};

//-----------------------------------------------------------------------------
// Purpose: Team Manager
//-----------------------------------------------------------------------------
class C_RnLAxisTeam : public C_RnLGameTeam
{
	DECLARE_CLASS( C_RnLAxisTeam, C_RnLGameTeam );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_RnLAxisTeam();

	// Initialization
	virtual void	InitSquads( KeyValues* pKeyVals  );
	virtual void	PostDataUpdate( DataUpdateType_t updateType );
};


#endif // C_RNL_AXIS_TEAM_H
