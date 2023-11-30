//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: RnL resupply crate
//
//=============================================================================//

#include "cbase.h"
#include "rnl_player.h"
#include "rnl_gamerules.h"
#include "weapon_rnlballisticbase.h"
#include "weapon_rnl_basegrenade.h"
#include "rnl_ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Ammo drop
class CRnLResupplyAmmo : public CBaseAnimating
{
public:
	DECLARE_CLASS( CRnLResupplyAmmo, CBaseAnimating );

	DECLARE_DATADESC();
	/*
	DECLARE_SERVERCLASS();
	*/
	CRnLResupplyAmmo();

	void	Spawn( void ) OVERRIDE;
	void	Precache( void ) OVERRIDE;
	bool	CreateVPhysics( void ) OVERRIDE;

	int		ObjectCaps( void ) OVERRIDE { return (BaseClass::ObjectCaps() | (FCAP_IMPULSE_USE|FCAP_USE_IN_RADIUS)); };
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) OVERRIDE;

	void	InputKill( inputdata_t &data );

	void SetAmmoModel( const char szAmmoModel[] );

	int		GetAmmoType( void );
	void	SetAmmoType( int iType );

	int		GetAmmCount( void );
	void	SetAmmoCount( int cnt );

	void	SetLifeTime( float fLength );

	void	SetSourceClass( const char *szSourceClass );
	char*	GetSourceClass( void );

	COutputEvent	m_OnUsed;

protected:

	float m_flLifeTime;
	int m_iAmmoType;
	int m_iAmmoCount;
	char m_szSourceClass[32];
	char m_szAmmoModel[80];
};

LINK_ENTITY_TO_CLASS( rnl_resupply_ammo, CRnLResupplyAmmo );

BEGIN_DATADESC( CRnLResupplyAmmo )

	DEFINE_KEYFIELD( m_iAmmoType,	FIELD_INTEGER, "Type" ),	
	DEFINE_KEYFIELD( m_iAmmoCount,  FIELD_INTEGER, "Amount" ),
	DEFINE_KEYFIELD( m_szSourceClass, FIELD_STRING, "SourceClass" ),

	DEFINE_OUTPUT( m_OnUsed, "OnUsed" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Kill", InputKill ),

END_DATADESC()

// This table encodes the CBaseEntity data.
/*
IMPLEMENT_SERVERCLASS_ST(CRnLResupplyAmmo, DT_RnLResupplyAmmo)
END_SEND_TABLE()
*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CRnLResupplyAmmo::CRnLResupplyAmmo()
{
	m_iAmmoType = -1;
	m_iAmmoCount = 0;
	m_flLifeTime = -1;
	Q_strcpy(m_szSourceClass, "");
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLResupplyAmmo::Spawn( void )
{
	BaseClass::Spawn();

	SetModel( m_szAmmoModel );
	if( CreateVPhysics() )
		SetMoveType( MOVETYPE_VPHYSICS );
	else
		SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );

	SetCollisionGroup( COLLISION_GROUP_DEBRIS );

	SetGroundEntity( NULL );
	RemoveEffects( EF_NODRAW );

	//ResetSequence( ACT_IDLE );
	//SetCycle( 0 );
	UseClientSideAnimation();
	m_takedamage = DAMAGE_NO;

	if( m_flLifeTime > 0 )
	{
		SetThink( &CBaseEntity::SUB_Remove );
		SetNextThink( m_flLifeTime );
	}
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
bool CRnLResupplyAmmo::CreateVPhysics( void )
{
	return ( VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, false ) != NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLResupplyAmmo::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLResupplyAmmo::SetAmmoModel( const char szAmmoModel[] )
{
	Q_strncpy( m_szAmmoModel, szAmmoModel, sizeof( m_szAmmoModel ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CRnLResupplyAmmo::GetAmmoType( void )
{
	return m_iAmmoType;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLResupplyAmmo::SetAmmoType( int iType )
{
	m_iAmmoType = iType;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CRnLResupplyAmmo::GetAmmCount( void )
{
	return m_iAmmoCount;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLResupplyAmmo::SetLifeTime( float fLength )
{
	m_flLifeTime = gpGlobals->curtime + fLength;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLResupplyAmmo::SetAmmoCount( int cnt )
{
	m_iAmmoCount = cnt;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLResupplyAmmo::SetSourceClass( const char *szSourceClass )
{
	Q_strcpy(m_szSourceClass, szSourceClass);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
char *CRnLResupplyAmmo::GetSourceClass( void )
{
	return m_szSourceClass;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActivator - 
//			*pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CRnLResupplyAmmo::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CRnLPlayer *pPlayer = ToRnLPlayer( pActivator );

	if ( pPlayer == NULL )
		return;

	m_OnUsed.FireOutput( pActivator, this );

	// If this is grenade ammo, and the player doesn't have this weapon, but CAN own it, give it to them.
	if( ((Q_strcmp(GetSourceClass(), "weapon_m18grenade") == 0) ||
		 (Q_strcmp(GetSourceClass(), "weapon_mk2grenade") == 0) ||
		 (Q_strcmp(GetSourceClass(), "weapon_sticky24grenade") == 0) ||
		 (Q_strcmp(GetSourceClass(), "weapon_sticky39grenade") == 0)) && 
		 !pPlayer->Weapon_OwnsThisType( GetSourceClass() ) )
	{
		int iAmmoCount = pPlayer->GetAmmoCount( m_iAmmoType );

		pPlayer->GiveNamedItem( GetSourceClass() );
		pPlayer->SetAmmoCount( iAmmoCount + m_iAmmoCount, m_iAmmoType );
		UTIL_Remove( this );
		return;
	}

	CBaseCombatWeapon* pWeapon = NULL;
	for( int i = 0; i < pPlayer->WeaponCount(); i++ )
	{
		pWeapon = pPlayer->GetWeapon( i );
		if( pWeapon )
		{
			if( pWeapon->GetPrimaryAmmoType() == m_iAmmoType ||
				pWeapon->GetSecondaryAmmoType() == m_iAmmoType )
			{
				if( pPlayer->GiveAmmo( m_iAmmoCount, m_iAmmoType ) > 0 )
					UTIL_Remove( this );
				if( pPlayer->GetUseEntity() == this )
					pPlayer->SetUseEntity( NULL );

				return;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CRnLResupplyAmmo::InputKill( inputdata_t &data )
{
	UTIL_Remove( this );
}

//----------------------------------------------------------------------------- 
// Purpose: Drops player's current weapon
//-----------------------------------------------------------------------------
void CC_Player_DropAmmo( void )
{
	CRnLPlayer *pPlayer = ToRnLPlayer( UTIL_GetCommandClient() );

	if( !pPlayer )
		return;

	if( pPlayer->IsAlive() && ( pPlayer->GetTeamNumber() == TEAM_ALLIES || pPlayer->GetTeamNumber() == TEAM_AXIS ) )
	{
		if( pPlayer->GetMovementPosture() != MOVEMENT_POSTURE_CLIMBING && 
			pPlayer->GetMovementPosture() != MOVEMENT_POSTURE_CLIMBING_TO_CROUCH && 
			pPlayer->GetMovementPosture() != MOVEMENT_POSTURE_PARACHUTING && 
			pPlayer->GetMoveType() != MOVETYPE_LADDER &&
			pPlayer->IsDeployed() == false )
		{
			CWeaponRnLBase* pRnLWep = pPlayer->GetActiveRnLWeapon();
			if( pRnLWep )
			{
				int ammoType = pRnLWep->GetPrimaryAmmoType();
				int ammoCount = pPlayer->GetAmmoCount( ammoType );

				// Don't let the player drop ammo if they
				// are in the process of throwing a grenade.
				if( pRnLWep->IsGrenade() && !pRnLWep->CanHolster() )
					return;

				if( ammoCount > 0 )
				{
					float throwForce = random->RandomFloat( 190.0f, 200.0f );
					Vector vecThrow = pPlayer->BodyDirection2D() * throwForce;

					CRnLResupplyAmmo* pAmmo = static_cast<CRnLResupplyAmmo*>(CBaseEntity::CreateNoSpawn( "rnl_resupply_ammo", pPlayer->Weapon_ShootPosition() - Vector(0,0,random->RandomFloat(10.0, 12.0)) + (vecThrow * 0.1f), pPlayer->GetAbsAngles() ));

					int amnt = pRnLWep->IsGrenade() ? 1 : min( ammoCount, pRnLWep->GetMaxClip1() );
					pPlayer->RemoveAmmo( amnt, ammoType );

					pAmmo->SetAmmoModel( pRnLWep->GetRnLWpnData().m_szClipModel );
					pAmmo->SetAmmoCount( amnt );
					pAmmo->SetAmmoType( ammoType );
					pAmmo->SetLifeTime( 30.0f );
					pAmmo->SetSourceClass( pRnLWep->GetClassname() );
					DispatchSpawn( pAmmo );
					pAmmo->ApplyAbsVelocityImpulse( vecThrow );
				}

				pPlayer->DoAnimationEvent( PLAYERANIMEVENT_DRAW );
				pRnLWep->HandleViewAnimation( WEAPON_ANIMATION_DRAW );
			}
		}
	}
}

static ConCommand dropammo("dropammo", CC_Player_DropAmmo, "dropammo: Drops a clip of the player's current weapon's ammo type.");