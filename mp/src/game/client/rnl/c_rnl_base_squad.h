//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTeam class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_RNL_BASE_SQUAD_H
#define C_RNL_BASE_SQUAD_H
#ifdef _WIN32
	#pragma once
#endif

#include "rnl_shareddefs.h"
#include "c_rnl_player.h"
#include "rnl_squad.h"
#include "c_rnl_game_team.h"


extern void SquadGUIGetIdealProportions( int iCurrSquad, int iTotalSquads, int &x, int &y, int &w, int &h );

class C_RnLBaseSquad : public C_BaseEntity, public IRnLSquad
{
	DECLARE_CLASS( C_RnLBaseSquad, C_BaseEntity );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

					C_RnLBaseSquad();
	virtual			~C_RnLBaseSquad();

	void			Update( void ) {};
	bool			NeedsUpdate( void ) { return false; }

public:
	virtual bool	Load( KeyValues* pKey ){ return true; }

	bool		AddPlayer( CRnLPlayer* pPlayer, int iSlot );
	bool		RemovePlayer( CRnLPlayer* pPlayer );

	const char*	GetSquadName( void );
	int				GetSlotCount( void );
	CSquadSlotInfo&	GetSlotInfo( int indx );

	int			SquadSize( void );
	bool		IsSquadFull( void );
	bool		AreRequirementsMet( void );

	bool		IsKitAvailable( int iKit );
	int			GetTotalAvailableKits( void );
	int			GetKitDescription( int iKit );

	int			GetKitMaxCount( int iKit );

	int			GetEntIndex( void ) { return entindex(); }

	CRnLPlayer*		GetSquadLeader( void ) { return m_hSquadLeader.Get(); }
	C_RnLGameTeam*	GetParentTeam( void ) { return m_hParentTeam.Get(); }

	int			GetMemberCount( int iKit );
	CRnLPlayer* GetMember( int iKit, int idx );

public:
	CNetworkHandle( C_RnLGameTeam, m_hParentTeam );
	CNetworkHandle( CRnLPlayer, m_hSquadLeader);
	CUtlVector<CSquadSlotInfo>	m_aSlotInfo;
	char							m_szSquadTitle[MAX_TEAM_NAME_LENGTH];
	char							m_szSquadReferenceName[MAX_TEAM_NAME_LENGTH];
};


#endif // C_RNL_SQUAD_H
