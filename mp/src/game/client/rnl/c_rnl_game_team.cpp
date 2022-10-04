//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side C_RnLTeam class
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "engine/IEngineSound.h"
#include "dt_utlvector_recv.h"
#include "hud.h"
#include "recvproxy.h"
#include "c_rnl_game_team.h"
#include "rnl_squad.h"
#include "rnl_shareddefs.h"

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
			CRnLLoadoutKitInfo& desc = pGameTeam->GetKitDescription( j );
			Msg( "			%s\n", desc.name.Get() );
			
		}

		Msg( "	Squads:\n" );

		for( int j = 0; j < pGameTeam->GetNumberOfSquads(); j++ )
		{
			CRnLSquad* pSquad = pGameTeam->GetSquad(j);
			if( !pSquad )
				continue;

			Msg( "		%s\n", pSquad->GetSquadTitle() );

			if( pSquad->GetSquadLeader() != NULL )
			{
				Msg( "			Squad Leader: %s\n", pSquad->GetSquadLeader()->GetPlayerName() );
			}
			else
			{
				Msg( "			No Squad Leader\n" );
			}

			Msg( "			Loadouts:\n" );

			for( int k = 0; k < pSquad->GetKitCount(); k++ )
			{
				CRnLSquadKitInfo& info = pSquad->GetKitInfo(k);
				CRnLLoadoutKitInfo& desc = pGameTeam->GetKitDescription( info.iKitID );

				Msg( "				%s: (max %d)\n", desc.name.Get(), info.iMaxCount );

				CRnLPlayer* pMember = pSquad->GetNextMember(info.iKitID, nullptr);
				while (pMember != nullptr)
				{
					Msg( "					%s\n", pMember->GetPlayerName() );
					pMember = pSquad->GetNextMember(info.iKitID, nullptr);
				}
			}

		}
	}
}

IMPLEMENT_NETWORKCLASS_ALIASED(RnLGameTeam, DT_RnLGameTeam)

BEGIN_NETWORK_TABLE(C_RnLGameTeam, DT_RnLGameTeam)
	PropUtlVectorDataTable(m_aClassDescriptions, RNL_SQUAD_SLOTS_MAX, DT_RnLLoadoutKitInfo),
	RecvPropUtlVector(RECVINFO_UTLVECTOR(m_aSquads), RNL_SQUAD_SLOTS_MAX, RecvPropEHandle(NULL, 0)),
END_NETWORK_TABLE()

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

CRnLSquad* C_RnLGameTeam::GetSquad( int idx )
{
	if( idx < 0 || idx >= m_aSquads.Count() )
		return NULL;

	return m_aSquads[idx];
}

