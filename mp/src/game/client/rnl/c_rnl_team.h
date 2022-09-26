//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTFTeam class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_RNL_TEAM_H
#define C_RNL_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_team.h"
#include "shareddefs.h"

//-----------------------------------------------------------------------------
// Purpose: TF's Team manager
//-----------------------------------------------------------------------------
class C_RnLTeam : public C_Team
{
	DECLARE_CLASS( C_RnLTeam, C_Team );
	DECLARE_CLIENTCLASS();

public:

					C_RnLTeam();
	virtual			~C_RnLTeam();

	virtual	bool	LoadClassDescriptions( KeyValues* pKey ) { return true; }
};

extern C_RnLTeam *GetGlobalRnLTeam( int iIndex );

#endif // C_RNL_TEAM_H
