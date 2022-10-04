//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTFTeam class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_RNL_GAME_TEAM_H
#define C_RNL_GAME_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_rnl_team.h"
#include "rnl_shareddefs.h"

class CRnLSquad;

#define CRnLGameTeam C_RnLGameTeam

//-----------------------------------------------------------------------------
// Purpose: TF's Team manager
//-----------------------------------------------------------------------------
class C_RnLGameTeam : public C_RnLTeam
{
	DECLARE_CLASS( C_RnLGameTeam, C_RnLTeam );

public:
	DECLARE_CLIENTCLASS();

					C_RnLGameTeam();
	virtual			~C_RnLGameTeam();

	int					GetNumberOfSquads( void );
	CRnLSquad*			GetSquad( int idx );

	virtual int						LookupKitDescription( const char* pName );
	virtual bool					IsKitDescriptionValid( int iIndex );
	virtual CRnLLoadoutKitInfo&		GetKitDescription( int iIndex );
	virtual int						GetKitDescriptionCount( void );

public:
	CUtlVector<CRnLLoadoutKitInfo>	m_aClassDescriptions;
	CUtlVector<CHandle<CRnLSquad>>			m_aSquads;

	bool							m_bSquadChange;
};


#endif // C_RNL_GAME_TEAM_H
