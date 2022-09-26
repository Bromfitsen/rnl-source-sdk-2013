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

	virtual const char *GetName( void ) { return PANEL_TEAM; }
	virtual void SetData(KeyValues *data) {};
	virtual void Reset() {};
	virtual void Update() {};
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
		
protected:

	// VGUI2 overrides
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui::KeyCode code);

	// helper functions
	virtual void LoadMapPage();
	virtual void PaintBackground( void );
	virtual void PerformLayout();

	// command callbacks
	// MESSAGE_FUNC_INT( OnTeamButton, "TeamButton", team );
	void OnCommand( const char *command );
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
