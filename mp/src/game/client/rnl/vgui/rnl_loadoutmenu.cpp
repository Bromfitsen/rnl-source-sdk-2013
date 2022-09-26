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
#include "rnl_itemselectionpanel.h"
#include "rnl_loadoutmenu.h"
#include "rnl_gamerules.h"
#include "spectatorgui.h"
#include "engine/IEngineSound.h"

#include "c_rnl_player.h"
#include "c_rnl_game_team.h"
#include "rnl_weapon_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern int nSelectionSongGuid;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLLoadoutMenu::CRnLLoadoutMenu(IViewPort *pViewPort) : BaseClass( NULL, PANEL_LOADOUT )
{
	m_pViewPort = pViewPort;

	m_iDisplayTeam = 0; // cjd @add

	// initialize dialog
	SetProportional(true);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );

	// set the scheme before any child control is created
	SetScheme("ClientScheme");

	// set up panels and labels
	m_pPreviewPanel = new CModelPanel( this, "preview_panel" );
	m_pPrimaryWeapon = new ItemSelectionPanel( this, "selection_primary_weapon" );
	m_pPrimaryWeapon->AddActionSignalTarget( this );

	m_pSecondaryWeapon = new ItemSelectionPanel( this, "selection_secondary_weapon" );
	m_pPrimaryWeapon->AddActionSignalTarget( this );

	m_pGrenades1 = new ItemSelectionPanel( this, "selection_grenade1" );
	m_pGrenades1->AddActionSignalTarget( this );

	m_pGrenades2 = new ItemSelectionPanel( this, "selection_grenade2" );
	m_pGrenades2->AddActionSignalTarget( this );

	m_pExtraPrimaryAmmo = new ItemSelectionPanel( this, "selection_extra_primary" );
	m_pExtraPrimaryAmmo->AddActionSignalTarget( this );

	m_pExtraSecondaryAmmo = new ItemSelectionPanel( this, "selection_extra_secondary" );
	m_pExtraSecondaryAmmo->AddActionSignalTarget( this );

	m_pWeightBar = new ProgressBar( this, "weight_bar" );

	LoadControlSettings("resource/ui/rnl_loadoutmenu.res");

}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CRnLLoadoutMenu::~CRnLLoadoutMenu()
{
	m_pPrimaryWeapon->MarkForDeletion();
	m_pSecondaryWeapon->MarkForDeletion();
	m_pGrenades1->MarkForDeletion();
	m_pGrenades2->MarkForDeletion();
	m_pExtraPrimaryAmmo->MarkForDeletion();
	m_pExtraSecondaryAmmo->MarkForDeletion();
}

//-----------------------------------------------------------------------------
// Purpose: Sets up screen
//-----------------------------------------------------------------------------
void CRnLLoadoutMenu::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	Color cPanelBg = pScheme->GetColor( "LoadoutPanelBackground", Color( 255, 255, 255, 255 ) );

	// set panel styles
	m_pPreviewPanel->SetBgColor( cPanelBg );
	m_pPreviewPanel->SetPaintBackgroundType(2);

	SetBgColor( cPanelBg );
	SetPaintBackgroundType( 2 );
}

bool CRnLLoadoutMenu::NeedsUpdate( void )
{
	if( m_flNextUpdateTime < gpGlobals->curtime )
		return true;
	return false;
}

void CRnLLoadoutMenu::Update( void )
{
	// Keep it in the foreground if its open
	MoveToFront();
}
//-----------------------------------------------------------------------------
// Purpose: Show the damn thing
//-----------------------------------------------------------------------------
void CRnLLoadoutMenu::ShowPanel(bool bShow)
{
	if ( IsVisible() == bShow )
		return;

	CRnLPlayer* pPlayer = CRnLPlayer::GetLocalRnLPlayer();
	if( !pPlayer )
		return;

	if ( bShow )
	{
		Update();
		Activate();
		SetMouseInputEnabled( true );
		SetKeyBoardInputEnabled( true );

		MoveToCenterOfScreen();
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Send the loadout string (RNL_PLAYER ADAPTER CODE)
//-----------------------------------------------------------------------------
void CRnLLoadoutMenu::SendLoadoutString()
{
	char szLoadOut[64];
	sprintf( szLoadOut, "changeloadout 0 0 0 0" );
	/*
	sprintf(szLoadOut, "changeloadout %i %i %i %i %i %i %i %i %i %i", pLoadout.iPrimaryWeapon, pLoadout.iExtraPrimaryAmmo, 
		pLoadout.iSecondaryWeapon, pLoadout.iExtraSecondaryAmmo, pLoadout.iFragGrenades, pLoadout.iFragGrenades, 
		pLoadout.iSmokeGrenades, pLoadout.iSmokeGrenades, pLoadout.iHelmet, pLoadout.iFaceSkin);
	*/
	engine->ClientCmd( szLoadOut );
}

//-----------------------------------------------------------------------------
// Purpose: handle button commands
//-----------------------------------------------------------------------------
void CRnLLoadoutMenu::OnCommand( const char *command )
{
	if ( FStrEq( command, "submit" ) )
	{
		// SendLoadoutString();
		KeyValues *data = new KeyValues("EquipmentSelected");
		data->SetInt( "PrimaryWeapon", LoadoutData.m_iPrimaryWeapon );
		data->SetInt( "PrimaryAmmoExtra", LoadoutData.m_iPrimaryAmmoExtra );
		data->SetInt( "SecondaryWeapon", LoadoutData.m_iSecondaryWeapon );
		data->SetInt( "SecondaryAmmoExtra", LoadoutData.m_iSecondaryAmmoExtra );
		data->SetInt( "FragGrenades", LoadoutData.m_iFragGrenades );
		data->SetInt( "SmokeGrenades", LoadoutData.m_iSmokeGrenades );
		PostActionSignal( data );
		ShowPanel( false );
	}
	else if( FStrEq( command, "cancel" ) )
	{
		ShowPanel( false );
	}
	else if( FStrEq( command, "reload" ) )
	{
		LoadControlSettings( "resource/ui/rnl_loadoutmenu.res" );
	}
	else
	{
		engine->ClientCmd( command );
	}
}

inline const int CRnLLoadoutMenu::ComputeTotalLoadoutWeight( void ) const
{
	// Update the weight bar
	int iTotalWeight = 0;
	iTotalWeight += iPrimaryWeaponWeights[LoadoutData.m_iPrimaryWeapon];
	iTotalWeight += iPrimaryWeaponWeights[LoadoutData.m_iSecondaryWeapon];
	switch( m_iDisplayTeam )
	{
	case TEAM_ALLIES:
		iTotalWeight += (iAlliesEquipmentWeights[EQUIPMENT_ALLIES_FRAGGRENADES] * LoadoutData.m_iFragGrenades);
		iTotalWeight += (iAlliesEquipmentWeights[EQUIPMENT_ALLIES_SMOKEGRENADE] * LoadoutData.m_iSmokeGrenades);
		iTotalWeight += (iAlliesEquipmentWeights[EQUIPMENT_ALLIES_EXTRAAMMO] * LoadoutData.m_iPrimaryAmmoExtra);
		// TODO: Secondary ammo?
		break;
	case TEAM_AXIS:
		iTotalWeight += (iAxisEquipmentWeights[EQUIPMENT_AXIS_STIELHANDGRANATE] * LoadoutData.m_iFragGrenades);
		iTotalWeight += (iAxisEquipmentWeights[EQUIPMENT_AXIS_SMOKEGRENADE] * LoadoutData.m_iSmokeGrenades);
		iTotalWeight += (iAxisEquipmentWeights[EQUIPMENT_AXIS_EXTRAAMMO] * LoadoutData.m_iPrimaryAmmoExtra);
		// TODO: Secondary ammo?
		break;
	}

	return iTotalWeight;
}
void CRnLLoadoutMenu::OnSelectionPanelChanged( KeyValues *data )
{
	const char *pszSender = data->GetString("Sender");
	const char *pszCommand = data->GetString("Command");
	int iCurrentValue = data->GetInt("CurrentValue");

	// TODO: Do something?
	if( FStrEq( pszSender, "selection_primary_weapon" ) )
	{
		// Subtract the old weight and add the new one, see if its too heavy
		int iWeight = m_iWeight - iPrimaryWeaponWeights[LoadoutData.m_iPrimaryWeapon];
		if( iWeight + iPrimaryWeaponWeights[iCurrentValue] > MAX_EQUIPMENT_WEIGHT )
		{
			DevMsg("Too heavy\n");
			m_pPrimaryWeapon->SetSelectionValue( LoadoutData.m_iPrimaryWeapon ); // set it back
			return;
		}
		LoadoutData.m_iPrimaryWeapon = iCurrentValue;
	}
	else if( FStrEq( pszSender, "selection_secondary_weapon" ) )
	{
		// Subtract the old weight and add the new one, see if its too heavy
		int iWeight = m_iWeight - iPrimaryWeaponWeights[LoadoutData.m_iSecondaryWeapon];
		if( iWeight + iPrimaryWeaponWeights[iCurrentValue] > MAX_EQUIPMENT_WEIGHT )
		{
			DevMsg("Too heavy\n");
			m_pSecondaryWeapon->SetSelectionValue( LoadoutData.m_iSecondaryWeapon ); // set it back
			return;
		}
		LoadoutData.m_iSecondaryWeapon = iCurrentValue;
	}
	else if( FStrEq( pszSender, "selection_grenade1" ) )
	{
		// Can we have another grenade?
		int iSingleGrenadeWeight = 0;
		int iOldGrenadeWeight = 0;
		int iNewGrenadeWeight = 0;
		switch( m_iDisplayTeam )
		{
		case TEAM_ALLIES:
			iSingleGrenadeWeight = iAlliesEquipmentWeights[EQUIPMENT_ALLIES_FRAGGRENADES];
			break;
		case TEAM_AXIS:
			iSingleGrenadeWeight = iAxisEquipmentWeights[EQUIPMENT_AXIS_STIELHANDGRANATE];
			break;
		}
		iOldGrenadeWeight = iSingleGrenadeWeight * LoadoutData.m_iFragGrenades;
		iNewGrenadeWeight = iSingleGrenadeWeight * iCurrentValue;
		
		if( m_iWeight + (iNewGrenadeWeight - iOldGrenadeWeight) > MAX_EQUIPMENT_WEIGHT )
		{
			DevMsg("Too heavy\n");
			m_pGrenades1->SetSelectionValue( LoadoutData.m_iFragGrenades );	// set it back to the original
			return;
		}

		LoadoutData.m_iFragGrenades = iCurrentValue;
	}
	else if( FStrEq( pszSender, "selection_grenade2" ) )
	{
		// Can we have another grenade?
		int iSingleGrenadeWeight = 0;
		int iOldGrenadeWeight = 0;
		int iNewGrenadeWeight = 0;
		switch( m_iDisplayTeam )
		{
		case TEAM_ALLIES:
			iSingleGrenadeWeight = iAlliesEquipmentWeights[EQUIPMENT_ALLIES_SMOKEGRENADE];
			break;
		case TEAM_AXIS:
			iSingleGrenadeWeight = iAxisEquipmentWeights[EQUIPMENT_AXIS_SMOKEGRENADE];
			break;
		}
		iOldGrenadeWeight = iSingleGrenadeWeight * LoadoutData.m_iSmokeGrenades;
		iNewGrenadeWeight = iSingleGrenadeWeight * iCurrentValue;
		
		if( m_iWeight + (iNewGrenadeWeight - iOldGrenadeWeight) > MAX_EQUIPMENT_WEIGHT )
		{
			DevMsg("Too heavy\n");
			m_pGrenades2->SetSelectionValue( LoadoutData.m_iSmokeGrenades );	// set it back to the original
			return;
		}

		LoadoutData.m_iSmokeGrenades = iCurrentValue;
	}
	else if( FStrEq( pszSender, "selection_extra_primary" ) )
	{
		// Can we have any more ammo?
		int iSingleAmmoWeight = 0;
		int iOldAmmoWeight = 0;
		int iNewAmmoWeight = 0;
		switch( m_iDisplayTeam )
		{
		case TEAM_ALLIES:
			iSingleAmmoWeight = iAlliesEquipmentWeights[EQUIPMENT_ALLIES_EXTRAAMMO];
			break;
		case TEAM_AXIS:
			iSingleAmmoWeight = iAxisEquipmentWeights[EQUIPMENT_AXIS_EXTRAAMMO];
			break;
		}
		iOldAmmoWeight = iSingleAmmoWeight * LoadoutData.m_iPrimaryAmmoExtra;
		iNewAmmoWeight = iSingleAmmoWeight * iCurrentValue;
		
		if( m_iWeight + (iNewAmmoWeight - iOldAmmoWeight) > MAX_EQUIPMENT_WEIGHT )
		{
			DevMsg("Too heavy\n");
			m_pExtraPrimaryAmmo->SetSelectionValue( LoadoutData.m_iPrimaryAmmoExtra ); // set it back
			return;
		}

		LoadoutData.m_iPrimaryAmmoExtra = iCurrentValue;
	}


	
	// Update the preview
	SetupPreviewPanel();
	
	m_iWeight = ComputeTotalLoadoutWeight();

	float flWeightFrac = (float)(m_iWeight / MAX_EQUIPMENT_WEIGHT );	// This is just an arbitrary number for now
	m_pWeightBar->SetProgress( flWeightFrac );
}

void CRnLLoadoutMenu::SetDefaultLoadout( int iClass )
{
	// Get info about the class and weapons
	C_RnLGameTeam *pTeam = (C_RnLGameTeam *)GetGlobalRnLTeam( m_iDisplayTeam );

	if( pTeam == NULL )
		return;

	// Purge any old data in the selection panels
	m_pPrimaryWeapon->GetData().PurgeAndDeleteElements();
	m_pSecondaryWeapon->GetData().PurgeAndDeleteElements();
	m_pGrenades1->GetData().PurgeAndDeleteElements();
	m_pGrenades2->GetData().PurgeAndDeleteElements();
	m_pExtraPrimaryAmmo->GetData().PurgeAndDeleteElements();
	m_pExtraSecondaryAmmo->GetData().PurgeAndDeleteElements();

	WEAPON_FILE_INFO_HANDLE	hWpnInfo = NULL;
	CRnLWeaponInfo *pWeaponInfo = NULL;
	const char *pszWeaponAlias = NULL;
	int iWeaponId = WEAPON_NONE;
	int iFragMaxCarry = 0;
	int iSmokeMaxCarry = 0;
	int iExtraAmmoMaxCarry = 0;
	switch( m_iDisplayTeam )
	{
	case TEAM_ALLIES:
		iFragMaxCarry = iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_FRAGGRENADES];
		iSmokeMaxCarry = iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_SMOKEGRENADE];
		iExtraAmmoMaxCarry = iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_EXTRAAMMO];
		break;
	case TEAM_AXIS:
		iFragMaxCarry = iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_STIELHANDGRANATE];
		iSmokeMaxCarry = iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_SMOKEGRENADE];
		iExtraAmmoMaxCarry = iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_EXTRAAMMO];
		break;
	}

	// Primary weapon data population
	LoadoutData.m_iPrimaryWeapon = pTeam->GetClassDescription( iClass ).iPrimaryWeapon[0];
	for( int i = 0; i < MAX_LOADOUT_CHOICES; i++ )
	{
		iWeaponId = pTeam->GetClassDescription( iClass ).iPrimaryWeapon[i];
		if( iWeaponId == WEAPON_NONE )
			continue;

		pszWeaponAlias = WeaponIDToAlias( iWeaponId );
		hWpnInfo = LookupWeaponInfoSlot( VarArgs( "weapon_%s", pszWeaponAlias ) );
		if( hWpnInfo != GetInvalidWeaponInfoHandle() )
		{
			pWeaponInfo = static_cast< CRnLWeaponInfo *>( GetFileWeaponInfoFromHandle( hWpnInfo ) );
			if( pWeaponInfo )
				m_pPrimaryWeapon->AddValue( pWeaponInfo->szPrintName, iWeaponId );
		}
	}
	m_pPrimaryWeapon->SetSelectedIndex( 0 );

	// Secondary weapon data population
	LoadoutData.m_iSecondaryWeapon = pTeam->GetClassDescription( iClass ).iSecondaryWeapon[0];
	for( int i = 0; i < MAX_LOADOUT_CHOICES; i++ )
	{
		iWeaponId = pTeam->GetClassDescription( iClass ).iSecondaryWeapon[i];
		if( iWeaponId == WEAPON_NONE )
			continue;

		pszWeaponAlias = WeaponIDToAlias( iWeaponId );
		hWpnInfo = LookupWeaponInfoSlot( VarArgs( "weapon_%s", pszWeaponAlias ) );
		if( hWpnInfo != GetInvalidWeaponInfoHandle() )
		{
			pWeaponInfo = static_cast< CRnLWeaponInfo *>( GetFileWeaponInfoFromHandle( hWpnInfo ) );
			if( pWeaponInfo )
			{
				m_pSecondaryWeapon->AddValue( pWeaponInfo->szPrintName, iWeaponId );
			}
		}
	}
	m_pSecondaryWeapon->SetSelectedIndex( 0 );

	// Frags data population... time to get creative because it isn't read the same way as primary/secondary weapons
	LoadoutData.m_iFragGrenades = 0;
	iWeaponId = pTeam->GetClassDescription( iClass ).iGrenades[0];
	pszWeaponAlias = WeaponIDToAlias( iWeaponId );
	hWpnInfo = LookupWeaponInfoSlot( VarArgs( "weapon_%s", pszWeaponAlias ) );
	if( hWpnInfo != GetInvalidWeaponInfoHandle() )
	{
		pWeaponInfo = static_cast< CRnLWeaponInfo *>( GetFileWeaponInfoFromHandle( hWpnInfo ) );
		if( pWeaponInfo )
		{
			if( !m_pGrenades1->IsVisible() )
				m_pGrenades1->SetVisible( true );

			for( int i = 0; i <= iFragMaxCarry; i++ )
			{
				m_pGrenades1->AddValue( VarArgs( "%s (%d)", pWeaponInfo->szPrintName, i ), i );
			}
			LoadoutData.m_iFragGrenades = 2;	// some arbitrary default number
			m_pGrenades1->SetSelectionValue( LoadoutData.m_iFragGrenades );
		}
	}
	else
	{
		m_pGrenades1->SetSelectionValue( 0 );
		m_pGrenades1->SetVisible( false );
	}

	// Smoke grenades population.... also get creative...
	LoadoutData.m_iSmokeGrenades = 0;
	iWeaponId = pTeam->GetClassDescription( iClass ).iGrenades[1];
	pszWeaponAlias = WeaponIDToAlias( iWeaponId );
	hWpnInfo = LookupWeaponInfoSlot( VarArgs( "weapon_%s", pszWeaponAlias ) );
	if( hWpnInfo != GetInvalidWeaponInfoHandle() )
	{
		pWeaponInfo = static_cast< CRnLWeaponInfo *>( GetFileWeaponInfoFromHandle( hWpnInfo ) );
		if( pWeaponInfo )
		{
			if( !m_pGrenades2->IsVisible() )
				m_pGrenades2->SetVisible( true );
			for( int i = 0; i <= iSmokeMaxCarry; i++ )
			{
				m_pGrenades2->AddValue( VarArgs( "%s (%d)", pWeaponInfo->szPrintName, i ), i );
			}
			LoadoutData.m_iSmokeGrenades = 1; // some arbitrary default number
			m_pGrenades2->SetSelectionValue( LoadoutData.m_iSmokeGrenades );
		}
	}
	else
	{
		m_pGrenades2->SetSelectionValue( 0 );
		m_pGrenades2->SetVisible( false );
	}

	// Extra ammo population
	for( int i = 0; i <= iExtraAmmoMaxCarry; i++ )
	{
		m_pExtraPrimaryAmmo->AddValue( VarArgs( "Extra Ammo (%d)", i ), i );
	}
	LoadoutData.m_iPrimaryAmmoExtra = 1;	// some arbitrary default number
	LoadoutData.m_iSecondaryAmmoExtra = 1;	// some arbitrary default number

	m_pExtraPrimaryAmmo->SetSelectionValue( LoadoutData.m_iPrimaryAmmoExtra );

}

void CRnLLoadoutMenu::SetupPreviewPanel( void )
{
	// Get primary weapon data and attach the model to the screen
	const char *pszWeaponAlias = WeaponIDToAlias( LoadoutData.m_iPrimaryWeapon );
	WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( VarArgs( "weapon_%s", pszWeaponAlias ) );
	if( hWpnInfo != GetInvalidWeaponInfoHandle() )
	{
		CRnLWeaponInfo *pWeaponInfo = static_cast< CRnLWeaponInfo *>( GetFileWeaponInfoFromHandle( hWpnInfo ) );
		if( pWeaponInfo )
		{
			m_pPreviewPanel->ClearAttachedModelInfos();
			m_pPreviewPanel->AddAttachedModel( pWeaponInfo->szWorldModel );
			m_pPreviewPanel->AddAnimation( VarArgs( "sprint_upper_%s", pWeaponInfo->m_szAnimExtension ) );
			m_pPreviewPanel->SetDefaultAnimation( VarArgs( "sprint_upper_%s", pWeaponInfo->m_szAnimExtension ) );
			m_pPreviewPanel->SetPlaybackRate( 0 );
		}
	}

	const char *pszSecondaryBodygroupName = NULL;
	switch( m_iDisplayTeam )
	{
	case TEAM_ALLIES:
		pszSecondaryBodygroupName = COLTHOLSTER_BODYGROUP;
		break;
	case TEAM_AXIS:
		pszSecondaryBodygroupName = P38HOLSTER_BODYGROUP;
		break;
	}

	m_pPreviewPanel->SetBodyGroup( pszSecondaryBodygroupName, LoadoutData.m_iSecondaryWeapon != 0 ? 1 : 0 );
	m_pPreviewPanel->SetBodyGroup( AMMO_POUCHES_BODYGROUP, sWeaponAmmoGroups[LoadoutData.m_iPrimaryWeapon][LoadoutData.m_iPrimaryAmmoExtra] );
	m_pPreviewPanel->SetBodyGroup( GRENADES_BODYGROUP, LoadoutData.m_iFragGrenades );
	m_pPreviewPanel->SetBodyGroup( SMOKES_BODYGROUP, LoadoutData.m_iSmokeGrenades );
	
	m_pPreviewPanel->SetPanelDirty();
}

//-----------------------------------------------------------------------------
// Purpose: React to the confirmation of the joinsquad command
//-----------------------------------------------------------------------------
void CRnLLoadoutMenu::SetData( KeyValues *data )
{
	bool bLoadDefaults = data->GetInt("Defaults") == 1 ? true : false;
	m_iDisplayTeam = data->GetInt("Team");
	int iClass = data->GetInt("Class");
	int iHelmet = data->GetInt("HelmetNumber");
	int iFaceSkin = data->GetInt("FaceSkin");
	const char *pszPlayerModel = data->GetString("PlayerModel");
	
	// Setup the preview panel
	m_pPreviewPanel->SwapModel( pszPlayerModel );
	m_pPreviewPanel->Paint(); // HACKHACKHACKHACKITYHACKHACKERS!!!!!

	if( bLoadDefaults )
	{
		SetDefaultLoadout(iClass);
	}
	else
	{
		LoadoutData.m_iPrimaryWeapon = data->GetInt("PrimaryWeapon");
		LoadoutData.m_iPrimaryAmmoExtra = data->GetInt("PrimaryAmmoExtra");
		LoadoutData.m_iSecondaryWeapon = data->GetInt("SecondaryWeapon");
		LoadoutData.m_iSecondaryAmmoExtra = data->GetInt("SecondaryAmmoExtra");
		LoadoutData.m_iFragGrenades = data->GetInt("FragGrenades");
		LoadoutData.m_iSmokeGrenades = data->GetInt("SmokeGrenades");
	}

	SetupPreviewPanel();	// sets the extra data on the preview panel since the model, helmet, and face are already done
	m_pPreviewPanel->SetBodyGroup( HELMET_BODYGROUP, iHelmet );
	m_pPreviewPanel->SetSkin( iFaceSkin );
	m_pPreviewPanel->SetPanelDirty();
}