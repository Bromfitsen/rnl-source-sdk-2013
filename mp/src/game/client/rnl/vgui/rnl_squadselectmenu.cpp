/*********************************
* Resistance and Liberation Loadout UI
*
* Copyright (c) 2008, Blackened Interactive
* Author: Cale "Mazor" Dunlap (cale@blackenedinteractive.com)
* Date: 8/24/2008
*
* Purpose: The squad selection menu (sub-menu from the loadout room)
**********************************/


#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>
#include <cdll_util.h>
#include <globalvars_base.h>
#include <igameresources.h>

#include "rnl_squadselectmenu.h"

#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vstdlib/IKeyValuesSystem.h>
#include <KeyValues.h>
#include <vgui_controls/ListPanel.h>
#include <game/client/iviewport.h>
#include <igameresources.h>
#include "rnl_gamerules.h"
#include "c_playerresource.h"

#include "voice_status.h"

#include "c_rnl_player.h"
#include "c_rnl_base_squad.h"
#include "c_rnl_game_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SQUAD_BUTTON_SPACER_Y		scheme()->GetProportionalScaledValue( 5 )
#define SQUAD_BUTTON_START_X		scheme()->GetProportionalScaledValue( 5 )
#define SQUAD_BUTTON_START_Y		scheme()->GetProportionalScaledValue( 28 )
#define SQUAD_BUTTON_WIDE			scheme()->GetProportionalScaledValue( 250 )
#define SQUAD_BUTTON_TALL			scheme()->GetProportionalScaledValue( 16 )
#define SQUAD_LIST_START_Y			SQUAD_BUTTON_START_Y
#define SQUAD_LIST_START_X			SQUAD_BUTTON_START_X + SQUAD_BUTTON_WIDE + scheme()->GetProportionalScaledValue( 5 )
#define SQUAD_LIST_WIDE				scheme()->GetProportionalScaledValue( 278 )
#define SQUAD_LIST_TALL				scheme()->GetProportionalScaledValue( 205 )

using namespace vgui;

const char *ConvertToClassName( int i );


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLSquadSelectMenu::CRnLSquadSelectMenu(IViewPort *pViewPort) : BaseClass( NULL, PANEL_SQUAD_SELECT )
{
	m_pViewPort = pViewPort;

	// initialize dialog
	SetProportional(true);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(true);
	SetSizeable(false);

	SetPaintBackgroundEnabled( true );

	// hide the system buttons
	SetTitleBarVisible( false );

	// set the scheme before any child control is created
	SetScheme("ClientScheme");
	
	LoadControlSettings("resource/ui/rnl_squadselectmenu.res");

	// update scoreboard instantly if on of these events occure
	gameeventmanager->AddListener(this, "hltv_status", false );
	gameeventmanager->AddListener(this, "server_spawn", false );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CRnLSquadSelectMenu::~CRnLSquadSelectMenu()
{
	gameeventmanager->RemoveListener(this);
}

inline void CRnLSquadSelectMenu::ClearSquadButtonsAndLists()
{
	for( int i = 0; i < m_pSquadList.Count(); i++ )
	{
		if( m_pSquadList[i] )
		{
			m_pSquadList[i]->DeletePanel();
			m_pSquadList[i] = NULL;
		}

		if( m_pSquadButtons[i] )
		{
			m_pSquadButtons[i]->DeletePanel();
			m_pSquadButtons[i] = NULL;
		}
	}

	m_pSquadButtons.Purge();
	m_pSquadList.Purge();
}

void CRnLSquadSelectMenu::Reset()
{
	ClearSquadButtonsAndLists();
}

//-----------------------------------------------------------------------------
// Purpose: sets up screen
//-----------------------------------------------------------------------------
void CRnLSquadSelectMenu::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	ClearSquadButtonsAndLists();

	Color cPanelBg = pScheme->GetColor( "LoadoutPanelBackground", Color( 255, 255, 255, 255 ) );

	// set panel styles
	SetBgColor( cPanelBg );
	SetPaintBackgroundType( 2 );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLSquadSelectMenu::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;


	if ( bShow )
	{
		SetMouseInputEnabled( true );

		Reset();
		Update();

		Activate();
	}
	else
	{
		BaseClass::SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}
}

void CRnLSquadSelectMenu::FireGameEvent( IGameEvent *event )
{
	if( IsVisible() )
		Update();
}

bool CRnLSquadSelectMenu::NeedsUpdate( void )
{
	return (m_fNextUpdateTime < gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose: Recalculate squad list sizes and info
//-----------------------------------------------------------------------------
void CRnLSquadSelectMenu::Update( void )
{
	// our key values for the items
	KeyValues *itemData = new KeyValues("selectiondata");
	C_RnLGameTeam* pSquadSelectTeam = NULL;
	C_RnLPlayer *pPlayer = C_RnLPlayer::GetLocalRnLPlayer();

	if( !pPlayer )
		return;

	switch( pPlayer->GetTeamNumber() )
	{
	case TEAM_ALLIES:
		pSquadSelectTeam = (C_RnLGameTeam*)GetGlobalRnLTeam( TEAM_ALLIES );
		break;
	case TEAM_AXIS:
		pSquadSelectTeam = (C_RnLGameTeam*)GetGlobalRnLTeam( TEAM_AXIS );
		break;
	}

	if( !pSquadSelectTeam )
		return;

	if( pSquadSelectTeam->GetNumberOfSquads() < 1 )
		return;

	// Keep it in the foreground if its open
	MoveToFront();

	C_RnLBaseSquad* pSquad = NULL;
	int iClassData = 0;
	KeyValues *classData = NULL;
	bool bStillOpen = true;
//	int x,y,w,h;
	int buttonY = SQUAD_BUTTON_START_Y;

	for( int iSquad = 0; iSquad < pSquadSelectTeam->GetNumberOfSquads(); iSquad++ )
	{
		
		pSquad = dynamic_cast<C_RnLBaseSquad*>(pSquadSelectTeam->GetSquad(iSquad));
		iClassData = 0;
		pPlayer = NULL;

		if( pSquad )
		{
			if( m_pSquadList.Count() <= iSquad )
			{
				m_pSquadList.AddToTail();
				m_pSquadList[iSquad] = new ListPanel( this, pSquad->m_szSquadTitle );
				m_pSquadList[iSquad]->SetPos( SQUAD_LIST_START_X, SQUAD_LIST_START_Y );
				m_pSquadList[iSquad]->SetSize( SQUAD_LIST_WIDE, SQUAD_LIST_TALL );
				m_pSquadList[iSquad]->SetVisible( iSquad == 0 ? true : false ); // show the first list, hide the rest

				m_pSquadList[iSquad]->AddActionSignalTarget( this );
				m_pSquadList[iSquad]->AddColumnHeader(0, "name", pSquad->m_szSquadTitle, XRES(NAME_WIDTH), ListPanel::COLUMN_FIXEDSIZE  );
				m_pSquadList[iSquad]->AddColumnHeader(1, "class", "#PlayerClass", XRES(CLASS_WIDTH), ListPanel::COLUMN_FIXEDSIZE );
				m_pSquadList[iSquad]->SetColumnSortable( 0, false );
				m_pSquadList[iSquad]->SetColumnSortable( 1, false );
				m_pSquadList[iSquad]->SetMultiselectEnabled( false );

				// Create the buttons
				m_pSquadButtons.AddToTail();
				m_pSquadButtons[iSquad] = new CRnLSquadButton( this, VarArgs( "squadButton %d", iSquad ), 
					pSquad->m_szSquadTitle, iSquad, this, NULL );
				m_pSquadButtons[iSquad]->SetPos( SQUAD_BUTTON_START_X, buttonY );
				m_pSquadButtons[iSquad]->SetSize( SQUAD_BUTTON_WIDE, SQUAD_BUTTON_TALL );
				m_pSquadButtons[iSquad]->SetVisible(true);
				buttonY += SQUAD_BUTTON_TALL + SQUAD_BUTTON_SPACER_Y;
			}

			// fill list so there is 4 entries
			for( int i = 0; i < pSquad->SquadSize(); i++ )
			{
				iClassData = pSquad->GetClassDescriptionForSlot( i );

				if( iClassData > -1 )
				{
					if( i < m_pSquadList[iSquad]->GetItemCount() )
					{
						classData = m_pSquadList[iSquad]->GetItem( i );
					}
					else
					{
						classData = itemData;
					}

					if( pSquad->IsSlotAvailable( i ) )
					{
						if( pSquad->AreRequirementsMet( i ) && bStillOpen )
							classData->SetString( "name", "Available" );
						else
							classData->SetString( "name", "Not Available" );	
					}
					else
					{
						pPlayer = pSquad->GetPlayer( i );
						if( pPlayer )
						{
							classData->SetString( "name", pPlayer->GetPlayerName() );
						}
						else
						{
							classData->SetString( "name", "(OCCUPIED)" );
						}
					}
					
					classData->SetString( "class", pSquadSelectTeam->GetClassDescription( iClassData ).title );
					
					if( classData == itemData )
						m_pSquadList[iSquad]->AddItem( classData, 0, false, false );

					if( !(pSquad->AreRequirementsMet( i )) || !bStillOpen )
					{
						m_pSquadList[iSquad]->SetItemDisabled( i, true );
					}

				}
			}

			if( !pSquad->AreRequirementsMet() )
				bStillOpen = false;
		}
	}
	itemData->deleteThis();

	// update four times a second
	m_fNextUpdateTime = gpGlobals->curtime + 0.25f; 
}


void CRnLSquadSelectMenu::OnSquadListItemSelected( KeyValues *data )
{
	char squadJoin[32];
	bool sent = false;
	for( int i = 0; i < m_pSquadList.Count(); i++ )
	{
		for( int j = 0; j < m_pSquadList[i]->GetItemCount(); j++ )
		{
			if( m_pSquadList[i]->IsItemSelected( j ) )
			{
				if( m_pSquadList[i]->GetItem( j ) )
				{
					if( m_pSquadList[i]->GetItem( j )->GetInt( "disabled" ) == 0 )
					{
						if( !sent )
						{
							sent = true;
							Q_snprintf( squadJoin, 32, "joinsquad %d %d", i, j );
							engine->ClientCmd( squadJoin );

							// Tell the loadout room that this player's selected a squad and what class it was
							KeyValues *msg = new KeyValues( "SquadSelected" );
							msg->SetInt( "Squad", i );
							msg->SetInt( "Position", j );
							PostActionSignal( msg );

							ShowPanel( false );
						}
					}
				}
			}
		}
		m_pSquadList[i]->ClearSelectedItems();
	}
}

void CRnLSquadSelectMenu::OnSquadButtonPressed( KeyValues *data )
{
	int iSquadNumber = data->GetInt("SquadNumber",-1);
	
	if( iSquadNumber != -1 && m_pSquadList[iSquadNumber] != NULL )
	{
		// Hide all the squad lists and pick out the one we want
		for( int i = 0; i < m_pSquadList.Count(); i++ )
		{
			if( iSquadNumber != i )
				m_pSquadList[i]->SetVisible(false);
			else
				m_pSquadList[i]->SetVisible(true);
		}
	}
}
void CRnLSquadSelectMenu::OnCommand( const char *command )
{
	// player trying to join a squad
	if( Q_strstr( command, "joinsquad" ) )
	{
		engine->ClientCmd( command );
		ShowPanel( false );
	}
	else if( FStrEq( command, "cancel" ) )
	{
		ShowPanel( false );	
	}
	else
	{
		engine->ClientCmd( command );
	}
}

void CRnLSquadSelectMenu::SetData( KeyValues *data )
{
	if( FStrEq( data->GetName(), "confirmselect" ) )
	{
		// true
		if( data->GetInt("success") == 1 )
		{
			// m_pCancelButton->SetEnabled( true );

			Close();
			gViewPortInterface->ShowBackGround( false );
		}
	}
}