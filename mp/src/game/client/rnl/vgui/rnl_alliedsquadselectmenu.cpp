//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>
#include <cdll_util.h>
#include <globalvars_base.h>
#include <igameresources.h>

#include "rnl_alliedsquadselectmenu.h"
#include "spectatorgui.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vstdlib/IKeyValuesSystem.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ListPanel.h>

#include "vgui_imagebutton.h"

#include <game/client/iviewport.h>
#include <igameresources.h>
#include "rnl_gamerules.h"
#include "c_playerresource.h"

#include "voice_status.h"

#include "c_rnl_player.h"
#include "rnl_squad.h"
#include "c_rnl_game_team.h"

#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

const char *ConvertToClassName( int i );

extern int nSelectionSongGuid;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLAlliedSquadSelectMenu::CRnLAlliedSquadSelectMenu(IViewPort *pViewPort) : Frame( NULL, PANEL_SQUAD_ALLIES )
{
	m_pViewPort = pViewPort;

	// initialize dialog
	SetProportional(true);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );

	// set the scheme before any child control is created
	SetScheme("ClientScheme");

	m_pCancelButton = new ImageButton(this, "CancelButton", "cancel_btn", "cancel_btn_rollover", NULL, "cancel");
	
	LoadControlSettings("resource/ui/rnl_alliedsquadselect.res");

	// update scoreboard instantly if on of these events occure
	gameeventmanager->AddListener(this, "hltv_status", false );
	gameeventmanager->AddListener(this, "server_spawn", false );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CRnLAlliedSquadSelectMenu::~CRnLAlliedSquadSelectMenu()
{
	gameeventmanager->RemoveListener(this);
}

void CRnLAlliedSquadSelectMenu::Reset()
{
	// show all of the fireteam lists
	for( int i = 0; i < m_pSquadList.Count(); i++ )
	{
		m_pSquadList[i]->DeletePanel();
		m_pSquadList[i] = NULL;
	}

	m_pSquadList.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: sets up screen
//-----------------------------------------------------------------------------
void CRnLAlliedSquadSelectMenu::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// show all of the fireteam lists
	for( int i = 0; i < m_pSquadList.Count(); i++ )
	{
		m_pSquadList[i]->DeletePanel();
		m_pSquadList[i] = NULL;
	}

	m_pSquadList.Purge();

	// light up scoreboard a bit
	//SetBgColor( Color( 0,0,0,128) );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLAlliedSquadSelectMenu::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;


	if ( bShow )
	{
		SetMouseInputEnabled( true );

		Reset();
		Update();

		/*
		CSpectatorGUI *specPanel = (CSpectatorGUI*)gViewPortInterface->FindPanelByName( PANEL_SPECGUI );
		if (specPanel) 
		{
			bool bSpecGUIVisible = specPanel->IsVisible();
			gViewPortInterface->ShowPanel( PANEL_SPECGUI, false );
			if ( bSpecGUIVisible )
				specPanel->SetWantsToBeShown( true );
		}
		*/
		Activate();

		// cjd @remove - loadout music players from the room now
		/*
		if( nSelectionSongGuid == 0 )
		{
			enginesound->EmitAmbientSound( "music/Allied_Select.wav", 1.0f, PITCH_NORM, 0, 0.0f );
			nSelectionSongGuid = enginesound->GetGuidForLastSoundEmitted();
		}
		*/
	}
	else
	{
		//Andrew: Cheeky Monkey!
		//
		//// cjd @removed - loadout music players from the room now
		///*
		if ( nSelectionSongGuid != 0 )
		{
			enginesound->StopSoundByGuid( nSelectionSongGuid );
			nSelectionSongGuid = 0;
		}

		SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );

		Reset();

		CSpectatorGUI *specPanel = (CSpectatorGUI*)gViewPortInterface->FindPanelByName( PANEL_SPECGUI );
		CRnLPlayer* pRnLPlayer = CRnLPlayer::GetLocalRnLPlayer();

		if( pRnLPlayer && !pRnLPlayer->IsAlive() && specPanel )
			gViewPortInterface->ShowPanel( PANEL_SPECGUI, true );
	}
}

void CRnLAlliedSquadSelectMenu::FireGameEvent( IGameEvent *event )
{
	if( IsVisible() )
		Update();
}

bool CRnLAlliedSquadSelectMenu::NeedsUpdate( void )
{
	return (m_fNextUpdateTime < gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose: Recalculate squad list sizes and info
//-----------------------------------------------------------------------------
void CRnLAlliedSquadSelectMenu::Update( void )
{
	// our key values for the items
	KeyValues *itemData = new KeyValues("selectiondata");
	C_RnLGameTeam* pAlliedTeam = (C_RnLGameTeam*)GetGlobalRnLTeam( TEAM_ALLIES );
	if( !pAlliedTeam )
		return;

	if( pAlliedTeam->GetNumberOfSquads() < 1 )
		return;

	CRnLSquad* pSquad = NULL;
	int iClassData = 0;
	int itemCount = 0;
	CRnLPlayer* pPlayer = NULL;
	KeyValues *classData = NULL;
	bool bStillOpen = true;
	int x,y,w,h;

	for( int iSquad = 0; iSquad < pAlliedTeam->GetNumberOfSquads(); iSquad++ )
	{
		itemCount = 0;
		pSquad = pAlliedTeam->GetSquad(iSquad);
		iClassData = 0;
		pPlayer = NULL;

		if( pSquad )
		{
			if( m_pSquadList.Count() <= iSquad )
			{
				m_pSquadList.AddToTail();
				m_pSquadList[iSquad] = new ListPanel( this, pSquad->GetSquadTitle());
				m_pSquadList[iSquad]->SetVisible( true );

				m_pSquadList[iSquad]->AddActionSignalTarget( this );
				m_pSquadList[iSquad]->AddColumnHeader(0, "name", pSquad->GetSquadTitle(), XRES(NAME_WIDTH), ListPanel::COLUMN_FIXEDSIZE  );
				m_pSquadList[iSquad]->AddColumnHeader(1, "class", "#PlayerClass", XRES(CLASS_WIDTH), ListPanel::COLUMN_FIXEDSIZE );
				m_pSquadList[iSquad]->SetColumnSortable( 0, false );
				m_pSquadList[iSquad]->SetColumnSortable( 1, false );
				m_pSquadList[iSquad]->SetPaintBorderEnabled( false );
				m_pSquadList[iSquad]->SetMultiselectEnabled( false );
			}

			// fill list so there is 4 entries
			for( int i = 0; i < pSquad->GetTotalAvailableKits(); i++ )
			{
				iClassData = pSquad->GetKitDescription( i );

				if( iClassData > -1 )
				{
					for( int j = 0; j < pSquad->GetKitMaxCount( i ); j++ )
					{
						if( itemCount < m_pSquadList[iSquad]->GetItemCount() )
						{
							classData = m_pSquadList[iSquad]->GetItem( itemCount );
						}
						else
						{
							classData = itemData;
						}

						pPlayer = pSquad->GetMember( i, j );
						if( pPlayer )
						{
							classData->SetString( "name", pPlayer->GetPlayerName() );

						}
						else
						{
							classData->SetString( "name", "Available" );
						}
						
						classData->SetString( "class", pAlliedTeam->GetKitDescription( iClassData ).title );
						
						if( classData == itemData )
							m_pSquadList[iSquad]->AddItem( classData, 0, false, false );

						if( pPlayer )
						{
							m_pSquadList[iSquad]->SetItemDisabled( itemCount, true );
						}
						else
						{
							m_pSquadList[iSquad]->SetItemDisabled( itemCount, false );
						}
						itemCount++;
					}
				}
			}

			if( !pSquad->AreRequirementsMet() )
				bStillOpen = false;

			float fixRatio = 1;
			int screenWide, screenTall;
			vgui::surface()->GetScreenSize( screenWide, screenTall );
			float aspectRatio = (float)screenWide/(float)screenTall;
			if(aspectRatio >= 1.6f)
				fixRatio = 1.33;

			SquadGUIGetIdealProportions( iSquad, pAlliedTeam->GetNumberOfSquads(), x, y, w, h );
			m_pSquadList[iSquad]->SetPos( (XRES(320)-(XRES(x)/fixRatio)), YRES(y) );
			m_pSquadList[iSquad]->SetSize( (XRES(w)/fixRatio), YRES(h) );
		}
	}
	itemData->deleteThis();

	SetSize( XRES(640), YRES(480) );

	// move master panel to center of screen
	MoveToCenterOfScreen();

	// update four times a second
	m_fNextUpdateTime = gpGlobals->curtime + 0.25f; 
}


void CRnLAlliedSquadSelectMenu::OnSquadListItemSelected( KeyValues *data )
{
	char squadJoin[32];
	C_RnLGameTeam* pAlliedTeam = (C_RnLGameTeam*)GetGlobalRnLTeam( TEAM_ALLIES );
	if( !pAlliedTeam )
		return;
	if( pAlliedTeam->GetNumberOfSquads() < 1 )
		return;
	CRnLSquad* pSquad = NULL;
	bool bSearch = true;
	int itemCount = 0;

	for( int iSquad = 0; iSquad < pAlliedTeam->GetNumberOfSquads(); iSquad++ )
	{
		itemCount = 0;
		pSquad = pAlliedTeam->GetSquad(iSquad);
		if( bSearch )
		{
			for( int i = 0; i < pSquad->GetTotalAvailableKits() && bSearch; i++ )
			{
				for( int j = 0; j < pSquad->GetKitMaxCount( i ) && bSearch; j++ )
				{
					if( m_pSquadList[iSquad]->IsItemSelected( itemCount ) )
					{
						if( m_pSquadList[iSquad]->GetItem( itemCount )->GetInt( "disabled" ) == 0 )
						{
							Q_snprintf( squadJoin, 32, "joinsquad %d %d", iSquad, i );
							engine->ClientCmd( squadJoin );
							bSearch = false;
						}
					}
					itemCount++;
				}
			}
		}
		m_pSquadList[iSquad]->ClearSelectedItems();
	}

	if( !bSearch )
		ShowPanel( false );
}

void CRnLAlliedSquadSelectMenu::OnCommand( const char *command )
{
	// player trying to join a squad
	if( Q_strstr( command, "joinsquad" ) )
	{
		//for(int i = 0; i < MAX_SQUADS; i++ )
		//{
		//	for(int j = 0; j < MAX_FIRETEAMS; j++ )
		//	{
		//		m_pSquadButtons[i][j]->SetEnabled( false );
		//	}
		//}

		//m_pCancelButton->SetEnabled( false );

		engine->ClientCmd( command );
		ShowPanel( false );
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

void CRnLAlliedSquadSelectMenu::SetData( KeyValues *data )
{
	if( FStrEq( data->GetName(), "confirmselect" ) )
	{
		// true
		if( data->GetInt("success") == 1 )
		{
			m_pCancelButton->SetEnabled( true );

			Close();
			gViewPortInterface->ShowBackGround( false );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw RnL Menu Background
//-----------------------------------------------------------------------------
void CRnLAlliedSquadSelectMenu::PaintBackground( void )
{
}