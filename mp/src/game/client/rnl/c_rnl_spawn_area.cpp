#include "cbase.h"
//#include "cl_dll\iviewport.h"
#include "c_rnl_spawn_area.h"
//#include "rnl_ncomap.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_RnLSpawnArea, DT_RnLSpawnArea, CRnLSpawnArea )
	RecvPropEHandle( RECVINFO( m_hNextSpawnAllies ) ),	
	RecvPropEHandle( RECVINFO( m_hNextSpawnAxis ) ),
	RecvPropEHandle	(RECVINFO(m_hFallbackSpawnAllies)),
	RecvPropEHandle	(RECVINFO(m_hFallbackSpawnAxis)),
	RecvPropVector( RECVINFO( m_vecCenter ) ),
	RecvPropBool( RECVINFO( m_bFallback ) ),
END_RECV_TABLE()

void C_RnLSpawnArea::UpdateOnRemove()
{
	/*
	IViewPortPanel *NCOMapPanel = gViewPortInterface->FindPanelByName( PANEL_NCOMAP );

	if( NCOMapPanel )
	{
		CRnLNCOMap *pRnLNCOMap = (CRnLNCOMap*)NCOMapPanel;
		if( pRnLNCOMap )
		{
			pRnLNCOMap->RemoveSpawnAreaFromMap( GetRefEHandle() );
		}
	}

	if( RnLTacticalEngagementGameRules() )
	{
		RnLTacticalEngagementGameRules()->RemoveSpawnArea( GetRefEHandle() );
	}*/
	BaseClass::UpdateOnRemove();
}

void C_RnLSpawnArea::OnDataChanged( DataUpdateType_t type )
{
	/*
	// link into the ncomap here
	if( type == DATA_UPDATE_CREATED )
	{
		IViewPortPanel *NCOMapPanel = gViewPortInterface->FindPanelByName( PANEL_NCOMAP );

		if( NCOMapPanel )
		{
			CRnLNCOMap *pRnLNCOMap = (CRnLNCOMap*)NCOMapPanel;
			if( pRnLNCOMap )
			{
				pRnLNCOMap->AddSpawnAreaToMap( GetRefEHandle() );
			}
		}

		if( RnLTacticalEngagementGameRules() )
		{
			RnLTacticalEngagementGameRules()->AddSpawnArea( GetRefEHandle() );
		}
	}
	*/
	BaseClass::OnDataChanged( type );
}