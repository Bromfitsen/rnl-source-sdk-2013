//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef RNL_RADIALDIALOG_H
#define RNL_RADIALDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/frame.h>
#include <game/client/iviewport.h>
#include "rnl_mappanel.h"
#include "rnl_shareddefs.h"

//-----------------------------------------------------------------------------
// Purpose: RnL Radial Menu
//-----------------------------------------------------------------------------
class CRnLRadialDialog : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CRnLRadialDialog, vgui::Frame );

public:
	CRnLRadialDialog( IViewPort *pViewPort );
	~CRnLRadialDialog();

	const char *GetName( void ) OVERRIDE { return PANEL_RADIAL; }
	void SetData(KeyValues *data) OVERRIDE {}
	void Reset() OVERRIDE;
	void Update() OVERRIDE;
	bool NeedsUpdate( void ) OVERRIDE;
	bool HasInputElements( void ) OVERRIDE { return false; }
	void ShowPanel( bool bShow ) OVERRIDE;

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) OVERRIDE { return BaseClass::GetVPanel(); }
  	bool IsVisible() OVERRIDE  { return BaseClass::IsVisible(); }
	void SetParent( vgui::VPANEL parent ) OVERRIDE { BaseClass::SetParent( parent ); }
	
protected:

	 void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
	 void PostApplySchemeSettings( vgui::IScheme *pScheme );

private:
	float m_fNextUpdateTime;
	IViewPort	*m_pViewPort;
};


#endif //RNL_RADIALDIALOG_H