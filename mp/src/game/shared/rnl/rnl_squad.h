#ifndef RNL_BASE_SQUAD_H
#define RNL_BASE_SQUAD_H
#ifdef _WIN32
	#pragma once
#endif

#include "rnl_shareddefs.h"

#ifdef CLIENT_DLL
	#include "c_rnl_player.h"
#else
	#include "rnl_player.h"
#endif

#ifdef CLIENT_DLL
void SquadGUIGetIdealProportions(int iCurrSquad, int iTotalSquads, int& x, int& y, int& w, int& h);
#endif

#ifdef CLIENT_DLL
class C_RnLGameTeam;
#define CRnLGameTeam C_RnLGameTeam
#else
class CRnLGameTeam;
#endif

class RnLSquadKitInfo
{
public:
	DECLARE_CLASS_NOBASE(RnLSquadKitInfo);
	DECLARE_EMBEDDED_NETWORKVAR();

	RnLSquadKitInfo() { iKitID = -1; }
	bool IsValid() const { return iKitID >= 0; }

	CNetworkVar(int, iKitID);
	CNetworkVar(int, iMaxCount);
};

class CRnLSquad : public CBaseEntity
{
public:
	DECLARE_CLASS(CRnLSquad, CBaseEntity);
	DECLARE_NETWORKCLASS();

	CRnLSquad();
	virtual ~CRnLSquad();

#ifndef CLIENT_DLL
public:
	virtual bool		Load(CRnLGameTeam* OwnerTeam, KeyValues* pKey);

	virtual int			UpdateTransmitState(void) OVERRIDE;

	bool				AddPlayer(CRnLPlayer* pPlayer, int iKitId);
	bool				RemovePlayer(CRnLPlayer* pPlayer);

	const char*			GetSquadName(void) const;
#endif
public:

	bool				IsValid() const;
	const char*			GetSquadTitle(void) const;

	int						GetKitCount(void) const;
	const RnLSquadKitInfo&	GetKitInfo(int indx) const;

	int					GetMemberCount(void) const;
	int					GetMemberCount(int iKitId) const;
	CRnLPlayer*			GetMember(int indx) const;
	CRnLPlayer*			GetMember(int iKit, int idx) const;
	CRnLPlayer*			GetNextMember(int iKitID, CRnLPlayer* CurrentMember = nullptr) const;

	bool			IsSquadFull( void ) const;
	int				GetNextAvailableSlot( void ) const;
	bool			AreRequirementsMet( void ) const;

	bool			IsKitAvailable( int iKit ) const;
	int				GetKitMaxCount(int iKit) const;
	int				GetTotalAvailableKits( void ) const;
	int				GetKitDescription( int iKit ) const;

	CRnLPlayer*		GetSquadLeader(void) const { return m_hSquadLeader.Get(); }

#ifndef CLIENT_DLL
	virtual bool	CanBeSquadLeader(CRnLGameTeam* OwnerTeam, CRnLPlayer* pEnt) const;
	void			SetSquadLeader(CRnLPlayer* pEnt) { m_hSquadLeader = pEnt; }
#endif

public:
	CNetworkHandle(CRnLGameTeam,		m_hTeam);
	CNetworkVar(int,					m_SquadId);
	CNetworkString(						m_szSquadTitle, MAX_TEAM_NAME_LENGTH);
	CNetworkHandle(CRnLPlayer,			m_hSquadLeader);
	CUtlVector<RnLSquadKitInfo>			m_KitInfo;

#ifndef CLIENT_DLL
	char						m_szSquadReferenceName[MAX_TEAM_NAME_LENGTH];
	float						m_flSquadLeaderSelectionTimer;
#endif
};

#endif // RNL_SQUAD_H
