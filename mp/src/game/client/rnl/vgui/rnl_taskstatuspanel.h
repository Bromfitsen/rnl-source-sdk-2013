//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: NCO map class. Contains all the details for the map specific nco map
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_TASKSTATUSPANEL_H
#define RNL_TASKSTATUSPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <UtlVector.h>
#include <vgui/ISurface.h>

#include <vgui/IBorder.h>

#include "rnl_itask.h"

// Capture zones and spawn icon stuff
#define ICON_ALLIES_LOCALPLAYER		"overview/player_blue"
#define ICON_AXIS_LOCALPLAYER		"overview/player_blue"

//#define ICON_ALLIES_AVAILABLE_SPAWN "VGUI/overview/airborne_available_spawn"
//#define ICON_AXIS_AVAILABLE_SPAWN	"VGUI/overview/axis_available_spawn"

#define ICON_ALLIES_SELECTED_NODE	"VGUI/overview/us_selected_spawn"
#define ICON_AXIS_SELECTED_NODE		"VGUI/overview/ger_selected_spawn"

#define ICON_ALLIES_UNCAPTURED_NODE	"VGUI/overview/us_enemy_node"
#define ICON_AXIS_UNCAPTURED_NODE	"VGUI/overview/ger_enemy_node"

#define ICON_ALLIES_CAPTURED_NODE	"VGUI/overview/us_captured_node"
#define ICON_AXIS_CAPTURED_NODE		"VGUI/overview/ger_captured_node"

#define ICON_ALLIES_BLOCKED_NODE	"VGUI/overview/us_blocked_spawn"
#define ICON_AXIS_BLOCKED_NODE		"VGUI/overview/ger_blocked_spawn"

#define ICON_ALLIES_NEUTRAL_NODE	"VGUI/overview/us_neutral_node"
#define ICON_AXIS_NEUTRAL_NODE		"VGUI/overview/ger_neutral_node"

#define ICON_ALLIES_NEUTRAL_DESTROY	"VGUI/overview/rnl_map_icons_destr_allies"
#define ICON_AXIS_NEUTRAL_DESTROY	"VGUI/overview/rnl_map_icons_destr_axis"

//#define ICON_ALLIES_CAPTURING_NODE	"VGUI/overview/airborne_capturing_node"
//#define ICON_AXIS_CAPTURING_NODE	"VGUI/overview/axis_capturing_node"

#define PLAYER_ICON_SIZE_X 32
#define PLAYER_ICON_SIZE_Y 32

#define	NODE_ICON_SIZE_X 32
#define NODE_ICON_SIZE_Y 32

#define SPAWN_ICON_SIZE_X 32
#define SPAWN_ICON_SIZE_Y 32

#define TASK_NAME_MAX 64
#define TASK_CAPTION_MAX 64

typedef struct tRnLTaskStatusInfo {
	char szTaskName[TASK_NAME_MAX+1];
	char szCaption[TASK_CAPTION_MAX+1];
	int textPos;
	int iconPos;
	Vector2D statusPos;
	CUtlVector<CUtlVector<Vector>>			worldPolies;
	CUtlVector<CUtlVector<vgui::Vertex_t>>	mapPolies;

	bool visible;
	int taskStatus;
	int taskType;
	Vector taskOrigin;
	Vector taskExtentMin;
	Vector taskExtentMax;
}RnLTaskStatusInfo;

class CRnLMapPanel;
class CRnLTaskStatusPanel : public vgui::Panel
{
	typedef vgui::Panel BaseClass;
public:
	// constructor
	CRnLTaskStatusPanel( CRnLMapPanel *pMapParent, vgui::Panel *pParent, const char *pName );
	~CRnLTaskStatusPanel();

	virtual void Paint( void );
	virtual void PaintBackground( void ) {}
	virtual void Update( void );
	virtual void Initialise( KeyValues* pMapInfo );
	virtual CRnLMapPanel* GetMapParent( void ) { return m_pMapParent; }
	CUtlVector<RnLTaskStatusInfo>& GetTaskInfo( void ) { return m_TaskInfo; }

private:
	CUtlVector<RnLTaskStatusInfo> m_TaskInfo;
	CRnLMapPanel*				m_pMapParent;
	int							m_iTaskStateTextures[RNL_TASK_TYPE_MAX][6];
};


#endif //RNL_TASKSTATUSPANEL_H