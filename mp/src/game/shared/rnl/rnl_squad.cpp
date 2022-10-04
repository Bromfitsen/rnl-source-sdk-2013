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

BEGIN_NETWORK_TABLE_NOBASE(CRnLSquadKitInfo, DT_RnLSquadKitInfo)
	PropInt(PROPINFO(iKitID)),
	PropInt(PROPINFO(iMaxCount)),
END_NETWORK_TABLE()

BEGIN_NETWORK_TABLE_NOBASE(CRnLSquadMember, DT_RnLSquadMember)
	PropInt(PROPINFO(iKitDesc)),
	PropEHandle(PROPINFO(m_Player))
END_NETWORK_TABLE()

// Datatable
IMPLEMENT_NETWORKCLASS_DT(CRnLSquad, DT_RnLSquad)
	PropString(PROPINFO(m_szSquadTitle)),
	PropEHandle(PROPINFO( m_hParentTeam ) ),
	PropEHandle(PROPINFO( m_hSquadLeader ) ),
	PropUtlVectorDataTable(m_KitInfo, RNL_SQUAD_SLOTS_MAX, DT_RnLSquadKitInfo),
	PropUtlVectorDataTable(m_Members, RNL_SQUAD_SLOTS_MAX, DT_RnLSquadMember),
END_NETWORK_TABLE()

BEGIN_DATADESC(CRnLSquad)
END_DATADESC()

LINK_ENTITY_TO_CLASS(rnl_squad, CRnLSquad);

//-----------------------------------------------------------------------------
// Purpose: Needed because this is an entity, but should never be used
//-----------------------------------------------------------------------------
CRnLSquad::CRnLSquad()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CRnLSquad::~CRnLSquad()
{
}

#ifndef CLIENT_DLL
bool CRnLSquad::AddPlayer( CRnLPlayer* pPlayer, int iKit )
{
	if(iKit < 0 || iKit >= m_KitInfo.Count() )
		return false;

	if( GetMemberCount(iKit) >= m_KitInfo[iKit].iMaxCount)
		return false;

	int indx = m_Members.AddToTail();
	CRnLSquadMember& mem = m_Members[indx];
	mem.iKitDesc = iKit;
	mem.m_Player = pPlayer;
	pPlayer->SetKitNumber(iKit);

	NetworkStateChanged();
	return true;
}

bool CRnLSquad::RemovePlayer( CRnLPlayer* pPlayer )
{
	if( !pPlayer )
		return false;

	bool found = false;
	if (m_hSquadLeader.Get() == pPlayer)
	{
		found = true;
		m_hSquadLeader = NULL;
	}
	
	for (int i = m_Members.Count() - 1; i >= 0; i--)
	{
		if (m_Members[i].m_Player.Get() == pPlayer)
		{
			m_Members.Remove(i);
			found = true;
		}
	}

	NetworkStateChanged();
	return found;
}
#endif

int CRnLSquad::GetKitCount(void)
{
	return m_KitInfo.Count();
}

CRnLSquadKitInfo& CRnLSquad::GetKitInfo(int indx)
{
	return m_KitInfo[indx];
}

int CRnLSquad::GetMemberCount(void)
{
	return m_Members.Count();
}

int CRnLSquad::GetMemberCount(int iKit)
{
	int count = 0;
	for (int i = 0; i < m_Members.Count(); i++)
	{
		if (m_Members[i].iKitDesc == iKit)
			count++;
	}
	return count;
}

CRnLPlayer* CRnLSquad::GetMember(int indx)
{
	if (indx < 0 || indx >= m_Members.Count())
		return NULL;

	return m_Members[indx].m_Player;
}

CRnLPlayer* CRnLSquad::GetNextMember(int iKitID, CRnLPlayer* CurrentMember)
{
	for (int i = 0; i < m_Members.Count(); i++)
	{
		if (m_Members[i].iKitDesc == iKitID)
		{
			if (CurrentMember == nullptr)
			{
				return m_Members[i].m_Player;
			}
			else if (CurrentMember == m_Members[i].m_Player)
			{
				CurrentMember = nullptr;
			}
		}
	}
	return nullptr;
}

CRnLPlayer* CRnLSquad::GetMember(int iKit, int idx)
{
	for (int i = 0; i < m_Members.Count(); i++)
	{
		if (m_Members[i].iKitDesc == iKit)
		{
			if (idx == 0)
			{
				return m_Members[i].m_Player;
			}
			idx--;
		}
	}
	return NULL;
}

int CRnLSquad::GetKitMaxCount(int iKit)
{
	if (iKit < 0 || iKit >= m_KitInfo.Count())
		return NULL;

	return m_KitInfo[iKit].iMaxCount;
}


int CRnLSquad::SquadSize( void )
{
	return m_Members.Count();
}

bool CRnLSquad::IsSquadFull( void )
{
	for( int i = 0; i < m_KitInfo.Count(); i++ )
	{
		if(GetMemberCount(m_KitInfo[i].iKitID) < m_KitInfo[i].iMaxCount)
			return false;
	}
	return true;
}

bool CRnLSquad::AreRequirementsMet( void )
{
	return true;
}

bool CRnLSquad::IsKitAvailable( int iKit )
{
	if( iKit < 0 || iKit >= m_KitInfo.Count() )
		return false;

	return (GetMemberCount(iKit) < m_KitInfo[iKit].iMaxCount);
}

int CRnLSquad::GetKitDescription( int iKit )
{
	if( iKit < 0 || iKit >= m_KitInfo.Count() )
		return false;

	return (m_KitInfo[iKit].iKitID);
}

int CRnLSquad::GetTotalAvailableKits( void )
{
	return m_KitInfo.Count();
}

const char* CRnLSquad::GetSquadTitle(void)
{
	return m_szSquadTitle.Get();
}

#ifndef CLIENT_DLL
const char* CRnLSquad::GetSquadName(void)
{
	return m_szSquadReferenceName;
}

int CRnLSquad::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

extern void respawn(CBaseEntity* pEdict, bool fCopyCorpse);
void CRnLSquad::Update( void )
{
	CRnLGameTeam* pTeam = (CRnLGameTeam*)(m_hParentTeam.Get());
	
	if( !pTeam )
		return;

	// Clear the current squad leader.
	if (GetSquadLeader() == nullptr)
	{
		for (int j = 0; j < m_Members.Count(); j++)
		{
			CRnLPlayer*  pPlayer = m_Members[j].m_Player.Get();

			if (!pPlayer)
				continue;

			if (CanBeSquadLeader(pPlayer))
			{
				SetSquadLeader(pPlayer);
				break;
			}
		}
	}

	if (RnLGameRules() && RnLGameRules()->GetGameManager() &&
		(RnLGameRules()->GetGameManager()->GetSpawnTimer(GetTeamNumber()) <= 0.0f) &&
		(RnLGameRules()->GetGameManager()->GetTicketsRemaining(GetTeamNumber()) > 0))
	{
		for (int j = 0; j < m_Members.Count(); j++)
		{
			if (!m_Members[j].m_Player->IsAlive() && m_Members[j].m_Player->IsReadyToSpawn())
			{
				respawn(m_Members[j].m_Player, false);
			}
		}
	}

	// TODO: Uncomment and delete above when Squad Leader voting is finished.
	/*if( m_hSquadLeader == NULL )
	{
		if( m_flSquadLeaderSelectionTimer == -1.0f )
		{
			m_flSquadLeaderSelectionTimer = gpGlobals->curtime + 15.0f;
		}
		else if( m_flSquadLeaderSelectionTimer <= gpGlobals->curtime )
		{
			CUtlMap<int,int> playerVotes(DefLessFunc(int));
			for( int i = 0; i < m_aSlotInfo.Count(); i++ )
			{
				for( int j = 0; j < m_aSlotInfo[i].pMembers.Count(); j++ )
				{
					int vote = m_aSlotInfo[i].pMembers[j]->GetSquadLeaderVote();
					if( playerVotes.Find( vote ) != playerVotes.InvalidIndex() )
					{
						playerVotes[playerVotes.Find(vote)] += 1;
					}
					else
					{
						playerVotes.Insert( vote );
						playerVotes[playerVotes.Find(vote)] = 1;
					}
				}
			}

			int highestVote = 0;
			int voteCount = 0;
			for( int i = playerVotes.FirstInorder(); i != playerVotes.InvalidIndex(); i = playerVotes.NextInorder( i ) )
			{
				int key = playerVotes.Key( i );
				if( key > 0 && key < gpGlobals->maxClients )
				{
					int votes = playerVotes[playerVotes.Find(key)];
					if( votes > voteCount )
					{
						highestVote = key;
						voteCount = votes;
					}
				}
			}

			if( highestVote > 0 && highestVote < gpGlobals->maxClients )
			{
				CHandle<CRnLPlayer> chosenPlayer = highestVote;
				if( CanBeSquadLeader( chosenPlayer.Get() ) )
				{
					SetSquadLeader( chosenPlayer.Get() );
				}
			}
			
			m_flSquadLeaderSelectionTimer = -1.0f;
		}
	}*/
}

int CRnLSquad::GetNextAvailableSlot( void )
{
	for( int i = 0; i < m_KitInfo.Count(); i++ )
	{
		if( GetMemberCount(m_KitInfo[i].iKitID) < m_KitInfo[i].iMaxCount)
			return i;
	}

	return -1;
}

void CRnLSquad::SetParentTeam(CRnLGameTeam* pEnt)
{
	m_hParentTeam = pEnt; 
	if (pEnt)
	{
		ChangeTeam(pEnt->GetTeamNumber());
	}
	else
	{
		ChangeTeam(TEAM_UNASSIGNED);
	}
}

bool CRnLSquad::Load( KeyValues* pKey )
{
	// Only detect changes every half-second.
	NetworkProp()->SetUpdateInterval( 0.75f );

	if (pKey && m_hParentTeam)
	{
		Q_strncpy(m_szSquadReferenceName, pKey->GetName(), MAX_TEAM_NAME_LENGTH);
		Q_strncpy(m_szSquadTitle.GetForModify(), pKey->GetString("name"), MAX_TEAM_NAME_LENGTH);

		KeyValues* pSlots = pKey->FindKey("slots");
		if (pSlots)
		{
			KeyValues* pClassKV = pSlots->GetFirstSubKey();
			while (pClassKV)
			{
				int iClassId = m_hParentTeam->LookupKitDescription(pClassKV->GetName());

				if (iClassId > -1)
				{
					DevMsg("Squad Adding Kit Def: %s with max of %d\n", pClassKV->GetName(), pClassKV->GetInt());
					int index = m_KitInfo.AddToTail();

					m_KitInfo[index].iKitID = iClassId;
					m_KitInfo[index].iMaxCount = pClassKV->GetInt();
				}
				else
				{
					Msg("Error Finding Data For Squad Class %s\n", pClassKV->GetName());
				}

				pClassKV = pClassKV->GetNextKey();
			}
		}
	}
	return true; 
}

bool CRnLSquad::CanBeSquadLeader( CRnLPlayer* pEnt )
{
	if( pEnt == NULL )
		return false;

	//if( m_hParentTeam.Get()->GetKitDescription( m_aSlotInfo[i].iKitDesc ).iClass == RNL_CLASS_INFANTRY )
	if( m_hParentTeam.Get()->GetKitDescription(pEnt->GetKitNumber()).bSquadLeader )
	{
		return true;
	}
	return false;
}
#endif