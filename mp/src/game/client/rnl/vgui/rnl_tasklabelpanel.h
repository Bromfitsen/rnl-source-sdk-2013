//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: NCO map class. Contains all the details for the map specific nco map
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_TASKLABELPANEL_H
#define RNL_TASKLABELPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <UtlVector.h>
#include <vgui/ISurface.h>
#include <vgui/IBorder.h>

#include "rnl_itask.h"

class CRnLTaskStatusPanel;
class CRnLTaskLabelPanel : public vgui::Panel
{
	typedef vgui::Panel BaseClass;
public:
	// constructor
	CRnLTaskLabelPanel( CRnLTaskStatusPanel *pStatusPanelParent, vgui::Panel *pParent, const char *pName );
	~CRnLTaskLabelPanel();

	void Paint( void ) OVERRIDE;

private:
	CRnLTaskStatusPanel*		m_pStatusPanelParent;
	vgui::HFont					m_hSelfFont;
};

#endif //RNL_TASKLABELPANEL_H