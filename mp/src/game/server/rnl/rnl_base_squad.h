#ifndef RNL_BASE_SQUAD_H
#define RNL_BASE_SQUAD_H
#ifdef _WIN32
	#pragma once
#endif

#include "rnl_shareddefs.h"
#include "rnl_player.h"
#include "rnl_squad.h"
#include "rnl_game_team.h"


class CRnLBaseSquad : public CBaseEntity, public IRnLSquad
{
	DECLARE_CLASS( CRnLBaseSquad, CBaseEntity );
public:
	CRnLBaseSquad( void );
	virtual ~CRnLBaseSquad( void );

	DECLARE_SERVERCLASS();

	int UpdateTransmitState();

	void Update( void );

public:
	virtual bool		Load( KeyValues* pKey );

	bool		AddPlayer( CRnLPlayer* pPlayer, int iSlot );
	bool		RemovePlayer( CRnLPlayer* pPlayer );

	const char*	GetSquadName( void );
	int				GetSlotCount( void );
	CSquadSlotInfo&	GetSlotInfo( int indx );

	int			SquadSize( void );
	bool		IsSquadFull( void );
	int			GetNextAvailableSlot( void );
	bool		AreRequirementsMet( void );

	bool		IsKitAvailable( int iKit );
	int			GetTotalAvailableKits( void );
	int			GetKitDescription( int iKit );

	int			GetEntIndex( void ) { return entindex(); }

	CRnLPlayer*	GetSquadLeader( void ) { return m_hSquadLeader.Get(); }
	virtual bool CanBeSquadLeader( CRnLPlayer* pEnt );
	void		SetSquadLeader( CRnLPlayer* pEnt ) { m_hSquadLeader = pEnt; }

	CRnLGameTeam*	GetParentTeam( void ) { return m_hParentTeam; }
	void			SetParentTeam( CRnLGameTeam* pEnt ) { m_hParentTeam = pEnt; }

public:
	CNetworkHandle( CRnLGameTeam, m_hParentTeam );
	CNetworkHandle( CRnLPlayer, m_hSquadLeader );
	CUtlVector<CSquadSlotInfo>	m_aSlotInfo;
	float						m_flSquadLeaderSelectionTimer;
	char						m_szSquadReferenceName[MAX_TEAM_NAME_LENGTH];
};

#endif // RNL_BASE_SQUAD_H
