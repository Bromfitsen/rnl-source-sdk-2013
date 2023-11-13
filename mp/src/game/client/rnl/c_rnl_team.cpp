//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side C_SDKTeam class
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "engine/IEngineSound.h"
#include "hud.h"
#include "recvproxy.h"
#include "c_rnl_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_NETWORKCLASS_ALIASED(RnLTeam, DT_RnLTeam);

BEGIN_NETWORK_TABLE(CRnLTeam, DT_RnLTeam)
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Get a pointer to the specified TF team manager
//-----------------------------------------------------------------------------
C_RnLTeam *GetGlobalRnLTeam( int iIndex )
{
	return (C_RnLTeam*)GetGlobalTeam( iIndex );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_RnLTeam::C_RnLTeam()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_RnLTeam::~C_RnLTeam()
{
}

