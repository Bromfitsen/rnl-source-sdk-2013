//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side C_RnLTeam class
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "engine/IEngineSound.h"
#include "hud.h"
#include "recvproxy.h"
#include "c_rnl_game_team.h"
#include "c_rnl_base_squad.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CON_COMMAND_F( rnl_spew_client_team_data, "Don't you ever dare use this or your STEAM ID WILL Be banned.", FCVAR_CHEAT )
{
	for( int i = TEAM_SPECTATOR; i < TEAMS_COUNT; i++ )
	{
		C_RnLTeam* pTeam = GetGlobalRnLTeam( i );
		if( !pTeam )
			continue;

		Msg( "\nData For Team: %s\n",  pTeam->Get_Name() );
		Msg( "	Players:\n" );

		for( int j = 0; j < pTeam->GetNumPlayers(); j++ )
		{
			CBasePlayer* pTeamPlayer = pTeam->GetPlayer(j);
			if( !pTeamPlayer )
				continue;

			Msg( "		%s\n",  pTeamPlayer->GetPlayerName() );
		}

		C_RnLGameTeam* pGameTeam = dynamic_cast<C_RnLGameTeam*>(pTeam);
		if( !pGameTeam )
			continue;

		Msg( "	Kits:\n" );

		for( int j = 0; j < pGameTeam->GetKitDescriptionCount(); j++ )
		{
			RnLKitDescription& desc = pGameTeam->GetKitDescription( j );
			Msg( "			%s\n", desc.name );
			
		}

		Msg( "	Squads:\n" );

		for( int j = 0; j < pGameTeam->GetNumberOfSquads(); j++ )
		{
			IRnLSquad* pSquad = pGameTeam->GetSquad(j);
			if( !pSquad )
				continue;

			Msg( "		%s\n", pSquad->GetSquadName() );

			if( pSquad->GetSquadLeader() != NULL )
			{
				Msg( "			Squad Leader: %s\n", pSquad->GetSquadLeader()->GetPlayerName() );
			}
			else
			{
				Msg( "			No Squad Leader\n" );
			}

			Msg( "			Loadouts:\n" );

			for( int k = 0; k < pSquad->GetSlotCount(); k++ )
			{
				CSquadSlotInfo& info = pSquad->GetSlotInfo(k);
				RnLKitDescription& desc = pGameTeam->GetKitDescription( info.iKitDesc );

				Msg( "				%s: (max %d)\n", desc.name, info.iMaxCount );

				for( int l = 0; l < info.pMembers.Count(); l++ )
				{
					Msg( "					%s\n", info.pMembers[l]->GetPlayerName() );
				}
			}

		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: RecvProxy that converts the Team's player UtlVector to entindexes
//-----------------------------------------------------------------------------
void RecvProxy_SquadList(  const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_RnLGameTeam *pTeam = (C_RnLGameTeam*)pOut;
	pTeam->m_aSquads[pData->m_iElement] = pData->m_Value.m_Int;
	pTeam->m_bSquadChange = true;
}


void RecvProxyArrayLength_SquadArray( void *pStruct, int objectID, int currentArrayLength )
{
	C_RnLGameTeam *pTeam = (C_RnLGameTeam*)pStruct;
	pTeam->m_aSquads.SetSize( currentArrayLength );
}

IMPLEMENT_CLIENTCLASS_DT(C_RnLGameTeam, DT_RnLGameTeam, CRnLGameTeam)
	RecvPropArray2( 
		RecvProxyArrayLength_SquadArray,
		RecvPropInt("squad_element", 0, SIZEOF_IGNORE, 0, RecvProxy_SquadList), 
		MAX_PLAYERS, 
		0, 
		"squad_array"
		),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_RnLGameTeam )
END_PREDICTION_DATA();

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_RnLGameTeam::C_RnLGameTeam()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_RnLGameTeam::~C_RnLGameTeam()
{
}

C_RnLBaseSquad* C_RnLGameTeam::GetSquad( int idx )
{
	if( idx < 0 || idx >= m_aSquads.Count() )
		return NULL;

	return (C_RnLBaseSquad*)(cl_entitylist->GetBaseEntity( m_aSquads[idx]));
}

