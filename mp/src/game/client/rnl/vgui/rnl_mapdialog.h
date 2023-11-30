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

	const char *GetName( void ) OVERRIDE { return PANEL_MAP; }
	void SetData(KeyValues *data) OVERRIDE {};
	void Reset() OVERRIDE;
	void Update() OVERRIDE;
	bool NeedsUpdate( void ) OVERRIDE;
	bool HasInputElements( void ) OVERRIDE  { return false; }
	void ShowPanel( bool bShow ) OVERRIDE;

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) OVERRIDE { return BaseClass::GetVPanel(); }
  	bool IsVisible() OVERRIDE { return BaseClass::IsVisible(); }
  	void SetParent( vgui::VPANEL parent ) OVERRIDE { BaseClass::SetParent( parent ); }
	
protected:

	void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
	void PostApplySchemeSettings( vgui::IScheme *pScheme );

	void MoveToCenterOfScreen();

private:
	float m_fNextUpdateTime;
	IViewPort	*m_pViewPort;
	CRnLMapPanel* m_pMapPanel;
};


#endif // RNL_MAPDIALOG_H
