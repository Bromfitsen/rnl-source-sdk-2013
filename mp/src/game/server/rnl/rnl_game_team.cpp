//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "rnl_game_team.h"
#include "entitylist.h"
#include "weapon_rnl_base.h"
#include "rnl_ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CON_COMMAND_F( rnl_spew_server_team_data, "Don't you ever dare use this or your STEAM ID WILL Be banned.", FCVAR_CHEAT )
{
	CRnLPlayer *pPlayer = ToRnLPlayer( UTIL_GetCommandClient() ); 
	
	for( int i = TEAM_SPECTATOR; i < TEAMS_COUNT; i++ )
	{
		CRnLTeam* pTeam = GetGlobalRnLTeam( i );
		if( !pTeam )
			continue;

		if (pPlayer)
			ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "\nData For Team: %s\n",  pTeam->GetName() ) );
		else
			Msg( "\nData For Team: %s\n",  pTeam->GetName() );


		if (pPlayer)
			ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "	Players:\n" ) );
		else
			Msg( "	Players:\n" );

		for( int j = 0; j < pTeam->GetNumPlayers(); j++ )
		{
			CBasePlayer* pTeamPlayer = pTeam->GetPlayer(j);
			if( !pTeamPlayer )
				continue;

			if (pPlayer)
				ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "		%s\n",  pTeamPlayer->GetPlayerName() ) );
			else
				Msg( "		%s\n",  pTeamPlayer->GetPlayerName() );

		}

		CRnLGameTeam* pGameTeam = dynamic_cast<CRnLGameTeam*>(pTeam);
		if( !pGameTeam )
			continue;

		if (pPlayer)
			ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "	Kits:\n" ) );
		else
			Msg( "	Kits:\n" );

		for( int j = 0; j < pGameTeam->GetKitDescriptionCount(); j++ )
		{
			RnLKitDescription& desc = pGameTeam->GetKitDescription( j );

			if (pPlayer)
				ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "			%s\n", desc.name ) );
			else
				Msg( "			%s\n", desc.name );
			
		}

		if (pPlayer)
			ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "	Squads:\n" ) );
		else
			Msg( "	Squads:\n" );

		for( int j = 0; j < pGameTeam->GetNumberOfSquads(); j++ )
		{
			IRnLSquad* pSquad = pGameTeam->GetSquad(j);
			if( !pSquad )
				continue;

			if (pPlayer)
				ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "		%s\n", pSquad->GetSquadName() ) );
			else
				Msg( "		%s\n", pSquad->GetSquadName() );

			if( pSquad->GetSquadLeader() != NULL )
			{
				if (pPlayer)
					ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "			Squad Leader: %s\n", pSquad->GetSquadLeader()->GetPlayerName() ) );
				else
					Msg( "			Squad Leader: %s\n", pSquad->GetSquadLeader()->GetPlayerName() );
			}
			else
			{
				if (pPlayer)
					ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "			No Squad Leader\n" ) );
				else
					Msg( "			No Squad Leader\n" );
			}

			if (pPlayer)
				ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "			Loadouts:\n" ) );
			else
				Msg( "			Loadouts:\n" );

			for( int k = 0; k < pSquad->GetSlotCount(); k++ )
			{
				CSquadSlotInfo& info = pSquad->GetSlotInfo(k);
				RnLKitDescription& desc = pGameTeam->GetKitDescription( info.iKitDesc );

				if (pPlayer)
					ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "				%s: (max %d)\n", desc.name, info.iMaxCount  ) );
				else
					Msg( "				%s: (max %d)\n", desc.name, info.iMaxCount );

				for( int l = 0; l < info.pMembers.Count(); l++ )
				{
					if (pPlayer)
						ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "					%s\n", info.pMembers[l]->GetPlayerName()  ) );
					else
						Msg( "					%s\n", info.pMembers[l]->GetPlayerName() );
				}
			}

		}
	}
}

//-------------------------
//
//Proxies
//
//------------------------
void SendProxy_SquadList( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CRnLGameTeam *pTeam = (CRnLGameTeam*)pData;
	pOut->m_Int = pTeam->m_aSquads[iElement]->GetEntIndex();
}


int SendProxyArrayLength_SquadArray( const void *pStruct, int objectID )
{
	CRnLGameTeam* pTeam = (CRnLGameTeam*)pStruct;
	return pTeam->m_aSquads.Count();
}

// Datatable
IMPLEMENT_SERVERCLASS_ST(CRnLGameTeam, DT_RnLGameTeam)
	SendPropArray2( 
		SendProxyArrayLength_SquadArray,
		SendPropInt("squad_element", 0, sizeof( int ), -1, 0, SendProxy_SquadList), 
		MAX_PLAYERS, 
		0, 
		"squad_array"
		),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( rnl_game_team, CRnLGameTeam );


CRnLGameTeam::CRnLGameTeam()
{
	m_pBaseSpawnArea = NULL;
}

CRnLGameTeam::~CRnLGameTeam()
{
	m_aSquads.Purge();
}

IRnLSquad* CRnLGameTeam::GetSquad( int idx )
{
	if( idx < 0 || idx >= m_aSquads.Count() )
		return NULL;

	return m_aSquads[idx];
}

int CRnLGameTeam::GetNextAvailableSquad( void )
{
	for( int i = 0; i < m_aSquads.Count(); i++ )
	{
		if( !m_aSquads[i]->IsSquadFull() )
			return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Needed because this is an entity, but should never be used
//-----------------------------------------------------------------------------
void CRnLGameTeam::Init( const char *pName, int iNumber, KeyValues* pVal )
{
	BaseClass::Init( pName, iNumber, pVal );

	if( pVal )
	{
		LoadClassDescriptions( pVal->FindKey( "kits" ) );
	}
}

void CRnLGameTeam::Update( void )
{
	for( int i = 0; i < m_aSquads.Count(); i++ )
	{
		m_aSquads[i]->Update();
	}
}

void CRnLGameTeam::SetBaseSpawn( CRnLSpawnArea* pArea )
{
	m_pBaseSpawnArea = pArea;
}

void CRnLGameTeam::OnPlayerSpawn( CRnLPlayer* pPlayer )
{
	int iSquad = pPlayer->GetSquadNumber();
	if( iSquad < 0 || iSquad > m_aSquads.Count() )
		return;

	int iDesc = m_aSquads[iSquad]->GetKitDescription(pPlayer->GetKitNumber());

	if( iDesc < 0 || iDesc > m_aClassDescriptions.Count() )
		return;

	pPlayer->SetModel( m_aClassDescriptions[iDesc].model.file );

	for( int i = 0; i < m_aClassDescriptions[iDesc].model.vecBodyGroups.Count(); i++ )
	{
		pPlayer->SetBodygroup( pPlayer->FindBodygroupByName( m_aClassDescriptions[iDesc].model.vecBodyGroups[i].groupName.Get() ), m_aClassDescriptions[iDesc].model.vecBodyGroups[i].iVal );
	}

	if( m_pBaseSpawnArea )
	{
		m_pBaseSpawnArea->OnPlayerSpawn( pPlayer );
	}

	CBaseCombatWeapon* pWeapon = NULL;
	for( int i = 0; i < m_aClassDescriptions[iDesc].weapons.Count(); i++ )
	{
		pWeapon = (CBaseCombatWeapon*)(pPlayer->GiveNamedItem( m_aClassDescriptions[iDesc].weapons[i].Get() ));

		if( pWeapon )
		{
			pPlayer->GiveAmmo( GetRnLAmmoDef()->MaxCarry(pWeapon->GetPrimaryAmmoType()), pWeapon->GetPrimaryAmmoType() );
		}
	}

	if( pPlayer->GetTeamNumber() == TEAM_ALLIES )
		pPlayer->GiveNamedItem( "weapon_alliedfists" );
	else
		pPlayer->GiveNamedItem( "weapon_axisfists" );
}