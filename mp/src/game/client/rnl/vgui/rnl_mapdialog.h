//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_MAPDIALOG_H
#define RNL_MAPDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include <game/client/iviewport.h>
#include "rnl_mappanel.h"
#include "rnl_shareddefs.h"

//-----------------------------------------------------------------------------
// Purpose: RnL Map
//-----------------------------------------------------------------------------
class CRnLMapDialog : public vgui::EditablePanel, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CRnLMapDialog, vgui::EditablePanel );

public:
	CRnLMapDialog( IViewPort *pViewPort );
	~CRnLMapDialog();

	virtual const char *GetName( void ) { return PANEL_MAP; }
	virtual void SetData(KeyValues *data) {};
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate( void );
	virtual bool HasInputElements( void ) { return false; }
	virtual void ShowPanel( bool bShow );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
	
protected:

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PostApplySchemeSettings( vgui::IScheme *pScheme );

	void MoveToCenterOfScreen();

private:
	float m_fNextUpdateTime;
	IViewPort	*m_pViewPort;
	CRnLMapPanel* m_pMapPanel;
};


#endif // RNL_MAPDIALOG_H
