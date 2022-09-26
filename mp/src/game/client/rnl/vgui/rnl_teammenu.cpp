//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <cdll_client_int.h>

#include "rnl_teammenu.h"
#include "spectatorgui.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>

#include "vgui_imagebutton.h"

#include "IGameUIFuncs.h" // for key bindings
#include <igameresources.h>
extern IGameUIFuncs *gameuifuncs; // for key binding details

#include "baseviewport.h"

#include <stdlib.h> // MAX_PATH define
#include <stdio.h>

#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

void UpdateCursorState();
// void DuckMessage(const char *str);

int nSelectionSongGuid = 0;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLTeamMenu::CRnLTeamMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_TEAM )
{
	m_pViewPort = pViewPort;
	m_iJumpKey = KEY_NONE; // this is looked up in Activate()
	m_iScoreBoardKey = KEY_NONE; // this is looked up in Activate()

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );
	SetProportional( true );

	// info window about this map
	m_pRnLSite = new HTML(this, "MapInfo");
	m_pSpectatorButton = new ImageButton(this, "SpectatorButton", "spectator_btn", "spectator_btn_rollover", NULL, "jointeam 1");
	m_pAlliesButton = new ImageButton(this, "AlliesButton", "allies_btn", "allies_btn_rollover", NULL, "jointeam 2");
	m_pAxisButton = new ImageButton(this, "AxisButton", "axis_btn", "axis_btn_rollover", NULL, "jointeam 3");
	m_pAutoButton = new ImageButton(this, "AutoButton", "autoassign_btn", "autoassign_btn_rollover", NULL, "jointeam 0");
	m_pCancelButton = new ImageButton(this, "CancelButton", "cancel_btn", "cancel_btn_rollover", NULL, "cancel");

	LoadControlSettings("Resource/UI/rnl_teammenu.res");
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CRnLTeamMenu::~CRnLTeamMenu()
{
}

//-----------------------------------------------------------------------------
// Purpose: sets the text color of the map description field
//-----------------------------------------------------------------------------
void CRnLTeamMenu::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	LoadMapPage();
}

//-----------------------------------------------------------------------------
// Purpose: makes the GUI fill the screen
//-----------------------------------------------------------------------------
void CRnLTeamMenu::PerformLayout()
{
	int w,h;
	GetHudSize(w, h);

	// fill the screen
	SetBounds(0,0,w,h);
}

//-----------------------------------------------------------------------------
// Purpose: shows the team menu
//-----------------------------------------------------------------------------
void CRnLTeamMenu::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		CSpectatorGUI *specPanel = (CSpectatorGUI*)gViewPortInterface->FindPanelByName( PANEL_SPECGUI );
		if (specPanel) 
		{
			bool bSpecGUIVisible = specPanel->IsVisible();
			gViewPortInterface->ShowPanel( PANEL_SPECGUI, false );
			if ( bSpecGUIVisible )
				specPanel->SetWantsToBeShown( true );
		}

		Activate();
		LoadMapPage();
		SetVisible( true );
		SetMouseInputEnabled( true );
	
		// get key bindings if shown
		if( m_iJumpKey == BUTTON_CODE_INVALID ) // you need to lookup the jump key AFTER the engine has loaded
		{
			m_iJumpKey = gameuifuncs->GetButtonCodeForBind( "jump" );
		}

		if ( m_iScoreBoardKey == BUTTON_CODE_INVALID ) 
		{
			m_iScoreBoardKey = gameuifuncs->GetButtonCodeForBind( "showscores" );
		}
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );

		CSpectatorGUI *specPanel = (CSpectatorGUI*)gViewPortInterface->FindPanelByName( PANEL_SPECGUI );
		if (specPanel && specPanel->GetWantsToBeShown())
			gViewPortInterface->ShowPanel( PANEL_SPECGUI, true );
	}
}

//-----------------------------------------------------------------------------
// Purpose: chooses and loads the text page to display that describes mapName map
//-----------------------------------------------------------------------------
void CRnLTeamMenu::LoadMapPage()
{
	char mapname[ 256 ];
	char mapRES[ MAX_PATH ];
	Q_FileBase( engine->GetLevelName(), mapname, sizeof(mapname) );
	Q_snprintf( mapRES, sizeof( mapRES ), "media/html/%s.html", mapname);

	DevMsg(1,"%s\n",mapRES);
	// if no map specific description exists, load default text
	if( !g_pFullFileSystem->FileExists( mapRES ) )
		Q_snprintf ( mapRES, sizeof( mapRES ), "media/html/rnl_default.html");

	// convert to local URL
	char localURL[ _MAX_PATH + 7 ];
	Q_strncpy( localURL, "file://", sizeof( localURL ) );
	char pPathData[ _MAX_PATH ];
	g_pFullFileSystem->GetLocalPath( mapRES, pPathData, sizeof(pPathData) );
	Q_strncat( localURL, pPathData, sizeof( localURL ), COPY_ALL_CHARACTERS );

	m_pRnLSite->OpenURL(localURL, true);
}

//-----------------------------------------------------------------------------
// Purpose: Act on access keys
//-----------------------------------------------------------------------------
void CRnLTeamMenu::OnKeyCodePressed(KeyCode code)
{
	if( m_iJumpKey >= 0 && m_iJumpKey == code )
	{
		engine->ClientCmd("jointeam 0");
	}
	else if ( m_iScoreBoardKey >= 0 && m_iScoreBoardKey == code )
	{
		gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, true );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Act on commands
//-----------------------------------------------------------------------------
void CRnLTeamMenu::OnCommand( const char *command )
{
	if( !Q_stricmp( command, "jointeam 2" ) )
	{
		engine->ClientCmd( command );
		/*
		if( nSelectionSongGuid != 0 )
		{
			enginesound->StopSoundByGuid( nSelectionSongGuid );
			nSelectionSongGuid = 0;
		}

		if( C_BasePlayer::GetLocalPlayer() && C_BasePlayer::GetLocalPlayer()->GetTeamNumber() != TEAM_ALLIES )
		{
			enginesound->EmitAmbientSound( "music/Allied_Select.wav", 1.0f, PITCH_NORM, 0, 0.0f );
			nSelectionSongGuid = enginesound->GetGuidForLastSoundEmitted();
		}
		*/
	}
	else if( !Q_stricmp( command, "jointeam 3" ) )
	{
		engine->ClientCmd( command );
		/*
		if( nSelectionSongGuid != 0 )
		{
			enginesound->StopSoundByGuid( nSelectionSongGuid );
			nSelectionSongGuid = 0;
		}

		if( C_BasePlayer::GetLocalPlayer() && C_BasePlayer::GetLocalPlayer()->GetTeamNumber() != TEAM_AXIS )
		{
			enginesound->EmitAmbientSound( "music/Axis_Select.wav", 1.0f, PITCH_NORM, 0, 0.0f );
			nSelectionSongGuid = enginesound->GetGuidForLastSoundEmitted();
		}
		*/
	}
	else if( FStrEq( command, "cancel" ) )
	{
		/*
		if ( nSelectionSongGuid != 0 )
		{
			enginesound->StopSoundByGuid( nSelectionSongGuid );
			nSelectionSongGuid = 0;
		}
		*/
	}
	else
	{
		engine->ClientCmd( command );
	}

	ShowPanel( false );
}

//-----------------------------------------------------------------------------
// Purpose: Draw RnL Menu Background
//-----------------------------------------------------------------------------
void CRnLTeamMenu::PaintBackground( void )
{
}