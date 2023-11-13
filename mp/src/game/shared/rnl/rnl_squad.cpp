//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "dt_shared.h"
#ifdef CLIENT_DLL
#include "dt_utlvector_recv.h"
#else
#include "dt_utlvector_send.h"
#endif

#include "rnl_squad.h"
#include "rnl_gamerules.h"
#include "rnl_game_manager.h"
#include "rnl_player_resource.h"

#ifdef CLIENT_DLL
#include "c_rnl_game_team.h"
#else
#include "rnl_game_team.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
int g_iInsertPositions6[][2] = { {20, 20}, {340, 20},
								{20, 160}, {340, 160},
								{20, 300}, {340, 300 } };

int g_iInsertPositions4[][2] = { {170, 180}, {-10, 180},
								{170, 300}, {-10, 300} };

int g_iInsertPositions2[][2] = { {170, 180}, {-10, 180} };

int g_iInsertSize[][2] = { {160,220}, {160,100}, {160,100} };

void SquadGUIGetIdealProportions(int iCurrSquad, int iTotalSquads, int& x, int& y, int& w, int& h)
{
	if (iTotalSquads <= 2)
	{
		iCurrSquad = min(iCurrSquad, 1);
		x = g_iInsertPositions2[iCurrSquad][0];
		y = g_iInsertPositions2[iCurrSquad][1];
		w = g_iInsertSize[0][0];
		h = g_iInsertSize[0][1];
	}
	else if (iTotalSquads <= 4)
	{
		iCurrSquad = min(iCurrSquad, 3);
		x = g_iInsertPositions4[iCurrSquad][0];
		y = g_iInsertPositions4[iCurrSquad][1];
		w = g_iInsertSize[1][0];
		h = g_iInsertSize[1][1];
	}
	else
	{
		iCurrSquad = min(iCurrSquad, 5);
		x = g_iInsertPositions6[iCurrSquad][0];
		y = g_iInsertPositions6[iCurrSquad][1];
		w = g_iInsertSize[2][0];
		h = g_iInsertSize[2][1];
	}
}
#endif

BEGIN_NETWORK_TABLE_NOBASE(RnLSquadKitInfo, DT_RnLSquadKitInfo)
	PropInt(PROPINFO(iKitID)),
	PropInt(PROPINFO(iMaxCount)),
END_NETWORK_TABLE()


IMPLEMENT_NETWORKCLASS(CRnLSquad, DT_RnLSquad);

// Datatable
BEGIN_NETWORK_TABLE(CRnLSquad, DT_RnLSquad)
	PropEHandle(PROPINFO(m_hTeam)),
	PropInt(PROPINFO(m_SquadId)),
	PropString(PROPINFO(m_szSquadTitle)),
	PropEHandle(PROPINFO(m_hSquadLeader)),
	PropUtlVectorDataTable(m_KitInfo, RNL_KITS_PER_SQUAD_MAX,DT_RnLSquadKitInfo),
END_NETWORK_TABLE()


LINK_ENTITY_TO_CLASS(rnl_squad, CRnLSquad);

//-----------------------------------------------------------------------------
// Purpose: Needed because this is an entity, but should never be used
//-----------------------------------------------------------------------------
CRnLSquad::CRnLSquad()
{
	m_SquadId = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CRnLSquad::~CRnLSquad()
{
}

bool CRnLSquad::IsValid() const
{
	return m_SquadId > -1;
}

#ifndef CLIENT_DLL
int CRnLSquad::UpdateTransmitState(void)
{
	return SetTransmitState(FL_EDICT_ALWAYS);
}

bool CRnLSquad::AddPlayer( CRnLPlayer* pPlayer, int iKit )
{
	if (m_SquadId < 0)
	{
		return false;
	}

	if(iKit < 0 || iKit >= m_KitInfo.Count() )
		return false;

	if( GetMemberCount(iKit) >= m_KitInfo[iKit].iMaxCount)
		return false;

	CRnLPlayerResource* pRnLPR = GetRnLPlayerResource();
	pRnLPR->ChangeSquad(pPlayer->entindex(), m_SquadId);
	pRnLPR->ChangeKit(pPlayer->entindex(), iKit);

	// Immediately tell all clients that he's changing team. This has to be done
	// first, so that all user messages that follow as a result of the team change
	// come after this one, allowing the client to be prepared for them.
	IGameEvent* event = gameeventmanager->CreateEvent("player_squad");
	if (event)
	{
		event->SetInt("userid", pPlayer->GetUserID());
		event->SetInt("team", GetTeamNumber());
		event->SetInt("squad", m_SquadId);
		event->SetInt("slot", iKit);
		event->SetBool("disconnect", pPlayer->IsDisconnecting());

		gameeventmanager->FireEvent(event);
	}
	return true;
}

bool CRnLSquad::RemovePlayer( CRnLPlayer* pPlayer )
{
	if( !pPlayer )
		return false;

	if (m_hSquadLeader.Get() == pPlayer)
	{
		m_hSquadLeader = NULL;
		NetworkStateChanged();
	}

	if (pPlayer->GetSquadNumber() != m_SquadId)
	{
		return false;
	}

	CRnLPlayerResource* pRnLPR = GetRnLPlayerResource();
	pRnLPR->ChangeSquad(pPlayer->entindex(), RNL_SQUAD_INVALID);
	pRnLPR->ChangeKit(pPlayer->entindex(), RNL_KIT_INVALID);
	return true;
}
#endif

int CRnLSquad::GetKitCount(void) const
{
	int count = 0;
	for (int i = 0; i < m_KitInfo.Count(); i++)
	{
		if (m_KitInfo[i].iKitID >= 0)
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

const RnLSquadKitInfo& CRnLSquad::GetKitInfo(int indx) const
{
	return m_KitInfo[indx];
}

int CRnLSquad::GetMemberCount(void) const
{
	if (!m_hTeam)
	{
		return 0;
	}

	return GetRnLPlayerResource()->GetSquadMemberCount(m_hTeam->GetTeamNumber(), m_SquadId);
}

int CRnLSquad::GetMemberCount(int iKit) const
{
	if (!m_hTeam)
	{
		return 0;
	}

	return GetRnLPlayerResource()->GetKitMemberCount(m_hTeam->GetTeamNumber(), m_SquadId, iKit);
}

CRnLPlayer* CRnLSquad::GetMember(int indx) const
{
	if (!m_hTeam)
	{
		return nullptr;
	}

	int PlayerIndex = GetRnLPlayerResource()->GetSquadMemberIndex(m_hTeam->GetTeamNumber(), m_SquadId, indx);
	if (PlayerIndex < 0)
	{
		return nullptr;
	}
	return (CRnLPlayer*)UTIL_PlayerByIndex(PlayerIndex);
}

CRnLPlayer* CRnLSquad::GetNextMember(int iKit, CRnLPlayer* CurrentMember) const
{
	if (!m_hTeam)
	{
		return nullptr;
	}

	CRnLPlayerResource* pRnLPR = GetRnLPlayerResource();
	int MaxKitMembers = pRnLPR->GetKitMemberCount(m_hTeam->GetTeamNumber(), m_SquadId, iKit);
	for (int i = 0; i < MaxKitMembers; i++)
	{
		int PlayerIndex = pRnLPR->GetKitMemberIndex(m_hTeam->GetTeamNumber(), m_SquadId, iKit, i);
		if (PlayerIndex < 0)
		{
			continue;
		}

		CRnLPlayer* pRnLPlayer = (CRnLPlayer*)UTIL_PlayerByIndex(PlayerIndex);
		if (pRnLPlayer != nullptr)
		{
			if (CurrentMember == nullptr)
			{
				return pRnLPlayer;
			}
			else if (CurrentMember == pRnLPlayer)
			{
				CurrentMember = nullptr;
			}
		}
	}
	return nullptr;
}

CRnLPlayer* CRnLSquad::GetMember(int iKit, int idx) const
{
	if (!m_hTeam)
	{
		return nullptr;
	}

	CRnLPlayerResource* pRnLPR = GetRnLPlayerResource();
	int PlayerIndex = pRnLPR->GetKitMemberIndex(m_hTeam->GetTeamNumber(), m_SquadId, iKit, idx);
	if (PlayerIndex < 0)
	{
		return nullptr;
	}
	return (CRnLPlayer*)UTIL_PlayerByIndex(PlayerIndex);
}

int CRnLSquad::GetKitMaxCount(int iKit) const
{
	if (iKit < 0 || iKit >= m_KitInfo.Count())
		return 0;
	if (m_KitInfo[iKit].iKitID < 0)
		return 0;
	return m_KitInfo[iKit].iMaxCount;
}

bool CRnLSquad::IsSquadFull( void ) const
{
	for( int i = 0; i < m_KitInfo.Count(); i++ )
	{
		if(m_KitInfo[i].IsValid() &&
			GetMemberCount(m_KitInfo[i].iKitID) < m_KitInfo[i].iMaxCount)
			return false;
	}
	return true;
}

bool CRnLSquad::AreRequirementsMet( void ) const
{
	return true;
}

bool CRnLSquad::IsKitAvailable( int iKit ) const
{
	if( iKit < 0 || iKit >= m_KitInfo.Count() ||
		m_KitInfo[iKit].iKitID < 0)
		return false;

	return (GetMemberCount(iKit) < m_KitInfo[iKit].iMaxCount);
}

int CRnLSquad::GetKitDescription( int iKit ) const
{
	if( iKit < 0 || iKit >= m_KitInfo.Count() )
		return false;

	return (m_KitInfo[iKit].iKitID);
}

int CRnLSquad::GetTotalAvailableKits( void ) const
{
	return m_KitInfo.Count();
}

const char* CRnLSquad::GetSquadTitle(void) const
{
	return m_szSquadTitle.Get();
}

#ifndef CLIENT_DLL
const char* CRnLSquad::GetSquadName(void) const
{
	return m_szSquadReferenceName;
}

int CRnLSquad::GetNextAvailableSlot( void ) const
{
	for( int i = 0; i < m_KitInfo.Count(); i++ )
	{
		if( GetMemberCount(m_KitInfo[i].iKitID) < m_KitInfo[i].iMaxCount)
			return i;
	}

	return -1;
}

bool CRnLSquad::Load( CRnLGameTeam* OwnerTeam, KeyValues* pKey )
{
	if (pKey && OwnerTeam)
	{
		Q_strncpy(m_szSquadReferenceName, pKey->GetName(), MAX_TEAM_NAME_LENGTH);
		Q_strncpy(m_szSquadTitle.GetForModify(), pKey->GetString("name"), MAX_TEAM_NAME_LENGTH);

		KeyValues* pSlots = pKey->FindKey("slots");
		if (pSlots)
		{
			int CurrentKitIndex = 0;
			KeyValues* pClassKV = pSlots->GetFirstSubKey();
			while (pClassKV)
			{
				int iClassId = OwnerTeam->LookupKitDescription(pClassKV->GetName());
				if (iClassId > -1)
				{
					DevMsg("Squad Adding Kit Def: %s with max of %d\n", pClassKV->GetName(), pClassKV->GetInt());
					int KidIndex = m_KitInfo.AddToTail();
					RnLSquadKitInfo& Kit = m_KitInfo[KidIndex];

					Kit.iKitID = iClassId;
					Kit.iMaxCount = pClassKV->GetInt();

					CurrentKitIndex++;
				}
				else
				{
					Msg("Error Finding Data For Squad Class %s\n", pClassKV->GetName());
				}

				pClassKV = pClassKV->GetNextKey();
			}
		}
	}
	NetworkStateChanged(&m_KitInfo);
	return true; 
}

bool CRnLSquad::CanBeSquadLeader(CRnLGameTeam* OwnerTeam, CRnLPlayer* pEnt ) const
{
	if(OwnerTeam == NULL || pEnt == NULL )
		return false;

	//if( m_hParentTeam.Get()->GetKitDescription( m_aSlotInfo[i].iKitDesc ).iClass == RNL_CLASS_INFANTRY )
	if(OwnerTeam->GetKitDescription(pEnt->GetKitNumber()).bSquadLeader )
	{
		return true;
	}
	return false;
}
#endif