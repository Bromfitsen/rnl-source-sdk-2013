//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that propagates general data needed by clients for every player.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "igameresources.h"

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


CRnLPlayerResource* GetRnLPlayerResource(void)
{
#ifdef CLIENT_DLL
	return (CRnLPlayerResource*)g_PR;
#else
	return (CRnLPlayerResource*)g_pPlayerResource;
#endif
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLPlayerResource::Spawn( void )
{
	for (int i = 0; i < MAX_PLAYERS + 1; i++)
	{
		m_iSquad.Set(i, RNL_SQUAD_INVALID);
		m_iKit.Set(i, RNL_KIT_INVALID);
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
			// Update Values here...
			
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


void CRnLPlayerResource::ChangeSquad(int iPlayer, int iSquad)
{
	m_iSquad.Set(iPlayer, iSquad);
}

void CRnLPlayerResource::ChangeKit(int iPlayer, int iKit)
{
	m_iKit.Set(iPlayer, iKit);
}
#endif


int CRnLPlayerResource::GetSquadMemberIndex(int iTeam, int iSquad, int iMemberIndex) const
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		if (GetTeam(i) == iTeam &&
			GetSquad(i) == iSquad)
		{
			if (iMemberIndex == 0)
			{
				return i;
			}
			iMemberIndex--;
		}
	}
	return -1;
}

int CRnLPlayerResource::GetSquadMemberCount(int iTeam, int iSquad) const
{
	int MemberCount = 0;
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		if (GetTeam(i) == iTeam &&
			GetSquad(i) == iSquad)
		{
			MemberCount++;
		}
	}
	return MemberCount;
}

int CRnLPlayerResource::GetKitMemberCount(int iTeam, int iSquad, int iKit) const
{
	int MemberCount = 0;
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		if (GetTeam(i) == iTeam &&
			GetSquad(i) == iSquad &&
			GetKit(i) == iKit)
		{
			MemberCount++;
		}
	}
	return MemberCount;
}

int CRnLPlayerResource::GetKitMemberIndex(int iTeam, int iSquad, int iKit, int iMemberIndex) const
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		if (GetTeam(i) == iTeam &&
			GetSquad(i) == iSquad &&
			GetKit(i) == iKit)
		{
			if (iMemberIndex == 0)
			{
				return i;
			}
			iMemberIndex--;
		}
	}
	return -1;
}
