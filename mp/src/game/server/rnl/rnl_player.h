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

	// This passes the event to the client's and server's CPlayerAnimState.
	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );

	void PreThink() OVERRIDE;
	void PostThink() OVERRIDE;
	void ItemPostFrame( void ) OVERRIDE;
	void Spawn() OVERRIDE;
	void InitialSpawn() OVERRIDE;
	void Precache() OVERRIDE;
	int	 OnTakeDamage( const CTakeDamageInfo &info ) OVERRIDE;
	void Event_Killed( const CTakeDamageInfo &info ) OVERRIDE;
	void TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr, CDmgAccumulator* pAccumulator = NULL) OVERRIDE;
	void LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles ) OVERRIDE;

	CBaseEntity		*GiveNamedItem( const char *szName, int iSubType = 0 ) OVERRIDE;

	bool BumpWeapon( CBaseCombatWeapon *pWeapon ) OVERRIDE;

	//RnL : MovementMod : Begin
	//MovementMod : Add these to gain access to the movement posture stuff
	RnLMovementPostures_t	GetMovementPosture(void) const;
	void			SetMovementPosture(RnLMovementPostures_t iType);
	RnLMovementPostures_t	GetMovementPostureFrom(void) const;
	float			GetMovementPostureDuration(void) const;
	void			SetMovementPostureDuration(float flTime);
	QAngle			GetMovementPostureAngle(void) const;
	void			SetMovementPostureAngle(QAngle angle);


	// adam: new stance changing method
	float GetMovementPostureOffset(void) const;
	void SetMovementPostureOffset(float flOffset);
	float GetMovementPostureTarget(void) const;
	void SetMovementPostureTarget(float fTarget);
	float GetMovementPostureVelocity(void) const;
	void SetMovementPostureVelocity(float fTarget);
	bool IsPostureChanging(float flVelLimit = 0.01, float flDiffLimit = 0.01) const;

	//Moved to public
	// Accessors for gamemovement
	bool IsStanding(void) const;
	bool IsStandingUp(void) const;

	bool IsDucked(void) const;
	bool IsDucking(void) const;

	bool IsProne(void) const;
	bool IsProning(void) const;
	//RnL : MovementMod : End

	void CheckWeaponSwitch( void );
	void SelectItem( const char *pstr, int iSubType );

	//Michael Lebson: Equipment handling and stuff
	void	AddEquipment(RnLEquipmentTypes_t iType );
	void	RemoveEquipment(RnLEquipmentTypes_t iType );
	void	OpenParachute( void );

	bool ClientCommand( const CCommand &args ) OVERRIDE;

	//RnL : MovementMod : Begin
	void GetStepSoundVelocities( float *velwalk, float *velrun ) OVERRIDE;
	void SetStepSoundTime( stepsoundtimes_t iStepSoundTime, bool bWalking ) OVERRIDE;
	Vector EyePosition( void ) OVERRIDE;
	
	QAngle		GetWeaponAngle( void );
	void		SetWeaponAngle( const QAngle& angle );
	void AdjustWeaponAngle( const QAngle &angleOffset );
	void AdjustWeaponSway( const QAngle& offset );

	int		GetSquadNumber();
	int		GetKitNumber();

	void	ChangeTeam( int iTeamNum ) OVERRIDE;
	bool	ChangeSquad( int iSquad, int iSlot );

	int		GetSquadLeaderVote( void ) { return m_iSquadLeaderVote; }
	//RnL : MovementMod : End
	
	CWeaponRnLBase* GetActiveRnLWeapon() const;
	void	CreateViewModel( int viewmodelindex = 0 ) OVERRIDE;

	void	CheatImpulseCommands( int iImpulse ) OVERRIDE;

	//MovementMod : Our  movement posture
	CNetworkVar(int, m_nMovementPosture);
	CNetworkVar(int, m_nMovementPostureFrom);
	CNetworkQAngle(m_angMovementPostureAngle);
	//RnL : MovmentMod : End

	CNetworkVar( int, m_iThrowGrenadeCounter );	// used to trigger grenade throw animations.
	CNetworkQAngle( m_angEyeAngles );	// Copied from EyeAngles() so we can send it to the client.
	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently

	// Tracks our ragdoll entity.
	CNetworkHandle( CBaseEntity, m_hRagdoll );	// networked entity handle 
	CNetworkHandle( CBaseEntity, m_hKnockDownRagdoll );	// networked entity handle 

	CNetworkVar( float, m_flDeathViewTime );

	//Damage Report and TK Kick
	int				m_iTeamKills;

	float			m_flNextMoraleUpdate;

	// Fade to black handling
	void FadeToBlackThink( void );

	// Round gamerules
	bool	IsReadyToSpawn( void ) OVERRIDE;
	void	ResetPerRoundStats( void ) OVERRIDE;

	// cjd @add
	void	PlayVocalization( const char *pszVocalizationName, bool bRadio = false );
	// cjd end

	// knockdown on hit
	void KnockDown( void );
	void KnockDownThink( void );

// In shared code.
public:
	// IRnLPlayerAnimState overrides.
	CWeaponRnLBase* RnLAnim_GetActiveWeapon() OVERRIDE;
	bool CanMove() OVERRIDE;

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

	void FireBulletImmediately(RnLBulletInfo_t& bullet );
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

	CUtlVector<RnLBulletInfo_t> m_aBullets;
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
