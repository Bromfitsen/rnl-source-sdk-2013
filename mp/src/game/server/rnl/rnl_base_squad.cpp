//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "rnl_base_squad.h"
#include "dt_utlvector_send.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_SEND_TABLE_NOBASE( CSquadSlotInfo, DT_RnLSquadSlotData )
	SendPropInt( SENDINFO( iKitDesc ) ),
	SendPropInt( SENDINFO( iMaxCount ) ),
	SendPropUtlVector( 
		SENDINFO_UTLVECTOR( pMembers ), 
		RNL_SQUAD_SLOTS_MAX,
		SendPropEHandle(NULL, 0, 0)),
END_SEND_TABLE()

// Datatable
IMPLEMENT_SERVERCLASS_ST(CRnLBaseSquad, DT_RnLBaseSquad)
	SendPropEHandle( SENDINFO( m_hParentTeam ) ),
	SendPropEHandle( SENDINFO( m_hSquadLeader ) ),
	SendPropUtlVectorDataTable( m_aSlotInfo, RNL_SQUAD_SLOTS_MAX, DT_RnLSquadSlotData ),
END_SEND_TABLE()

int CRnLBaseSquad::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

void CRnLBaseSquad::Update( void )
{
	CRnLGameTeam* pTeam = (CRnLGameTeam*)(m_hParentTeam.Get());
	
	if( !pTeam )
		return;

	// Clear the current squad leader.
	SetSquadLeader( NULL );

	CRnLPlayer *pPlayer = NULL;

	for( int i = 0; i < m_aSlotInfo.Count(); i++ )
	{
		for( int j = 0; j < m_aSlotInfo[i].pMembers.Count(); j++ )
		{
			pPlayer = m_aSlotInfo[i].pMembers[j].Get();

			if( !pPlayer )
				continue;

			if( CanBeSquadLeader( pPlayer ) )
				SetSquadLeader( pPlayer );
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

int CRnLBaseSquad::GetNextAvailableSlot( void )
{
	for( int i = 0; i < m_aSlotInfo.Count(); i++ )
	{
		if( m_aSlotInfo[i].pMembers.Count() < m_aSlotInfo[i].iMaxCount )
			return i;
	}

	return -1;
}

bool CRnLBaseSquad::Load( KeyValues* pKey )
{ 
	// Only detect changes every half-second.
	NetworkProp()->SetUpdateInterval( 0.75f );
	return true; 
}

bool CRnLBaseSquad::CanBeSquadLeader( CRnLPlayer* pEnt )
{
	if( pEnt == NULL )
		return false;

	for( int i = 0; i < m_aSlotInfo.Count(); i++ )
	{
		
		//if( m_hParentTeam.Get()->GetKitDescription( m_aSlotInfo[i].iKitDesc ).iClass == RNL_CLASS_INFANTRY )
		if( m_hParentTeam.Get()->GetKitDescription( m_aSlotInfo[i].iKitDesc ).bSquadLeader )
		{
			if( m_aSlotInfo[i].pMembers.Find( pEnt ) >= 0 )
				return true;
		}
	}
	return false;
}