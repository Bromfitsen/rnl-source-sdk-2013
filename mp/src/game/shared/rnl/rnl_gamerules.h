//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: The TF Game rules object
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_GAMERULES_H
#define RNL_GAMERULES_H

#ifdef _WIN32
#pragma once
#endif


#include "teamplay_gamerules.h"
#include "convar.h"
#include "gamevars_shared.h"
#include "rnl_iobjectivelistener.h"
#include "rnl_basetimer.h"
#include "rnl_game_manager.h"

#ifdef CLIENT_DLL
	#include "c_baseplayer.h"
#else
	#include "player.h"
#endif

#define MAX_CAMPAIGN_FILE	256
#define MAX_CAMPAIGN_SECTION 256
#define MAX_CAMPAIGN_SCRIPT 256

#ifdef CLIENT_DLL
	#define CRnLGameRules C_RnLGameRules
	#define CRnLGameRulesProxy C_RnLGameRulesProxy
#endif


class CRnLGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CRnLGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();

#ifdef CLIENT_DLL
	void OnDataChanged( DataUpdateType_t type );
#endif

};

//Lifted from Teamplay Round Based Rules
//-----------------------------------------------------------------------------
// Round states
//-----------------------------------------------------------------------------
enum rnl_gamerules_roundstate_t
{
	// initialize the game, create teams
	RNL_GR_STATE_INIT = 0,
	//Before players have joined the game. Periodically checks to see if enough players are ready
	//to start a game. Also reverts to this when there are no active players
	RNL_GR_STATE_PREGAME,
	//The game is about to start, wait a bit and spawn everyone
	RNL_GR_STATE_STARTGAME,
	//All players are respawned, frozen in place
	RNL_GR_STATE_PREROUND,
	//Round is on, playing normally
	RNL_GR_STATE_RND_RUNNING,
	//Someone has won the round
	RNL_GR_STATE_TEAM_WIN,
	//Noone has won, manually restart the game, reset scores
	RNL_GR_STATE_RESTART,
	//Noone has won, restart the game
	RNL_GR_STATE_STALEMATE,
	//Game is over, showing the scoreboard etc
	RNL_GR_STATE_GAME_OVER,

	RNL_GR_NUM_ROUND_STATES
};

enum eRnLWinReasons{
	RNL_WINREASON_NONE =0,
	RNL_WINREASON_ALL_POINTS_CAPTURED,
	RNL_WINREASON_OPPONENTS_DEAD,
	RNL_WINREASON_FLAG_CAPTURE_LIMIT,
	RNL_WINREASON_DEFEND_UNTIL_TIME_LIMIT,
	RNL_WINREASON_STALEMATE,
	RNL_WINREASON_TIMELIMIT,
	RNL_WINREASON_WINLIMIT,
// 	WINREASON_HUNTED_DEAD,
// 	WINREASON_HUNTED_ESCAPED,
};

enum rnl_stalemate_reasons_t
{
	RNL_STALEMATE_JOIN_MID,
	RNL_STALEMATE_TIMER,
	RNL_STALEMATE_SERVER_TIMELIMIT,

	RNL_NUM_STALEMATE_REASONS,
};

class CRnLGameRules;
//-----------------------------------------------------------------------------
// Purpose: Per-state data
//-----------------------------------------------------------------------------
class CRnLGameRulesRoundStateInfo
{
public:
	rnl_gamerules_roundstate_t	m_iRoundState;
	const char*					m_pStateName;

	void (CRnLGameRules::*pfnEnterState)();	// Init and deinit the state.
	void (CRnLGameRules::*pfnLeaveState)();
	void (CRnLGameRules::*pfnThink)();	// Do a PreThink() in this state.
};

class CRnLGameRules : public CTeamplayRules, public IRnLObjectiveListener
{
public:
	DECLARE_CLASS( CRnLGameRules, CTeamplayRules );

	CRnLGameRules();
	virtual ~CRnLGameRules();
	
	virtual const	unsigned char *GetEncryptionKey() { return (unsigned char *)"?rmh$SFV"; }
	virtual bool	ShouldCollide( int collisionGroup0, int collisionGroup1 );

	virtual int		PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	virtual bool	IsTeamplay( void ) { return true;	}

	KeyValues*		GetTeamData( int iTeam );
	int				GetTeamCount( int iTeam );

	//IRnLObjectiveListener
	virtual void			RegisterObjective( IRnLObjective* pObjective );
	virtual void			RemoveObjective( IRnLObjective* pObjective );
	virtual IRnLObjective*	GetObjective( IRnLObjective* pCurrent );
	virtual IRnLObjective*	GetObjective( const char* pszName );
	virtual void			ObjectiveStateChange( IRnLObjective* pObjective );

	inline rnl_gamerules_roundstate_t State_Get( void ) { return m_iRoundState; }

	//Timer Access
#ifndef CLIENT_DLL
	virtual void				InitialiseGameManager( void );
#endif
	virtual CRnLGameManager*	GetGameManager( void );

	//Campaign Info
	const char*				GetCampaignFile( void )		{ return m_szCurrentCampaignFile.Get(); }
	const char*				GetCampaignSection( void )	{ return m_szCurrentCampaignSection.Get(); }

	void InitialiseWeapons( void );
	void InitialiseCampaign( void );
	void InitialiseSquads( void );

#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.

	virtual bool Init();
	virtual bool ClientCommand( CBaseEntity *pEdict, const CCommand &args );
	virtual void ClientSettingsChanged( CBasePlayer *pPlayer );
	virtual void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore );
	virtual bool FPlayerCanRespawn( CBasePlayer *pPlayer );
	virtual CBaseEntity *GetPlayerSpawnSpot( CBasePlayer *pPlayer );
	virtual void Think();
	virtual void GetNextLevelName( char *szNextMap, int bufsize );

	virtual const char *GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer );

	//
	virtual void RoundStart( void );
	virtual void RoundEnd( int iWinningTeam = -1, bool bForceMapChange = false );
	virtual void RoundRespawn( void );
	virtual void CheckRoundRestart( void );

	virtual void CleanUpMap( void );
	virtual bool ShouldCreateEntity( const char *pszClassName );
	virtual bool RoundCleanupShouldIgnore( CBaseEntity *pEnt );

	virtual void PlayStartRoundVoice( void );
	virtual void PlayWinSong( int team );
	virtual void PlayStalemateSong( void );
	virtual void PlaySuddenDeathSong( void );
	virtual void BroadcastSound( int iTeam, const char *sound );

	virtual void CheckRespawnWaves( void );
	virtual void CheckObjectives( void );
	virtual void CheckTickets( void );
	

	
public:
	virtual const char *GetGameDescription( void ) { return "Resistance and Liberation"; }  // this is the game name that gets seen in the server browser

	//State Machine lifted from Teamplay Roundbased
	static CRnLGameRulesRoundStateInfo* State_LookupInfo( rnl_gamerules_roundstate_t state );	// Find the state info for the specified state.

	// State machine handling
	void State_Enter( rnl_gamerules_roundstate_t newState );	// Initialize the new state.
	void State_Leave();										// Cleanup the previous state.
	void State_Transition( rnl_gamerules_roundstate_t newState );
	void State_Think();										// Update the current state.

	// State Functions
	void State_Enter_INIT( void );
	void State_Think_INIT( void );

	void State_Enter_PREGAME( void );
	void State_Think_PREGAME( void );

	void State_Enter_STARTGAME( void );
	void State_Think_STARTGAME( void );

	void State_Enter_PREROUND( void );
	void State_Think_PREROUND( void );

	void State_Enter_RND_RUNNING( void );
	void State_Think_RND_RUNNING( void );

	void State_Enter_TEAM_WIN( void );
	void State_Think_TEAM_WIN( void );

	void State_Enter_RESTART( void );
	void State_Think_RESTART( void );

	void State_Enter_STALEMATE( void );
	void State_Think_STALEMATE( void );
	void State_Leave_STALEMATE( void );

private:

	void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, bool bIgnoreWorld );


#endif
private:
	CNetworkString( m_szCurrentCampaignFile, MAX_CAMPAIGN_FILE+1 );
	CNetworkString( m_szCurrentCampaignSection, MAX_CAMPAIGN_SECTION+1 );
	CNetworkHandle( CRnLGameManager, m_hGameManager );

	CNetworkVar( rnl_gamerules_roundstate_t, m_iRoundState );
	CNetworkVar( int, m_iWinningTeam );
#ifndef CLIENT_DLL
	CRnLGameRulesRoundStateInfo	*m_pCurStateInfo;			// Per-state data 
	float						m_flStateTransitionTime;	// Timer for round states
	float						m_flRestartRoundTime;
	float						m_flRestartGameTime;

	bool						m_bChangeLevelAfterWin;
	float						m_flRoundEndTime;
	int							m_iRoundsPlayed;
#endif
	
#ifdef CLIENT_DLL
	KeyValues* m_pCampaignFile;
	KeyValues* m_pCampaignSection;
#endif

	KeyValues* m_pTeamData;
	CUtlVector<IRnLObjective*>	m_vObjectives;
};

//-----------------------------------------------------------------------------
// Gets us at the team fortress game rules
//-----------------------------------------------------------------------------

inline CRnLGameRules* RnLGameRules()
{
	return static_cast<CRnLGameRules*>(g_pGameRules);
}


#endif // RNL_GAMERULES_H
