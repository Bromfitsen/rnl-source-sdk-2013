//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "rnl_axis_team.h"
#include "rnl_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Datatable
IMPLEMENT_SERVERCLASS_ST(CRnLAxisSquad, DT_RnLAxisSquad)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( rnl_axis_squad, CRnLAxisSquad );

CRnLAxisSquad::CRnLAxisSquad()
{
	m_pOwner = NULL;
}

void CRnLAxisSquad::SetOwnerTeam( CRnLAxisTeam* pOwner )
{
	SetParentTeam( pOwner );
	m_pOwner = pOwner;
}

CRnLAxisTeam* CRnLAxisSquad::GetOwnerTeam( void )
{
	return m_pOwner;
}

bool CRnLAxisSquad::AddPlayer( CRnLPlayer* pPlayer, int iSlot )
{
	if( BaseClass::AddPlayer( pPlayer, iSlot ) )
	{
		pPlayer->SetKitNumber( iSlot );
		return true;
	}

	return false;
}

extern void respawn(CBaseEntity *pEdict, bool fCopyCorpse);
void CRnLAxisSquad::Update()
{
	if( RnLGameRules() && RnLGameRules()->GetGameManager() &&
	   (RnLGameRules()->GetGameManager()->GetAxisSpawnTimer() <= 0.0f) &&
	   (RnLGameRules()->GetGameManager()->GetAxisTicketsRemaining() > 0) )
	{
		for( int i = 0; i < m_aSlotInfo.Count(); i++ )
		{
			for( int j = 0; j < m_aSlotInfo[i].pMembers.Count(); j++ )
			{
				if( !m_aSlotInfo[i].pMembers[j]->IsAlive() && m_aSlotInfo[i].pMembers[j]->IsReadyToSpawn() )
				{
					respawn( m_aSlotInfo[i].pMembers[j], false);
				}
			}
		}
	}

	BaseClass::Update();
}

bool CRnLAxisSquad::Load( KeyValues* pKey )
{
	if( pKey && m_pOwner )
	{
		Q_strncpy( m_szSquadReferenceName, pKey->GetName(), MAX_TEAM_NAME_LENGTH );

		KeyValues* pSlots = NULL;

		pSlots = pKey->FindKey( "slots" );
		if( pSlots )
		{
			KeyValues* pClass = pSlots->GetFirstSubKey();;
			int index = 0;
			int iClassId = 0;
			int iLimit = 0;

			while( pClass )
			{
				iClassId = m_pOwner->LookupKitDescription( pClass->GetName() );

				if( iClassId > -1 )
				{
					DevMsg( "Server: Axis Team Adding Kit Def: %s with max of %d\n", pClass->GetName(), pClass->GetInt() );
					iLimit = pClass->GetInt();
					index = m_aSlotInfo.AddToTail();

					m_aSlotInfo[index].iKitDesc  = iClassId;
					m_aSlotInfo[index].iMaxCount = iLimit;
				}
				else
				{
					Msg( "Server: Error Finding Data For Class %s\n", pClass->GetName() );
				}

				pClass = pClass->GetNextKey();
			}
		}
	}

	return true;
}

// Datatable
IMPLEMENT_SERVERCLASS_ST(CRnLAxisTeam, DT_RnLAxisTeam)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( rnl_team_axis, CRnLAxisTeam );

CRnLAxisTeam::CRnLAxisTeam()
{
}

// Initialization
void CRnLAxisTeam::Init( const char *pName, int iNumber, KeyValues* pKeyVals  )
{
	BaseClass::Init( pName, iNumber, pKeyVals );

	if( pKeyVals )
	{
		CRnLAxisSquad* pSquad = NULL;
		KeyValues* pSquads = pKeyVals->FindKey( "squads" );
		if( pSquads )
		{
			KeyValues* pSquadInfo = pSquads->GetFirstSubKey();
			while( pSquadInfo )
			{
				pSquad = (CRnLAxisSquad*)(CreateEntityByName( "rnl_axis_squad" ));
				pSquad->SetOwnerTeam( this );
				pSquad->Load( pSquadInfo ); 
				m_aSquads.AddToTail( pSquad );

				pSquadInfo = pSquadInfo->GetNextKey();
			}
		}
	}
}