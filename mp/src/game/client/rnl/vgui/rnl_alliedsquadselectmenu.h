//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef ALLIEDSQUADSELECT_H
#define ALLIEDSQUADSELECT_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <game/client/iviewport.h>
#include <vgui_imagebutton.h>

#include <igameevents.h>
#include "rnl_shareddefs.h"

//-----------------------------------------------------------------------------
// Purpose: Squad selection panel
//-----------------------------------------------------------------------------
class CRnLAlliedSquadSelectMenu : public vgui::Frame, public IViewPortPanel, public IGameEventListener2
{
private:
	DECLARE_CLASS_SIMPLE( CRnLAlliedSquadSelectMenu, vgui::Frame );

protected:
	// column widths at 640
	enum { NAME_WIDTH = 80, CLASS_WIDTH = 70 };
	// total = 260

public:
	CRnLAlliedSquadSelectMenu( IViewPort *pViewPort );
	virtual ~CRnLAlliedSquadSelectMenu();

	virtual const char *GetName( void ) { return PANEL_SQUAD_ALLIES; }
	virtual void SetData(KeyValues *data);
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate( void );
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	// IGameEventListener interface:
	virtual void FireGameEvent( IGameEvent *event);

	MESSAGE_FUNC_PARAMS( OnSquadListItemSelected, "ItemSelected", data);

	// command callbacks
	void OnCommand( const char *command );

protected:

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PaintBackground( void );

	vgui::ImageButton *m_pCancelButton;

	// the two lists, broken into two fireteams
	CUtlVector< vgui::ListPanel*> m_pSquadList;

private:
	IViewPort	*m_pViewPort;
	float		m_fNextUpdateTime;

};


#endif // ALLIEDSQUADSELECT_H
