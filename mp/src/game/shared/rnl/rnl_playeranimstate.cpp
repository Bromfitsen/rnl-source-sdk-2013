//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "base_playeranimstate.h"
#include "tier0/vprof.h"
#include "animation.h"
#include "studio.h"
#include "apparent_velocity_helper.h"
#include "utldict.h"

#include "rnl_playeranimstate.h"
#include "weapon_rnl_base.h"
#include "weapon_rnl_basegrenade.h"
#include "weapon_rnlbasemachinegun.h"

#ifdef CLIENT_DLL
	#include "c_rnl_player.h"
	#include "bone_setup.h"
	#include "interpolatedvar.h"
	#include "input.h"
#else
	#include "rnl_player.h"
#endif


#define LEANSEQUENCE_LAYER		1

#define ANIM_TOPSPEED_WALK			65
#define ANIM_TOPSPEED_RUN			150
#define ANIM_TOPSPEED_SPRINT		250
#define ANIM_TOPSPEED_RUN_CROUCH	100
#define ANIM_TOPSPEED_PRONE			50


#define DEFAULT_IDLE_NAME "idle_upper_"
#define DEFAULT_CROUCH_IDLE_NAME "crouch_idle_upper_"
#define DEFAULT_CROUCH_WALK_NAME "crouch_walk_upper_"
#define DEFAULT_WALK_NAME "walk_upper_"
#define DEFAULT_RUN_NAME "run_upper_"
#define DEFAULT_SPRINT_NAME "sprint_upper_"
#define DEFAULT_PRONE_NAME "prone_upper_"
#define DEFAULT_PRONE_IDLE_NAME "prone_idle_upper_"

#define DEFAULT_IDLE_SIGHTED_NAME "idle_aim_sight_"
#define DEFAULT_IDLE_SHOULDER_NAME "idle_aim_shoulder_"
#define DEFAULT_IDLE_PRIMED_NAME "idle_aim_primed_"

#define DEFAULT_RUN_SIGHTED_NAME "run_aim_sight_"
#define DEFAULT_RUN_SHOULDER_NAME "run_aim_shoulder_"
#define DEFAULT_RUN_PRIMED_NAME "run_aim_primed_"

#define DEFAULT_WALK_SIGHTED_NAME "walk_aim_sight_"
#define DEFAULT_WALK_SHOULDER_NAME "walk_aim_shoulder_"
#define DEFAULT_WALK_PRIMED_NAME "walk_aim_primed_"

#define DEFAULT_CROUCH_WALK_SIGHTED_NAME "crouch_walk_aim_sight_"
#define DEFAULT_CROUCH_WALK_SHOULDER_NAME "crouch_walk_aim_shoulder_"
#define DEFAULT_CROUCH_WALK_PRIMED_NAME "crouch_walk_aim_primed_"

#define DEFAULT_CROUCH_IDLE_SIGHTED_NAME "crouch_idle_aim_sight_"
#define DEFAULT_CROUCH_IDLE_SHOULDER_NAME "crouch_idle_aim_shoulder_"
#define DEFAULT_CROUCH_IDLE_PRIMED_NAME "crouch_idle_aim_primed_"

/* unneeded with new shoulder/aight aims/shoots
#define DEFAULT_FIRE_IDLE_NAME "idle_shoot_"
#define DEFAULT_FIRE_CROUCH_NAME "crouch_idle_shoot_"
#define DEFAULT_FIRE_CROUCH_WALK_NAME "crouch_walk_shoot_"
#define DEFAULT_FIRE_WALK_NAME "walk_shoot_"
#define DEFAULT_FIRE_RUN_NAME "run_shoot_"
*/

#define DEFAULT_FIRE_SPRINT_NAME "sprint_shoot_"
#define DEFAULT_FIRE_PRONE_NAME "prone_shoot_"

#define DEFAULT_FIRE_IDLE_SIGHTED_NAME "idle_shoot_sight_"
#define DEFAULT_FIRE_IDLE_SHOULDER_NAME "idle_shoot_shoulder_"

#define DEFAULT_FIRE_RUN_SIGHTED_NAME "run_shoot_sight_"
#define DEFAULT_FIRE_RUN_SHOULDER_NAME "run_shoot_shoulder_"

#define DEFAULT_FIRE_WALK_SIGHTED_NAME "walk_shoot_sight_"
#define DEFAULT_FIRE_WALK_SHOULDER_NAME "walk_shoot_shoulder_"

#define DEFAULT_FIRE_CROUCH_WALK_SIGHTED_NAME "crouch_walk_shoot_sight_"
#define DEFAULT_FIRE_CROUCH_WALK_SHOULDER_NAME "crouch_walk_shoot_shoulder_"

#define DEFAULT_FIRE_CROUCH_IDLE_SIGHTED_NAME "crouch_idle_shoot_sight_"
#define DEFAULT_FIRE_CROUCH_IDLE_SHOULDER_NAME "crouch_idle_shoot_shoulder_"

#define DEFAULT_SECONDARY_NAME "secondary_"

#define POSTURESEQUENCE_LAYER	(AIMSEQUENCE_LAYER+NUM_AIMSEQUENCE_LAYERS)
#define POSTUREBLEND_LAYERS		(POSTURESEQUENCE_LAYER + 2)
#define FIRESEQUENCE_LAYER		(POSTUREBLEND_LAYERS+2)
#define RELOADSEQUENCE_LAYER	(FIRESEQUENCE_LAYER + 1)
#define GRENADESEQUENCE_LAYER	(RELOADSEQUENCE_LAYER + 1)
#define NUM_LAYERS_WANTED		(GRENADESEQUENCE_LAYER + 1)

// ------------------------------------------------------------------------------------------------ //
// CRnLPlayerAnimState declaration.
// ------------------------------------------------------------------------------------------------ //

class CRnLPlayerAnimState : public CBasePlayerAnimState, public IRnLPlayerAnimState
{
public:
	DECLARE_CLASS( CRnLPlayerAnimState, CBasePlayerAnimState );
	friend IRnLPlayerAnimState* CreatePlayerAnimState( CBaseAnimatingOverlay *pEntity, IRnLPlayerAnimStateHelpers *pHelpers, LegAnimType_t legAnimType, bool bUseAimSequences );

	CRnLPlayerAnimState();

	virtual void Update( float eyeYaw, float eyePitch );
	virtual void DoAnimationEvent( PlayerAnimEvent_t event, int nData );
	virtual bool IsThrowingGrenade();
	virtual int CalcAimLayerSequence( float *flCycle, float *flAimSequenceWeight, bool bForceIdle );
	virtual void ClearAnimationState();
	virtual bool CanThePlayerMove();
	virtual float GetCurrentMaxGroundSpeed();
	virtual Activity CalcMainActivity();
	virtual void DebugShowAnimState( int iStartLine );
	virtual void ComputeSequences( CStudioHdr *pStudioHdr );
	virtual void ClearAnimationLayers();
	virtual Activity TranslateActivity( Activity actDesired );
	
	void InitRnL( CBaseAnimatingOverlay *pPlayer, IRnLPlayerAnimStateHelpers *pHelpers, LegAnimType_t legAnimType, bool bUseAimSequences );
	
protected:

	void ComputePostureLayerSequence( CStudioHdr *pStudioHdr );
	int CalcPostureLayerSequence( );
	void ComputeLeaningLayer(void);
	
	void UpdatePostureSequenceLayers( float flCycle, int iFirstLayer, bool bForceIdle, 
		CSequenceTransitioner *pTransitioner, float flWeightScale );

	int CalcFireLayerSequence(PlayerAnimEvent_t event);
	void ComputeFireSequence( CStudioHdr *pStudioHdr );

	void ComputeReloadSequence( CStudioHdr * pStudioHdr);
	int CalcReloadLayerSequence( bool bMidReload );

	int CalcDrawHolsterLayerSequence( bool bDraw );

	//void ComputeProneTransitionSequence();
	int CalcProneTransitionSequence( void );
	
	//Non Layered transtions baby
	bool HandleProneTransition( void );
	bool HandleClimbing( void );
	bool HandleParachuting( void );

	//Andrew : And Yet More climbing
	int CalcClimbingSequence( void );

	int CalcBoltCycleSequence();

	int CalcBayonetSequence( PlayerAnimEvent_t event );

	bool IsOuterGrenadePrimed();
	void ComputeGrenadeSequence( CStudioHdr *pStudioHdr );
	int CalcGrenadePrimeSequence();
	int CalcGrenadeThrowSequence();
	int GetOuterGrenadeThrowCounter();

	const char* GetWeaponSuffix();
	int ProneTransition();

	void UpdateLayerSequenceGeneric( CStudioHdr *pStudioHdr, int iLayer, bool &bEnabled, float &flCurCycle, int &iSequence, bool bWaitAtEnd );

private:

	bool m_bProneTransitioning;
	float m_flProneTransitionCycle;
	int m_iProneTransitionSequence;

	// Aim sequence plays reload while this is on.
	bool m_bReloading;
	float m_flReloadCycle;
	int m_iReloadSequence;

	//Andrew : Are we climbing?
	float m_flClimbingCycle;
	int m_iClimbingSequence;

	float m_flParachutingCycle;
	int m_iParachuteSequence;
	
	// This is set to true if ANY animation is being played in the fire layer.
	bool m_bFiring;						// If this is on, then it'll continue the fire animation in the fire layer
										// until it completes.
	int m_iFireSequence;				// (For any sequences in the fire layer, including grenade throw).
	float m_flFireCycle;

	// These control grenade animations.
	bool m_bThrowingGrenade;
	bool m_bPrimingGrenade;
	float m_flGrenadeCycle;
	int m_iGrenadeSequence;
	int m_iLastThrowGrenadeCounter;	// used to detect when the guy threw the grenade.
	
	IRnLPlayerAnimStateHelpers *m_pHelpers;

	CSequenceTransitioner	m_PostureSequenceTransitioner;
	CSequenceTransitioner	m_IdlePostureSequenceTransitioner;
};


IRnLPlayerAnimState* CreatePlayerAnimState( CBaseAnimatingOverlay *pEntity, IRnLPlayerAnimStateHelpers *pHelpers, LegAnimType_t legAnimType, bool bUseAimSequences )
{
	CRnLPlayerAnimState *pRet = new CRnLPlayerAnimState;
	pRet->InitRnL( pEntity, pHelpers, legAnimType, bUseAimSequences );
	return pRet;
}

// ------------------------------------------------------------------------------------------------ //
// CRnLPlayerAnimState implementation.
// ------------------------------------------------------------------------------------------------ //

CRnLPlayerAnimState::CRnLPlayerAnimState()
{
	m_pOuter = NULL;
	m_bReloading = false;
	m_bFiring = false;
	m_bProneTransitioning = false;
	m_iClimbingSequence = 0;
	m_iParachuteSequence = 0;
}


void CRnLPlayerAnimState::InitRnL( CBaseAnimatingOverlay *pEntity, IRnLPlayerAnimStateHelpers *pHelpers, LegAnimType_t legAnimType, bool bUseAimSequences )
{
	CModAnimConfig config;
	config.m_flMaxBodyYawDegrees = 90;
	config.m_LegAnimType = legAnimType;
	config.m_bUseAimSequences = bUseAimSequences;
	m_bFiring = false;

	m_pHelpers = pHelpers;

	BaseClass::Init( pEntity, config );
}


void CRnLPlayerAnimState::ClearAnimationState()
{
	m_bFiring = false;
	m_bReloading = false;
	m_bProneTransitioning = false;
	m_bThrowingGrenade = m_bPrimingGrenade = false;
	m_iLastThrowGrenadeCounter = GetOuterGrenadeThrowCounter();
	m_iParachuteSequence = 0;
	
	BaseClass::ClearAnimationState();
}

Activity CRnLPlayerAnimState::TranslateActivity( Activity actDesired )
{
	CRnLPlayer* pPlayer = ToRnLPlayer( m_pOuter );
	
	if( !pPlayer )
		return actDesired;

	CWeaponRnLBase* pWeapon = pPlayer->GetActiveRnLWeapon();

	if( pWeapon && pWeapon->ShouldUseWeaponActivities() )
	{
		bool aBooleanForFun = false;
		return pWeapon->ActivityOverride( actDesired, &aBooleanForFun );
	}

	return actDesired; 
}

void CRnLPlayerAnimState::DoAnimationEvent( PlayerAnimEvent_t event, int nData  )
{
	//Assert( event != PLAYERANIMEVENT_THROW_GRENADE );

	if ( event == PLAYERANIMEVENT_FIRE_GUN_PRIMARY || 
		 event == PLAYERANIMEVENT_FIRE_GUN_SECONDARY ||
		 event == PLAYERANIMEVENT_THROW_GRENADE )
	{
		// Regardless of what we're doing in the fire layer, restart it.
		m_flFireCycle = 0;
		m_iFireSequence = CalcFireLayerSequence( event );
		m_bFiring = m_iFireSequence != -1;
	}
	else if ( event == PLAYERANIMEVENT_RELOAD )
	{
		m_iReloadSequence = CalcReloadLayerSequence( false );
		if ( m_iReloadSequence != -1 )
		{
			m_bReloading = true;
			m_flReloadCycle = 0.0f;
		}
	}
	else if ( event == PLAYERANIMEVENT_PRONE )
	{
		int iTemp = CalcProneTransitionSequence();
		if ( iTemp != -1 )
		{
			m_iProneTransitionSequence = iTemp;
			m_bProneTransitioning = true;
			m_flProneTransitionCycle = 0.0f;
		}
	}
	else if ( event == PLAYERANIMEVENT_PRONECANCEL )
	{
		int iTemp = CalcProneTransitionSequence();
		if ( iTemp != -1 )
		{
			m_iProneTransitionSequence = iTemp;
			m_bProneTransitioning = true;
			m_flProneTransitionCycle = 1.0f - m_flProneTransitionCycle;
		}
	}

	// player reloaded between clips
	else if ( event == PLAYERANIMEVENT_MIDRELOAD )
	{
		m_iReloadSequence = CalcReloadLayerSequence( true );
		if( m_iReloadSequence != -1 )
		{
			m_bReloading = true;
			m_flReloadCycle = 0.0f;
		}
	}
	// use reload layer to display bolt cycle - nuke
	else if ( event == PLAYERANIMEVENT_BOLTCYCLE )
	{
		m_iReloadSequence = CalcBoltCycleSequence();
		if( m_iReloadSequence != -1 )
		{
			m_bReloading = true;
			m_flReloadCycle = 0.0f;
		}
	}
	else if( event == PLAYERANIMEVENT_BAYONET_ON ||
		event == PLAYERANIMEVENT_BAYONET_OFF )
	{
		m_iReloadSequence = CalcBayonetSequence( event );
		if( m_iReloadSequence != -1 )
		{
			m_bReloading = true;
			m_flReloadCycle = 0.0f;
		}
	}
	else if( event == PLAYERANIMEVENT_HOLSTER )
	{
		m_iReloadSequence = CalcDrawHolsterLayerSequence( false );
		if( m_iReloadSequence != -1 )
		{
			m_bReloading = true;
			m_flReloadCycle = 0.0f;
		}
		else
		{
			m_bReloading = false;
			m_flReloadCycle = 0.0f;
		}
	}
	else if( event == PLAYERANIMEVENT_DRAW )
	{
		m_iReloadSequence = CalcDrawHolsterLayerSequence( true );
		if( m_iReloadSequence != -1 )
		{
			m_bReloading = true;
			m_flReloadCycle = 0.0f;
		}
		else
		{
			m_bReloading = false;
			m_flReloadCycle = 0.0f;
		}
	}
	else if ( event == PLAYERANIMEVENT_CLIMB )
	{
		m_iClimbingSequence = CalcClimbingSequence();
		m_flClimbingCycle = 0.0f;
	}
	else if ( event == PLAYERANIMEVENT_STARTPARACHUTING )
	{
		m_iParachuteSequence = CalcSequenceIndex( "parachute_idle" );
		m_flParachutingCycle = 0.0f;
	}
	else if ( event == PLAYERANIMEVENT_ENDPARACHUTING )
	{
		m_iParachuteSequence = CalcSequenceIndex( "ragdoll" );
		m_flParachutingCycle = 0.0f;
	}
	else if ( event == PLAYERANIMEVENT_ROLLLEFT )
	{
		m_iParachuteSequence = CalcSequenceIndex( "prone_roll_left" );
		m_flParachutingCycle = 0.0f;
	}
	else if ( event == PLAYERANIMEVENT_ROLLRIGHT )
	{
		m_iParachuteSequence = CalcSequenceIndex( "prone_roll_right" );
		m_flParachutingCycle = 0.0f;
	}
	else if( event == PLAYERANIMEVENT_JUMP )
	{
	}
	else
	{
		Assert( !"CRnLPlayerAnimState::DoAnimationEvent" );
	}
}


float g_flThrowGrenadeFraction = 0.25;
bool CRnLPlayerAnimState::IsThrowingGrenade()
{
	if ( m_bThrowingGrenade )
	{
		// An animation event would be more appropriate here.
		return m_flGrenadeCycle < g_flThrowGrenadeFraction;
	}
	else
	{
		bool bThrowPending = (m_iLastThrowGrenadeCounter != GetOuterGrenadeThrowCounter());
		return bThrowPending || IsOuterGrenadePrimed();
	}
}


int CRnLPlayerAnimState::CalcReloadLayerSequence( bool bMidReload )
{
	// make the player aim when firing, to line up the animations and make them look
	//  smooth
	m_flLastTurnTime = gpGlobals->curtime - 2.0f;


	const char *pSuffix = GetWeaponSuffix();
	if ( !pSuffix )
		return -1;

	CWeaponRnLBase *pWeapon = m_pHelpers->RnLAnim_GetActiveWeapon();
	if ( !pWeapon )
		return -1;

	int iReloadSequence = -1;

	if( !pWeapon->ShouldUseWeaponActivities() )
	{

		// First, look for reload_<weapon name>.
		char szName[512];
		if( bMidReload )
		{
			if( GetCurrentMainSequenceActivity() == ACT_PRONEIDLE )
				Q_snprintf( szName, sizeof( szName ), "prone_midreload_%s", pSuffix );
			else
				Q_snprintf( szName, sizeof( szName ), "midreload_%s", pSuffix );
		}
		else
		{
			if( GetCurrentMainSequenceActivity() == ACT_PRONEIDLE )
				Q_snprintf( szName, sizeof( szName ), "prone_reload_%s", pSuffix );
			else
				Q_snprintf( szName, sizeof( szName ), "reload_%s", pSuffix );
		}

		iReloadSequence = m_pOuter->LookupSequence( szName );
	}
	else
	{
		if( GetCurrentMainSequenceActivity() == ACT_PRONEIDLE )
			iReloadSequence = SelectWeightedSequence( TranslateActivity( ACT_GESTURE_RELOAD ) );
		else
			iReloadSequence = SelectWeightedSequence( TranslateActivity( ACT_DOD_RELOAD_PRONE ) );
	}

	if ( iReloadSequence != -1 )
		return iReloadSequence;
	
	// Fall back to mp40 reload sequence.
	iReloadSequence = CalcSequenceIndex( "reload_mp40" );
	
	if ( iReloadSequence > 0 )
		return iReloadSequence;


	return -1;
}

int CRnLPlayerAnimState::CalcDrawHolsterLayerSequence( bool bDraw )
{
	// make the player aim when drawing or holstering, to line up the animations and make them look
	//  smooth
	m_flLastTurnTime = gpGlobals->curtime - 2.0f;


	const char *pSuffix = GetWeaponSuffix();
	if ( !pSuffix )
		return -1;

	CWeaponRnLBase *pWeapon = m_pHelpers->RnLAnim_GetActiveWeapon();
	if ( !pWeapon )
		return -1;

	// First, look for reload_<weapon name>.
	char szName[512];
	if( bDraw )
	{
		if( GetCurrentMainSequenceActivity() == ACT_PRONEIDLE )
			Q_snprintf( szName, sizeof( szName ), "prone_draw_%s", pSuffix );
		else
			Q_snprintf( szName, sizeof( szName ), "draw_%s", pSuffix );
	}
	else
	{
		if( GetCurrentMainSequenceActivity() == ACT_PRONEIDLE )
			Q_snprintf( szName, sizeof( szName ), "prone_holster_%s", pSuffix );
		else
			Q_snprintf( szName, sizeof( szName ), "holster_%s", pSuffix );
	}

	return m_pOuter->LookupSequence( szName );
}

// calculates the prone transition to use
int CRnLPlayerAnimState::CalcProneTransitionSequence()
{
	CRnLPlayer *pPlayer = dynamic_cast<CRnLPlayer*>( m_pOuter );
	if ( !pPlayer )
		return -1;

	if( pPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE_DIVE )
		return CalcSequenceIndex( "prone_dive" );

	if ( pPlayer->GetMovementPosture() == MOVEMENT_POSTURE_PRONE) {

		if ( pPlayer->GetMovementPostureFrom() == MOVEMENT_POSTURE_CROUCH )
			return CalcSequenceIndex( "ProneDown_Crouch" );
		else
			return CalcSequenceIndex( "ProneDown_Stand" );

	}
	// else, they are getting up from prone
	else if ( pPlayer->GetMovementPostureFrom() == MOVEMENT_POSTURE_PRONE ) {

		if ( pPlayer->GetMovementPosture() == MOVEMENT_POSTURE_CROUCH )
			return CalcSequenceIndex( "ProneUp_Crouch" );
		else
			return CalcSequenceIndex( "ProneUp_Stand" );
	}

	return -1;
}

//Andrew : Do stuff for climbing!
int CRnLPlayerAnimState::CalcClimbingSequence( void )
{
	CRnLPlayer *pPlayer = ToRnLPlayer( m_pOuter );

	if( !pPlayer )
	{
		return CalcSequenceIndex( "climb16_m1" );
	}

	//I use the mid point height + 8 and height - 8 so it doesn't look to bad
	//16 units: <24
	//32 units: >=24 and <40
	//48 units: >=40 and <56
	//64 units: >=56 and <72
	//80 units: >=72 and <88
	//96 units: >=86
	// they are going into prone

	int iHeight = pPlayer->GetClimbheight();
	if( iHeight < 24 )
	{
		return CalcSequenceIndex( "climb16_m1" );
		//return ACT_CLIMB_LOWEST;
	}
	else if( iHeight >= 24 && iHeight < 40 )
	{
		return CalcSequenceIndex( "climb32_m1" );
		//return ACT_CLIMB_LOW;
	}
	else if( iHeight >= 40 && iHeight < 56 )
	{
		return CalcSequenceIndex( "climb48_m1" );
		//return ACT_CLIMB_MIDDLE_LOW;
	}
	else if( iHeight >= 56 && iHeight < 72 )
	{
		return CalcSequenceIndex( "climb64_m1" );
		//return ACT_CLIMB_MIDDLE_HIGH;
	}
	else if( iHeight >= 72 && iHeight < 88 )
	{
		return CalcSequenceIndex( "climb80_m1" );
		//return ACT_CLIMB_HIGH;
	}
	else
	{
		return CalcSequenceIndex( "climb96_m1" );
		//return ACT_CLIMB_HIGHEST;
	}
}

int CRnLPlayerAnimState::CalcBoltCycleSequence()
{
	const char *pSuffix = GetWeaponSuffix();
	if ( !pSuffix )
		return -1;

	// check player has an active weapon
	CWeaponRnLBase *pWeapon = m_pHelpers->RnLAnim_GetActiveWeapon();
	if ( !pWeapon )
		return -1;

	// First, look for reload_<weapon name>.
	char szName[512];
	Q_snprintf( szName, sizeof( szName ), "boltcycle_%s", pSuffix );

	int iBoltCycleSequence = m_pOuter->LookupSequence( szName );
	if ( iBoltCycleSequence != -1 )
		return iBoltCycleSequence;

	// Fall back to k98 boltcycle
	iBoltCycleSequence = CalcSequenceIndex( "boltcycle_k98" );

	if ( iBoltCycleSequence > 0 )
		return iBoltCycleSequence;

	return -1;
}

//OMG ROBBLES
int CRnLPlayerAnimState::CalcBayonetSequence( PlayerAnimEvent_t event )
{
	const char *pSuffix = GetWeaponSuffix();
	if ( !pSuffix )
		return -1;

	// check player has an active weapon
	CWeaponRnLBase *pWeapon = m_pHelpers->RnLAnim_GetActiveWeapon();
	if ( !pWeapon )
		return -1;

	// First, look for reload_<weapon name>.
	char szName[512];
	if( event == PLAYERANIMEVENT_BAYONET_ON )
	{
		Q_snprintf( szName, sizeof( szName ), "bayonet_on_%s", pSuffix );
	}
	else
	{
		Q_snprintf( szName, sizeof( szName ), "bayonet_off_%s", pSuffix );
	}

	int iBoltCycleSequence = m_pOuter->LookupSequence( szName );
	if ( iBoltCycleSequence != -1 )
		return iBoltCycleSequence;

	// Fall back to k98 boltcycle
	iBoltCycleSequence = CalcSequenceIndex( "bayonet_on_k98" );

	if ( iBoltCycleSequence > 0 )
		return iBoltCycleSequence;

	return -1;
}


#ifdef CLIENT_DLL
	void CRnLPlayerAnimState::UpdateLayerSequenceGeneric( CStudioHdr *pStudioHdr, int iLayer, bool &bEnabled, float &flCurCycle, int &iSequence, bool bWaitAtEnd )
	{
		if ( !bEnabled )
			return;

		// Increment the fire sequence's cycle.
		flCurCycle += m_pOuter->GetSequenceCycleRate( pStudioHdr, iSequence ) * gpGlobals->frametime;
		if ( flCurCycle > 1 )
		{
			if ( bWaitAtEnd )
			{
				flCurCycle = 1;
			}
			else
			{
				// Not firing anymore.
				bEnabled = false;
				iSequence = 0;
				return;
			}
		}

		// Now dump the state into its animation layer.
		C_AnimationLayer *pLayer = m_pOuter->GetAnimOverlay( iLayer );

		pLayer->m_flCycle = flCurCycle;
		pLayer->m_nSequence = iSequence;

		pLayer->m_flPlaybackRate = 1.0;
		pLayer->m_flWeight = 1.0f;
		pLayer->m_nOrder = iLayer;
	}
#endif



bool CRnLPlayerAnimState::IsOuterGrenadePrimed()
{
	CBaseCombatCharacter *pChar = m_pOuter->MyCombatCharacterPointer();
	if ( pChar )
	{
		CBaseRnLGrenade *pGren = dynamic_cast<CBaseRnLGrenade*>( pChar->GetActiveWeapon() );
		return pGren && pGren->IsGrenadePrimed();
	}
	else
	{
		return NULL;
	}
}


void CRnLPlayerAnimState::ComputeGrenadeSequence( CStudioHdr *pStudioHdr )
{
#ifdef CLIENT_DLL
	if ( m_bThrowingGrenade )
	{
		UpdateLayerSequenceGeneric( pStudioHdr, GRENADESEQUENCE_LAYER, m_bThrowingGrenade, m_flGrenadeCycle, m_iGrenadeSequence, false );
	}
	else
	{
		// Priming the grenade isn't an event.. we just watch the player for it.
		// Also play the prime animation first if he wants to throw the grenade.
		bool bThrowPending = (m_iLastThrowGrenadeCounter != GetOuterGrenadeThrowCounter());
		if ( IsOuterGrenadePrimed() || bThrowPending )
		{
			if ( !m_bPrimingGrenade )
			{
				// If this guy just popped into our PVS, and he's got his grenade primed, then
				// let's assume that it's all the way primed rather than playing the prime
				// animation from the start.
				if ( TimeSinceLastAnimationStateClear() < 0.4f )
					m_flGrenadeCycle = 1;
				else
					m_flGrenadeCycle = 0;
					
				m_iGrenadeSequence = CalcGrenadePrimeSequence();
			}

			m_bPrimingGrenade = true;
			UpdateLayerSequenceGeneric( pStudioHdr, GRENADESEQUENCE_LAYER, m_bPrimingGrenade, m_flGrenadeCycle, m_iGrenadeSequence, true );
			
			// If we're waiting to throw and we're done playing the prime animation...
			if ( bThrowPending && m_flGrenadeCycle == 1 )
			{
				m_iLastThrowGrenadeCounter = GetOuterGrenadeThrowCounter();

				// Now play the throw animation.
				m_iGrenadeSequence = CalcGrenadeThrowSequence();
				if ( m_iGrenadeSequence != -1 )
				{
					// Configure to start playing 
					m_bThrowingGrenade = true;
					m_bPrimingGrenade = false;
					m_flGrenadeCycle = 0;
				}
			}
		}
		else
		{
			m_bPrimingGrenade = false;
		}
	}
#endif
}


int CRnLPlayerAnimState::CalcGrenadePrimeSequence()
{
	return CalcSequenceIndex( "idle_shoot_gren1" );
}


int CRnLPlayerAnimState::CalcGrenadeThrowSequence()
{
	return CalcSequenceIndex( "idle_shoot_gren2" );
}


int CRnLPlayerAnimState::GetOuterGrenadeThrowCounter()
{
	CRnLPlayer *pPlayer = dynamic_cast<CRnLPlayer*>( m_pOuter );
	if ( pPlayer )
		return pPlayer->m_iThrowGrenadeCounter;
	else
		return 0;
}


void CRnLPlayerAnimState::ComputeReloadSequence( CStudioHdr *pStudioHdr )
{
#ifdef CLIENT_DLL
	UpdateLayerSequenceGeneric( pStudioHdr, RELOADSEQUENCE_LAYER, m_bReloading, m_flReloadCycle, m_iReloadSequence, false );
#else
	// Server doesn't bother with different fire sequences.
#endif
}

//void CRnLPlayerAnimState::ComputeProneTransitionSequence()
//{
//#ifdef CLIENT_DLL
//	UpdateLayerSequenceGeneric( PRONETRANSITION_LAYER, m_bProneTransitioning, m_flProneTransitionCycle, m_iProneTransitionSequence, false );
//#else
//	// Server doesn't bother with different fire sequences.
//#endif
//}

int CRnLPlayerAnimState::CalcAimLayerSequence( float *flCycle, float *flAimSequenceWeight, bool bForceIdle )
{
	const char *pSuffix = GetWeaponSuffix();
	if ( !pSuffix )
		return 0;

	CRnLPlayer *pPlayer = ToRnLPlayer( m_pOuter );
	if( !pPlayer )
		return 0;

	if ( bForceIdle )
	{
		switch ( GetCurrentMainSequenceActivity() )
		{
			case ACT_CROUCHIDLE:
				return CalcSequenceIndex( "%s%s", DEFAULT_CROUCH_IDLE_NAME, pSuffix );

			case ACT_PRONEIDLE:
			case ACT_PRONE:
				return CalcSequenceIndex( "%s%s", DEFAULT_PRONE_IDLE_NAME, pSuffix );

			default:
				return CalcSequenceIndex( "%s%s", DEFAULT_IDLE_NAME, pSuffix );
		}
	}

	else
	{
		switch ( GetCurrentMainSequenceActivity() )
		{
			case ACT_PRONE:
				return -1;//CalcSequenceIndex( "%s%s", DEFAULT_PRONE_NAME, pSuffix );

			case ACT_PRONEIDLE:
				return CalcSequenceIndex( "%s%s", DEFAULT_PRONE_IDLE_NAME, pSuffix );

			case ACT_RUN:
				return CalcSequenceIndex( "%s%s", DEFAULT_RUN_NAME, pSuffix );

			case ACT_SPRINT:
				return CalcSequenceIndex( "%s%s", DEFAULT_SPRINT_NAME, pSuffix );

			case ACT_WALK:
				return CalcSequenceIndex( "%s%s", DEFAULT_WALK_NAME, pSuffix ); //run or walk?

			case ACT_RUNTOIDLE:
			case ACT_IDLETORUN:
				return CalcSequenceIndex( "%s%s", DEFAULT_WALK_NAME, pSuffix );

			case ACT_CROUCHIDLE:
				return CalcSequenceIndex( "%s%s", DEFAULT_CROUCH_IDLE_NAME, pSuffix );

			case ACT_WALK_CROUCH:
				return CalcSequenceIndex( "%s%s", DEFAULT_CROUCH_WALK_NAME, pSuffix );

			case ACT_IDLE:
			default:
				return CalcSequenceIndex( "%s%s", DEFAULT_IDLE_NAME, pSuffix );
		}
	}
}

void CRnLPlayerAnimState::UpdatePostureSequenceLayers(
	float flCycle,
	int iFirstLayer,
	bool bForceIdle,
	CSequenceTransitioner *pTransitioner,
	float flWeightScale
	)
{
	float flPostureSequenceWeight = 1;
	int iPostureSequence = CalcPostureLayerSequence();
	if ( iPostureSequence == -1 )
		return;

	// Feed the current state of the animation parameters to the sequence transitioner.
	// It will hand back either 1 or 2 animations in the queue to set, depending on whether
	// it's transitioning or not. We just dump those into the animation layers.
	pTransitioner->CheckForSequenceChange(
		m_pOuter->GetModelPtr(),
		iPostureSequence,
		false,	// don't force transitions on the same anim
		true	// yes, interpolate when transitioning
		);

	pTransitioner->UpdateCurrent(
		m_pOuter->GetModelPtr(),
		iPostureSequence,
		flCycle,
		GetOuter()->GetPlaybackRate(),
		gpGlobals->curtime
		);

	CAnimationLayer *pDest0 = m_pOuter->GetAnimOverlay( iFirstLayer );
	CAnimationLayer *pDest1 = m_pOuter->GetAnimOverlay( iFirstLayer+1 );

	if ( pTransitioner->m_animationQueue.Count() == 1 )
	{
		// If only 1 animation, then blend it in fully.
		CAnimationLayer *pSource0 = &pTransitioner->m_animationQueue[0];
		*pDest0 = *pSource0;
		
		pDest0->m_flWeight = 1;
		pDest1->m_flWeight = 0;
		pDest0->m_nOrder = iFirstLayer;

#ifndef CLIENT_DLL
		pDest0->m_fFlags |= ANIM_LAYER_ACTIVE;
#endif
	}
	else if ( pTransitioner->m_animationQueue.Count() >= 2 )
	{
		// The first one should be fading out. Fade in the new one inversely.
		CAnimationLayer *pSource0 = &pTransitioner->m_animationQueue[0];
		CAnimationLayer *pSource1 = &pTransitioner->m_animationQueue[1];

		*pDest0 = *pSource0;
		*pDest1 = *pSource1;
		Assert( pDest0->m_flWeight >= 0.0f && pDest0->m_flWeight <= 1.0f );
		pDest1->m_flWeight = 1 - pDest0->m_flWeight;	// This layer just mirrors the other layer's weight (one fades in while the other fades out).

		pDest0->m_nOrder = iFirstLayer;
		pDest1->m_nOrder = iFirstLayer+1;

#ifndef CLIENT_DLL
		pDest0->m_fFlags |= ANIM_LAYER_ACTIVE;
		pDest1->m_fFlags |= ANIM_LAYER_ACTIVE;
#endif
	}
	
	pDest0->m_flWeight *= flWeightScale * flPostureSequenceWeight;
	pDest0->m_flWeight = clamp( pDest0->m_flWeight, 0.0f, 1.0f );

	pDest1->m_flWeight *= flWeightScale * flPostureSequenceWeight;
	pDest1->m_flWeight = clamp( pDest1->m_flWeight, 0.0f, 1.0f );

	pDest0->m_flCycle = pDest1->m_flCycle = flCycle;
}

void CRnLPlayerAnimState::ComputePostureLayerSequence( CStudioHdr *pStudioHdr )
{
	// Synchronize the lower and upper body cycles.
	float flCycle = m_pOuter->GetCycle();

	UpdatePostureSequenceLayers( flCycle, POSTURESEQUENCE_LAYER, true, &m_IdlePostureSequenceTransitioner, 1 );
	// Figure out the new cycle time.
	UpdatePostureSequenceLayers( flCycle, POSTUREBLEND_LAYERS, false, &m_PostureSequenceTransitioner, 1 );
}

int CRnLPlayerAnimState::CalcPostureLayerSequence()
{
	const char *pSuffix = GetWeaponSuffix();
	if ( !pSuffix )
		return 0;

	CRnLPlayer *pPlayer = ToRnLPlayer( m_pOuter );
	if( !pPlayer )
		return -1;

	// bAiming based off yaw and/or pitch
	bool bAiming = false;

	if( m_flLastTurnTime > gpGlobals->curtime - 3.0f 
		|| ( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS) )
		bAiming = true;

	if( !bAiming )
	{
		return -1;
	}

	switch ( GetCurrentMainSequenceActivity() )
	{
		case ACT_PRONE:
			return -1;//CalcSequenceIndex( "%s%s", DEFAULT_PRONE_NAME, pSuffix );

		case ACT_PRONEIDLE:
			return -1;

		case ACT_RUN:
			if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS )
			{
				return CalcSequenceIndex( "%s%s", DEFAULT_RUN_SIGHTED_NAME, pSuffix ); //run or walk?
			}
			else if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_THROWING_GRENADE )
				return CalcSequenceIndex( "%s%s", DEFAULT_RUN_PRIMED_NAME, pSuffix ); //run or walk?
			else
			{
				return CalcSequenceIndex( "%s%s", DEFAULT_RUN_SHOULDER_NAME, pSuffix ); //run or walk?
			}

		case ACT_SPRINT:
			return -1;

		case ACT_WALK:
			if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
			{
				return CalcSequenceIndex( "%s%s", DEFAULT_WALK_SIGHTED_NAME, pSuffix ); //run or walk?
			}
			else if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_THROWING_GRENADE )
				return CalcSequenceIndex( "%s%s", DEFAULT_WALK_PRIMED_NAME, pSuffix ); //run or walk?
			else
			{
				return CalcSequenceIndex( "%s%s", DEFAULT_WALK_SHOULDER_NAME, pSuffix ); //run or walk?
			}

		case ACT_RUNTOIDLE:
		case ACT_IDLETORUN:
			if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
			{
				return CalcSequenceIndex( "%s%s", DEFAULT_IDLE_SIGHTED_NAME, pSuffix ); //run or walk?
			}
			else if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_THROWING_GRENADE )
				return CalcSequenceIndex( "%s%s", DEFAULT_IDLE_PRIMED_NAME, pSuffix ); //run or walk?
			else
			{
				return CalcSequenceIndex( "%s%s", DEFAULT_IDLE_SHOULDER_NAME, pSuffix ); //run or walk?
			}

		case ACT_CROUCHIDLE:
			if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
				return CalcSequenceIndex( "%s%s", DEFAULT_CROUCH_IDLE_SIGHTED_NAME, pSuffix ); //run or walk?

			else if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_THROWING_GRENADE )
				return CalcSequenceIndex( "%s%s", DEFAULT_CROUCH_IDLE_PRIMED_NAME, pSuffix ); //run or walk?
			
			else
				return CalcSequenceIndex( "%s%s", DEFAULT_CROUCH_IDLE_SHOULDER_NAME, pSuffix ); //run or walk?

		case ACT_WALK_CROUCH:
			if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
			{
				return CalcSequenceIndex( "%s%s", DEFAULT_CROUCH_WALK_SIGHTED_NAME, pSuffix ); //run or walk?
			}
			else if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_THROWING_GRENADE )
				return CalcSequenceIndex( "%s%s", DEFAULT_CROUCH_WALK_PRIMED_NAME, pSuffix ); //run or walk?
			else
			{
				return CalcSequenceIndex( "%s%s", DEFAULT_CROUCH_WALK_SHOULDER_NAME, pSuffix ); //run or walk?
			}

		case ACT_IDLE:
		default:
			if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
			{
				return CalcSequenceIndex( "%s%s", DEFAULT_IDLE_SIGHTED_NAME, pSuffix ); //run or walk?
			}
			else if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_THROWING_GRENADE )
				return CalcSequenceIndex( "%s%s", DEFAULT_IDLE_PRIMED_NAME, pSuffix ); //run or walk?
			else
			{
				return CalcSequenceIndex( "%s%s", DEFAULT_IDLE_SHOULDER_NAME, pSuffix ); //run or walk?
			}
	}
}

const char* CRnLPlayerAnimState::GetWeaponSuffix()
{
	// Figure out the weapon suffix.
	CWeaponRnLBase *pWeapon = m_pHelpers->RnLAnim_GetActiveWeapon();
	if ( !pWeapon )
		return "Pistol";

	//suffix refers to the anim extension (ie the unique weapon name) - nuke
	const char *pSuffix = pWeapon->GetRnLWpnData().m_szAnimExtension;
	
	return pSuffix;
}


int CRnLPlayerAnimState::CalcFireLayerSequence(PlayerAnimEvent_t event)
{
	// Figure out the weapon suffix.
	CWeaponRnLBase *pWeapon = m_pHelpers->RnLAnim_GetActiveWeapon();
	if ( !pWeapon )
		return 0;

	CRnLPlayer *pPlayer = ToRnLPlayer( m_pOuter );
	if( !pPlayer )
		return 0;

	if( pWeapon->ShouldUseWeaponActivities() )
	{
		if( GetCurrentMainSequenceActivity() == ACT_PRONEIDLE )
			return SelectWeightedSequence( TranslateActivity( ACT_DOD_PRIMARYATTACK_PRONE ) );
		else
			return SelectWeightedSequence( TranslateActivity( ACT_GESTURE_RANGE_ATTACK1 ) );
	}
	else
	{
		const char *pSuffix = GetWeaponSuffix();
		if ( !pSuffix )
			return 0;

		if( event == PLAYERANIMEVENT_FIRE_GUN_SECONDARY )
		{
			return CalcSequenceIndex( "%s%s", DEFAULT_SECONDARY_NAME, pSuffix );
		}
			
		// Don't rely on their weapon here because the player has usually switched to their 
		// pistol or rifle by the time the PLAYERANIMEVENT_THROW_GRENADE message gets to the client.
		if ( event == PLAYERANIMEVENT_THROW_GRENADE )
		{
			pSuffix = "frag"; 
		}

		// make the player aim when firing, to line up the animations and make them look
		//  smooth
		m_flLastTurnTime = gpGlobals->curtime;

		switch ( GetCurrentMainSequenceActivity() )
		{
			case ACT_PLAYER_SPRINT_FIRE:
			case ACT_SPRINT:
				return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_SPRINT_NAME, pSuffix );

			case ACT_PLAYER_PRONE_FIRE:
			case ACT_PRONEIDLE:
				return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_PRONE_NAME, pSuffix );

			case ACT_PLAYER_RUN_FIRE:
			case ACT_RUN:
				if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
				{
					return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_RUN_SIGHTED_NAME, pSuffix );
				}
				else
				{
					return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_RUN_SHOULDER_NAME, pSuffix );
				}
			case ACT_PLAYER_WALK_FIRE:
			case ACT_WALK:
				if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
				{
					return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_WALK_SIGHTED_NAME, pSuffix ); //run or walk?
				}
				else
				{
					return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_WALK_SHOULDER_NAME, pSuffix ); //run or walk?
				}
			case ACT_PLAYER_CROUCH_FIRE:
			case ACT_CROUCHIDLE:
				if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
				{
					return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_CROUCH_IDLE_SIGHTED_NAME, pSuffix ); //run or walk?
				}
				else
				{
					return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_CROUCH_IDLE_SHOULDER_NAME, pSuffix ); //run or walk?
				}
			case ACT_PLAYER_CROUCH_WALK_FIRE:
			case ACT_WALK_CROUCH:
				if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
				{
					return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_CROUCH_WALK_SIGHTED_NAME, pSuffix ); //run or walk?
				}
				else
				{
					return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_CROUCH_WALK_SHOULDER_NAME, pSuffix ); //run or walk?
				}
			default:
			case ACT_PLAYER_IDLE_FIRE:
				if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
				{
					return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_IDLE_SIGHTED_NAME, pSuffix ); //run or walk?
				}
				else
				{
					return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_IDLE_SHOULDER_NAME, pSuffix ); //run or walk?
				}
		}
	}
}


bool CRnLPlayerAnimState::CanThePlayerMove()
{
	return m_pHelpers->CanMove();
}


float CRnLPlayerAnimState::GetCurrentMaxGroundSpeed()
{
	Activity currentActivity =  GetCurrentMainSequenceActivity();//m_pOuter->GetSequenceActivity( m_pOuter->GetSequence() );
	if ( currentActivity == ACT_WALK || currentActivity == ACT_IDLE )
		return ANIM_TOPSPEED_WALK;
	else if ( currentActivity == ACT_RUN )
		return ANIM_TOPSPEED_RUN;
	else if ( currentActivity == ACT_WALK_CROUCH )
		return ANIM_TOPSPEED_RUN_CROUCH;
	else if ( currentActivity == ACT_SPRINT )
		return ANIM_TOPSPEED_SPRINT;
	else if ( currentActivity == ACT_PRONE )
		return ANIM_TOPSPEED_PRONE;
	else
		return 0;
}

Activity CRnLPlayerAnimState::CalcMainActivity()
{
	float flOuterSpeed = GetOuterXYSpeed();

	Activity idealActivity = ACT_IDLE;

	CRnLPlayer *pPlayer = dynamic_cast<CRnLPlayer*>( m_pOuter );
	if( !pPlayer )
		return idealActivity;

	if( pPlayer->IsDeployed() )
	{
		if( pPlayer->GetMovementPosture() == MOVEMENT_POSTURE_DEPLOYED_PRONE )
			idealActivity = ACT_DOD_PRONE_DEPLOYED;
		else
			idealActivity = ACT_DOD_DEPLOYED;
	}
	else
	{
		//MovementMod
		//Crouching or going to crouch
		if ( pPlayer->IsDucked() )
		{
			/*if ( flOuterSpeed > ARBITRARY_CROUCH_RUN_SPEED )	Our models lack the RUN_CROUCH Activities...
				idealActivity = ACT_RUN_CROUCH;
			else*/ if ( flOuterSpeed > MOVING_MINIMUM_SPEED || 
					(pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS) )
				idealActivity = ACT_WALK_CROUCH;
			else
				idealActivity = ACT_CROUCHIDLE;
		}
		else if ( pPlayer->IsProne() )
		{
			if ( flOuterSpeed > MOVING_MINIMUM_SPEED )
				idealActivity = ACT_PRONE;
			else
				idealActivity = ACT_PRONEIDLE;
		}
		else
		{
			if ( flOuterSpeed > MOVING_MINIMUM_SPEED )
			{
				if( flOuterSpeed > ARBITRARY_SPRINT_SPEED )
					idealActivity = ACT_SPRINT;
				else if ( flOuterSpeed > ARBITRARY_RUN_SPEED )
					idealActivity = ACT_RUN;
				else
					idealActivity = ACT_WALK;
			}
			else
			{
				idealActivity = ACT_IDLE;
			}
		}
	}

	return idealActivity;
}


void CRnLPlayerAnimState::DebugShowAnimState( int iStartLine )
{
#ifdef CLIENT_DLL
	engine->Con_NPrintf( iStartLine++, "fire  : %s, cycle: %.2f\n", m_bFiring ? GetSequenceName( m_pOuter->GetModelPtr(), m_iFireSequence ) : "[not firing]", m_flFireCycle );
	engine->Con_NPrintf( iStartLine++, "reload: %s, cycle: %.2f\n", m_bReloading ? GetSequenceName( m_pOuter->GetModelPtr(), m_iReloadSequence ) : "[not reloading]", m_flReloadCycle );
	BaseClass::DebugShowAnimState( iStartLine );
#endif
}

void CRnLPlayerAnimState::Update( float eyeYaw, float eyePitch )
{
	if( ShouldUpdateAnimState() )
	{
		if( HandleParachuting() )
		{
			return;
		}
		else if( HandleClimbing() )
		{
			return;
		}
		else if( HandleProneTransition() )
		{
			return;
		}
	}

#ifdef CLIENT_DLL
	if( m_pOuter == C_BasePlayer::GetLocalPlayer() )
	{
		m_AnimConfig.m_flMaxBodyYawDegrees = 15;
		eyePitch = 0;
	}
#endif

	BaseClass::Update( eyeYaw, eyePitch );
}


void CRnLPlayerAnimState::ComputeLeaningLayer(void)
{
	//Leaning : HAX OMG
	int iLeanSequence = CalcSequenceIndex("Upper_Lean");

#ifdef CLIENT_DLL
	if (m_pOuter == C_BasePlayer::GetLocalPlayer())
		iLeanSequence = CalcSequenceIndex("fp_upper_Lean");
#endif

	// Synchronize the lower and upper body cycles.
	float flCycle = m_pOuter->GetCycle();

	// Now dump the state into its animation layer.
	CAnimationLayer* pLayer = m_pOuter->GetAnimOverlay(LEANSEQUENCE_LAYER);

	pLayer->m_nSequence = iLeanSequence;
	pLayer->m_flWeight = 1.0f;
	pLayer->m_nOrder = LEANSEQUENCE_LAYER;
	pLayer->m_flCycle = flCycle;
#ifndef CLIENT_DLL
	pLayer->m_fFlags |= ANIM_LAYER_ACTIVE;
#endif

	int iLeaning = GetOuter()->LookupPoseParameter("body_lean");
	if (iLeaning >= 0)
	{
		CRnLPlayer* pPlayer = ToRnLPlayer(m_pOuter);

#ifdef CLIENT_DLL
		if (pPlayer == CRnLPlayer::GetLocalRnLPlayer() && !input->CAM_IsThirdPerson())
		{
			iLeanSequence = CalcSequenceIndex("fp_upper_Lean");
		}
#endif
		Vector vecOffset(0, 0, 0);
		if (pPlayer)
			vecOffset = pPlayer->GetLeaningOffset();

		float flLean = -(vecOffset.y / 18.0f);
		SetOuterPoseParameter(iLeaning, flLean);
	}
}

void CRnLPlayerAnimState::ComputeSequences( CStudioHdr *pStudioHdr )
{
	CRnLPlayer* pPlayer = ToRnLPlayer( m_pOuter );
	
	if( !pPlayer )
		return;
	
	CWeaponRnLBase* pWeapon = pPlayer->GetActiveRnLWeapon();

	if( pWeapon &&  pWeapon->ShouldUseWeaponActivities() )
	{
		ComputeMainSequence();		// Lower body (walk/run/idle).
		ComputeLeaningLayer();
		UpdateInterpolators();		// The groundspeed interpolator uses the main sequence info.

		if( GetCurrentMainSequenceActivity() != ACT_SPRINT )
		{
			ComputeFireSequence( pStudioHdr );
			ComputeReloadSequence( pStudioHdr );
			//ComputeGrenadeSequence( pStudioHdr );
		}

		if( pPlayer->IsDeployed() && !pPlayer->IsProne() )
		{
			int body_height = pPlayer->LookupPoseParameter( "body_height" );
			if ( body_height >= 0 )
				SetOuterPoseParameter( body_height, pPlayer->m_flBodyHeight );
		}

	}
	else
	{
		BaseClass::ComputeSequences( pStudioHdr );

#ifdef CLIENT_DLL
		if( m_pOuter == C_BasePlayer::GetLocalPlayer() && !input->CAM_IsThirdPerson() )
			return;
#endif

		if( GetCurrentMainSequenceActivity() != ACT_SPRINT )
		{
			ComputePostureLayerSequence( pStudioHdr );
			ComputeFireSequence( pStudioHdr );
			ComputeReloadSequence( pStudioHdr );
			//ComputeGrenadeSequence( pStudioHdr );
		}
	}
}


void CRnLPlayerAnimState::ClearAnimationLayers()
{
	if ( !m_pOuter )
		return;

	m_pOuter->SetNumAnimOverlays( NUM_LAYERS_WANTED );
	for ( int i=0; i < m_pOuter->GetNumAnimOverlays(); i++ )
	{
		m_pOuter->GetAnimOverlay( i )->SetOrder( CBaseAnimatingOverlay::MAX_OVERLAYS );
#ifndef CLIENT_DLL
		m_pOuter->GetAnimOverlay( i )->m_fFlags = 0;
#endif
	}
}

void CRnLPlayerAnimState::ComputeFireSequence( CStudioHdr *pStudioHdr )
{
#ifdef CLIENT_DLL
	UpdateLayerSequenceGeneric( pStudioHdr, FIRESEQUENCE_LAYER, m_bFiring, m_flFireCycle, m_iFireSequence, false );
#else
	// Server doesn't bother with different fire sequences.
#endif
}

bool CRnLPlayerAnimState::HandleParachuting( void )
{
	if ( !m_pOuter )
		return false;

	CStudioHdr *pStudioHdr = m_pOuter->GetModelPtr();

	if( pStudioHdr == NULL )
		return false;
	
	CRnLPlayer *pPlayer = dynamic_cast<CRnLPlayer*>( m_pOuter );
	if( !pPlayer )
		return false;

	if( pPlayer->GetMovementPosture() != MOVEMENT_POSTURE_PARACHUTING &&
		pPlayer->GetMovementPosture() != MOVEMENT_POSTURE_PRONE_ROLL_LEFT &&
		pPlayer->GetMovementPosture() != MOVEMENT_POSTURE_PRONE_ROLL_RIGHT )
		return false;

	ClearAnimationLayers();
	// Increment the fire sequence's cycle.
	m_flParachutingCycle += m_pOuter->GetSequenceCycleRate( pStudioHdr, m_iParachuteSequence ) * gpGlobals->frametime;
	
	if ( m_flParachutingCycle > 1 )
	{
		m_flParachutingCycle = 1.0f;
	}

	m_pOuter->GetAnimOverlay( 0 )->m_nOrder = 0;
	m_pOuter->GetAnimOverlay( 0 )->m_flCycle = m_flParachutingCycle;
	m_pOuter->GetAnimOverlay( 0 )->m_nSequence = m_iParachuteSequence;
	m_pOuter->GetAnimOverlay( 0 )->m_flPlaybackRate = 1.0f;
	m_pOuter->GetAnimOverlay( 0 )->m_flWeight = 1.0f;
	return true;
}

bool CRnLPlayerAnimState::HandleClimbing( void )
{
	if ( !m_pOuter )
		return false;

	CRnLPlayer *pPlayer = dynamic_cast<CRnLPlayer*>( m_pOuter );
	if( !pPlayer )
		return false;

	if( pPlayer->GetMovementPosture() != MOVEMENT_POSTURE_CLIMBING && pPlayer->GetMovementPosture() != MOVEMENT_POSTURE_CLIMBING_TO_CROUCH )
		return false;

	CStudioHdr *pStudioHdr = m_pOuter->GetModelPtr();

	if( pStudioHdr == NULL )
		return false;
	
	ClearAnimationLayers();
	// Increment the fire sequence's cycle.
	m_flClimbingCycle += m_pOuter->GetSequenceCycleRate( pStudioHdr, m_iClimbingSequence ) * gpGlobals->frametime;

	if ( m_flClimbingCycle > 1.0f )
	{
		m_flClimbingCycle = 1.0f;
	}

	m_pOuter->GetAnimOverlay( 0 )->m_nOrder  = 0;
	m_pOuter->GetAnimOverlay( 0 )->m_flCycle = m_flClimbingCycle;
	m_pOuter->GetAnimOverlay( 0 )->m_nSequence = m_iClimbingSequence;
	m_pOuter->GetAnimOverlay( 0 )->m_flPlaybackRate = 1.0f;
	m_pOuter->GetAnimOverlay( 0 )->m_flWeight = 1.0f;
	return true;
}

bool CRnLPlayerAnimState::HandleProneTransition( void )
{
	if ( !m_pOuter )
		return false;
	
	if( !(m_bProneTransitioning ) )
		return false;

	CStudioHdr *pStudioHdr = m_pOuter->GetModelPtr();

	if( pStudioHdr == NULL )
		return false;

	ClearAnimationLayers();

	// Increment the fire sequence's cycle.
	m_flProneTransitionCycle += m_pOuter->GetSequenceCycleRate( pStudioHdr, m_iProneTransitionSequence ) * gpGlobals->frametime;
	
	if ( m_flProneTransitionCycle > 1 )
	{
		m_flProneTransitionCycle = 1;
		m_bProneTransitioning = false;
	}

	m_pOuter->GetAnimOverlay( 0 )->m_nOrder  = 0;
	m_pOuter->GetAnimOverlay( 0 )->m_flCycle = m_flProneTransitionCycle;
	m_pOuter->GetAnimOverlay( 0 )->m_nSequence = m_iProneTransitionSequence;
	m_pOuter->GetAnimOverlay( 0 )->m_flPlaybackRate = 1.0f;
	m_pOuter->GetAnimOverlay( 0 )->m_flWeight = 1.0f;
	return true;
}