//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Model Entity of the Mines
//			
//
//=============================================================================//
#include "cbase.h"
#include "rnl_mine_entity.h"

extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
extern short	g_sModelIndexWExplosion;

LINK_ENTITY_TO_CLASS( rnl_mine_entity, CRnLMineEntity );

// Start of our data description for the class
BEGIN_DATADESC( CRnLMineEntity )

	// Declare our think function
	DEFINE_THINKFUNC( ExplodeThink ),

END_DATADESC()

void CRnLMineEntity::Spawn( void )
{
	BaseClass::Spawn();

	SetSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_FLYGRAVITY );
	SetSolid( SOLID_BBOX );	// So it will collide with physics props!

	// smaller, cube bounding box so we rest on the ground
	SetSize( Vector ( -2, -2, -2 ), Vector ( 2, 2, 2 ) );
}

void CRnLMineEntity::CreateMine( const char* pszModelName )
{
	SetModel( pszModelName );
	Spawn();
}


void CRnLMineEntity::ExplodeThink( void )
{
	trace_t		tr;
	Vector		vecSpot;// trace starts here!
	Vector vecAbsOrigin = GetAbsOrigin();
	trace_t		*pTrace;

	Vector velDir = GetAbsVelocity();
	VectorNormalize( velDir );
	vecSpot = GetAbsOrigin() - velDir * 32;
	UTIL_TraceLine( vecSpot, vecSpot + velDir * 64, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );
	
	int contents = UTIL_PointContents ( vecAbsOrigin );

	pTrace = &tr;

	Vector vecNormal = pTrace->plane.normal;
	surfacedata_t *pdata = physprops->GetSurfaceData( pTrace->surface.surfaceProps );	
	CPASFilter filter( vecAbsOrigin );
	te->Explosion( filter, -1.0, // don't apply cl_interp delay
	&vecAbsOrigin,
	!( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
	m_DmgRadius * .03, 
	25,
	TE_EXPLFLAG_NONE,
	m_DmgRadius,
	m_flDamage * 10.0,
	&vecNormal,
	(char) pdata->game.material );

	CTakeDamageInfo info( this, this, GetBlastForce(), GetAbsOrigin(), m_flDamage, DMG_BLAST, 0 );

	RadiusDamage( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

	EmitSound( "BaseGrenade.Explode" );

	SetThink(&CBaseEntity::SUB_Remove);
	SetNextThink( gpGlobals->curtime );
}