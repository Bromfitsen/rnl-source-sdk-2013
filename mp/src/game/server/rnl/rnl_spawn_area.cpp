#include "cbase.h"
#include "rnl_spawn_area.h"
#include "rnl_game_team.h"
#include "viewport_panel_names.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CRnLSpawnArea )
	//Done so we can start active if we want to
	DEFINE_KEYFIELD( m_angPlayerFaceDir, FIELD_VECTOR, "spawndir" ),
	DEFINE_KEYFIELD( m_bBaseSpawn, FIELD_BOOLEAN, "basespawn" ),
	DEFINE_KEYFIELD( m_iIntitialOwner, FIELD_INTEGER, "initialowner" ),

	DEFINE_KEYFIELD( m_NextSpawnArea[0], FIELD_STRING, "AlliedSpawn" ),
	DEFINE_KEYFIELD( m_NextSpawnArea[1], FIELD_STRING, "AxisSpawn" ),

	DEFINE_KEYFIELD( m_FallbackSpawnArea[0], FIELD_STRING, "AlliedFallbackSpawn" ),
	DEFINE_KEYFIELD( m_FallbackSpawnArea[1], FIELD_STRING, "AxisFallbackSpawn" ),
	
	DEFINE_INPUTFUNC( FIELD_VOID, "EnableFallback", InputEnableFallback ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisableFallback", InputDisableFallback ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnableAllies", InputEnableAllies ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnableAxis", InputEnableAxis ),

	//Added this for good measure
	DEFINE_THINKFUNC( Think ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST_NOBASE(CRnLSpawnArea, DT_RnLSpawnArea)
	SendPropEHandle	(SENDINFO(m_hNextSpawnAllies)),
	SendPropEHandle	(SENDINFO(m_hNextSpawnAxis)),
	SendPropEHandle	(SENDINFO(m_hFallbackSpawnAllies)),
	SendPropEHandle	(SENDINFO(m_hFallbackSpawnAxis)),
	SendPropVector	(SENDINFO(m_vecCenter), -1,  SPROP_COORD ),
	SendPropBool	(SENDINFO(m_bFallback) ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( rnl_spawn_area, CRnLSpawnArea  );

CRnLSpawnArea::CRnLSpawnArea()
{
	m_hNextSpawnAllies = NULL;
	m_hNextSpawnAxis = NULL;

	m_hFallbackSpawnAllies = NULL;
	m_hFallbackSpawnAxis = NULL;

	m_vecCenter.Init();
	m_bFallback = false;

	m_iIntitialOwner = TEAM_NONE;
	m_bBaseSpawn = false;
}

void CRnLSpawnArea::Activate()
{
	BaseClass::Activate();

	ChangeTeam( m_iIntitialOwner );
	
	if( m_bBaseSpawn )
	{
		CRnLGameTeam* pGameTeam = (CRnLGameTeam*)GetGlobalRnLTeam(m_iIntitialOwner);
		if( pGameTeam )
		{
			pGameTeam->SetBaseSpawn( this );
		}
	}

	CBaseEntity* pSpawn = gEntList.FindEntityByName( NULL, m_NextSpawnArea[0] );
	if( pSpawn != NULL )
		m_hNextSpawnAllies = (CRnLSpawnArea*)(pSpawn);
	pSpawn = gEntList.FindEntityByName( NULL, m_NextSpawnArea[1] );
	if( pSpawn != NULL )
		m_hNextSpawnAxis = (CRnLSpawnArea*)(pSpawn);
	
	pSpawn = gEntList.FindEntityByName( NULL, m_FallbackSpawnArea[0] );
	if( pSpawn != NULL )
		m_hFallbackSpawnAllies = (CRnLSpawnArea*)(pSpawn);
	pSpawn = gEntList.FindEntityByName( NULL, m_FallbackSpawnArea[1] );
	if( pSpawn != NULL )
		m_hFallbackSpawnAxis = (CRnLSpawnArea*)(pSpawn);
}

void CRnLSpawnArea::Spawn()
{
	m_bFallback = false;

	// Bind to our bmodel.
	SetModel( STRING( GetModelName() ) );
	AddSolidFlags( FSOLID_VOLUME_CONTENTS );
	AddEffects( EF_NODRAW );
	
	// calculate center of this entity
	Vector vecSurroundMins, vecSurroundMaxs;

	// get the boundaries in world space to set player height
	if ( edict())
		CollisionProp()->WorldSpaceAABB( &vecSurroundMins, &vecSurroundMaxs );

	m_vecCenter = 0.5f * (vecSurroundMins + vecSurroundMaxs);

	// start thinking
	//Think();

	BaseClass::Spawn();
}

void CRnLSpawnArea::InputEnableFallback( inputdata_t &inputdata )
{
	m_bFallback = true;
}

void CRnLSpawnArea::InputDisableFallback( inputdata_t &inputdata )
{
	m_bFallback = false;
}

void CRnLSpawnArea::InputDisable( inputdata_t &inputdata )
{
	if( m_bBaseSpawn )
		return;

	ChangeTeam( TEAM_NONE );
}

void CRnLSpawnArea::InputEnable( inputdata_t &inputdata )
{
	if( m_bBaseSpawn )
		return;

	ChangeTeam( m_iIntitialOwner );
}

void CRnLSpawnArea::InputEnableAllies( inputdata_t &inputdata )
{
	if( m_bBaseSpawn )
		return;

	ChangeTeam( TEAM_ALLIES );
}

void CRnLSpawnArea::InputEnableAxis( inputdata_t &inputdata )
{
	if( m_bBaseSpawn )
		return;

	ChangeTeam( TEAM_AXIS );
}

bool CRnLSpawnArea::OnPlayerSpawn( CBasePlayer* pPlayer )
{
	if( !pPlayer )
		return false;

	if( pPlayer->GetTeamNumber() != GetTeamNumber() )
		return false;

	if( pPlayer->GetTeamNumber() == TEAM_AXIS && m_hNextSpawnAxis.Get() != NULL )
	{
		if( m_hNextSpawnAxis->OnPlayerSpawn( pPlayer ) )
			return true;
	}
	else if( pPlayer->GetTeamNumber() == TEAM_ALLIES && m_hNextSpawnAllies.Get() != NULL )
	{
		if( m_hNextSpawnAllies->OnPlayerSpawn( pPlayer ) )
			return true;
	}

	if( m_bFallback )
	{
		if( pPlayer->GetTeamNumber() == TEAM_AXIS && m_hFallbackSpawnAxis.Get() != NULL )
			return m_hFallbackSpawnAxis->OnPlayerSpawn( pPlayer );
		else if( pPlayer->GetTeamNumber() == TEAM_ALLIES && m_hFallbackSpawnAllies.Get() != NULL )
			return m_hFallbackSpawnAllies->OnPlayerSpawn( pPlayer );
		else if( !m_bBaseSpawn )//If this is the root of the spawn tree don't skip
			return false;
	}

	Vector vecSpawnpoint;
	

	for( int i = 0; i < 10; i++ )
	{
		// determine first spawn "point" to test
		CollisionProp()->RandomPointInBounds( Vector(0.1,0.1, 0.1), Vector(0.9,0.9,0.2), &vecSpawnpoint );

		trace_t tr;
		UTIL_TraceHull( vecSpawnpoint, vecSpawnpoint, VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER_MOVEMENT, &tr );

		if( !(tr.DidHit()) )
		{
			pPlayer->SetLocalOrigin( vecSpawnpoint );
			pPlayer->SetAbsVelocity( vec3_origin );
			pPlayer->SetLocalAngles( m_angPlayerFaceDir );
			pPlayer->SetAbsAngles( m_angPlayerFaceDir );
			pPlayer->m_Local.m_vecPunchAngle = vec3_angle;
			pPlayer->m_Local.m_vecPunchAngleVel = vec3_angle;
			pPlayer->SnapEyeAngles( m_angPlayerFaceDir );
			pPlayer->ShowViewPortPanel( PANEL_SPECGUI, false );
			return true;
		}
	}

	return false;
}

static ConVar rnl_drawspawnareadebug( "rnl_drawspawnareadebug", "0", FCVAR_CHEAT,
      "Draw outline around spawn areas. Red = Axis controlled, Green = Allied controlled, Yellow = Neither" );

void CRnLSpawnArea::Think()
{
	if( rnl_drawspawnareadebug.GetInt() > 0 )
	{
		int red = 0, green = 0, blue = 0;

		// get team colour
		switch( GetTeamNumber() )
		{
		case TEAM_ALLIES:
			green = 255;
			break;
		case TEAM_AXIS:
			red = 255;
			break;
		default:
			red = 255; green = 255;
			break;
		}

		if (edict())
		{
			char szDisplay[64];
			Q_snprintf(szDisplay, sizeof(szDisplay), "AbsOrigin: (%i, %i, %i)", (int)m_vecCenter.GetX(), (int)m_vecCenter.GetY(), (int)m_vecCenter.GetZ() );

			NDebugOverlay::EntityBounds(this, red, green, blue, 0,10.0f);
			NDebugOverlay::EntityText(entindex(), 0, szDisplay, 10.0f, red, green, blue );
		}
	}

	SetNextThink( gpGlobals->curtime + 10.0f );
}