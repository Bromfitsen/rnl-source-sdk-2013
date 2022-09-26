//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"

#ifdef CLIENT_DLL
	
	#include "c_rnl_player.h"
	#include "prediction.h"

#else

	#include "rnl_player.h"
	#include "team.h"
	#include "collisionutils.h"	
	#include "ilagcompensationmanager.h"

#endif

#include "weapon_rnl_base.h"
#include "gamevars_shared.h"
#include "takedamageinfo.h"
#include "effect_dispatch_data.h"
#include "engine/ivdebugoverlay.h"
#include "ammodef.h"
#include "decals.h"
#include "util_shared.h"
#include "rnl_ammodef.h"
#include "weapon_rnlbasemachinegun.h"
#include "obstacle_pushaway.h"

// ----------------------------------------------------------------------------- //
// Class Global functions.
// ----------------------------------------------------------------------------- //

//--------------------------------------------------------------------------------------------------------
static const char * s_ClassAliasInfo[] = 
{
	"invalid",		// WEAPON_NONE
	"infantry",
	"support",
	NULL,
};

//--------------------------------------------------------------------------------------------------------
//
// Given an alias, return the associated weapon ID
//
int AliasToClassID( const char *alias )
{
	if (alias)
	{
		for( int i=0; s_ClassAliasInfo[i] != NULL; ++i )
			if (!Q_stricmp( s_ClassAliasInfo[i], alias ))
				return i;
	}

	return RNL_CLASS_INVALID;
}

//--------------------------------------------------------------------------------------------------------
//
// Given a weapon ID, return its alias
//
const char *ClassIDToAlias( int id )
{
	if ( (id >= RNL_CLASS_MAX) || (id < 0) )
		return NULL;

	return s_ClassAliasInfo[id];
}

ConVar mp_legshotimpedance( "mp_legshotimpedance", "20.0", FCVAR_NOTIFY|FCVAR_REPLICATED, "Amount of impedance caused when hit in the leg. ( 0 min to 100 max )", true, 0.0f, true, 100.0 );
ConVar mp_armshotimpedance( "mp_armshotimpedance", "20.0", FCVAR_NOTIFY|FCVAR_REPLICATED, "Amount of impedance caused when hit in the arm. ( 0 min to 100 max )", true, 0.0f, true, 100.0 );
ConVar sv_showenemyclass( "sv_showenemyclass", "1", FCVAR_REPLICATED, "Show the enemy's classes on the scoreboard." );
ConVar sv_showenemytickets( "sv_showenemytickets", "1", FCVAR_REPLICATED, "Show the enemy's tickets on the scoreboard." );
ConVar sv_showenemysquads( "sv_showenemysquads", "1", FCVAR_REPLICATED, "Show the enemy's squads on the scoreboard." );
ConVar sv_fadetoblack( "sv_fadetoblack", "0", FCVAR_REPLICATED, "Fade players' views to black when they die." );
ConVar sv_deathtime( "sv_deathtime", "6.0", FCVAR_REPLICATED, "Fade players' views to black when they die." );
ConVar sv_showimpacts( "sv_showimpacts", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Shows client (red) and server (blue) bullet impact point" );
ConVar mp_bulletmaxlifetime( "mp_bulletmaxlifetime", "3.0", FCVAR_REPLICATED, "Maximum time a bullet can remain active." );
ConVar mp_useballistics( "mp_useballistics", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Use Ballistics" );


#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CRnLPlayer )
	DEFINE_PRED_TYPEDESCRIPTION( m_RnLLocal, C_RnLPlayerLocalData ),
	DEFINE_PRED_FIELD( m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_iShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nWeaponPosture, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vecLeanOffset, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),

END_PREDICTION_DATA()

#else

ConVar mp_moraledistance( "mp_moraledistance", "200.0", FCVAR_NOTIFY|FCVAR_REPLICATED, "The maximum distance from a teammate before they stop offering a morale boost." );

#define MORALE_UPDATE_TIME 0.75f

#endif

void DispatchEffect( const char *pName, const CEffectData &data );

CWeaponRnLBase* CRnLPlayer::RnLAnim_GetActiveWeapon()
{
	return GetActiveRnLWeapon();
}

bool CRnLPlayer::CanMove()
{
					  //MG Movement
	if( (IsDeployed() /*&& (m_nButtons & IN_SPEED) == 0*/) || IsWounded() )
		return false;

	return true;
}

bool CRnLPlayer::IsWounded( void )
{
	return ( m_nMovementPosture == MOVEMENT_POSTURE_MORTAL_WOUND );
}

bool CRnLPlayer::IsDeployed( void ) const
{ 
	return ( m_nMovementPosture == MOVEMENT_POSTURE_DEPLOYED 
		|| m_nMovementPosture == MOVEMENT_POSTURE_DEPLOYED_CROUCH 
		|| m_nMovementPosture == MOVEMENT_POSTURE_DEPLOYED_PRONE ); 
}

void CRnLPlayer::Deploy( QAngle angAngles, Vector Origin )
{
	SetMovementPostureAngle( angAngles );
	m_flBodyHeight = (Origin.z - GetAbsOrigin().z);
	m_RnLLocal.m_vecMovementPos = (Origin + Vector( 0, 0, 5)) - GetAbsOrigin();

	//AddFlag( FL_DEPLOYED );
	if ( IsProne() )
	{
		SetMovementPosture( MOVEMENT_POSTURE_DEPLOYED_PRONE );
	}
	else if( IsDucked() )
	{
		SetMovementPosture( MOVEMENT_POSTURE_DEPLOYED_CROUCH );
	}
	else
	{
		SetMovementPosture( MOVEMENT_POSTURE_DEPLOYED );
	}
}

void CRnLPlayer::UnDeploy( void )
{
	m_flBodyHeight = GetViewOffset().z;
	//RemoveFlag( FL_DEPLOYED );
	if( GetMovementPosture() == MOVEMENT_POSTURE_DEPLOYED_PRONE )
	{
		SetMovementPosture( MOVEMENT_POSTURE_PRONE );
	}
	else if( GetMovementPosture() == MOVEMENT_POSTURE_DEPLOYED_CROUCH )
	{
		SetMovementPosture( MOVEMENT_POSTURE_CROUCH );
	}
	else
	{
		SetMovementPosture( MOVEMENT_POSTURE_STAND );
	}
}

float CRnLPlayer::GetViewRollOffset( void )
{
	return m_RnLLocal.m_flViewRollOffset;
}

void CRnLPlayer::SetViewRollOffset( float newOffset )
{
	m_RnLLocal.m_flViewRollOffset = newOffset;
}

bool CRnLPlayer::IsDuckToggled( void )
{
	return m_bIsDuckToggled;
}

void CRnLPlayer::ToggleDuck( void )
{
	m_bIsDuckToggled = !m_bIsDuckToggled;
}

void CRnLPlayer::SetMoraleLevel( int iLev )
{
	if( iLev > 100 )
	{
		iLev = 100;
	}
	else if( iLev < 1 )
	{
		iLev = 1;
	}

	m_RnLLocal.m_iMoraleLevel = iLev;
}

int CRnLPlayer::GetMoraleLevel( void )
{
	return m_RnLLocal.m_iMoraleLevel;
}

void CRnLPlayer::UpdatePlayerMorale( void )
{
	if( !IsAlive() || IsObserver() || (GetTeamNumber() != TEAM_ALLIES && GetTeamNumber() != TEAM_AXIS) )
		return;

#ifndef CLIENT_DLL
	if( m_flNextMoraleUpdate <= gpGlobals->curtime )
	{
		m_flNextMoraleUpdate = gpGlobals->curtime + MORALE_UPDATE_TIME;

		int iNearPlayerCount = 0;

		CTeam* pTeam = g_Teams[GetTeamNumber()];
		Vector absOrigin = GetAbsOrigin();
		if( pTeam )
		{
			for( int i = 0; i < pTeam->GetNumPlayers(); i++ )
			{
				CBasePlayer *pPlayer = pTeam->GetPlayer( i );
				if( pPlayer && pPlayer != this && pPlayer->IsAlive() )
				{
					if( pPlayer->GetAbsOrigin().DistTo( absOrigin ) < mp_moraledistance.GetFloat() )
						iNearPlayerCount++;
				}
			}
		}

		int iCurrentMorale = GetMoraleLevel();
		int iAlteredMorale = 0;

		if( iNearPlayerCount > 0 )
		{
			if( iNearPlayerCount > 5 )
				iNearPlayerCount = 5;

			int iPlayerValues = ( iNearPlayerCount * 10 ) + 50;

			if( iCurrentMorale > iPlayerValues )
				iAlteredMorale = -1;
			else if( iCurrentMorale < iPlayerValues )
				iAlteredMorale = 1;
		}
		else
		{
			if( iCurrentMorale > 50 )
				iAlteredMorale = -1;
			else if( iCurrentMorale < 50  )
				iAlteredMorale = 1;
		}

		if( iAlteredMorale != 0 )
			SetMoraleLevel( ( iCurrentMorale + iAlteredMorale ) );
	}
#endif
}

bool CRnLPlayer::IsSprinting( void )
{
	return m_RnLLocal.m_bIsSprinting;
}

bool CRnLPlayer::StartSprinting( void )
{
	if( m_RnLLocal.m_bIsSprinting )
	{
		return true;
	}

	CWeaponRnLBase *pWeapon = GetActiveRnLWeapon();

	if( pWeapon )
	{
		if( pWeapon->GetWeaponID() != WEAPON_DEVCAM )
		{
			if( !pWeapon->StartSprinting() )
			{
				return false;
			}
		}
	}

	m_RnLLocal.m_bIsSprinting = true;

	return true;
}

bool CRnLPlayer::StopSprinting( void )
{
	m_RnLLocal.m_bIsSprinting = false;
	return true;
}

void CRnLPlayer::SetStamina( float fStamina )
{
	m_RnLLocal.m_flStamina = fStamina;
}

//-----------------------------------------------------------------------------
// Handle decreasing the max stamina when shot in the legs
//-----------------------------------------------------------------------------
float CRnLPlayer::GetMaxStamina( void )
{
	if( GetDamageBasedSpeedModifier( 1 ) > 0.0f )
	{
		// Modified stamina = Max Stamina - (# of times shot in the leg * 10)
		float flStamina = 100 - ((GetDamageBasedSpeedModifier( 1 ) / mp_legshotimpedance.GetFloat()) * 10);

		if( flStamina < 60 )
			flStamina = 60;

		return flStamina;
	}

	return 100.0f;
}

float CRnLPlayer::GetStamina()
{
	return m_RnLLocal.m_flStamina;
}

void CRnLPlayer::SetSprintTimer( float flSprintTime )
{
	m_RnLLocal.m_flSprintTime = flSprintTime;
}

float CRnLPlayer::GetSprintTimer()
{
	return m_RnLLocal.m_flSprintTime;
}

void CRnLPlayer::SetRecoverTime( float flRecoverTime )
{
	m_RnLLocal.m_flRecoverTime = flRecoverTime;
}

float CRnLPlayer::GetRecoverTime()
{
	return m_RnLLocal.m_flRecoverTime;
}

int CRnLPlayer::GetSquadNumber()
{
	return m_iSquadNumber;
}

void CRnLPlayer::SetSquadNumber( int iNum )
{
	m_iSquadNumber = iNum;
}

int CRnLPlayer::GetKitNumber()
{
	return m_iKitNumber;
}

void CRnLPlayer::SetKitNumber( int iNum )
{
	m_iKitNumber = iNum;
}

float CRnLPlayer::GetDamageBasedSpeedModifier( int iType )
{
	switch( iType )
	{
	case 0:
		return m_RnLLocal.m_flDamageBasedSpeedModifierArms;

	case 1:
		return m_RnLLocal.m_flDamageBasedSpeedModifierLegs;
	}

	return 0.0f;
}

void CRnLPlayer::SetDamageBasedSpeedModifier( int iType, float val )
{
	switch( iType )
	{
	case 0:
		m_RnLLocal.m_flDamageBasedSpeedModifierArms = val;
		m_RnLLocal.m_flDamageBasedSpeedModifierArms = clamp( m_RnLLocal.m_flDamageBasedSpeedModifierArms, 0.0f, 100.0f );
		break;

	case 1:
		m_RnLLocal.m_flDamageBasedSpeedModifierLegs = val;
		m_RnLLocal.m_flDamageBasedSpeedModifierLegs = clamp( m_RnLLocal.m_flDamageBasedSpeedModifierLegs, 0.0f, 100.0f );
		break;
	}
}

void CRnLPlayer::AddDamageBasedSpeedModifier( int iType, float val )
{
	switch( iType )
	{
	case 0:
		m_RnLLocal.m_flDamageBasedSpeedModifierArms += val;
		m_RnLLocal.m_flDamageBasedSpeedModifierArms = clamp( m_RnLLocal.m_flDamageBasedSpeedModifierArms, 0.0f, 100.0f );
		break;

	case 1:
		m_RnLLocal.m_flDamageBasedSpeedModifierLegs += val;
		m_RnLLocal.m_flDamageBasedSpeedModifierLegs = clamp( m_RnLLocal.m_flDamageBasedSpeedModifierLegs, 0.0f, 100.0f );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLPlayer::SelectItem( const char *pstr, int iSubType )
{
	if (!IsAlive())
		return;

	if (!pstr)
		return;

	CBaseCombatWeapon *pItem = Weapon_OwnsThisType( pstr, iSubType );

	if (!pItem)
		return;

	if( GetObserverMode() != OBS_MODE_NONE )
		return;// Observers can't select things.

	if ( !Weapon_ShouldSelectItem( pItem ) )
		return;

	// FIX, this needs to queue them up and delay
	// Make sure the current weapon can be holstered
	if ( GetActiveWeapon() )
	{
		if ( !GetActiveWeapon()->CanHolster() )
			return;

		ResetAutoaim( );
		GetActiveWeapon()->Holster( pItem );

		if( GetActiveWeapon()->GetModelPtr() && GetActiveWeapon()->GetSequence() )
			m_flNextAttack = gpGlobals->curtime + GetActiveWeapon()->SequenceDuration(GetActiveWeapon()->GetModelPtr(), GetActiveWeapon()->GetSequence());
		else
			m_flNextAttack = gpGlobals->curtime + 0.659f;

		m_pSwitchingToWeapon = pItem;
	}
	else
		Weapon_Switch( pItem );
}

void CRnLPlayer::CheckWeaponSwitch( void )
{
	if( m_pSwitchingToWeapon == NULL )
		return;

	if( !IsAlive() )
		return;

	if( GetActiveRnLWeapon()->GetAnimationState() != WEAPON_ANIMATION_HOLSTER || GetActiveRnLWeapon()->IsSequenceFinished() )
	{
		Weapon_Switch( m_pSwitchingToWeapon );
		m_pSwitchingToWeapon = NULL;
	}
}

//-----------------------------------------------------------------------------
// Actual Eye position + angles
//-----------------------------------------------------------------------------
Vector CRnLPlayer::EyePosition( )
{
#ifdef CLIENT_DLL
	IClientVehicle *pVehicle = GetVehicle();
#else
	IServerVehicle *pVehicle = GetVehicle();
#endif

	Vector vecPos = BaseClass::EyePosition();

	if( IsDeployed() )
	{
		// GetRenderOrigin() returns ragdoll pos if player is ragdolled
		Vector origin = m_RnLLocal.m_vecMovementPos + GetAbsOrigin();
		Vector forward;
		QAngle eyeAngles = GetWeaponAngle();
		
		AngleVectors( eyeAngles, &forward );
		VectorNormalize( forward );

		float flDistance = 16.0f;
		CWeaponRnLBaseMachineGun *pMachineGun = dynamic_cast<CWeaponRnLBaseMachineGun*>(GetActiveWeapon());
		if( pMachineGun )
			flDistance = pMachineGun->GetStandOffset();

		matrix3x4_t fRotateMatrix;
		QAngle worldAng( 0, forward[YAW], 0 );
		Vector vecDiff = -flDistance * forward;
		Vector output;
		AngleMatrix( worldAng, fRotateMatrix);
		VectorRotate( vecDiff, fRotateMatrix, output );
		origin = origin+output;

		////FUCK YOU STEF you made me do this with your animations!
		//if( GetMovementPosture() == MOVEMENT_POSTURE_DEPLOYED_PRONE )
		//	origin += Vector( 0, 0, 5 );

		return origin;
	}

	if ( !pVehicle )
	{
		matrix3x4_t fRotateMatrix;
		QAngle worldAng( 0, EyeAngles()[YAW], 0 );
		Vector vecOffset = GetLeaningOffset();
		Vector output;
		AngleMatrix( worldAng, fRotateMatrix);
		VectorRotate( vecOffset, fRotateMatrix, output );
		vecPos += ( output );

		output.Init();
		AngleVectors( worldAng, &output );
		if ( GetMovementPostureOffset() < 40.0f ) // offset forward when prone
		{
			float flForward = 1.0f - clamp((GetMovementPostureOffset() - 10.0f) / 30.0f, 0.0f, 1.0f);
			vecPos += output * flForward * 10.0f;
		}
	}

	return vecPos;
}

QAngle CRnLPlayer::GetWeaponAngle( void )
{
#ifdef CLIENT_DLL
	return m_angWeaponAngle + m_angWeaponSway;// + m_angMoraleEffect;
#else
	return m_angWeaponAngle;
#endif
}

void CRnLPlayer::SetWeaponAngle( const QAngle& angle )
{
	m_angWeaponAngle = angle;
}

void CRnLPlayer::AdjustWeaponSway( const QAngle& offset )
{
#ifdef CLIENT_DLL
	m_angWeaponSway = offset;
#endif
}

// adjusts view angles by the angle supplied
void CRnLPlayer::AdjustWeaponAngle( const QAngle &angleOffset )
{
#ifdef CLIENT_DLL
	//m_angWeaponSway = angleOffset;
#endif
	m_angWeaponAngle += angleOffset;
}

void CRnLPlayer::AdjustViewAngles( const QAngle &angleOffset )
{
#ifdef CLIENT_DLL
	if ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		return;

	QAngle angle;
	engine->GetViewAngles( angle );
	engine->SetViewAngles( angle + angleOffset );
	SetLocalViewAngles( angle + angleOffset );
#endif
}

#ifndef CLIENT_DLL
	CUserCmd bulletCmD;
	Vector absOrigin;
#endif

//-----------------------------------------------------------------------------
// Purpose: Called every usercmd by the player PostThink
//-----------------------------------------------------------------------------
void CRnLPlayer::ItemPostFrame()
{
#ifdef CLIENT_DLL
	if( !prediction->InPrediction() || (prediction->InPrediction() && prediction->IsFirstTimePredicted()) )
	{
#endif
		UpdatePlayerMorale();

		if( IsAlive() )
			PerformObstaclePushaway( this );

		/*
		bool bSave = false;

		for( int i = 0; i < m_aBullets.Count(); i++ )
		{
#ifndef CLIENT_DLL
			bulletCmD.tick_count = m_aBullets[i].m_iTickCount;
			VectorAngles(  m_aBullets[i].m_vecDirShooting, bulletCmD.viewangles );
			absOrigin = GetAbsOrigin();
			SetAbsOrigin( m_aBullets[i].m_vecPosition );
			lagcompensation->StartLagCompensation( this, &bulletCmD );
			SetAbsOrigin( absOrigin );
#endif

			bSave = UpdateBullet(i);

#ifndef CLIENT_DLL
			lagcompensation->FinishLagCompensation( this );
#endif

			if( !bSave )
			{
				m_aBullets.Remove( i );
				i--;
			}
		}
		*/
#ifdef CLIENT_DLL
	}
#endif

	CheckWeaponSwitch();

	BaseClass::ItemPostFrame();
}

extern ConVar sv_gravity;

bool CRnLPlayer::UpdateBullet( int iIndex )
{
	if( iIndex < 0 || iIndex >= m_aBullets.Count() )
		return false;

	RnLBulletInfo* pInfo = &(m_aBullets[iIndex]);

	if( !pInfo )
		return false;

	if( pInfo->m_flLifeTime > mp_bulletmaxlifetime.GetFloat() )
		return false;

	if( pInfo->m_iPrevUpdate == gpGlobals->tickcount )
		return true;
		
	if( pInfo->m_flPower < 0.15 )
		return false;

	if( pInfo->m_flDistance > GetRnLAmmoDef()->GetEffectiveRange( pInfo->m_iAmmoType ) )
		return false;

	pInfo->m_iPrevUpdate = gpGlobals->tickcount;
#ifndef CLIENT_DLL
	pInfo->m_iTickCount++;
#endif

	pInfo->m_flLifeTime += gpGlobals->frametime;

	Vector vecEnd = pInfo->m_vecPosition + ((pInfo->m_vecDirShooting * GetRnLAmmoDef()->GetEffectiveRange( pInfo->m_iAmmoType )) * pInfo->m_flPower); // max bullet range is 10000 units
	
	if( mp_useballistics.GetBool() )
	{
		vecEnd = pInfo->m_vecPosition + ((pInfo->m_vecVelocity * gpGlobals->frametime) * pInfo->m_flPower); // max bullet range is 10000 units
		pInfo->m_vecVelocity[2] -= sv_gravity.GetFloat() * (gpGlobals->frametime*gpGlobals->frametime);
		pInfo->m_vecDirShooting = pInfo->m_vecVelocity;
		pInfo->m_vecDirShooting.NormalizeInPlace();
	}

	trace_t tr; // main enter bullet trace
	UTIL_TraceLine( pInfo->m_vecPosition, vecEnd, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	if ( sv_showimpacts.GetBool() )
	{
#ifdef CLIENT_DLL
		// draw red client bullet lines
		debugoverlay->AddLineOverlay( tr.startpos, tr.endpos, 255, 0, 0, true, 4.0 );
#else
		// draw blue server impact markers
		NDebugOverlay::Box( tr.endpos, Vector(-1,-1,-1), Vector(1,1,1), 0,0,255,127, 4 );
		// draw blue server impact markers
		NDebugOverlay::Box( tr.startpos, Vector(-1,-1,-1), Vector(1,1,1), 0,0,255,127, 4 );
		// draw blue server bullet lines
		//NDebugOverlay::Line( tr.startpos, tr.endpos, 0, 0, 255, true, 4 );
#endif
	}

	// Always trace through foliage
	// TODO: Trace through more than just the first foliage entity
	surfacedata_t *phit = physprops->GetSurfaceData( tr.surface.surfaceProps );
	if( phit )
	{
		char tex = phit->game.material;
		if ( tex == CHAR_TEX_HEDGEROW || tex == CHAR_TEX_FOLIAGE )
		{
			bool bStillInside = true;
			int maxChecks = 0;
			do
			{
				maxChecks++;
				UTIL_TraceLine( tr.endpos + (pInfo->m_vecDirShooting * 8), vecEnd, MASK_SHOT, tr.m_pEnt, COLLISION_GROUP_NONE, &tr ); 
				// needs a better solution, the ignore part doesn't seem to work, hence this crappy workaround
				if (!tr.allsolid || maxChecks == 10) bStillInside = false;
			} while (bStillInside);
		}
	}

	if ( tr.fraction == 1.0f )
	{
		pInfo->m_flDistance += abs((tr.endpos - pInfo->m_vecPosition).Length());
		pInfo->m_vecPosition = tr.endpos;
		return true; // we didn't hit anything, stop tracing shot
	}

	if ( sv_showimpacts.GetBool() )
	{
#ifdef CLIENT_DLL
		// draw red client impact markers
		debugoverlay->AddBoxOverlay( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 255,0,0,127, 4 );

		if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
		{
			C_BasePlayer *player = ToBasePlayer( tr.m_pEnt );
			player->DrawClientHitboxes( 4, true );
		}
#else
		// draw blue server impact markers
		NDebugOverlay::Box( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), 0,0,255,127, 4 );

		if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
		{
			CBasePlayer *player = ToBasePlayer( tr.m_pEnt );
			player->DrawServerHitboxes( 4, true );
		}
#endif
	}

	int iDamageType = GetAmmoDef()->DamageType( pInfo->m_iAmmoType );
	if( pInfo->m_bDoEffects )
	{
		// See if the bullet ended up underwater + started out of the water
		if ( enginetrace->GetPointContents( tr.endpos ) & (CONTENTS_WATER|CONTENTS_SLIME) )
		{	
			trace_t waterTrace;
			UTIL_TraceLine( pInfo->m_vecPosition, tr.endpos, (MASK_SHOT|CONTENTS_WATER|CONTENTS_SLIME), this, COLLISION_GROUP_NONE, &waterTrace );

			if( waterTrace.allsolid != 1 )
			{
				CEffectData	data;
				data.m_vOrigin = waterTrace.endpos;
				data.m_vNormal = waterTrace.plane.normal;
				data.m_flScale = random->RandomFloat( 8, 12 );

				if ( waterTrace.contents & CONTENTS_SLIME )
				{
					data.m_fFlags |= FX_WATER_IN_SLIME;
				}

				DispatchEffect( "gunshotsplash", data );
			}
		}
		else
		{
			//Do Regular hit effects

			// Don't decal nodraw surfaces
			if ( !( tr.surface.flags & (SURF_SKY|SURF_NODRAW|SURF_HINT|SURF_SKIP) ) )
			{
				CBaseEntity *pEntity = tr.m_pEnt;
				if ( !( !friendlyfire.GetBool() && pEntity && pEntity->IsPlayer() && pEntity->GetTeamNumber() == GetTeamNumber() ) )
				{
					UTIL_ImpactTrace( &tr, iDamageType );
				}
			}
		}
	} // bDoEffects

	MakeTracer( tr.startpos, tr, GetAmmoDef()->TracerType( pInfo->m_iAmmoType ) );

	// add damage to entity that we hit
	// GAME_DLL means server-side only - nuke

#ifdef GAME_DLL
	ClearMultiDamage();

	CTakeDamageInfo info( pInfo->m_pInflictor, pInfo->m_pAttacker, pInfo->m_pWeapon, pInfo->m_iDamage, iDamageType );
	CalculateBulletDamageForce( &info, pInfo->m_iAmmoType, pInfo->m_vecDirShooting, tr.endpos );
	tr.m_pEnt->DispatchTraceAttack( info, pInfo->m_vecDirShooting, &tr );

	TraceAttackToTriggers( info, tr.startpos, tr.endpos, pInfo->m_vecDirShooting );

	ApplyMultiDamage();
#endif
	float dist = 0.0f;
	surfacedata_t *psurf = physprops->GetSurfaceData( tr.surface.surfaceProps );
	float dotNormal = abs(tr.plane.normal.Dot( pInfo->m_vecDirShooting ));
	float defAngle = 0.0f;
	bool canDeflect = GetRnLAmmoDef()->GetDeflection( pInfo->m_iAmmoType, psurf->game.material, defAngle );
	if( canDeflect && pInfo->m_flPower > 0.70 && dotNormal <= defAngle )
	{
		pInfo->m_flDistance += abs((tr.endpos - pInfo->m_vecPosition).Length());
		//Vector outDir;
		VectorAdd( tr.plane.normal, pInfo->m_vecDirShooting, pInfo->m_vecDirShooting );
		//VectorAdd( tr.plane.normal, outDir, vecDir );
		pInfo->m_vecPosition = tr.endpos;
		pInfo->m_flPower *= 0.75f;
		if ( sv_showimpacts.GetBool() )
		{
#ifdef CLIENT_DLL
			// draw red client impact markers
			debugoverlay->AddLineOverlay( pInfo->m_vecPosition, vecEnd, 255,0,0,127, 6 );
#else
			NDebugOverlay::Line( pInfo->m_vecPosition, vecEnd, 0,0, 255, false, 6 );
#endif
		}
	}
	else
	{

		if ( psurf ) 
			dist = GetRnLAmmoDef()->GetPenetrationDistance( pInfo->m_iAmmoType, psurf->game.material ) * pInfo->m_flPower; 
		
		// In case we found a wall we could penetrate, check the surfaceprop of the other side of the wall (e.g. for wallpaper|stone walls) --> use hardest material
		trace_t	penetrationTrace;
		UTIL_TraceLine( tr.endpos + ( pInfo->m_vecDirShooting * 32 ), tr.endpos, MASK_SHOT, this, COLLISION_GROUP_NONE, &penetrationTrace );
		psurf = physprops->GetSurfaceData( penetrationTrace.surface.surfaceProps );
		if ( psurf  ) 
		{
			float dist2 = GetRnLAmmoDef()->GetPenetrationDistance( pInfo->m_iAmmoType, psurf->game.material ) * pInfo->m_flPower;
			if (dist2 < dist)
				dist = dist2;
		}
		//DevMsg(1,"penetrating %f\n",dist);
		if( dist > 0 )
		{
			// Move through the glass until we're at the other side
			Vector	testPos = tr.endpos + ( pInfo->m_vecDirShooting * dist );

			// Re-trace as if the bullet had passed right through
			UTIL_TraceLine( testPos, tr.endpos, MASK_SHOT, this, COLLISION_GROUP_NONE, &penetrationTrace );

			// See if we found the surface again
			if ( penetrationTrace.startsolid || tr.fraction == 0.0f || penetrationTrace.fraction == 1.0f )
			{
				return false;
			}

			UTIL_ImpactTrace( &penetrationTrace, iDamageType );
			pInfo->m_flDistance += abs((penetrationTrace.endpos - pInfo->m_vecPosition).Length());
			pInfo->m_vecPosition = penetrationTrace.endpos;
		}
		else
		{
			return false;
		}
	}

	return true;
}

#ifdef GAME_DLL
static int gMoraleValues[MAX_WEAPONS] =
{
	0,	//WEAPON_NONE
	6,	//WEAPON_BAR
	6,	//WEAPON_BROWNING
	4,	//WEAPON_COLT
	4,	//WEAPON_GREASEGUN
	4,	//WEAPON_M1CARBINE
	4,	//WEAPON_M1GARAND
	4,	//WEAPON_SPRINGFIELD
	4,	//WEAPON_THOMPSON
	4,	//WEAPON_M3KNIFE
	10,	//WEAPON_MK2GRENADE
	6,	//WEAPON_M18GRENADE
	10,	//WEAPON_STICKY24GRENADE
	6,	//WEAPON_STICKY39GRENADE
	4,	//WEAPON_STG44
	4,	//WEAPON_WALTHER
	6,	//WEAPON_K98K
	4,	//WEAPON_MP40
	4,	//WEAPON_MG34
	8,	//WEAPON_MG42
	4,	//WEAPON_TRENCHKNIFE
	4,	//WEAPON_FG42
	4,	//WEAPON_G43
	4,	//WEAPON_K98SCOPED
	0,	//WEAPON_NCOMAP
	0,	//WEAPON_DEVCAM
	2,	//WEAPON_ALLIEDFISTS
	2,	//WEAPON_AXISFISTS
	10,	//WEAPON_ALLIEDEXPLOSIVES
	10,	//WEAPON_AXISEXPLOSIVES
	4,	//WEAPON_SCRIPTED
		//WEAPON_MAX
};
#endif

//modified by NuclearFriend to take into effect shooting through objects
void CRnLPlayer::FireBullet( 
						   Vector vecSrc,	// shooting postion
						   const QAngle &shootAngles,  //shooting angle
						   float vecSpread, // spread vector
						   int iDamage, // base damage
						   int iBulletType, // ammo type
						   CBaseEntity *pevAttacker, // shooter
						   bool bDoEffects,	// create impact effect ?
						   float x,	// spread x factor
						   float y	// spread y factor
						   )
{
	Vector vecDirShooting, vecRight, vecUp;
	AngleVectors( shootAngles, &vecDirShooting, &vecRight, &vecUp );

	if ( !pevAttacker )
		pevAttacker = this;  // the default attacker is ourselves

	// add the spray 
	Vector vecDir = vecDirShooting +
		x * vecSpread * vecRight +
		y * vecSpread * vecUp;

	VectorNormalize( vecDir );

	CWeaponRnLBase *pWeapon = GetActiveRnLWeapon();


	if( !mp_useballistics.GetBool() )
	{
		RnLBulletInfo bullet;
		bullet.m_vecPosition = vecSrc;
		bullet.m_vecDirShooting = vecDir;
		bullet.m_vecVelocity = vecDir * GetRnLAmmoDef()->GetVelocity( iBulletType );
		bullet.m_flDistance = 0;
		bullet.m_flPower = 1.0f;
		bullet.m_iAmmoType = iBulletType;
		bullet.m_iDamage = iDamage;
		bullet.m_flLifeTime = 0.0f;
		bullet.m_bDoEffects = bDoEffects;
		bullet.m_pAttacker = pevAttacker;
		bullet.m_pWeapon = pWeapon;
		bullet.m_pInflictor = pWeapon;
		bullet.m_iPrevUpdate = 0;

		FireBulletImmediately( bullet );
	}
	else
	{
		int iIndex = m_aBullets.AddToTail();

		if( iIndex < 0 )
			return;

		trace_t tr;
		UTIL_TraceLine( vecSrc, vecSrc + (vecDir*GetRnLAmmoDef()->GetEffectiveRange( iBulletType )), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
		MakeTracer( vecSrc, tr, GetAmmoDef()->TracerType( iBulletType ) );

		m_aBullets[iIndex].m_vecPosition = vecSrc;
		m_aBullets[iIndex].m_vecDirShooting = vecDir;
		m_aBullets[iIndex].m_vecVelocity = vecDir * GetRnLAmmoDef()->GetVelocity( iBulletType );
		m_aBullets[iIndex].m_flDistance = 0;
		m_aBullets[iIndex].m_flPower = 1.0f;
		m_aBullets[iIndex].m_iAmmoType = iBulletType;
		m_aBullets[iIndex].m_iDamage = iDamage;
		m_aBullets[iIndex].m_flLifeTime = 0.0f;
		m_aBullets[iIndex].m_bDoEffects = bDoEffects;
		m_aBullets[iIndex].m_pAttacker = pevAttacker;
		m_aBullets[iIndex].m_pWeapon = pWeapon;
		m_aBullets[iIndex].m_pInflictor = pWeapon;
		m_aBullets[iIndex].m_iPrevUpdate = 0;
	#ifndef CLIENT_DLL
		m_aBullets[iIndex].m_iTickCount = GetCurrentUserCommand()->tick_count;
	#endif
	}
}

//modified by NuclearFriend to take into effect shooting through objects
void CRnLPlayer::FireBulletImmediately( RnLBulletInfo& bullet )
{
	bool bTraceRefire = true;
	do
	{
		Vector vecEnd = bullet.m_vecPosition + ((bullet.m_vecDirShooting * GetRnLAmmoDef()->GetEffectiveRange( bullet.m_iAmmoType )) * bullet.m_flPower); // max bullet range is 10000 units
		trace_t tr; // main enter bullet trace
		UTIL_TraceLine( bullet.m_vecPosition, vecEnd, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

#ifdef GAME_DLL
		if( bullet.m_flPower > 0.70 )
		{
			CWeaponRnLBase *pWeapon = GetActiveRnLWeapon();

			if( pWeapon )
			{
				for ( int i=1; i <= gpGlobals->maxClients; i++ )
				{
					CRnLPlayer *pPlayer = ToRnLPlayer( UTIL_PlayerByIndex( i ) );

					if ( !pPlayer )
						continue;

					if( pPlayer->GetTeamNumber() == GetTeamNumber() || !pPlayer->IsAlive() )
						continue;

					if( IsRayIntersectingSphere( tr.startpos, tr.endpos - tr.startpos, pPlayer->EyePosition(), 96, 3 ) )
						pPlayer->SetMoraleLevel( pPlayer->GetMoraleLevel() - gMoraleValues[pWeapon->GetWeaponID()] );
				}
			}
		}
#endif

		if ( sv_showimpacts.GetBool() )
		{
#ifdef CLIENT_DLL
			// draw red client bullet lines
			debugoverlay->AddLineOverlay( tr.startpos, tr.endpos, 255, 0, 0, true, 4.0 );
#else
			// draw blue server impact markers
			NDebugOverlay::Box( tr.endpos, Vector(-1,-1,-1), Vector(1,1,1), 0,0,255,127, 4 );
			// draw blue server impact markers
			NDebugOverlay::Box( tr.startpos, Vector(-1,-1,-1), Vector(1,1,1), 0,0,255,127, 4 );
			// draw blue server bullet lines
			//NDebugOverlay::Line( tr.startpos, tr.endpos, 0, 0, 255, true, 4 );
#endif
		}

		// Always trace through foliage
		// TODO: Trace through more than just the first foliage entity
		surfacedata_t *phit = physprops->GetSurfaceData( tr.surface.surfaceProps );
		if( phit )
		{
			char tex = phit->game.material;
			if ( tex == CHAR_TEX_HEDGEROW || tex == CHAR_TEX_FOLIAGE )
			{
				bool bStillInside = true;
				int maxChecks = 0;
				do
				{
					maxChecks++;
					UTIL_TraceLine( tr.endpos + (bullet.m_vecDirShooting * 8), vecEnd, MASK_SHOT, tr.m_pEnt, COLLISION_GROUP_NONE, &tr ); 
					// needs a better solution, the ignore part doesn't seem to work, hence this crappy workaround
					if (!tr.allsolid || maxChecks == 10) 
						bStillInside = false;
				} while (bStillInside);
			}
		}

		if ( tr.fraction == 1.0f )
			return; // we didn't hit anything, stop tracing shot

		if ( sv_showimpacts.GetBool() )
		{
#ifdef CLIENT_DLL
			// draw red client impact markers
			debugoverlay->AddBoxOverlay( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 255,0,0,127, 4 );

			if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
			{
				C_BasePlayer *player = ToBasePlayer( tr.m_pEnt );
				player->DrawClientHitboxes( 4, true );
			}
#else
			// draw blue server impact markers
			NDebugOverlay::Box( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), 0,0,255,127, 4 );

			if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
			{
				CBasePlayer *player = ToBasePlayer( tr.m_pEnt );
				player->DrawServerHitboxes( 4, true );
			}
#endif
		}

		int iDamageType = GetAmmoDef()->DamageType( bullet.m_iAmmoType );
		if( bullet.m_bDoEffects )
		{
			// See if the bullet ended up underwater + started out of the water
			if ( enginetrace->GetPointContents( tr.endpos ) & (CONTENTS_WATER|CONTENTS_SLIME) )
			{	
				trace_t waterTrace;
				UTIL_TraceLine( bullet.m_vecPosition, tr.endpos, (MASK_SHOT|CONTENTS_WATER|CONTENTS_SLIME), this, COLLISION_GROUP_NONE, &waterTrace );

				if( waterTrace.allsolid != 1 )
				{
					CEffectData	data;
					data.m_vOrigin = waterTrace.endpos;
					data.m_vNormal = waterTrace.plane.normal;
					data.m_flScale = random->RandomFloat( 8, 12 );

					if ( waterTrace.contents & CONTENTS_SLIME )
					{
						data.m_fFlags |= FX_WATER_IN_SLIME;
					}

					DispatchEffect( "gunshotsplash", data );
				}
			}
			else
			{
				//Do Regular hit effects

				// Don't decal nodraw surfaces
				if ( !( tr.surface.flags & (SURF_SKY|SURF_NODRAW|SURF_HINT|SURF_SKIP) ) )
				{
					CBaseEntity *pEntity = tr.m_pEnt;
					if ( !( !friendlyfire.GetBool() && pEntity && pEntity->IsPlayer() && pEntity->GetTeamNumber() == GetTeamNumber() ) )
					{
						UTIL_ImpactTrace( &tr, iDamageType );
					}
				}
			}
		} // bDoEffects

		bool bShouldMakeTracer = true;

		// Only make tracers for MGs when the function tells us to
		if( GetActiveRnLWeapon() && GetActiveRnLWeapon()->IsMachineGun() )
		{
			CWeaponRnLBaseMachineGun *pActiveMG = dynamic_cast<CWeaponRnLBaseMachineGun *>( GetActiveRnLWeapon() );

			bShouldMakeTracer = pActiveMG ? pActiveMG->ShouldMakeTracer() : false;
		}

		if( bShouldMakeTracer )
			MakeTracer( tr.startpos, tr, GetAmmoDef()->TracerType( bullet.m_iAmmoType ) );

		// add damage to entity that we hit
		// GAME_DLL means server-side only - nuke

#ifdef GAME_DLL
		ClearMultiDamage();

		CTakeDamageInfo info( bullet.m_pInflictor, bullet.m_pAttacker, bullet.m_pWeapon, bullet.m_iDamage, iDamageType );
		CalculateBulletDamageForce( &info, bullet.m_iAmmoType, bullet.m_vecDirShooting, tr.endpos );
		tr.m_pEnt->DispatchTraceAttack( info, bullet.m_vecDirShooting, &tr );

		TraceAttackToTriggers( info, tr.startpos, tr.endpos, bullet.m_vecDirShooting );

		ApplyMultiDamage();
#endif
		float dist = 0.0f;
		surfacedata_t *psurf = physprops->GetSurfaceData( tr.surface.surfaceProps );
		float dotNormal = abs(tr.plane.normal.Dot( bullet.m_vecDirShooting ));
		float defAngle = 0.0f;
		bool canDeflect = GetRnLAmmoDef()->GetDeflection( bullet.m_iAmmoType, psurf->game.material, defAngle );
		if( canDeflect && bullet.m_flPower > 0.70 && dotNormal <= defAngle )
		{
			bullet.m_flDistance += abs((tr.endpos - bullet.m_vecPosition).Length());
			//Vector outDir;
			Vector temp;
			CrossProduct( bullet.m_vecDirShooting, tr.plane.normal, temp );
			CrossProduct( tr.plane.normal, temp, bullet.m_vecDirShooting );
			VectorNormalize( bullet.m_vecDirShooting );
			//VectorAdd( tr.plane.normal, bullet.m_vecDirShooting, bullet.m_vecDirShooting );
			//VectorAdd( tr.plane.normal, outDir, vecDir );
			bullet.m_vecPosition = tr.endpos;
			bullet.m_flPower *= 0.75f;
			if ( sv_showimpacts.GetBool() )
			{
#ifdef CLIENT_DLL
				// draw red client impact markers
				debugoverlay->AddLineOverlay( bullet.m_vecPosition, vecEnd, 255,0,0,127, 6 );
#else
				NDebugOverlay::Line( bullet.m_vecPosition, vecEnd, 0,0, 255, false, 6 );
#endif
			}
		}
		else
		{

			if ( psurf ) 
				dist = GetRnLAmmoDef()->GetPenetrationDistance( bullet.m_iAmmoType, psurf->game.material ) * bullet.m_flPower; 
			
			// In case we found a wall we could penetrate, check the surfaceprop of the other side of the wall (e.g. for wallpaper|stone walls) --> use hardest material
			trace_t	penetrationTrace;
			UTIL_TraceLine( tr.endpos + ( bullet.m_vecDirShooting * 32 ), tr.endpos, MASK_SHOT, this, COLLISION_GROUP_NONE, &penetrationTrace );
			psurf = physprops->GetSurfaceData( penetrationTrace.surface.surfaceProps );
			if ( psurf  ) 
			{
				float dist2 = GetRnLAmmoDef()->GetPenetrationDistance( bullet.m_iAmmoType, psurf->game.material ) * bullet.m_flPower;
				if (dist2 < dist)
					dist = dist2;
			}
			//DevMsg(1,"penetrating %f\n",dist);
			if( dist > 0 )
			{
				// Move through the glass until we're at the other side
				Vector	testPos = tr.endpos + ( bullet.m_vecDirShooting * dist );

				// Re-trace as if the bullet had passed right through
				UTIL_TraceLine( testPos, tr.endpos, MASK_SHOT, this, COLLISION_GROUP_NONE, &penetrationTrace );

				// See if we found the surface again
				if ( penetrationTrace.startsolid || tr.fraction == 0.0f || penetrationTrace.fraction == 1.0f )
				{
					bTraceRefire = false;
				}
				else
				{
					UTIL_ImpactTrace( &penetrationTrace, iDamageType );
					bullet.m_flDistance += abs((penetrationTrace.endpos - bullet.m_vecPosition).Length());
					bullet.m_vecPosition = penetrationTrace.endpos;
				}
			}
			else
			{
				bTraceRefire = false;
			}
		}
	}while( bTraceRefire );
}

void CRnLPlayer::RemoveAllAmmo( void )
{
	BaseClass::RemoveAllAmmo();

	CBaseCombatWeapon *pWeap;

	for( int i = 0; i < WeaponCount(); i++ )
	{
		pWeap = GetWeapon( i );

		if( pWeap )
		{
			if( pWeap->m_iClip1 > 0 )
				pWeap->m_iClip1 = 0;

			if( pWeap->m_iClip2 > 0 )
				pWeap->m_iClip2 = 0;
		}
	}
}

void CRnLPlayer::TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr )
{
	CDisablePredictionFiltering disabler;

	// get the hitbox that was hit, dont change health or dmg info here, but send info to member variable
	// don't care about the direction just yet
	int hit = ptr->hitgroup;
#ifndef CLIENT_DLL
	// Prevent team damage here so blood doesn't appear
	if ( inputInfo.GetAttacker()->IsPlayer() )
	{
		if ( !g_pGameRules->FPlayerCanTakeDamage( this, inputInfo.GetAttacker() ) )
			return;
	}

	SetLastHitGroup( hit );
#endif

	if( hit == HITGROUP_RNL_HEAD || hit == HITGROUP_HEAD || hit == HITGROUP_RNL_NECK || ptr->hitbox == 11 )
		UTIL_DecalTrace( ptr, "HeadShotBlood" );

	UTIL_BloodDrips( ptr->endpos, vecDir, BloodColor(), ((int)inputInfo.GetDamage()) / 5);
	TraceBleed( inputInfo.GetDamage(), vecDir, ptr, inputInfo.GetDamageType() );
	AddMultiDamage( inputInfo, this );
}

//RnL : MovementMod : Begin
void CRnLPlayer::PlayMovingWoundedSound()
{
	if( gpGlobals->curtime > m_flNextMovingWoundedSound && IsAlive() )
	{
		CPASAttenuationFilter filter( this );	
		filter.UsePredictionRules();
		EmitSound( filter, entindex(), "Player.MoveWoundedPain");
		m_flNextMovingWoundedSound = gpGlobals->curtime + random->RandomInt( 3.0f, 6.0f );
	}
}

float CRnLPlayer::PlayStaminaSound( int iState )
{
	if( iState < 0 || iState > 14 )
		return 1.0f;

#ifdef CLIENT_DLL
	EmitSound( g_sSprintSoundInfo[iState].szName );
#else
	CPASAttenuationFilter filter( this );	
	filter.UsePredictionRules();
	EmitSound( filter, entindex(), g_sSprintSoundInfo[iState].szName);
#endif
	return g_sSprintSoundInfo[iState].fTime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLPlayer::GetStepSoundVelocities( float *velwalk, float *velrun )
{
	//On a ladder
	if( GetMoveType() == MOVETYPE_LADDER )
	{
		*velwalk = 15;
		*velrun = 999;
	}
	//Crouching or moving to crouch
	else if( IsDucked() )
	{
		*velwalk = 30;
		*velrun = 70;
	}
	//Prone or moving to prone
	else if( IsProne() )
	{
		*velwalk = 25;		// These constants should be based on cl_movespeedkey * cl_forwardspeed somehow
		*velrun = 45;		
	}
	else
	{
		*velwalk = 70;
		*velrun = 165;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLPlayer::SetStepSoundTime( stepsoundtimes_t iStepSoundTime, bool bWalking )
{
	switch ( iStepSoundTime )
	{
	case STEPSOUNDTIME_NORMAL:
	case STEPSOUNDTIME_WATER_FOOT:
		m_flStepSoundTime = bWalking ? 400 : 300;
		break;

	case STEPSOUNDTIME_ON_LADDER:
		m_flStepSoundTime = 450;
		break;

	case STEPSOUNDTIME_WATER_KNEE:
		m_flStepSoundTime = 600;
		break;

	default:
		Assert(0);
		break;
	}

	// UNDONE: need defined numbers for run, walk, crouch, crouch run velocities!!!!	
	if ( IsDucked() )
	{
		m_flStepSoundTime += 100;
	}
	else if( IsProne() )
	{
		m_flStepSoundTime += 400;
	}
}
//RnL : MovementMod : End