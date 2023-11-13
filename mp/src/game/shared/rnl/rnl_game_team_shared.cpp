//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_rnl_base.h"

#include "rnl_squad.h"
#ifdef CLIENT_DLL
	#include "c_rnl_game_team.h"
#else
	#include "rnl_game_team.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(RnLGameTeam, DT_RnLGameTeam);

// Datatable
BEGIN_NETWORK_TABLE(CRnLGameTeam, DT_RnLGameTeam)
	PropTime(PROPINFO(m_fWaveSpawnTime)),
	PropInt(PROPINFO(m_iSpawnTickets)),
	PropEHandle(PROPINFO(m_hBaseSpawnArea)),
	PropUtlVectorDataTable(m_aClassDescriptions, RNL_KITS_MAX, DT_RnLLoadoutKitInfo),
	PropUtlVector(PROPINFO_UTLVECTOR(m_aSquads), RNL_SQUADS_MAX, PropEHandle("m_aSquads::entry", 0, 0)),
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(rnl_game_team, CRnLGameTeam);

CRnLGameTeam* GetRnLGameTeam(int iTeamNumber)
{
	CRnLTeam* pTeam = GetGlobalRnLTeam(iTeamNumber);
	if (pTeam != nullptr && pTeam->IsGameTeam())
	{
		return (CRnLGameTeam*)pTeam;
	}
	return nullptr;
}
int CRnLGameTeam::GetNumberOfSquads( void ) const
{
	int count = 0;
	for (int i = 0; i < m_aSquads.Count(); i++)
	{
		if (m_aSquads[i].IsValid())
		{
			count++;
		}
		else
		{
			break;
		}
	}
	return count;
}

const CRnLSquad* CRnLGameTeam::GetSquad(int idx) const
{
	if (idx < 0 || idx >= RNL_SQUADS_MAX)
		return NULL;

	if (m_aSquads[idx].IsValid() == false)
		return NULL;

	return m_aSquads[idx];
}

int CRnLGameTeam::LookupKitDescription( const char* pName ) const
{
	for (int i = 0; i < m_aClassDescriptions.Count(); i++)
	{
		if (m_aClassDescriptions[i].iKitId >= 0 &&
			Q_stricmp(pName, m_aClassDescriptions[i].name.Get()) == 0)
		{
			return i;
		}
	}
	return -1;
}

bool CRnLGameTeam::IsKitDescriptionValid( int iIndex ) const
{
	if( iIndex < 0 || iIndex >= m_aClassDescriptions.Count() )
		return false;
	if (m_aClassDescriptions[iIndex].iKitId < 0)
		return false;

	return true;
}

const RnLLoadoutKitInfo& CRnLGameTeam::GetKitDescription( int iIndex ) const
{
	return m_aClassDescriptions[iIndex];
}

int CRnLGameTeam::GetKitDescriptionCount( void ) const
{
	return m_aClassDescriptions.Count();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CRnLGameTeam::GetWaveSpawnTimer(void) const
{
	return (m_fWaveSpawnTime - gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CRnLGameTeam::GetSpawnTickets(void) const
{
	return m_iSpawnTickets;
}
