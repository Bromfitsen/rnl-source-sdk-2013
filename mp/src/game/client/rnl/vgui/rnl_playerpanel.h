//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_PLAYERPANEL_H
#define RNL_PLAYERPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <game/client/iviewport.h>
#include "gameeventlistener.h"

class CRnLMapPanel;
class CRnLPlayerPlayerPanel : public vgui::Panel
{
	typedef vgui::Panel BaseClass;
public:
	// constructor
	CRnLPlayerPlayerPanel( CRnLMapPanel *pMapParent, vgui::Panel *pParent, const char *pName );

	virtual void Paint( void );
	virtual void PaintBackground( void ) {}

private:
	void DrawPlayer( Vector2D panelPos, float angle, float size, int iTeam );

	CRnLMapPanel		*m_pNCOParent;
	int					m_iPlayerIcon;
	vgui::HFont			m_hSelfFont;
};

#endif //RNL_PLAYERPANEL_H