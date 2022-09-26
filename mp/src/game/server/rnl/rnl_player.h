//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for RnL Game
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_PLAYER_H
#define RNL_PLAYER_H
#pragma once

#include "basemultiplayerplayer.h"
#include "server_class.h"
#include "rnl_playeranimstate.h"
#include "rnl_shareddefs.h"
#include "rnl_playerlocaldata.h"
#include "rnl_equipment_prop.h"
#include "utlvector.h"
#include "utlstring.h"

int AliasToClassID( const char *alias );
const char *ClassIDToAlias( int id );

struct player_damage_info
{
	int iDamageType;
	int iDamageAmount;
	CUtlString strInfo;
};

//=============================================================================
// >> RnL Game player
//=============================================================================
class CRnLPlayer : public CBaseMultiplayerPlayer, public IRnLPlayerAnimStateHelpers
{
public:
	DECLARE_CLASS( CRnLPlayer, CBaseMultiplayerPlayer );
	DECLARE_SERVERCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_DATADESC();

	CRnLPlayer();
	~CRnLPlayer();

	static CRnLPlayer *CreatePlayer( const char *className, edict_t *ed );
	static CRnLPlayer* Instance( int iEnt );

	// This passes the event to the client's and server's CPlayerAnimState.
	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );

	virtual void PreThink();
	virtual void PostThink();
	virtual void ItemPostFrame( void );
	virtual void Spawn();
	virtual void InitialSpawn();
	virtual void Precache();
	virtual int	 OnTakeDamage( const CTakeDamageInfo &info );
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual void TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr );
	virtual void LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles );

	virtual CBaseEntity		*GiveNamedItem( const char *szName, int iSubType = 0 );

	virtual bool BumpWeapon( CBaseCombatWeapon *pWeapon );

	void CheckWeaponSwitch( void );
	void SelectItem( const char *pstr, int iSubType );

	//Michael Lebson: Equipment handling and stuff
	void	AddEquipment( int iType );
	void	RemoveEquipment( int iType );
	void	OpenParachute( void );

	virtual bool ClientCommand( const CCommand &args );

	//RnL : MovementMod : Begin
	virtual void GetStepSoundVelocities( float *velwalk, float *velrun );
	virtual void SetStepSoundTime( stepsoundtimes_t iStepSoundTime, bool bWalking );
	virtual Vector		EyePosition( void );
	
	virtual QAngle		GetWeaponAngle( void );
	virtual void		SetWeaponAngle( const QAngle& angle );
	void AdjustWeaponAngle( const QAngle &angleOffset );
	void AdjustWeaponSway( const QAngle& offset );

	virtual bool	IsDuckToggled( void );
	virtual void	ToggleDuck( void );

	virtual int		GetSquadNumber();
	virtual void	SetSquadNumber( int iNum );

	virtual int		GetKitNumber();
	virtual void	SetKitNumber( int iNum );

	virtual	void	ChangeTeam( int iTeamNum );
	virtual bool	ChangeSquad( int iSquad, int iSlot );

	virtual int		GetSquadLeaderVote( void ) { return m_iSquadLeaderVote; }
	//RnL : MovementMod : End
	
	CWeaponRnLBase* GetActiveRnLWeapon() const;
	virtual void	CreateViewModel( int viewmodelindex = 0 );

	virtual void	CheatImpulseCommands( int iImpulse );

	CNetworkVar( int, m_iThrowGrenadeCounter );	// used to trigger grenade throw animations.
	CNetworkQAngle( m_angEyeAngles );	// Copied from EyeAngles() so we can send it to the client.
	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently

	// Tracks our ragdoll entity.
	CNetworkHandle( CBaseEntity, m_hRagdoll );	// networked entity handle 
	CNetworkHandle( CBaseEntity, m_hKnockDownRagdoll );	// networked entity handle 

	CNetworkVar( int, m_iSquadNumber );
	CNetworkVar( int, m_iPreviousSquadNumber );
	CNetworkVar( int, m_iKitNumber );
	CNetworkVar( int, m_iPreviousKitNumber );
	CNetworkVar( float, m_flDeathViewTime );
	CNetworkVar( bool, m_bIsDuckToggled );

	//Damage Report and TK Kick
	int				m_iTeamKills;

	float			m_flNextMoraleUpdate;

	// Fade to black handling
	void FadeToBlackThink( void );

	// Round gamerules
	virtual bool	IsReadyToSpawn( void );
	virtual void	ResetPerRoundStats( void );

	// cjd @add
	void	PlayVocalization( const char *pszVocalizationName, bool bRadio = false );
	// cjd end

	// knockdown on hit
	void KnockDown( void );
	void KnockDownThink( void );

// In shared code.
public:
	// IRnLPlayerAnimState overrides.
	virtual CWeaponRnLBase* RnLAnim_GetActiveWeapon();
	virtual bool CanMove();

	//RnL : MovementMod : Begin
	CNetworkQAngle( m_angWeaponAngle );

	CNetworkVarEmbedded( CRnLPlayerLocalData, m_RnLLocal );
	
	CNetworkVar( int, m_iClimbheight );
	int		GetClimbheight( void ) { return m_iClimbheight; }

	CNetworkVar( float, m_flBodyHeight );
	CNetworkVar( int, m_nWeaponPosture );

	int		GetWeaponPosture( void ) { return m_nWeaponPosture; }
	void	SetWeaponPosture( int iPosture ) { m_nWeaponPosture = iPosture; }

	void	AdjustViewAngles( const QAngle &angleOffset );

	float	GetMaxStamina();
	float	GetStamina();
	void	SetStamina( float flStamina );
	void	SetSprintTimer( float flSprintTime );
	float	GetSprintTimer();
	void	SetRecoverTime( float flRecoverTime );
	float	GetRecoverTime();

	CNetworkVector( m_vecLeanOffset );
	Vector	GetLeaningOffset( void ) { return m_vecLeanOffset.Get(); }
	void	SetLeaningOffset( Vector offset ) { m_vecLeanOffset = offset; }

	float	GetViewRollOffset( void );
	void	SetViewRollOffset( float newOffset );

	void	SetMoraleLevel( int iLev );
	int		GetMoraleLevel( void );
	void	UpdatePlayerMorale( void );

	float	m_flNextSprintSound;
	float	GetNextSprintSoundTime( void ) { return m_flNextSprintSound; }
	void	SetNextSprintSoundTime( float flTime ) { m_flNextSprintSound = flTime; }

	float	PlayStaminaSound( int iState );
	void	PlayMovingWoundedSound();

	bool	IsSprinting( void );
	bool	StartSprinting( void );
	bool	StopSprinting( void );

	void Deploy( QAngle angAngles, Vector Origin );
	void UnDeploy( void );
	bool IsDeployed( void ) const;

	bool IsWounded( void );
	//RnL : MovementMod : End

	bool UpdateBullet( int iIndex ); 

	void FireBullet( 
		Vector vecSrc, 
		const QAngle &shootAngles, 
		float vecSpread, 
		int iDamage, 
		int iBulletType,
		CBaseEntity *pevAttacker,
		bool bDoEffects,
		float x,
		float y );

	void FireBulletImmediately( RnLBulletInfo& bullet );
	void RemoveAllAmmo( void );
	void DropActiveWeapon( void );

	float GetDamageBasedSpeedModifier( int iType );
	void SetDamageBasedSpeedModifier( int iType, float val );
	void AddDamageBasedSpeedModifier( int iType, float val );

	//Tony; spawn interp
	CNetworkVar( bool, m_bSpawnInterpCounter );

	CBaseCombatWeapon *m_pSwitchingToWeapon;

private:
	void CreateRagdollEntity(bool bDead = true);

	CRnLEquipmentParachute*		m_pRnLParachute;
	CRnLEquipmentRadio*			m_pRnLRadio;

	IRnLPlayerAnimState *m_PlayerAnimState;

	CUtlVector<RnLBulletInfo> m_aBullets;
	int m_iSquadLeaderVote;

	float	m_flNextVocalizationTime;	// cjd @add
	float	m_flNextMovingWoundedSound;

	CUtlVector<player_damage_info> m_vPlayerDamageInfo;

};

inline CRnLPlayer *ToRnLPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

#ifdef _DEBUG
	Assert( dynamic_cast<CRnLPlayer*>( pEntity ) != 0 );
#endif
	return static_cast< CRnLPlayer* >( pEntity );
}


#endif	// RNL_PLAYER_H
