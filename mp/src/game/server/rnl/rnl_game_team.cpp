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
#include "rnl_gamerules.h"
#include "rnl_game_manager.h"

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
			const RnLLoadoutKitInfo& desc = pGameTeam->GetKitDescription( j );

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
			const CRnLSquad* pSquad = pGameTeam->GetSquad(j);
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
				const RnLSquadKitInfo& info = pSquad->GetKitInfo(k);
				const RnLLoadoutKitInfo& desc = pGameTeam->GetKitDescription( info.iKitID );

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

IMPLEMENT_NETWORKCLASS_ALIASED(RnLGameTeam, DT_RnLGameTeam);

// Datatable
BEGIN_NETWORK_TABLE(CRnLGameTeam, DT_RnLGameTeam)
	PropEHandle(PROPINFO(m_hBaseSpawnArea)),
	PropUtlVectorDataTable(m_aClassDescriptions, RNL_KITS_MAX, DT_RnLLoadoutKitInfo),
	PropUtlVector(PROPINFO_UTLVECTOR(m_aSquads), RNL_SQUADS_MAX, PropEHandle("m_aSquads::entry", 0, 0)),
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(rnl_game_team, CRnLGameTeam );


CRnLGameTeam::CRnLGameTeam()
{
	m_hBaseSpawnArea = NULL;
}

CRnLGameTeam::~CRnLGameTeam()
{
}

int CRnLGameTeam::GetNextAvailableSquad( void ) const
{
	for( int i = 0; i < m_aSquads.Count(); i++)
	{
		if(m_aSquads[i].IsValid() &&
			!m_aSquads[i]->IsSquadFull())
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

extern void respawn(CBaseEntity* pEdict, bool fCopyCorpse);
void CRnLGameTeam::Update( void )
{
	for (int i = 0; i < m_aSquads.Count(); i++)
	{
		if (m_aSquads[i].IsValid())
		{
			// Clear the current squad leader.
			if (m_aSquads[i]->GetSquadLeader() == nullptr)
			{
				for (int j = 0; j < m_aPlayers.Count(); j++)
				{
					CRnLPlayer* pPlayer = ToRnLPlayer(m_aPlayers[j]);
					if (!pPlayer || pPlayer->GetSquadNumber() != m_aSquads[i]->m_SquadId)
						continue;

					if (m_aSquads[i]->CanBeSquadLeader(this, pPlayer))
					{
						m_aSquads[i]->SetSquadLeader(pPlayer);
						NetworkStateChanged(&m_aSquads);
						break;
					}
				}
			}
		}
	}

	// Respawn any players that are waiting.
	if (RnLGameRules() && RnLGameRules()->GetGameManager() &&
		(RnLGameRules()->GetGameManager()->GetSpawnTimer(GetTeamNumber()) <= 0.0f) &&
		(RnLGameRules()->GetGameManager()->GetTicketsRemaining(GetTeamNumber()) > 0))
	{
		for (int i = 0; i < m_aSquads.Count(); i++)
		{
			if (m_aSquads[i].IsValid())
			{
				CRnLSquad* pSquad = m_aSquads[i];
				for (int j = 0; j < pSquad->GetMemberCount(); j++)
				{
					CRnLPlayer* pPlayer = pSquad->GetMember(j);
					if (!pPlayer)
						continue;

					if (!pPlayer->IsAlive() && pPlayer->IsReadyToSpawn())
					{
						respawn(pPlayer, false);
					}
				}
			}
		}

	}
}

void CRnLGameTeam::AddPlayer(CBasePlayer* pPlayer)
{
	BaseClass::AddPlayer(pPlayer);
}

void CRnLGameTeam::RemovePlayer(CBasePlayer* pPlayer)
{
	CRnLPlayer* pRnLPlayer = ToRnLPlayer(pPlayer);
	if (pRnLPlayer)
	{
		int iSquad = pRnLPlayer->GetSquadNumber();
		if (m_aSquads.IsValidIndex(iSquad))
		{
			m_aSquads[iSquad]->RemovePlayer(pRnLPlayer);
		}
	}
	BaseClass::RemovePlayer(pPlayer);
}

bool CRnLGameTeam::JoinSquad(CRnLPlayer* pPlayer, int iSquad, int iKit)
{
	if (!IsKitDescriptionValid(iKit))
	{
		return false;
	}

	if (!m_aSquads.IsValidIndex(iSquad))
	{
		return false;
	}

	CRnLSquad* Squad = m_aSquads[iSquad];
	if (!Squad->IsValid())
	{
		return false;
	}

	if (pPlayer->GetSquadNumber() == iSquad &&
		pPlayer->GetKitNumber() == iKit)
	{
		return false;
	}

	// Remove the player from the root team list.
	RemovePlayer(pPlayer);

	if (!Squad->AddPlayer(pPlayer, iKit))
	{
		// Failed to add, put them back onto the root team.
		AddPlayer(pPlayer);
		return false;
	}
	return true;
}

void CRnLGameTeam::SetBaseSpawn( CRnLSpawnArea* pArea )
{
	m_hBaseSpawnArea = pArea;
}

void CRnLGameTeam::OnPlayerSpawn( CRnLPlayer* pPlayer )
{
	int iSquad = pPlayer->GetSquadNumber();
	if( iSquad < 0 || iSquad > m_aSquads.Count() ||
		m_aSquads[iSquad].IsValid() == false)
		return;

	int iDesc = m_aSquads[iSquad]->GetKitDescription(pPlayer->GetKitNumber());

	if( iDesc < 0 || iDesc > m_aClassDescriptions.Count() )
		return;

	pPlayer->SetModel( m_aClassDescriptions[iDesc].model.file );

	for( int i = 0; i < m_aClassDescriptions[iDesc].model.vecBodyGroups.Count(); i++ )
	{
		pPlayer->SetBodygroup( pPlayer->FindBodygroupByName( m_aClassDescriptions[iDesc].model.vecBodyGroups[i].groupName.Get() ), m_aClassDescriptions[iDesc].model.vecBodyGroups[i].iVal );
	}

	if(m_hBaseSpawnArea)
	{
		m_hBaseSpawnArea->OnPlayerSpawn( pPlayer );
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

	if( GetTeamNumber() == TEAM_ALLIES )
		pPlayer->GiveNamedItem( "weapon_alliedfists" );
	else if (GetTeamNumber() == TEAM_AXIS)
		pPlayer->GiveNamedItem( "weapon_axisfists" );
}

bool CRnLGameTeam::LoadClassDescriptions(KeyValues* pKey)
{
	if (!pKey)
		return false;

	KeyValues* pModelData = NULL;
	KeyValues* pWeaponData = NULL;
	KeyValues* pWeaponSubData = NULL;
	KeyValues* pBodyGroups = NULL;
	KeyValues* pGroup = NULL;

	KeyValues* pClassData = pKey->GetFirstSubKey();
	while (pClassData)
	{
		int kitIndex = m_aClassDescriptions.AddToTail();
		RnLLoadoutKitInfo& classInfo = m_aClassDescriptions[kitIndex];
		classInfo.iKitId = kitIndex;
		kitIndex++;

		Q_strncpy(classInfo.name.GetForModify(), pClassData->GetName(), KIT_DESC_TITLE_LEN);
		Q_strncpy(classInfo.title.GetForModify(), pClassData->GetString("title"), KIT_DESC_TITLE_LEN);

		// TODO: Remove when squad leader voting is finished.
		classInfo.bSquadLeader = FStrEq(pClassData->GetString("squadleader", "false"), "true");

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
					classInfo.weapons.AddToTail(weaponID);
				}
				pWeaponSubData = pWeaponSubData->GetNextKey();
			}
		}

		classInfo.iClass = AliasToClassID(pClassData->GetString("type"));
		pModelData = pClassData->FindKey("model");
		
		if (classInfo.iClass == RNL_CLASS_INVALID ||
			pModelData == NULL)
		{
			kitIndex--;
		}
		else
		{
			Q_strncpy(classInfo.model.file.GetForModify(), pModelData->GetString("file"), KIT_DESC_MODEL_LEN);
			if (PrecacheModel(classInfo.model.file) < 0)
			{
				Q_strncpy(classInfo.model.file.GetForModify(), RNL_DEFAULT_PLAYER_MODEL, KIT_DESC_MODEL_LEN);
			}

			classInfo.model.iSkin = pModelData->GetInt("skin");

			pBodyGroups = pModelData->FindKey("bodygroups");
			if (pBodyGroups)
			{
				pGroup = pBodyGroups->GetFirstSubKey();
				int bodyGroupIndex = -1;
				while (pGroup)
				{
					bodyGroupIndex = classInfo.model.vecBodyGroups.AddToTail();
					Q_strncpy(
						classInfo.model.vecBodyGroups[bodyGroupIndex].groupName.GetForModify(),
						pGroup->GetName(),
						KIT_DESC_MODEL_LEN
					);
					classInfo.model.vecBodyGroups[bodyGroupIndex].iVal = pGroup->GetInt();
					pGroup = pGroup->GetNextKey();
				}

			}
		}

		pClassData = pClassData->GetNextKey();
	}
	NetworkStateChanged(&m_aClassDescriptions);
	return true;
}

bool CRnLGameTeam::LoadSquadDescriptions(KeyValues* pKey)
{
	if (!pKey)
		return false;

	KeyValues* pSquadInfo = pKey->GetFirstSubKey();
	while (pSquadInfo)
	{
		if (m_aSquads.Count() < RNL_SQUADS_MAX)
		{
			CRnLSquad* pSquad = (CRnLSquad*)CreateEntityByName("rnl_squad");
			int SquadIndex = m_aSquads.AddToTail(pSquad);

			pSquad->m_SquadId = SquadIndex;
			pSquad->ChangeTeam(GetTeamNumber());
			pSquad->Load(this, pSquadInfo);
		}

		pSquadInfo = pSquadInfo->GetNextKey();
	}
	NetworkStateChanged();
	return true;
}