#ifndef RNL_BASE_SQUAD_H
#define RNL_BASE_SQUAD_H
#ifdef _WIN32
	#pragma once
#endif

#include "rnl_shareddefs.h"

#ifdef CLIENT_DLL
	#include "c_rnl_player.h"
	#include "c_rnl_game_team.h"
#else
	#include "rnl_player.h"
	#include "rnl_game_team.h"
#endif

#ifdef CLIENT_DLL
void SquadGUIGetIdealProportions(int iCurrSquad, int iTotalSquads, int& x, int& y, int& w, int& h);
#endif

class CRnLSquadMember
{
public:
	DECLARE_CLASS_NOBASE(CRnLSquadMember);
	DECLARE_EMBEDDED_NETWORKVAR();

	CRnLSquadMember() { }

	CNetworkVar( int, iKitDesc );
	CNetworkHandle(CRnLPlayer, m_Player);
};

class CRnLSquadKitInfo
{
public:
	DECLARE_CLASS_NOBASE(CRnLSquadKitInfo);
	DECLARE_EMBEDDED_NETWORKVAR();

	CRnLSquadKitInfo() { }

	CNetworkVar(int, iKitID);
	CNetworkVar(int, iMaxCount);
};

class CRnLSquad : public CBaseEntity
{
public:
	DECLARE_CLASS(CRnLSquad, CBaseEntity);
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

	CRnLSquad( void );
	virtual ~CRnLSquad( void );

#ifndef CLIENT_DLL
	int UpdateTransmitState();

	virtual void Update( void );

public:
	virtual bool		Load(KeyValues* pKey);

	bool		AddPlayer(CRnLPlayer* pPlayer, int iKitId);
	bool		RemovePlayer(CRnLPlayer* pPlayer);

	const char* GetSquadName(void);
#endif
public:

	const char* GetSquadTitle(void);

	int					GetKitCount(void);
	CRnLSquadKitInfo&	GetKitInfo(int indx);

	int					GetMemberCount(void);
	int					GetMemberCount(int iKitId);
	CRnLPlayer*			GetMember(int indx);
	CRnLPlayer*			GetMember(int iKit, int idx);
	CRnLPlayer*			GetNextMember(int iKitID, CRnLPlayer* CurrentMember = nullptr);

	int			SquadSize( void );
	bool		IsSquadFull( void );
	int			GetNextAvailableSlot( void );
	bool		AreRequirementsMet( void );

	bool		IsKitAvailable( int iKit );
	int			GetKitMaxCount(int iKit);
	int			GetTotalAvailableKits( void );
	int			GetKitDescription( int iKit );

	CRnLPlayer* GetSquadLeader(void) { return m_hSquadLeader.Get(); }
	int			GetEntIndex( void ) { return entindex(); }

	CRnLGameTeam*	GetParentTeam( void ) { return m_hParentTeam; }

#ifndef CLIENT_DLL
	void			SetParentTeam(CRnLGameTeam* pEnt);

protected:
	virtual bool CanBeSquadLeader(CRnLPlayer* pEnt);
	void		SetSquadLeader(CRnLPlayer* pEnt) { m_hSquadLeader = pEnt; }
#endif

public:
	CNetworkHandle( CRnLGameTeam, m_hParentTeam );
	CNetworkHandle( CRnLPlayer, m_hSquadLeader );
	CUtlVector<CRnLSquadKitInfo> m_KitInfo;
	CUtlVector<CRnLSquadMember>	m_Members;
	CNetworkString(m_szSquadTitle, MAX_TEAM_NAME_LENGTH);
#ifndef CLIENT_DLL
	char						m_szSquadReferenceName[MAX_TEAM_NAME_LENGTH];
	float						m_flSquadLeaderSelectionTimer;
#endif
};

#endif // RNL_SQUAD_H
