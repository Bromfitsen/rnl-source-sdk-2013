#include "cbase.h"
#include "rnl_hitgroupdata.h"
#include "rnl_shareddefs.h"

#include "tier0/memdbgon.h"

bool GetRnLHitGroupDetails_LessThan( const int& i, const int& j )
{
	return i < j;
}

CUtlMap<int, CRnLHitGroupDetails>& GetRnLHitGroupDetails( void )
{
	static CUtlMap<int, CRnLHitGroupDetails> g_RnLHitGroupDetailsDef(GetRnLHitGroupDetails_LessThan);
	static bool bInitted = false;

	if ( !bInitted )
	{
		bInitted = true;

		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_GENERIC )].Set( 1.0f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_HEAD )].Set( 5.0f, true );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_CHEST )].Set( 3.0f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_STOMACH )].Set( 2.0f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_LEFTARM )].Set( 1.0f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_LEFTLEG )].Set( 1.0f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RIGHTLEG )].Set( 1.0f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_GEAR )].Set( 1.0f, false );

		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RNL_GENERIC )].Set( 1.0f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RNL_HEAD )].Set( 5.0f, true );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RNL_NECK )].Set( 4.0f, true );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RNL_UPPER_ARM )].Set( 1.5f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RNL_LOWER_ARM )].Set( 1.25f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RNL_HAND )].Set( 1.0f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RNL_CHEST )].Set( 3.0f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RNL_STOMACH )].Set( 2.0f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RNL_WAIST )].Set( 1.5f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RNL_UPPER_LEG )].Set( 2.0f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RNL_LOWER_LEG )].Set( 1.0f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RNL_FOOT )].Set( 1.0f, false );
		g_RnLHitGroupDetailsDef[g_RnLHitGroupDetailsDef.Insert( HITGROUP_RNL_EQUIPMENT )].Set( 1.0f, false );
	}

	return g_RnLHitGroupDetailsDef;
}