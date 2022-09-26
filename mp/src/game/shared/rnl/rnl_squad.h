#ifndef RNL_ISQUAD_H
#define RNL_ISQUAD_H

#ifdef _WIN32
	#pragma once
#endif

#ifdef CLIENT_DLL
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
#endif

class CSquadSlotInfo
{
public:
	DECLARE_CLASS_NOBASE( CSquadSlotInfo );
	DECLARE_EMBEDDED_NETWORKVAR();

	CSquadSlotInfo() { pMembers.EnsureCapacity( RNL_SQUAD_SLOTS_MAX ); }

	CNetworkVar( int, iKitDesc );
	CNetworkVar( int, iMaxCount );

	CUtlVector< CHandle<CRnLPlayer> > pMembers;
};

abstract_class IRnLSquad
{
public:
	//virtual ~IRnLSquad(){};

	virtual	bool		Load( KeyValues* pKey ) = 0;
	virtual void		Update() = 0;

	virtual const char*	GetSquadName( void ) = 0;

	virtual int				GetSlotCount( void ) = 0;
	virtual CSquadSlotInfo&	GetSlotInfo( int indx ) = 0;

	virtual bool		AddPlayer( CRnLPlayer* pPlayer, int iKit ) = 0;
	virtual bool		RemovePlayer( CRnLPlayer* pPlayer ) = 0;

	virtual CRnLPlayer*	GetSquadLeader( void ) = 0;
	virtual int			SquadSize( void ) = 0;
	virtual bool		IsSquadFull( void ) = 0;
	virtual bool		AreRequirementsMet( void ) = 0;
	
	virtual bool		IsKitAvailable( int iKit ) = 0;
	virtual int			GetTotalAvailableKits( void ) = 0;
	virtual int			GetKitDescription( int iKit ) = 0;

	virtual int			GetEntIndex( void ) = 0;
};

#endif //RNL_ISQUAD_H