#include "cbase.h"
#include "rnl_mapentities.h"
#include "rnl_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static const char *s_RnLNoCreationEnts[] =
{
	/*
	//RnL
	"rnl_objective",
	"rnl_objective_order_filter",
	"rnl_task_area_capture",
	"rnl_task_perimeter_capture",
	*/
	"", // END Marker
};

static const char *s_RnLPreserveEnts[] =
{
	"player",
	"viewmodel",
	"worldspawn",
	"soundent",
	"ai_network",
	"ai_hint",
	"env_soundscape",
	"env_soundscape_proxy",
	"env_soundscape_triggerable",
	"env_sprite",
	"env_sun",
	"env_wind",
	"env_fog_controller",
	"func_wall",
	"func_illusionary",
	"info_node",
	"info_target",
	"info_node_hint",
	"point_commentary_node",
	"point_viewcontrol",
	"func_precipitation",
	"func_team_wall",
	"shadow_control",
	"sky_camera",
	"scene_manager",
	"trigger_soundscape",
	"commentary_auto",
	"point_commentary_node",
	"point_commentary_viewpoint",
	//RnL
	"rnl_gamerules",
	"rnl_team_manager",
	"rnl_team_allies",
	"rnl_team_axis",
	"", // END Marker
};

// Utility function
bool UTIL_FindInList( const char **pStrings, const char *pToFind )
{
	int i = 0;
	while ( pStrings[i][0] != 0 )
	{
		if ( Q_stricmp( pStrings[i], pToFind ) == 0 )
			return true;
		i++;
	}

	return false;
}

CRnLEntityFilter::CRnLEntityFilter()
{
}

bool CRnLEntityFilter::ShouldCreateEntity( const char *pClassname )
{
	if ( UTIL_FindInList( s_RnLNoCreationEnts, pClassname ) )
		return false;

	return CMapLoadEntityFilter::ShouldCreateEntity( pClassname );
}

CBaseEntity* CRnLEntityFilter::CreateNextEntity( const char *pClassname )
{
	return CMapLoadEntityFilter::CreateNextEntity( pClassname );
}

CRnLRespawnEntityFilter::CRnLRespawnEntityFilter()
{
}

bool CRnLRespawnEntityFilter::ShouldCreateEntity( const char *pClassname )
{
	// Don't recreate the preserved entities.
	if ( UTIL_FindInList( s_RnLPreserveEnts, pClassname ) )
		return false;

	// Increment our iterator since it's not going to call CreateNextEntity for this ent.
	if ( m_iIterator != g_MapEntityRefs.InvalidIndex() )
	{
		m_iIterator = g_MapEntityRefs.Next( m_iIterator );
	}

	return false;
}


CBaseEntity* CRnLRespawnEntityFilter::CreateNextEntity( const char *pClassname )
{
	if ( m_iIterator == g_MapEntityRefs.InvalidIndex() )
	{
		// This shouldn't be possible. When we loaded the map, it should have used 
		// CTeamplayMapEntityFilter, which should have built the g_MapEntityRefs list
		// with the same list of entities we're referring to here.
		Assert( false );
		return NULL;
	}
	else
	{
		CMapEntityRef &ref = g_MapEntityRefs[m_iIterator];
		m_iIterator = g_MapEntityRefs.Next( m_iIterator );	// Seek to the next entity.

		if ( ref.m_iEdict == -1 || engine->PEntityOfEntIndex( ref.m_iEdict ) )
		{
			// Doh! The entity was delete and its slot was reused.
			// Just use any old edict slot. This case sucks because we lose the baseline.
			return CreateEntityByName( pClassname );
		}
		else
		{
			// Cool, the slot where this entity was is free again (most likely, the entity was 
			// freed above). Now create an entity with this specific index.
			return CreateEntityByName( pClassname, ref.m_iEdict );
		}
	}
}