//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <stdio.h>

#include <globalvars_base.h>
#include <cdll_client_int.h>
#include <cdll_util.h>
#include <game/client/iviewport.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>

#include "rnl_squadmenu.h"
#include "rnl_gamerules.h"
#include "spectatorgui.h"

#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern int nSelectionSongGuid;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLSquadMenu::CRnLSquadMenu(IViewPort *pViewPort) : Frame( NULL, PANEL_SQUADS )
{
	m_pViewPort = pViewPort;

	// initialize dialog
	SetProportional(true);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );

	// set the scheme before any child control is created
	SetScheme("ClientScheme");

	// set up fireteam panels and labels
	m_pSquadPanels[0] = new Panel( this, "squadpanel1" );
	m_pSquadPanels[1] = new Panel( this, "squadpanel2" );

	// set up slot labels
	for (int u = 0; u < MAX_SQUADS; u++)
	{
		for (int v = 0; v < SLOTS_PER_SQUAD; v++)
		{
			// create elements
			m_pSlotClasses[v+u*SLOTS_PER_SQUAD] = new Label( m_pSquadPanels[u], "slotclass", "Class" );
			m_pSlotDescriptions[v+u*SLOTS_PER_SQUAD] = new Label( m_pSquadPanels[u], "slotdescription", "Description" );
			m_pSlotPlayerNames[v+u*SLOTS_PER_SQUAD] = new Label( m_pSquadPanels[u], "slotplayername", "not occupied" );
			m_pSlotButtons[v+u*SLOTS_PER_SQUAD] = new Button( m_pSquadPanels[u], "slotbutton", "-", this, "cancel" );
		}
	}

	LoadControlSettings("resource/ui/rnl_squadmenu.res");
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CRnLSquadMenu::~CRnLSquadMenu()
{
}

bool CRnLSquadMenu::NeedsUpdate( void )
{
	return (m_fNextUpdateTime < gpGlobals->curtime);
}

void CRnLSquadMenu::Reset()
{
	m_fNextUpdateTime = 0;
}

//-----------------------------------------------------------------------------
// Purpose: sets up screen
//-----------------------------------------------------------------------------
void CRnLSquadMenu::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	vgui::HFont playernameFont = pScheme->GetFont( "DefaultSmall" );
	vgui::HFont descriptionFont = pScheme->GetFont( "Default" );
	vgui::HFont classFont = pScheme->GetFont( "ClassLabel" );
	Color cPanelBg = pScheme->GetColor( "SquadPanelBackground", Color( 255, 255, 255, 255 ) );
	Color cDescription = pScheme->GetColor( "SlotClassDescription", Color( 128, 128, 128, 255 ) );

	// colors we'll need later on
	cPlayerName = pScheme->GetColor( "SlotPlayerName", Color( 128, 0, 0, 255 ) );
	cFree = pScheme->GetColor( "SlotFree", Color( 0, 128, 0, 255 ) );
	cDeactivated = pScheme->GetColor( "SlotDeactivated", Color( 0, 0, 128, 255 ) );
	cClass = pScheme->GetColor( "SlotClassName", Color( 0, 0, 0, 255 ) );
	cClassMouseOver = pScheme->GetColor( "SlotClassNameMouseOver", Color( 128, 0, 0, 255 ) );

	for (int u = 0; u < MAX_SQUADS; u++)
	{
		m_pSquadPanels[u]->SetBgColor( cPanelBg );
		m_pSquadPanels[u]->SetPaintBackgroundType(2);

		for (int v = 0; v < SLOTS_PER_SQUAD; v++)
		{
			int sWide = m_pSquadPanels[u]->GetWide();
			int sTall = m_pSquadPanels[u]->GetTall()/SLOTS_PER_SQUAD;
			int sOffsetX = sWide / 20;
			int sOffsetY = sTall / 2;
			
			m_pSlotButtons[v+u*SLOTS_PER_SQUAD]->SetPos( 0, v * sTall );
			m_pSlotClasses[v+u*SLOTS_PER_SQUAD]->SetPos( sOffsetX, v * sTall );
			m_pSlotDescriptions[v+u*SLOTS_PER_SQUAD]->SetPos( sWide / 2, v * sTall );
			m_pSlotPlayerNames[v+u*SLOTS_PER_SQUAD]->SetPos( sOffsetX, v * sTall + sOffsetY );
			
			m_pSlotButtons[v+u*SLOTS_PER_SQUAD]->SetSize( sWide, sTall );
			m_pSlotClasses[v+u*SLOTS_PER_SQUAD]->SetSize( sWide - 2 * sOffsetX, sOffsetY );
			m_pSlotDescriptions[v+u*SLOTS_PER_SQUAD]->SetSize( sWide / 2 - sOffsetX, sOffsetY );
			m_pSlotPlayerNames[v+u*SLOTS_PER_SQUAD]->SetSize( sWide - 2 * sOffsetX, sTall - sOffsetY );

			m_pSlotClasses[v+u*SLOTS_PER_SQUAD]->SetFont(classFont);
			m_pSlotDescriptions[v+u*SLOTS_PER_SQUAD]->SetFont(descriptionFont);
			m_pSlotPlayerNames[v+u*SLOTS_PER_SQUAD]->SetFont(playernameFont);

			m_pSlotClasses[v+u*SLOTS_PER_SQUAD]->SetFgColor( cClass );
			m_pSlotDescriptions[v+u*SLOTS_PER_SQUAD]->SetFgColor( cDescription );
			m_pSlotPlayerNames[v+u*SLOTS_PER_SQUAD]->SetFgColor( cFree );

			m_pSlotClasses[v+u*SLOTS_PER_SQUAD]->SetContentAlignment( vgui::Label::a_southwest );
			m_pSlotDescriptions[v+u*SLOTS_PER_SQUAD]->SetContentAlignment( vgui::Label::a_southeast );
			m_pSlotPlayerNames[v+u*SLOTS_PER_SQUAD]->SetContentAlignment( vgui::Label::a_west );

			m_pSlotButtons[v+u*SLOTS_PER_SQUAD]->SetButtonBorderEnabled(false);
			m_pSlotButtons[v+u*SLOTS_PER_SQUAD]->SetAlpha(0);
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Show the damn thing
//-----------------------------------------------------------------------------
void CRnLSquadMenu::ShowPanel(bool bShow)
{
	if ( IsVisible() == bShow )
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
		SetMouseInputEnabled( true );
		SetKeyBoardInputEnabled( true );

		if( nSelectionSongGuid == 0 )
		{
			if (iTeam == TEAM_ALLIES)
				enginesound->EmitAmbientSound( "music/Allied_Select.wav", 2.0f, PITCH_NORM, 0, 0.0f );
			else
				enginesound->EmitAmbientSound( "music/Axis_Select.wav", 2.0f, PITCH_NORM, 0, 0.0f );
			nSelectionSongGuid = enginesound->GetGuidForLastSoundEmitted();
		}
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
		CSpectatorGUI *specPanel = (CSpectatorGUI*)gViewPortInterface->FindPanelByName( PANEL_SPECGUI );
		if (specPanel) 
				gViewPortInterface->ShowPanel( PANEL_SPECGUI, specPanel->GetWantsToBeShown() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Update the displayed slot values
//-----------------------------------------------------------------------------
void CRnLSquadMenu::Update( void )
{
	m_fNextUpdateTime = gpGlobals->curtime + 0.1f; 

	// Fill in label contents
	SetLabelText( "squadlabel1", RnLGameRules()->GetSquadName(0,iTeam) );
	SetLabelText( "squadlabel2", RnLGameRules()->GetSquadName(1,iTeam) );

	// Free all slots
	for (int i = 0; i < MAX_SLOTS; i++)
	{
		int iClass = RnLGameRules()->GetSlotClassID(i,iTeam);
		m_pSlotClasses[i]->SetText( sClassNames[iClass] );
		m_pSlotDescriptions[i]->SetText( RnLGameRules()->GetSlotDescription(i,iTeam) );
		if (iClass)
		{
			m_pSlotButtons[i]->SetEnabled( true );
			m_pSlotPlayerNames[i]->SetText( "#RnL_Slot_Free" );
			m_pSlotPlayerNames[i]->SetFgColor( cFree );
		}
		else // closed slot (by script)
		{
			m_pSlotButtons[i]->SetEnabled( false ); 
			m_pSlotPlayerNames[i]->SetText( "#RnL_Slot_Closed" );
			m_pSlotPlayerNames[i]->SetFgColor( cDeactivated );
		}
	}

	UpdateDependancies();
	UpdatePlayerInfo();
}

//-----------------------------------------------------------------------------
// Purpose: Deactivate slots depending on other slots which aren't filled yet
//-----------------------------------------------------------------------------
void CRnLSquadMenu::UpdateDependancies()
{
	for ( int i=0; i<MAX_SLOTS; i++ )
	{
		if (RnLGameRules()->IsSlotLocked(i,iOldSlot,iTeam))
		{
			m_pSlotButtons[i]->SetEnabled( false );
			m_pSlotPlayerNames[i]->SetText( "#RnL_Slot_Locked" );
			m_pSlotPlayerNames[i]->SetFgColor( cDeactivated );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Display players on the server in the fireteam panels
//-----------------------------------------------------------------------------
void CRnLSquadMenu::UpdatePlayerInfo()
{
	for ( int i=0; i<MAX_SLOTS; i++ )
	{
		const char *szPlayerName = RnLGameRules()->GetSlotPlayerName(i,iTeam);
		if (szPlayerName)
		{
			// make playername displayable
			int bufsize = strlen(szPlayerName) * 2;
			char *szDisplayPlayerName = (char *)_alloca( bufsize );
			UTIL_MakeSafeName( szPlayerName, szDisplayPlayerName, bufsize );
			
			// and display it
			m_pSlotPlayerNames[i]->SetText( szDisplayPlayerName );
			m_pSlotPlayerNames[i]->SetFgColor( cPlayerName );
			m_pSlotButtons[i]->SetEnabled( false );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: handle button commands
//-----------------------------------------------------------------------------
void CRnLSquadMenu::OnCommand( const char *command )
{
	if( Q_strstr( command, "joinsquad" ) )
	{
		m_bSubmitting = true;

		DevMsg(1, "Join command\n");

		// disable buttons
		Button *cancelButton = dynamic_cast<Button *>(FindChildByName("CancelButton"));
		if (cancelButton)
			cancelButton->SetEnabled( false );
		for (int u = 0; u < MAX_SLOTS; u++)
			m_pSlotButtons[u]->SetEnabled( false );

		// send command
		engine->ClientCmd( command );
	}
	else if( FStrEq( command, "cancel" ) )
	{
		ShowPanel( false );
		if ( nSelectionSongGuid != 0 )
		{
			enginesound->StopSoundByGuid( nSelectionSongGuid );
			nSelectionSongGuid = 0;
		}
	}
	else
	{
		engine->ClientCmd( command );
	}
}

//-----------------------------------------------------------------------------
// Purpose: React to the confirmation of the joinsquad command
//-----------------------------------------------------------------------------
void CRnLSquadMenu::SetData( KeyValues *data )
{
	// Check if we successfully joined the squad
	if( FStrEq( data->GetName(), "confirmselect" ) )
	{
		m_bSubmitting = false;

		if( data->GetInt("success") == 1 )
		{
			Button *cancelButton = dynamic_cast<Button *>(FindChildByName("CancelButton"));
			if (cancelButton)
				cancelButton->SetEnabled( true );
			for (int u = 0; u < MAX_SLOTS; u++)
				m_pSlotButtons[u]->SetEnabled( true );
			Close();
			gViewPortInterface->ShowBackGround( false );
			gViewPortInterface->ShowPanel( PANEL_SQUADS, false );
		}
		else
		{
			Button *cancelButton = dynamic_cast<Button *>(FindChildByName("CancelButton"));
			if (cancelButton)
				cancelButton->SetEnabled( true );
			for (int u = 0; u < MAX_SLOTS; u++)
				m_pSlotButtons[u]->SetEnabled( true );
		}
	}
	else if ( FStrEq( data->GetName(), "team" ) ) 
	{
		iTeam = data->GetInt( "teamnumber", RNL_CLASS_NONE );
		iOldSlot = data->GetInt( "slotnumber", -1 );
		SetButtonCommands();
		Update();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Helper function to set label text
//-----------------------------------------------------------------------------
void CRnLSquadMenu::SetLabelText(const char *textEntryName, const char *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->SetText(text);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Assign join commands to buttons
//-----------------------------------------------------------------------------
void CRnLSquadMenu::SetButtonCommands()
{
	m_pSlotButtons[0]->SetCommand("joinsquad 0 0 0");
	m_pSlotButtons[1]->SetCommand("joinsquad 0 0 1");
	m_pSlotButtons[2]->SetCommand("joinsquad 0 0 2");
	m_pSlotButtons[3]->SetCommand("joinsquad 0 0 3");
	m_pSlotButtons[4]->SetCommand("joinsquad 0 1 4");
	m_pSlotButtons[5]->SetCommand("joinsquad 0 1 5");
	m_pSlotButtons[6]->SetCommand("joinsquad 0 1 6");
	m_pSlotButtons[7]->SetCommand("joinsquad 0 1 7");
	m_pSlotButtons[8]->SetCommand("joinsquad 1 0 8");
	m_pSlotButtons[9]->SetCommand("joinsquad 1 0 9");
	m_pSlotButtons[10]->SetCommand("joinsquad 1 0 10");
	m_pSlotButtons[11]->SetCommand("joinsquad 1 0 11");
	m_pSlotButtons[12]->SetCommand("joinsquad 1 1 12");
	m_pSlotButtons[13]->SetCommand("joinsquad 1 1 13");
	m_pSlotButtons[14]->SetCommand("joinsquad 1 1 14");
	m_pSlotButtons[15]->SetCommand("joinsquad 1 1 15");
}

//-----------------------------------------------------------------------------
// Purpose: Draw MouseOver Effects
//-----------------------------------------------------------------------------
void CRnLSquadMenu::Paint()
{
	for (int i=0; i<MAX_SLOTS; i++)
	{
		if (m_pSlotButtons[i]->IsEnabled() && m_pSlotButtons[i]->IsCursorOver())
			m_pSlotClasses[i]->SetFgColor( cClassMouseOver);
		else
			m_pSlotClasses[i]->SetFgColor( cClass );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw RnL Menu Background
//-----------------------------------------------------------------------------
void CRnLSquadMenu::PaintBackground( void )
{
	vgui::HScheme pScheme = vgui::scheme()->GetScheme( "ClientScheme" );
	
	surface()->DrawSetColor( vgui::scheme()->GetIScheme(pScheme)->GetColor( "MenuTitleBackground", Color(146,144,113,190) ) );
	surface()->DrawFilledRect( XRES(20), YRES(20), XRES(620), YRES(50) );
	surface()->DrawSetColor( vgui::scheme()->GetIScheme(pScheme)->GetColor( "MenuSubtitleBackground", Color(80,87,79,190) ) );
	surface()->DrawFilledRect( XRES(20), YRES(50), XRES(620), YRES(70) );	
	surface()->DrawSetColor( vgui::scheme()->GetIScheme(pScheme)->GetColor( "MenuContentBackground", Color(216,214,171,190) ) );
	surface()->DrawFilledRect( XRES(20), YRES(80), XRES(620), YRES(460) );	
	surface()->DrawSetColor( vgui::scheme()->GetIScheme(pScheme)->GetColor( "MenuLine", Color(0,0,0,190) ) );
	surface()->DrawLine( XRES(20), YRES(50), XRES(620), YRES(50) );
}