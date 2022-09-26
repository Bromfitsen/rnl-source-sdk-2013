//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_rnl_base.h"

#ifdef CLIENT_DLL
	
	#include "c_rnl_game_team.h"
	#include "c_rnl_base_squad.h"

	#define CRnLGameTeam C_RnLGameTeam
#else

	#include "rnl_game_team.h"
	#include "rnl_base_squad.h"

#endif

bool CUtlStringLessFunc( const CUtlString &lhs, const CUtlString &rhs )	
{ 
	return CaselessStringLessThan( lhs.Get(), rhs.Get() ); 
}

bool CRnLGameTeam::LoadClassDescriptions( KeyValues* pKey )
{
	if( !pKey )
		return false;

	m_aClassIndexLookUp.SetLessFunc( CUtlStringLessFunc );

	int index = -1;
	KeyValues* pModelData = NULL;
	KeyValues* pWeaponData = NULL;
	KeyValues* pWeaponSubData = NULL;
	KeyValues* pBodyGroups = NULL;
	KeyValues* pGroup = NULL;
	KeyValues* pClassData = pKey->GetFirstSubKey();
	bool precacheAllowed = IsPrecacheAllowed();

	SetAllowPrecache( true );

	m_aClassDescriptions.RemoveAll();

	while( pClassData )
	{
		index = m_aClassDescriptions.AddToTail();

		Q_strncpy( m_aClassDescriptions[index].name, pClassData->GetName(), KIT_DESC_TITLE_LEN );
		Q_strncpy( m_aClassDescriptions[index].title, pClassData->GetString( "title" ), KIT_DESC_TITLE_LEN );

		// TODO: Remove when squad leader voting is finished.
		if( FStrEq(pClassData->GetString( "squadleader", "false" ), "true") )
			m_aClassDescriptions[index].bSquadLeader = true;
		else
			m_aClassDescriptions[index].bSquadLeader = false;
		
		pWeaponData = pClassData->FindKey( "weapons" );
		if( pWeaponData != NULL )
		{
			pWeaponSubData = pWeaponData->GetFirstSubKey();
			char szFullTitle[64];
			while( pWeaponSubData )
			{
				Q_snprintf( szFullTitle, sizeof( szFullTitle ), "weapon_%s", pWeaponSubData->GetString() );
				m_aClassDescriptions[index].weapons.AddToTail( szFullTitle );
				pWeaponSubData = pWeaponSubData->GetNextKey();
			}
		}
		
		m_aClassDescriptions[index].iClass = AliasToClassID( pClassData->GetString( "type" ) );
		if( m_aClassDescriptions[index].iClass == RNL_CLASS_INVALID )
		{
			m_aClassDescriptions.Remove( index );
		}
		else
		{
			pModelData = pClassData->FindKey( "model" );
			if( pModelData != NULL )
			{
				Q_strncpy( m_aClassDescriptions[index].model.file, pModelData->GetString( "file" ), KIT_DESC_MODEL_LEN );
				if( PrecacheModel( m_aClassDescriptions[index].model.file ) < 0 )
				{
					Q_strncpy( m_aClassDescriptions[index].model.file, RNL_DEFAULT_PLAYER_MODEL, KIT_DESC_MODEL_LEN );
				}

				m_aClassDescriptions[index].model.iSkin = pModelData->GetInt( "skin" );
				
				pBodyGroups = pModelData->FindKey( "bodygroups" );
				if( pBodyGroups )
				{
					pGroup = pBodyGroups->GetFirstSubKey();
					int bodyGroupIndex = -1;
					while( pGroup )
					{
						bodyGroupIndex = m_aClassDescriptions[index].model.vecBodyGroups.AddToTail();
						m_aClassDescriptions[index].model.vecBodyGroups[bodyGroupIndex].groupName.Set( pGroup->GetName() );
						m_aClassDescriptions[index].model.vecBodyGroups[bodyGroupIndex].iVal = pGroup->GetInt();
						pGroup = pGroup->GetNextKey();
					}

				}

				m_aClassIndexLookUp.Insert( pClassData->GetName(), index );
			}
			else
			{
				m_aClassDescriptions.Remove( index );
			}
		}

		pClassData = pClassData->GetNextKey();
	}

	SetAllowPrecache( precacheAllowed );
	return true;
}

int CRnLGameTeam::GetNumberOfSquads( void )
{
	return m_aSquads.Count();
}

int CRnLGameTeam::LookupKitDescription( const char* pName )
{
	return m_aClassIndexLookUp.Find( pName );
}

bool CRnLGameTeam::IsKitDescriptionValid( int iIndex )
{
	if( iIndex < 0 || iIndex >= m_aClassDescriptions.Count() )
		return false;

	return true;
}

RnLKitDescription& CRnLGameTeam::GetKitDescription( int iIndex )
{
	return m_aClassDescriptions[iIndex];
}

int CRnLGameTeam::GetKitDescriptionCount( void )
{
	return m_aClassDescriptions.Count();
}