//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client DLL VGUI2 Viewport
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#pragma warning( disable : 4800  )  // disable forcing int to bool performance warning

// VGUI panel includes
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui/Cursor.h>
#include <vgui/IScheme.h>
#include <vgui/IVGUI.h>
#include <vgui/ILocalize.h>
#include <vgui/VGUI.h>

// client dll/engine defines
#include "hud.h"
#include <voice_status.h>

// viewport definitions
#include <baseviewport.h>
#include "rnl_viewport.h"

#include "vguicenterprint.h"
#include "text_message.h"

#include "rnl_scoreboard.h"
#include "rnl_teammenu.h"
#include "rnl_alliedsquadselectmenu.h"
#include "rnl_axissquadselectmenu.h"
//#include "rnl_loadoutmenu.h"
//#include "rnl_loadoutroom.h" // cjd @add
#include "rnl_mapdialog.h"
#include "rnl_radialdialog.h"


void CRnLViewport::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	gHUD.InitColors( pScheme );

	SetPaintBackgroundEnabled( false );
}


IViewPortPanel* CRnLViewport::CreatePanelByName(const char *szPanelName)
{
	IViewPortPanel* newpanel = NULL;

	if ( Q_strcmp( PANEL_SCOREBOARD, szPanelName) == 0 )
	{
		newpanel = new CRnLScoreboard( this );
	}
	else if ( Q_strcmp( PANEL_TEAM, szPanelName) == 0 )
	{
		newpanel = new CRnLTeamMenu( this );
	}
	else  if ( Q_strcmp( PANEL_SQUAD_ALLIES, szPanelName) == 0 )
	{
		newpanel = new CRnLAlliedSquadSelectMenu( this );
	}
	else  if ( Q_strcmp( PANEL_SQUAD_AXIS, szPanelName) == 0 )
	{
		newpanel = new CRnLAxisSquadSelectMenu( this );
	}
	/*
	else if ( Q_strcmp( PANEL_LOADOUT, szPanelName) == 0 )
	{
		newpanel = new CRnLLoadoutMenu( this );
	}
	// cjd @add
	else if( Q_strcmp( PANEL_LOADOUT_ROOM, szPanelName ) == 0 )
	{
		newpanel = new CRnLLoadoutRoom( this );
	}
	// cjd end
	*/
	else if( Q_strcmp( PANEL_MAP, szPanelName ) == 0 )
	{
		newpanel = new CRnLMapDialog( this );
	}
	else if( Q_strcmp( PANEL_RADIAL, szPanelName ) == 0 )
	{
		newpanel = new CRnLRadialDialog( this );
	}
	else
	{
		// create a generic base panel, don't add twice
		newpanel = BaseClass::CreatePanelByName( szPanelName );
	}

	return newpanel; 
}

void CRnLViewport::CreateDefaultPanels( void )
{
	AddNewPanel( CreatePanelByName( PANEL_TEAM ), "PANEL_TEAM" );
	AddNewPanel( CreatePanelByName( PANEL_SQUAD_ALLIES ), "PANEL_SQUAD_ALLIES" );
	AddNewPanel( CreatePanelByName( PANEL_SQUAD_AXIS ), "PANEL_SQUAD_AXIS" );
	AddNewPanel( CreatePanelByName( PANEL_MAP ), "PANEL_MAP" );
	AddNewPanel( CreatePanelByName( PANEL_RADIAL), "PANEL_RADIAL" );
	//AddNewPanel( CreatePanelByName( PANEL_LOADOUT ), "PANEL_LOADOUT" );
	//AddNewPanel( CreatePanelByName( PANEL_LOADOUT_ROOM ), "PANEL_LOADOUT_ROOM" ); // cjd @add

	BaseClass::CreateDefaultPanels();
}

int CRnLViewport::GetDeathMessageStartHeight( void )
{
	int x = YRES(2);

	IViewPortPanel *spectator = gViewPortInterface->FindPanelByName( PANEL_SPECGUI );

	//TODO: Link to actual height of spectator bar
	if ( spectator && spectator->IsVisible() )
	{
		x += YRES(52);
	}

	return x;
}

