//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "dt_utlvector_send.h"
#include "rnl_game_team.h"
#include "entitylist.h"
#include "weapon_rnl_base.h"
#include "rnl_ammodef.h"
#include "rnl_squad.h"

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
			CRnLLoadoutKitInfo& desc = pGameTeam->GetKitDescription( j );

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
			CRnLSquad* pSquad = pGameTeam->GetSquad(j);
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

			for( int k = 0; k < pSquad->GetKitCount(); k++ )
			{
				CRnLSquadKitInfo& info = pSquad->GetKitInfo(k);
				CRnLLoadoutKitInfo& desc = pGameTeam->GetKitDescription( info.iKitID );

				if (pPlayer)
					ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "				%s: (max %d)\n", desc.name, info.iMaxCount  ) );
				else
					Msg( "				%s: (max %d)\n", desc.name, info.iMaxCount );

				CRnLPlayer* pMember = pSquad->GetNextMember(info.iKitID, nullptr);
				while (pMember != nullptr)
				{
					if (pPlayer)
						ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "					%s\n", pMember->GetPlayerName()  ) );
					else
						Msg( "					%s\n", pMember->GetPlayerName() );
				}
			}

		}
	}
}

// Datatable
IMPLEMENT_SERVERCLASS_ST(CRnLGameTeam, DT_RnLGameTeam)
	PropUtlVectorDataTable(m_aClassDescriptions, RNL_SQUAD_SLOTS_MAX, DT_RnLLoadoutKitInfo),
	SendPropUtlVector(SENDINFO_UTLVECTOR(m_aSquads), RNL_SQUAD_SLOTS_MAX, SendPropEHandle(NULL, 0)),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(rnl_game_team, CRnLGameTeam );


CRnLGameTeam::CRnLGameTeam()
{
	m_pBaseSpawnArea = NULL;
}

CRnLGameTeam::~CRnLGameTeam()
{
	m_aSquads.Purge();
}

CRnLSquad* CRnLGameTeam::GetSquad( int idx )
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


	bool precacheAllowed = IsPrecacheAllowed();
	SetAllowPrecache(true);
	if( pVal )
	{
		LoadClassDescriptions(pVal->FindKey( "kits" ));
		LoadSquadDescriptions(pVal->FindKey("squads"));
	}
	SetAllowPrecache(precacheAllowed);
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
		char weaponFullName[128];
		Q_snprintf(weaponFullName, 127, "weapon_%s", WeaponIDToAlias(m_aClassDescriptions[iDesc].weapons[i]));
		pWeapon = (CBaseCombatWeapon*)(pPlayer->GiveNamedItem(weaponFullName));

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

bool CRnLGameTeam::LoadClassDescriptions(KeyValues* pKey)
{
	if (!pKey)
		return false;

	int index = -1;
	KeyValues* pModelData = NULL;
	KeyValues* pWeaponData = NULL;
	KeyValues* pWeaponSubData = NULL;
	KeyValues* pBodyGroups = NULL;
	KeyValues* pGroup = NULL;

	m_aClassDescriptions.RemoveAll();

	KeyValues* pClassData = pKey->GetFirstSubKey();
	while (pClassData)
	{
		index = m_aClassDescriptions.AddToTail();

		Q_strncpy(m_aClassDescriptions[index].name.GetForModify(), pClassData->GetName(), KIT_DESC_TITLE_LEN);
		Q_strncpy(m_aClassDescriptions[index].title.GetForModify(), pClassData->GetString("title"), KIT_DESC_TITLE_LEN);

		// TODO: Remove when squad leader voting is finished.
		if (FStrEq(pClassData->GetString("squadleader", "false"), "true"))
			m_aClassDescriptions[index].bSquadLeader = true;
		else
			m_aClassDescriptions[index].bSquadLeader = false;

		pWeaponData = pClassData->FindKey("weapons");
		if (pWeaponData != NULL)
		{
			pWeaponSubData = pWeaponData->GetFirstSubKey();
			while (pWeaponSubData)
			{
				int weaponID = AliasToWeaponID(pWeaponSubData->GetString());
				if (weaponID != WEAPON_NONE &&
					weaponID != WEAPON_MAX)
				{
					m_aClassDescriptions[index].weapons.AddToTail(weaponID);
				}
				pWeaponSubData = pWeaponSubData->GetNextKey();
			}
		}

		m_aClassDescriptions[index].iClass = AliasToClassID(pClassData->GetString("type"));
		if (m_aClassDescriptions[index].iClass == RNL_CLASS_INVALID)
		{
			m_aClassDescriptions.Remove(index);
		}
		else
		{
			pModelData = pClassData->FindKey("model");
			if (pModelData != NULL)
			{
				Q_strncpy(m_aClassDescriptions[index].model.file.GetForModify(), pModelData->GetString("file"), KIT_DESC_MODEL_LEN);
				if (PrecacheModel(m_aClassDescriptions[index].model.file) < 0)
				{
					Q_strncpy(m_aClassDescriptions[index].model.file.GetForModify(), RNL_DEFAULT_PLAYER_MODEL, KIT_DESC_MODEL_LEN);
				}

				m_aClassDescriptions[index].model.iSkin = pModelData->GetInt("skin");

				pBodyGroups = pModelData->FindKey("bodygroups");
				if (pBodyGroups)
				{
					pGroup = pBodyGroups->GetFirstSubKey();
					int bodyGroupIndex = -1;
					while (pGroup)
					{
						bodyGroupIndex = m_aClassDescriptions[index].model.vecBodyGroups.AddToTail();
						Q_strncpy(
							m_aClassDescriptions[index].model.vecBodyGroups[bodyGroupIndex].groupName.GetForModify(),
							pGroup->GetName(),
							KIT_DESC_MODEL_LEN
						);
						m_aClassDescriptions[index].model.vecBodyGroups[bodyGroupIndex].iVal = pGroup->GetInt();
						pGroup = pGroup->GetNextKey();
					}

				}
			}
			else
			{
				m_aClassDescriptions.Remove(index);
			}
		}

		pClassData = pClassData->GetNextKey();
	}
	return true;
}

bool CRnLGameTeam::LoadSquadDescriptions(KeyValues* pKey)
{
	if (!pKey)
		return false;

	KeyValues* pSquadInfo = pKey->GetFirstSubKey();
	while (pSquadInfo)
	{
		CRnLSquad* pSquad = (CRnLSquad*)(CreateEntityByName("rnl_squad"));
		pSquad->SetParentTeam(this);
		pSquad->Load(pSquadInfo);
		m_aSquads.AddToTail(pSquad);

		pSquadInfo = pSquadInfo->GetNextKey();
	}
	return true;
}