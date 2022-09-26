//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: NCO map class. Contains all the details for the map specific nco map
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_OBJECTIVESTATUSPANEL_H
#define RNL_OBJECTIVESTATUSPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <UtlVector.h>
#include <vgui/ISurface.h>

#include <vgui/IBorder.h>


#define ICON_NEUTRAL_OBJECTIVE	"VGUI/overview/objective_grey"

#define ICON_ALLIES_OBJECTIVE	"VGUI/overview/objective_allies_control"
#define ICON_AXIS_OBJECTIVE	"VGUI/overview/objective_axis_control"

#define ICON_ALLIES_ARROW	"VGUI/overview/task_allies_control"
#define ICON_AXIS_ARROW	"VGUI/overview/task_axis_control"

#define ARROW_TEXTURE "VGUI/overview/objective_arrow"

#define OBJECTIVE_NAME_MAX 64
#define OBJECTIVE_CAPTION_MAX 64

typedef struct tRnLObjectiveStatusInfo {
	
	char szObjectiveName[OBJECTIVE_NAME_MAX+1];
	char szCaption[OBJECTIVE_CAPTION_MAX+1];
	int textPos;
	Vector displayPos;

	CUtlVector<Vector>	taskPositions;
	CUtlVector<Vector>	requirementPositions;
	bool visible;
	bool iconVisible;
	int objStatus;
	bool primary;
}RnLObjectiveStatusInfo;

class CRnLMapPanel;
class CRnLObjectiveStatusPanel : public vgui::Panel
{
	typedef vgui::Panel BaseClass;
public:
	// constructor
	CRnLObjectiveStatusPanel( bool primaryObjectives, CRnLMapPanel *pMapParent, vgui::Panel *pParent, const char *pName );
	~CRnLObjectiveStatusPanel();

	virtual void Paint( void );
	virtual void PaintBackground( void ) {}
	virtual void Update( void );
	virtual void Initialise( KeyValues* pMapInfo );

private:
	CRnLMapPanel*				m_pMapParent;
	CUtlVector<RnLObjectiveStatusInfo> m_ObjectiveInfo;
	int							m_iObjectiveStateTextures[5];
	int							m_iArrowTexture;
	bool						m_PrimaryObjectives;
	vgui::HFont					m_hSelfFont;
	int							m_iArrowSize;
};


#endif //RNL_OBJECTIVESTATUSPANEL_H