//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that propagates general data needed by clients for every player.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "rnl_dt_shared.h"
#ifdef CLIENT_DLL
#include "c_rnl_player.h"
#else
#include "rnl_player.h"
#endif
#include "rnl_player_resource.h"
#include <coordsize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_NETWORKCLASS_ALIASED(RnLPlayerResource, DT_RnLPlayerResource)

// Datatable
BEGIN_NETWORK_TABLE(CRnLPlayerResource, DT_RnLPlayerResource)
	PropArray(PROPINFO_ARRAY3(m_iSquad), PropInt(PROPINFO_ARRAY(m_iSquad))),
	PropArray(PROPINFO_ARRAY3(m_iKit), PropInt(PROPINFO_ARRAY(m_iKit))),
END_NETWORK_TABLE()

BEGIN_DATADESC(CRnLPlayerResource)
END_DATADESC()

LINK_ENTITY_TO_CLASS( rnl_player_manager, CRnLPlayerResource);

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLPlayerResource::Spawn( void )
{
	for (int i = 0; i < MAX_PLAYERS + 1; i++)
	{
		m_iSquad.Set(i, -1);
		m_iKit.Set(i, -1);
	}

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLPlayerResource::UpdatePlayerData( void )
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CRnLPlayer* pPlayer = (CRnLPlayer*)UTIL_PlayerByIndex(i);
		if (pPlayer && pPlayer->IsConnected())
		{
			m_iSquad.Set(i, pPlayer->GetSquadNumber());
			m_iKit.Set(i, pPlayer->GetKitNumber());

			// Don't update transform / orientation everytime
			if (!(m_nUpdateCounter % 20))
			{
			}
		}
		else
		{
		}
	}

	BaseClass::UpdatePlayerData();
}
#endif
