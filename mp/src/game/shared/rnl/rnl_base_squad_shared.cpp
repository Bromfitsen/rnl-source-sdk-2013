//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"

#ifdef CLIENT_DLL
	#include "c_rnl_base_squad.h"

	#define CRnLBaseSquad C_RnLBaseSquad
#else
	#include "rnl_base_squad.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Needed because this is an entity, but should never be used
//-----------------------------------------------------------------------------
CRnLBaseSquad::CRnLBaseSquad()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CRnLBaseSquad::~CRnLBaseSquad()
{
}

bool CRnLBaseSquad::AddPlayer( CRnLPlayer* pPlayer, int iSlot )
{
	if( iSlot < 0 || iSlot >= m_aSlotInfo.Count() )
		return false;

	if( m_aSlotInfo[iSlot].pMembers.Count() >= m_aSlotInfo[iSlot].iMaxCount )
		return false;

	if( m_aSlotInfo[iSlot].pMembers.Find( pPlayer ) >= 0 )
		return true;

	m_aSlotInfo[iSlot].pMembers.AddToTail( pPlayer );
	NetworkStateChanged();
	return true;
}

bool CRnLBaseSquad::RemovePlayer( CRnLPlayer* pPlayer )
{
	if( !pPlayer )
		return false;

	if( m_hSquadLeader.Get() == pPlayer )
		m_hSquadLeader = NULL;

	if( pPlayer->GetKitNumber() < 0 || pPlayer->GetKitNumber() >= m_aSlotInfo.Count() )
		return false;

	NetworkStateChanged();
	return m_aSlotInfo[pPlayer->GetKitNumber()].pMembers.FindAndRemove( pPlayer );
}

int CRnLBaseSquad::SquadSize( void )
{
	int iSize = 0;
	for( int i = 0; i < m_aSlotInfo.Count(); i++ )
	{
		iSize += m_aSlotInfo[i].pMembers.Count();
	}
	return iSize;
}

bool CRnLBaseSquad::IsSquadFull( void )
{
	for( int i = 0; i < m_aSlotInfo.Count(); i++ )
	{
		if( m_aSlotInfo[i].pMembers.Count() < m_aSlotInfo[i].iMaxCount )
			return false;
	}
	return true;
}

bool CRnLBaseSquad::AreRequirementsMet( void )
{
	return true;
}

bool CRnLBaseSquad::IsKitAvailable( int iKit )
{
	if( iKit < 0 || iKit >= m_aSlotInfo.Count() )
		return false;

	return (m_aSlotInfo[iKit].pMembers.Count() <= m_aSlotInfo[iKit].iMaxCount);
}

int CRnLBaseSquad::GetKitDescription( int iKit )
{
	if( iKit < 0 || iKit >= m_aSlotInfo.Count() )
		return false;

	return (m_aSlotInfo[iKit].iKitDesc);
}

int CRnLBaseSquad::GetTotalAvailableKits( void )
{
	return m_aSlotInfo.Count();
}

const char* CRnLBaseSquad::GetSquadName( void )
{
	return m_szSquadReferenceName;
}

int CRnLBaseSquad::GetSlotCount( void )
{
	return m_aSlotInfo.Count();
}

CSquadSlotInfo& CRnLBaseSquad::GetSlotInfo( int indx )
{
	return m_aSlotInfo[indx];
}