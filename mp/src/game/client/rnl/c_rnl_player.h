//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_RNL_PLAYER_H
#define C_RNL_PLAYER_H
#ifdef _WIN32
#pragma once
#endif


#include "rnl_playeranimstate.h"
#include "c_baseplayer.h"
#include "rnl_shareddefs.h"
#include "rnl_playerlocaldata.h"
#include "baseparticleentity.h"


int AliasToClassID( const char *alias );
const char *ClassIDToAlias( int id );
void ClampAngles( QAngle& angCur, QAngle& angBase, bool bClampPitch, bool bClampYaw, Vector2D& vExtents, int iClampType = AIM_LOCK_NONE, float flDelta = 1.0f );

class C_RnLPlayer : public C_BasePlayer, public IRnLPlayerAnimStateHelpers
{
public:
	DECLARE_CLASS( C_RnLPlayer, C_BasePlayer );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_RnLPlayer();
	~C_RnLPlayer();

	virtual void Spawn( void );
	virtual ShadowType_t ShadowCastType( void );
	static C_RnLPlayer* GetLocalRnLPlayer();

	virtual const QAngle& GetRenderAngles();
	virtual const Vector& GetRenderOrigin( void );
	virtual void UpdateClientSideAnimation();
	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void ItemPostFrame( void );

	virtual void TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr, CDmgAccumulator* pAccumulator = NULL) override;

	//RnL : MovementMod : Begin
	//MovementMod : 
	virtual RnLMovementPostures_t	GetMovementPosture(void) const;
	virtual void			SetMovementPosture(RnLMovementPostures_t iType);
	virtual RnLMovementPostures_t	GetMovementPostureFrom(void) const;
	virtual float			GetMovementPostureDuration(void) const;
	virtual void			SetMovementPostureDuration(float flTime);
	virtual QAngle			GetMovementPostureAngle(void) const;
	virtual void			SetMovementPostureAngle(QAngle angle);

	// adam: new stance changing method
	virtual float GetMovementPostureOffset(void) const;
	virtual void SetMovementPostureOffset(float flOffset);
	virtual float GetMovementPostureTarget(void) const;
	virtual void SetMovementPostureTarget(float fTarget);
	virtual float GetMovementPostureVelocity(void) const;
	virtual void SetMovementPostureVelocity(float fTarget);
	virtual bool IsPostureChanging(float flVelLimit = 0.01, float flDiffLimit = 0.01) const;

	//RnL : MovementMod : Begin
	//Moved
	virtual bool IsStanding(void) const;
	virtual bool IsStandingUp(void) const;

	virtual bool IsDucked(void) const;
	virtual bool IsDucking(void) const;

	virtual bool IsProne(void) const;
	virtual bool IsProning(void) const;

	//RnL : MovementMod : End
	// 
	//RnL : MovementMod : Begin
	virtual bool		ShouldDrawViewModel( void );
	virtual bool		OverrideCamera( void );

	virtual bool		OverrideThirdPersonCamera( void );
	virtual bool		GetThirdPersonOverride( CViewSetup *pSetup );
	virtual bool		GetThirdPersonOverride( Vector& pos, QAngle& angle, Vector2D& limits, int& lockToExtents );
	
	virtual bool		OverrideFirstPersonCamera( void );
	virtual bool		GetFirstPersonOverride( CViewSetup *pSetup );
	virtual bool		GetFirstPersonOverride( Vector& pos, QAngle& angle, Vector2D& limits, int& lockToExtents );

	virtual void		ResetPlayerView( const QAngle& angle );

	// Michael Lebson (from EP1)
	void				ClientThink( void );
	int GetPlayerTargetID( void ) { return m_iPlayerTargetID; }

	virtual void		GetStepSoundVelocities( float *velwalk, float *velrun );
	virtual void		SetStepSoundTime( stepsoundtimes_t iStepSoundTime, bool bWalking );
	virtual Vector		EyePosition( void );
	
	virtual QAngle		GetWeaponAngle( void );
	virtual void		SetWeaponAngle( const QAngle& angle );
	void AdjustWeaponAngle( const QAngle &angleOffset );
	void AdjustWeaponSway( const QAngle& offset );

	virtual int		GetSquadNumber();
	virtual void	SetSquadNumber( int iNum );

	virtual int		GetKitNumber();
	virtual void	SetKitNumber( int iNum );
	//RnL : MovementMod : End

	//Andrew; Freeaim stuff
	virtual void PreMouseMove( QAngle& inAngles );
	virtual void PostMouseMove( QAngle& outAngles );

	virtual void AdjustYaw( float& speed, QAngle& viewangles );
	virtual void AdjustPitch( float& speed, QAngle& viewangles );

	// Called when not in tactical mode. Allows view to be overriden for things like driving a tank.
	virtual void		OverrideView( CViewSetup *pSetup );
	//Andrew; End Freeaim stuff

	// knock down view vars
	float fRagdollViewTime;
	bool bLastRagdollView;

	virtual void CalcViewRoll( QAngle& eyeAngles ) OVERRIDE;
	virtual void CalcViewModelView( const Vector& eyeOrigin, const QAngle& eyeAngles);
	virtual void CalcPlayerView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	virtual void CalcInEyeCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	virtual void CalcVehicleView(IClientVehicle *pVehicle, Vector& eyeOrigin, QAngle& eyeAngles, float& zNear, float& zFar, float& fov );
// Called by shared code.
public:
	
	// IRnLPlayerAnimState overrides.
	virtual CWeaponRnLBase* RnLAnim_GetActiveWeapon();
	virtual bool CanMove();

	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );
	virtual bool	ShouldDraw();
	virtual int		InternalDrawModel( int flags );

	//RnL : MovementMod : Begin
	int		GetClimbheight( void ) { return m_iClimbheight; }

	int		GetWeaponPosture( void ) { return m_nWeaponPosture; }
	float	GetWeaponPostureDuration( void ) { return (gpGlobals->curtime - m_flWeaponPostureTime); }
	void	SetWeaponPosture( int iPosture ) { if( m_nWeaponPosture != iPosture ){ m_flWeaponPostureTime = gpGlobals->curtime; m_nWeaponPosture = iPosture; } }

	void	AdjustViewAngles( const QAngle &angleOffset );

	float	GetMaxStamina();
	float	GetStamina();
	void	SetStamina( float flStamina );
	void	SetSprintTimer( float flSprintTime );
	float	GetSprintTimer();
	void	SetRecoverTime( float flRecoverTime );
	float	GetRecoverTime();

	Vector	GetLeaningOffset( void ) { return m_vecLeanOffset.Get(); }
	void	SetLeaningOffset( Vector offset ) { m_vecLeanOffset = offset; }

	float	GetViewRollOffset( void );
	void	SetViewRollOffset( float newOffset );

	float	GetDamageBasedSpeedModifier( int iType );
	void	SetDamageBasedSpeedModifier( int iType, float val );
	void	AddDamageBasedSpeedModifier( int iType, float val );

	void SetMoraleLevel( int iLev );
	int GetMoraleLevel( void );
	void UpdatePlayerMorale( void );

	float	m_flNextSprintSound;
	float	GetNextSprintSoundTime( void ) { return m_flNextSprintSound; }
	void	SetNextSprintSoundTime( float flTime ) { m_flNextSprintSound = flTime; }

	void	PlayMovingWoundedSound();
	float	PlayStaminaSound( int iState );

	void CheckWeaponSwitch( void );
	void SelectItem( const char *pstr, int iSubType );

	bool	IsSprinting( void );
	bool	StartSprinting( void );
	bool	StopSprinting( void );

	void Deploy( QAngle angAngles, Vector Origin );
	void UnDeploy( void );
	bool IsDeployed( void ) const;

	bool IsWounded( void );
	//RnL : MovementMod : End

	CWeaponRnLBase *GetActiveRnLWeapon() const;

	C_BaseAnimating*	BecomeRagdollOnClient();
	IRagdoll*			GetRepresentativeRagdoll() const;

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

	void FireBulletImmediately( RnLBulletInfo_t& bullet );
	void RemoveAllAmmo( void );

	// cjd @add
	void			PlayVoice( const char *pszSound, int playerindex );
	void			PlayRadio( const char *pszRadio, int playerindex );
	// cjd end

//Variables
public:

	IRnLPlayerAnimState *m_PlayerAnimState;

	QAngle	m_angEyeAngles;
	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	CNetworkVar( int, m_iThrowGrenadeCounter );	// used to trigger grenade throw animations.
	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently

	CNetworkVar( int, m_iSquadNumber );
	CNetworkVar( int, m_iPreviousSquadNumber );
	CNetworkVar( int, m_iKitNumber );
	CNetworkVar( int, m_iPreviousKitNumber );
	CNetworkVar( float, m_flDeathViewTime );

	EHANDLE	m_hRagdoll;
	EHANDLE	m_hKnockDownRagdoll;

	//RnL : MovementMod : Begin

	//Our acctual movement posture
	int m_nMovementPosture;
	int m_nMovementPostureFrom;
	CNetworkQAngle(m_angMovementPostureAngle);

	QAngle m_angWeaponAngle;
	QAngle m_angFreeLookAngle;
	QAngle m_angWeaponSway;
	bool m_bInFreelook;
	bool m_bInFreeAim;
	float m_flPreviousMouseUpdateTime;
	float m_flFreeLookResyncTime;

	// Climbing height
	CNetworkVar( int, m_iClimbheight );
	

	CNetworkVar( float, m_flBodyHeight );
	//RnL : Andrew : Current Weapon Posture
	CNetworkVar( int, m_nWeaponPosture );
	
	CRnLPlayerLocalData	m_RnLLocal;
	CNetworkVector( m_vecLeanOffset );

	float	m_fAfterClimbDrawDelay;

	float	m_flWeaponPostureTime;

	Vector	m_vThirdPersonOverrideOffset;
	QAngle	m_aThirdPersonOverrideAngle;
	
	Vector	m_vFirstPersonOverrideOffset;
	QAngle	m_aFirstPersonOverrideAngle;
	//RnL : MovementMod : End

	CBaseCombatWeapon *m_pSwitchingToWeapon;

private:
	C_RnLPlayer( const C_RnLPlayer & );

	CUtlVector<RnLBulletInfo_t> m_aBullets;

	float m_flNextMoraleEffect;
	QAngle m_angMoraleEffect;
	int m_iPlayerTargetID;
	float m_flNextMovingWoundedSound;

	//Tony; spawn interp stuff.
	void ClientRespawn();
	bool m_bSpawnInterpCounter;
	bool m_bSpawnInterpCounterCache;

	
	friend class CSDKPrediction;
};


inline C_RnLPlayer* ToRnLPlayer( CBaseEntity *pPlayer )
{
	Assert( dynamic_cast< C_RnLPlayer* >( pPlayer ) != NULL );
	return static_cast< C_RnLPlayer* >( pPlayer );
}


#endif // C_RNL_PLAYER_H
