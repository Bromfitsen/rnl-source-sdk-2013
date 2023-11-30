//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines the Resistance-And-Liberation team selection menu that precedes the class
// selection screen
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_TEAMMENU_H
#define RNL_TEAMMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_imagebutton.h>

#include <baseviewport.h>

#include <vgui/KeyCode.h>
#include <UtlVector.h>

namespace vgui
{
	class RichText;
	class HTML;
}


//-----------------------------------------------------------------------------
// Purpose: Displays the team menu
//-----------------------------------------------------------------------------
class CRnLTeamMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CRnLTeamMenu, vgui::Frame );
	int				m_nTextureID;

public:
	CRnLTeamMenu(IViewPort *pViewPort);
	virtual ~CRnLTeamMenu();

	const char *GetName( void ) OVERRIDE { return PANEL_TEAM; }
	void SetData(KeyValues *data) OVERRIDE {};
	void Reset() OVERRIDE {};
	void Update() OVERRIDE {};
	bool NeedsUpdate( void ) OVERRIDE { return false; }
	bool HasInputElements( void ) OVERRIDE { return true; }
	void ShowPanel( bool bShow ) OVERRIDE;

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) OVERRIDE { return BaseClass::GetVPanel(); }
  	bool IsVisible() OVERRIDE { return BaseClass::IsVisible(); }
	void SetParent( vgui::VPANEL parent ) OVERRIDE { BaseClass::SetParent( parent ); }
		
protected:

	// VGUI2 overrides
	void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
	void OnKeyCodePressed(vgui::KeyCode code) OVERRIDE;

	// helper functions
	virtual void LoadMapPage();
	void PaintBackground( void ) OVERRIDE;
	void PerformLayout() OVERRIDE;

	// command callbacks
	// MESSAGE_FUNC_INT( OnTeamButton, "TeamButton", team );
	void OnCommand( const char *command ) OVERRIDE;
	IViewPort	*m_pViewPort;
	//vgui::RichText *m_pMapInfo;
	vgui::ImagePanel *m_pBackgroundImage;
	vgui::HTML *m_pRnLSite;
	vgui::ImageButton *m_pAxisButton;
	vgui::ImageButton *m_pAlliesButton;
	vgui::ImageButton *m_pSpectatorButton;
	vgui::ImageButton *m_pAutoButton;
	vgui::ImageButton *m_pCancelButton;
	int m_iJumpKey;
	int m_iScoreBoardKey;
};


#endif // RNL_TEAMMENU_H
