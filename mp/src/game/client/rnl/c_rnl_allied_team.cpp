//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_rnl_allied_team.h"
#include "rnl_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Datatable
IMPLEMENT_CLIENTCLASS_DT( C_RnLAlliedSquad, DT_RnLAlliedSquad, CRnLAlliedSquad )
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_RnLAlliedSquad )
END_PREDICTION_DATA();

C_RnLAlliedSquad::C_RnLAlliedSquad()
{
	m_pOwner = NULL;
}

void C_RnLAlliedSquad::SetOwnerTeam( C_RnLAlliedTeam* pOwner )
{
	m_pOwner = pOwner;
}

C_RnLAlliedTeam* C_RnLAlliedSquad::GetOwnerTeam( void )
{
	return m_pOwner;
}

bool C_RnLAlliedSquad::Load( KeyValues* pKey )
{
	if( pKey && m_pOwner )
	{
		Q_strncpy( m_szSquadReferenceName, pKey->GetName(), MAX_TEAM_NAME_LENGTH );

		KeyValues* pSlots = NULL;

		Q_strncpy( m_szSquadTitle, pKey->GetString( "name" ), MAX_TEAM_NAME_LENGTH );
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
					DevMsg( "Client: Allies Team Adding Kit Def: %s with max of %d\n", pClass->GetName(), pClass->GetInt() );
					iLimit = pClass->GetInt();
					index = m_aSlotInfo.AddToTail();

					m_aSlotInfo[index].iKitDesc  = iClassId;
					m_aSlotInfo[index].iMaxCount = iLimit;
				}
				else
				{
					Msg( "Client: Error Finding Data For Class %s\n", pClass->GetName() );
				}

				pClass = pClass->GetNextKey();
			}
		}
	}

	return true;
}

// Datatable
IMPLEMENT_CLIENTCLASS_DT(C_RnLAlliedTeam, DT_RnLAlliedTeam, CRnLAlliedTeam)
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_RnLAlliedTeam )
END_PREDICTION_DATA();

C_RnLAlliedTeam::C_RnLAlliedTeam()
{
}

// Initialization
void C_RnLAlliedTeam::InitSquads( KeyValues* pKeyVals  )
{
	if( pKeyVals )
	{
		C_RnLAlliedSquad* pSquad = NULL;
		KeyValues* pSquadInfo = pKeyVals->FindKey( "squad" );
		KeyValues* pFireTeams = NULL;
		int iSquad = 0;
		
		while( pSquadInfo && iSquad < GetNumberOfSquads() )
		{
			pFireTeams = pSquadInfo->FindKey( "fireteam" );

			while( pFireTeams && iSquad < GetNumberOfSquads() )
			{
				pSquad = (C_RnLAlliedSquad*)(GetSquad( iSquad ));
				if( pSquad )
				{
					if( pSquad->GetOwnerTeam() != this || pSquad->NeedsUpdate() )
					{
						pSquad->SetOwnerTeam( this );
						pSquad->Load( pFireTeams ); 
					}
				}

				iSquad++;
				pFireTeams = pFireTeams->GetNextKey();
			}

			pSquadInfo = pSquadInfo->GetNextKey();
		}
	}
}

void C_RnLAlliedTeam::PostDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PostDataUpdate( updateType );
}