/*********************************
* Resistance and Liberation Loadout UI
*
* Copyright (c) 2008, Blackened Interactive
* Author: Cale "Mazor" Dunlap (cale@blackenedinteractive.com)
* Date: 7/27/2008
*
* Purpose: A giant pile of model panels and buttons to create the loadout "room"
**********************************/

#include "cbase.h"
#include "rnl_loadoutroom.h"
#include <vgui/IVgui.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/Button.h>
#include <FileSystem.h>
#include "engine/IEngineSound.h"
#include "c_rnl_player.h"
#include "c_rnl_game_team.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define LOADOUT_ROOM_HOVER_COLOR	Vector(1.0f, 1.0f, 0.8f)
#define LOADOUT_ROOM_COLOR			Vector(1.0f, 1.0f, 1.0f)

CRnLLoadoutRoom::CRnLLoadoutRoom( IViewPort *pViewPort ) : Frame( NULL, PANEL_LOADOUT_ROOM )
{
	m_iDisplayTeam = 0;
	m_flNextUpdateTime = 0.0f;
	m_iLoadoutSongGuid = 0;

	m_bPlayerSelectedEquipment = m_bPlayerSelectedAppearance = m_bPlayerSelectedSquad = false;

	m_pViewPort = pViewPort;

	SetTitle("",true);
	SetScheme( "ClientScheme" );
	SetMoveable( false );
	SetSizeable( false );

	SetTitleBarVisible( false );
	SetProportional( true );

	ivgui()->AddTickSignal( GetVPanel(), 100 ); // tick every 100 ms

	m_pBackgroundModelPanel = new CModelPanel( this, "BackgroundModelPanel" );
	m_pEquipmentModelPanel = new CModelPanel( this, "EquipmentModelPanel" );
	m_pTrashModelPanel = new CModelPanel( this, "TrashModelPanel" );
	m_pHelmetModelPanel = new CModelPanel( this, "HelmetModelPanel" );
	m_pSavedLoadoutsModelPanel = new CModelPanel( this, "SavedLoadoutsModelPanel" );
	m_pSquadModelPanel = new CModelPanel( this, "SquadModelPanel" );
	m_pMapInfoModelPanel = new CModelPanel( this, "MapInfoModelPanel" );

	m_pSavedLoadoutsButton = new CRnLLoadoutRoomButton(this, "SavedLoadoutsButton", "", this );
	m_pHelmetButton = new CRnLLoadoutRoomButton( this, "HelmetButton", "", this );
	m_pEquipmentButton = new CRnLLoadoutRoomButton( this, "EquipmentButton", "", this );
	m_pTrashButton = new CRnLLoadoutRoomButton( this, "TrashButton", "", this );
	m_pSquadButton = new CRnLLoadoutRoomButton( this, "SquadButton", "", this );
	m_pMapInfoButton = new CRnLLoadoutRoomButton( this, "MapInfoButton", "", this );

	m_pJoinGameButton = new Button( this, "JoinGameButton", "" );

	// Sub menus
	m_pSubLoadoutMenu = new CRnLLoadoutMenu( pViewPort );
	m_pSubLoadoutMenu->SetParent( GetVPanel() );
	m_pSubLoadoutMenu->AddActionSignalTarget( this );

	m_pSquadSelectMenu = new CRnLSquadSelectMenu( pViewPort );
	m_pSquadSelectMenu->SetParent( GetVPanel() );
	m_pSquadSelectMenu->AddActionSignalTarget( this );

	m_pAppearanceMenu = new CRnLAppearanceMenu( pViewPort );
	m_pAppearanceMenu->SetParent( GetVPanel() );
	m_pAppearanceMenu->AddActionSignalTarget( this );



	LoadControlSettings( "Resource/UI/rnl_loadoutroom_allies.res" );
}

CRnLLoadoutRoom::~CRnLLoadoutRoom()
{
}

void CRnLLoadoutRoom::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetSize( ScreenWidth(), ScreenHeight() );

	int wide = GetWide();
	int tall = GetTall();
	m_pBackgroundModelPanel->SetSize( wide, tall );
	m_pEquipmentModelPanel->SetSize( wide, tall );
	m_pTrashModelPanel->SetSize( wide, tall );
	m_pHelmetModelPanel->SetSize( wide, tall );
	m_pSavedLoadoutsModelPanel->SetSize( wide, tall );
	m_pSquadModelPanel->SetSize( wide, tall );
	m_pMapInfoModelPanel->SetSize( wide, tall );
	// TODO: Good place to swap out schemes for different teams if needed?
}

void CRnLLoadoutRoom::Reset( void )
{
	// Reset requirements
	m_bPlayerSelectedSquad = false;
	m_bPlayerSelectedEquipment = false;
	m_bPlayerSelectedAppearance = false;
	m_flNextUpdateTime = 0.0f;
}

void CRnLLoadoutRoom::Update( void )
{
	if( RequirementsMetToJoin() && !m_pJoinGameButton->IsVisible() )
	{
		m_pJoinGameButton->SetVisible( true );
	}
	else if( !RequirementsMetToJoin() && m_pJoinGameButton->IsVisible() )
	{
		m_pJoinGameButton->SetVisible( false );
	}

	m_flNextUpdateTime = gpGlobals->curtime + 0.25f;
}

bool CRnLLoadoutRoom::NeedsUpdate( void )
{
	if( m_flNextUpdateTime < gpGlobals->curtime )
		return true;
	return false;
}

inline const char *CRnLLoadoutRoom::GetSongFromPlayerTeam( void )
{
	switch( m_iDisplayTeam )
	{
	case TEAM_AXIS:
		return "music/Axis_Select.wav";
	case TEAM_ALLIES:
	default:
		return "music/Allied_Select.wav";
	}
}

inline bool CRnLLoadoutRoom::RequirementsMetToJoin( void ) const
{
	C_RnLPlayer *pPlayer = C_RnLPlayer::GetLocalRnLPlayer();
	if( !pPlayer )
		return false;
	else
	{
		return (m_bPlayerSelectedEquipment &&
			m_bPlayerSelectedAppearance && 
			m_bPlayerSelectedSquad );
	}
}

void CRnLLoadoutRoom::OnSquadSelected( KeyValues *data )
{
	int iPosition = data->GetInt("Position");
	LoadoutData.m_iSquad = data->GetInt("Squad");

	// Figure out what class ID for the squad position, then figure out the model
	C_RnLGameTeam	*pTeam = (C_RnLGameTeam *)GetGlobalRnLTeam( m_iDisplayTeam );
	if( pTeam )
	{
		C_RnLBaseSquad *pSquad = pTeam->GetSquad( LoadoutData.m_iSquad );
		if( pSquad )
		{
			// Load up the default loadout data for the class
			LoadoutData.m_iClass = pSquad->GetClassDescriptionForSlot( iPosition );
			Q_strncpy( LoadoutData.m_szModelName, pTeam->GetClassDescription( LoadoutData.m_iClass ).model.file, CLASS_DESC_MODEL_LEN );

			m_bPlayerSelectedSquad = true;

			// If the player's re-selected their squad/class, tell the equipment panel to display the defaults
			m_bPlayerSelectedEquipment = false;
		}
	}

	m_pHelmetButton->SetEnabled( true );
}

void CRnLLoadoutRoom::OnAppearanceSelected( KeyValues *data )
{
	LoadoutData.m_iHelmetNumber = data->GetInt("HelmetNumber");
	LoadoutData.m_iSkinNumber = data->GetInt("FaceSkin");

	m_pEquipmentButton->SetEnabled( true );
	m_bPlayerSelectedAppearance = true;
}

void CRnLLoadoutRoom::OnEquipmentSelected( KeyValues *data )
{
	LoadoutData.m_iPrimaryWeapon = data->GetInt("PrimaryWeapon");
	LoadoutData.m_iPrimaryAmmoExtra = data->GetInt("PrimaryAmmoExtra");
	LoadoutData.m_iSecondaryWeapon = data->GetInt("SecondaryWeapon");
	LoadoutData.m_iSecondaryAmmoExtra = data->GetInt("SecondaryAmmoExtra");
	LoadoutData.m_iFragGrenades = data->GetInt("FragGrenades");
	LoadoutData.m_iSmokeGrenades = data->GetInt("SmokeGrenades");

	m_bPlayerSelectedEquipment = true;
}

void CRnLLoadoutRoom::OnLoadoutButtonEntered( KeyValues *data )
{
	const char *pszSender = data->GetString("Sender");
	if( FStrEq( pszSender, m_pSavedLoadoutsButton->GetName() ) )
		m_pSavedLoadoutsModelPanel->SetColorModulation( LOADOUT_ROOM_HOVER_COLOR );
	else if( FStrEq( pszSender, m_pHelmetButton->GetName() ) )
		m_pHelmetModelPanel->SetColorModulation( LOADOUT_ROOM_HOVER_COLOR );
	else if( FStrEq( pszSender, m_pSquadButton->GetName() ) )
		m_pSquadModelPanel->SetColorModulation( LOADOUT_ROOM_HOVER_COLOR );
	else if( FStrEq( pszSender, m_pMapInfoButton->GetName() ) )
		m_pMapInfoModelPanel->SetColorModulation( LOADOUT_ROOM_HOVER_COLOR );
	else if( FStrEq( pszSender, m_pEquipmentButton->GetName() ) )
		m_pEquipmentModelPanel->SetColorModulation( LOADOUT_ROOM_HOVER_COLOR );
	else if( FStrEq( pszSender, m_pTrashButton->GetName() ) )
		m_pTrashModelPanel->SetColorModulation( LOADOUT_ROOM_HOVER_COLOR );
}

void CRnLLoadoutRoom::OnLoadoutButtonExited( KeyValues *data )
{
	const char *pszSender = data->GetString("Sender");
	if( FStrEq( pszSender, m_pSavedLoadoutsButton->GetName() ) )
		m_pSavedLoadoutsModelPanel->SetColorModulation( LOADOUT_ROOM_COLOR );
	else if( FStrEq( pszSender, m_pHelmetButton->GetName() ) )
		m_pHelmetModelPanel->SetColorModulation( LOADOUT_ROOM_COLOR );
	else if( FStrEq( pszSender, m_pSquadButton->GetName() ) )
		m_pSquadModelPanel->SetColorModulation( LOADOUT_ROOM_COLOR );
	else if( FStrEq( pszSender, m_pMapInfoButton->GetName() ) )
		m_pMapInfoModelPanel->SetColorModulation( LOADOUT_ROOM_COLOR );
	else if( FStrEq( pszSender, m_pEquipmentButton->GetName() ) )
		m_pEquipmentModelPanel->SetColorModulation( LOADOUT_ROOM_COLOR );
	else if( FStrEq( pszSender, m_pTrashButton->GetName() ) )
		m_pTrashModelPanel->SetColorModulation( LOADOUT_ROOM_COLOR );
}

void CRnLLoadoutRoom::OnTick( void )
{
	// This might be an expensive spot for this... not sure
	if( IsVisible() )
	{
		if( m_flNextSoundThinkTime <= gpGlobals->curtime )
		{
			const char *pszSongName = GetSongFromPlayerTeam();
			enginesound->EmitAmbientSound( pszSongName, 1.0f, PITCH_NORM, 0, 0.0f );
			m_iLoadoutSongGuid = enginesound->GetGuidForLastSoundEmitted();
			m_flNextSoundThinkTime = gpGlobals->curtime + enginesound->GetSoundDuration( pszSongName );
		}
	}

}
void CRnLLoadoutRoom::ShowPanel( bool bShow )
{
	if( BaseClass::IsVisible() == bShow )
		return;

	if( bShow )
	{
		Activate();
		SetVisible( bShow );
		SetMouseInputEnabled( true );
		m_flNextSoundThinkTime = gpGlobals->curtime;
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
		if ( m_iLoadoutSongGuid != 0 )
		{
			enginesound->StopSoundByGuid( m_iLoadoutSongGuid );
			m_iLoadoutSongGuid = 0;
			m_flNextSoundThinkTime = 0.0f; 
		}
	}

	m_pViewPort->ShowBackGround( bShow );
}

void CRnLLoadoutRoom::OnKeyCodePressed( vgui::KeyCode code )
{
	// TODO: Add any key handling here...

	BaseClass::OnKeyCodePressed( code );
}

void CRnLLoadoutRoom::SetData(KeyValues *data)
{
	int iTeam = data->GetInt( "Team" );

	if( iTeam != 0 )
	{
		ChangeDisplayTeam( iTeam );
		// Reset the song...
		m_flNextSoundThinkTime = gpGlobals->curtime;
	}
}

void CRnLLoadoutRoom::ChangeDisplayTeam( int iTeam )
{
	// Set all the model panels to dirty and load new control settings
	SetAllModelPanelsDirty();

	switch( iTeam )
	{
	case TEAM_ALLIES:
		LoadControlSettings( "Resource/UI/rnl_loadoutroom_allies.res" );
		break;
	case TEAM_AXIS:
		LoadControlSettings( "Resource/UI/rnl_loadoutroom_axis.res" );
		break;
	}

	m_iDisplayTeam = iTeam;
}

void CRnLLoadoutRoom::SendLoadoutString( void )
{
	char szLoadOut[64];
	Q_snprintf( szLoadOut, 64, "changeloadout %d %d %d %d %d %d %d %d",
		LoadoutData.m_iPrimaryWeapon, LoadoutData.m_iPrimaryAmmoExtra,
		LoadoutData.m_iSecondaryWeapon, LoadoutData.m_iSecondaryAmmoExtra,
		LoadoutData.m_iFragGrenades, LoadoutData.m_iSmokeGrenades,
		LoadoutData.m_iHelmetNumber, LoadoutData.m_iSkinNumber );
	engine->ClientCmd( szLoadOut );
}

void CRnLLoadoutRoom::OnCommand( const char *command )
{
	if( FStrEq( command, "refresh" ) )
	{
		SetAllModelPanelsDirty();
		LoadControlSettings( "Resource/UI/rnl_loadoutroom_allies.res" );
		return;
	}
	else if( FStrEq( command, "selectsquad" ) )
	{
		m_pSquadSelectMenu->ShowPanel( true );
		m_pSquadSelectMenu->Update();
		m_pSquadSelectMenu->MoveToCenterOfScreen();
		return;
	}
	else if( FStrEq( command, "selectequipment" ) )
	{
		KeyValues *data = new KeyValues("data");
		data->SetInt( "Team", m_iDisplayTeam );
		data->SetInt( "Class", LoadoutData.m_iClass );
		data->SetInt( "FaceSkin", LoadoutData.m_iSkinNumber );
		data->SetInt( "HelmetNumber", LoadoutData.m_iHelmetNumber );
		data->SetString( "PlayerModel", LoadoutData.m_szModelName );
		data->SetInt( "PrimaryWeapon", LoadoutData.m_iPrimaryWeapon );
		data->SetInt( "PrimaryAmmoExtra", LoadoutData.m_iPrimaryAmmoExtra );
		data->SetInt( "SecondaryWeapon", LoadoutData.m_iSecondaryWeapon );
		data->SetInt( "SecondaryAmmoExtra", LoadoutData.m_iSecondaryAmmoExtra );
		data->SetInt( "FragGrenades", LoadoutData.m_iFragGrenades );
		data->SetInt( "SmokeGrenades", LoadoutData.m_iSmokeGrenades );
		if( !m_bPlayerSelectedEquipment )
			data->SetInt("Defaults", 1);
		else
			data->SetInt("Defaults", 0);

		m_pSubLoadoutMenu->SetData( data );
		m_pSubLoadoutMenu->ShowPanel(true);
		m_pSubLoadoutMenu->Update();
		m_pSubLoadoutMenu->SetSize( XRES(460), YRES(370) );
		m_pSubLoadoutMenu->MoveToCenterOfScreen();
		return;
	}
	else if( FStrEq( command, "selectappearance" ) )
	{
		KeyValues *data = new KeyValues("data");
		data->SetInt("Team", m_iDisplayTeam);
		data->SetInt("SkinCount", 4);
		data->SetInt("CurrentFace", LoadoutData.m_iSkinNumber );
		data->SetInt("CurrentHelmet", LoadoutData.m_iHelmetNumber);
		data->SetString("PlayerModel", LoadoutData.m_szModelName);

		m_pAppearanceMenu->SetData( data );
		m_pAppearanceMenu->ShowPanel( true );
		m_pAppearanceMenu->Update();
		m_pAppearanceMenu->MoveToCenterOfScreen();
		return;
	}
	else if( FStrEq( command, "joingame" ) )
	{
		SendLoadoutString();
		ShowPanel( false );
		return;
	}

	return BaseClass::OnCommand(command);
}

inline void CRnLLoadoutRoom::SetAllModelPanelsDirty( void )
{
	m_pBackgroundModelPanel->SetPanelDirty();
	m_pEquipmentModelPanel->SetPanelDirty();
	m_pHelmetModelPanel->SetPanelDirty();
	m_pSavedLoadoutsModelPanel->SetPanelDirty();
	m_pTrashModelPanel->SetPanelDirty();
	m_pSquadModelPanel->SetPanelDirty();
	m_pMapInfoModelPanel->SetPanelDirty();
}