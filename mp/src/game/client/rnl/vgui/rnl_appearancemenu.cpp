#include "cbase.h"
#include <vgui/IScheme.h>
#include "rnl_appearancemenu.h"
#include "c_rnl_player.h"

#include "tier0/memdbgon.h"

using namespace vgui;

typedef struct HelmetData_s
{
	char HelmetName[32];
	int iBodyGroupModel;
}HelmetData_t; 

#define MAX_HELMETS	5
#define MAX_FACES	4
static HelmetData_t g_AxisHelmetData[MAX_HELMETS]=
{
	{ "Helmet", 1 },
	{ "Camo Helmet", 2 },
	{ "Helmet Again?", 3 },
	{ "Wire Helmet", 4 },
	{ "Schirmmuetze", 5 },
};

static HelmetData_t g_AlliedHelmetData[MAX_HELMETS]=
{
	{ "Helmet", 1 },
	{ "Net Helmet", 2 },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
};

static const char *g_pszFaceNames[MAX_FACES]=
{
	"Face 1",
	"Face 2",
	"Face 3",
	"Face 4",
};

CRnLAppearanceMenu::CRnLAppearanceMenu( IViewPort *pViewPort ) : BaseClass( NULL, PANEL_APPEARANCE )
{
	m_iDisplayTeam = 0;

	AppearanceData.m_iHelmetNumber = 1;
	AppearanceData.m_iSkinNumber = 0;

	m_pViewPort = pViewPort;
	SetTitle( "", true );
	SetScheme( "ClientScheme" );
	SetMoveable( false );
	SetSizeable( false );

	SetTitleBarVisible( false );
	SetProportional( true );

	m_pPreviewPanel = new CModelPanel( this, "preview_panel" );
	m_pHelmetSelection = new ItemSelectionPanel( this, "selection_helmet" );
	m_pHelmetSelection->AddActionSignalTarget( this );

	m_pSkinSelection = new ItemSelectionPanel( this, "selection_skin" );
	m_pSkinSelection->AddActionSignalTarget( this );

	LoadControlSettings( "Resource/UI/rnl_appearancemenu.res" );
}

CRnLAppearanceMenu::~CRnLAppearanceMenu()
{
	m_pPreviewPanel->MarkForDeletion();
	m_pHelmetSelection->MarkForDeletion();
	m_pSkinSelection->MarkForDeletion();
}

void CRnLAppearanceMenu::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	Color cPanelBg = pScheme->GetColor( "LoadoutPanelBackground", Color( 255, 255, 255, 255 ) );

	// set panel styles
	SetBgColor( cPanelBg );
	SetPaintBackgroundType( 2 );
}

// Handles data coming in from the loadout room, which gets the data about the player from the player's team/squad class
void CRnLAppearanceMenu::SetData( KeyValues *data )
{
	int iTeam = data->GetInt( "Team" );
	if( iTeam != m_iDisplayTeam )
	{
		m_iDisplayTeam = iTeam;
		m_pHelmetSelection->GetData().PurgeAndDeleteElements();
	}

	int iFaceSkins = data->GetInt( "SkinCount" );
	int iCurrentFaceSkin = data->GetInt( "CurrentFace" );
	int iCurrentHelmet = data->GetInt( "CurrentHelmet" );
	const char *pszModel = data->GetString( "PlayerModel" );

	m_pPreviewPanel->SwapModel( pszModel );
	m_pPreviewPanel->Paint(); // HACKHACKHACKITYHACKHACKHACKERS!!!!! Make a call to this so that the model object gets initialized -- otherwise the body groups break
	m_pPreviewPanel->SetNumSkins( iFaceSkins );
	m_pPreviewPanel->SetSkin( iCurrentFaceSkin );
	m_pPreviewPanel->SetBodyGroup( HELMET_BODYGROUP, iCurrentHelmet );
	m_pPreviewPanel->SetPanelDirty();
	
	
	// If the panels haven't been initialized with data, do it here
	if( m_pSkinSelection->GetData().Count() == 0 )
	{
		for( int i = 0; i < MAX_FACES; i++ )
		{
			m_pSkinSelection->AddValue( g_pszFaceNames[i], i );
		}
	}

	m_pSkinSelection->SetSelectionValue( iCurrentFaceSkin );

	if( m_pHelmetSelection->GetData().Count() == 0 )
	{
		HelmetData_t	*HelmetData = NULL;
		switch( m_iDisplayTeam )
		{
		case TEAM_ALLIES:
			HelmetData = g_AlliedHelmetData;
			break;
		case TEAM_AXIS:
			HelmetData = g_AxisHelmetData;
			break;
		}

		for( int i = 0; i < MAX_HELMETS; i++ )
		{
			if( FStrEq( HelmetData[i].HelmetName, "" ) )
				break;
			m_pHelmetSelection->AddValue( HelmetData[i].HelmetName, HelmetData[i].iBodyGroupModel );
		}
	}

	m_pHelmetSelection->SetSelectionValue( iCurrentHelmet );

	data->deleteThis();
}

void CRnLAppearanceMenu::Reset( void )
{
	m_iDisplayTeam = 0;
}

bool CRnLAppearanceMenu::NeedsUpdate( void )
{
	C_RnLPlayer *pPlayer = C_RnLPlayer::GetLocalRnLPlayer();
	if( pPlayer )
	{
		if( pPlayer->GetTeamNumber() != m_iDisplayTeam )
			return true;
	}

	return false;
}

void CRnLAppearanceMenu::Update( void )
{
	C_RnLPlayer *pPlayer = C_RnLPlayer::GetLocalRnLPlayer();
	if( pPlayer )
	{
		m_iDisplayTeam = pPlayer->GetTeamNumber();
	}
}

void CRnLAppearanceMenu::OnThink( void )
{
	if( !HasFocus() )
	{
		RequestFocus();
		MoveToFront();
	}
}

void CRnLAppearanceMenu::ShowPanel(bool bShow)
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

void CRnLAppearanceMenu::OnCommand( const char *command )
{
	if( FStrEq( command, "refresh" ) )
	{
		LoadControlSettings( "Resource/UI/rnl_appearancemenu.res" );
		m_iDisplayTeam = 0;
		return;
	}
	else if( FStrEq( command, "submit" ) )
	{
		// TODO: Send selected values back to the loadout panel
		KeyValues *data = new KeyValues("AppearanceSelected");
		data->SetInt( "FaceSkin", AppearanceData.m_iSkinNumber );
		data->SetInt( "HelmetNumber", AppearanceData.m_iHelmetNumber );
		PostActionSignal( data );

		ShowPanel( false );

		return;
	}
	else if( FStrEq( command, "cancel" ) )
	{
		ShowPanel( false );
		return;
	}
}

void CRnLAppearanceMenu::OnSelectionPanelChanged( KeyValues *data )
{
	const char *pszSender = data->GetString("Sender");
	const char *pszCommand = data->GetString("Command");
	int iValue = data->GetInt("CurrentValue");

	if( FStrEq( pszSender, "selection_helmet" ) )
	{
		m_pPreviewPanel->SetBodyGroup( HELMET_BODYGROUP, iValue );
		AppearanceData.m_iHelmetNumber = iValue;
		m_pPreviewPanel->SetPanelDirty();
	}
	else if( FStrEq( pszSender, "selection_skin" ) )
	{
		m_pPreviewPanel->SetSkin( iValue );
		AppearanceData.m_iSkinNumber = iValue;
		m_pPreviewPanel->SetPanelDirty();
	}
}