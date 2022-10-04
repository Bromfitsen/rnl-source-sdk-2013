//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_rnl_base.h"

#include "rnl_squad.h"
#ifdef CLIENT_DLL
	#include "c_rnl_game_team.h"
#else
	#include "rnl_game_team.h"
#endif

int CRnLGameTeam::GetNumberOfSquads( void )
{
	return m_aSquads.Count();
}

int CRnLGameTeam::LookupKitDescription( const char* pName )
{
	for (int i = 0; i < m_aClassDescriptions.Count(); i++)
	{
		if (Q_stricmp(pName, m_aClassDescriptions[i].name.Get()) == 0)
		{
			return i;
		}
	}
	return -1;
}

bool CRnLGameTeam::IsKitDescriptionValid( int iIndex )
{
	if( iIndex < 0 || iIndex >= m_aClassDescriptions.Count() )
		return false;

	return true;
}

CRnLLoadoutKitInfo& CRnLGameTeam::GetKitDescription( int iIndex )
{
	return m_aClassDescriptions[iIndex];
}

int CRnLGameTeam::GetKitDescriptionCount( void )
{
	return m_aClassDescriptions.Count();
}