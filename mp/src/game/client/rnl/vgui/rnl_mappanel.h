//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: NCO map class. Contains all the details for the map specific nco map
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_MAPPANEL_H
#define RNL_MAPPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <UtlVector.h>
#include <vgui/ISurface.h>
#include <vgui/IBorder.h>

#include "gameeventlistener.h"
#include "rnl_playerpanel.h"
#include "rnl_taskstatuspanel.h"
#include "rnl_tasklabelpanel.h"
#include "rnl_objectivestatuspanel.h"

typedef struct MapToWorldInfo_t
{
	Vector2D m_OffsetMapElements;
	bool m_bReverseY;
	float m_flMapScale;
	float angle;
}MapToWorldInfo;

class CRnLMapPanel : public vgui::Panel, public CGameEventListener
{
	typedef vgui::Panel BaseClass;
public:
	// constructor
	CRnLMapPanel( vgui::Panel *pParent, const char *pName );
	~CRnLMapPanel();

	// IGameEventListener interface:
	void FireGameEvent( IGameEvent *event) OVERRIDE;

	void Paint( void ) OVERRIDE;
	void PaintBackground( void ) OVERRIDE {}
	void Update( void ) ;
	void Initialise( const char* mapname );
	Vector2D WorldToMapCoords( Vector worldPos );
	void DrawArrow( Vector2D panelPos, Vector2D endPos, int iArrowSize );
	float GetMapAngle( int iTeam ) ;

private:
	MapToWorldInfo				m_mapToWorldInfo[2];
	int							m_iMapOffset;
	int							m_iMapFactor;
	vgui::ImagePanel*			m_pMapImage[2];
	CRnLPlayerPlayerPanel*		m_pPlayerPanel;
	CRnLTaskStatusPanel*		m_pRnLTaskPanel;
	CRnLTaskLabelPanel*			m_pRnLTaskLabelPanel;
	CRnLObjectiveStatusPanel*	m_pRnLPrimaryObjectivePanel;
	CRnLObjectiveStatusPanel*	m_pRnLSecondaryObjectivePanel;
};


#endif //RNL_MAPPANEL_H