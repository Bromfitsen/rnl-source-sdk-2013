// RnL Loading Dialog -
//			Most credit goes to Tony Sergi for the base code.
// 3mm loading dialog - based on tf2's stats summary / loading panel, except.. with no stats!!!!!
#include "cbase.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <filesystem.h>
#include "IGameUIFuncs.h" // for key bindings
#include "inputsystem/iinputsystem.h"

#include "rnl_loadingpanel.h"
#include "rnl_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifndef _XBOX
extern IGameUIFuncs *gameuifuncs; // for key binding details
#endif

CRnLLoadingPanel *g_pLoadingPanelInstance = NULL;
CRnLLoadingPanel* CRnLLoadingPanel::GetInstance( void )
{
	if( g_pLoadingPanelInstance == NULL )
		g_pLoadingPanelInstance = new CRnLLoadingPanel();

	return g_pLoadingPanelInstance;
}

void CRnLLoadingPanel::DestroyInstance( void )
{
	delete g_pLoadingPanelInstance;
	g_pLoadingPanelInstance = NULL;
}


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLLoadingPanel::CRnLLoadingPanel() : vgui::EditablePanel( NULL, "RnLLoadingPanel", 
										vgui::scheme()->LoadSchemeFromFile( "Resource/ClientScheme.res", "ClientScheme" ) )
{
	//Added to enable tips
	g_pVGuiLocalize->AddFile( "scripts/loading/loading_%language%.txt", "MOD" );

	m_fLastTipTime = -1;
	m_fLastContentTime = -1;
	m_bMapSet = false;

	KeyValues* pKVLoading = new KeyValues( "Loading" );
	if( pKVLoading->LoadFromFile( g_pFullFileSystem, "scripts/loading/loading.txt", "MOD" ) )
	{
		KeyValues* pTipKeys = pKVLoading->FindKey( "tips" );
		if( pTipKeys != NULL )
		{
			KeyValues* pKey = pTipKeys->GetFirstSubKey();
			while( pKey != NULL )
			{
				if( Q_stricmp( "tip", pKey->GetName() ) == 0 )
				{
					int indx = m_TipKeys.AddToTail();
					m_TipKeys[indx] = pKey->GetString();
				}
				pKey = pKey->GetNextKey();
			}
		}

		KeyValues* pPanels = pKVLoading->FindKey( "panels" );
		if( pPanels != NULL )
		{
			KeyValues* pKey = pPanels->GetFirstSubKey();
			while( pKey != NULL )
			{
				if( Q_stricmp( "panel", pKey->GetName() ) == 0 )
				{
					int indx = m_ContentPanelFiles.AddToTail();
					m_ContentPanelFiles[indx].m_WidescreenFile  = pKey->GetString( "wide" );
					m_ContentPanelFiles[indx].m_NormalFile = pKey->GetString( "normal" );

					DevMsg( "Loading Images: (%s) (%s)\n", 
						m_ContentPanelFiles[indx].m_NormalFile.Get(),
						m_ContentPanelFiles[indx].m_WidescreenFile.Get() );
				}
				pKey = pKey->GetNextKey();
			}
		}
	}
	pKVLoading->deleteThis();

	m_pTipLabel = new vgui::Label( this, "TipLabel", "" );

	ListenForGameEvent( "server_spawn" );

	Reset();
}

CRnLLoadingPanel::~CRnLLoadingPanel()
{
}

//-----------------------------------------------------------------------------
// Purpose: Resets the dialog
//-----------------------------------------------------------------------------
void CRnLLoadingPanel::Reset()
{
	m_fLastTipTime = -1;
	m_fLastContentTime = -1;
	m_bMapSet = false;

	InvalidateLayout( false, true );
	SetVisible( false );
	MakePopup( false );
}

//-----------------------------------------------------------------------------
// Purpose: Applies scheme settings
//-----------------------------------------------------------------------------
void CRnLLoadingPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetProportional( true );
	LoadControlSettings( "Resource/UI/rnl_loading.res" );
	
	Update();
	SetVisible( false );
	SetBgColor( Color( 32,32,25,255) );

	SetSize( XRES(640), YRES(480) );
}

//-----------------------------------------------------------------------------
// Purpose: Updates the dialog
//-----------------------------------------------------------------------------
void CRnLLoadingPanel::ClearMapLabel()
{
	vgui::Label *pMapLabel = dynamic_cast<vgui::Label *>( FindChildByName( "MapLabel" ) );

	if ( pMapLabel && pMapLabel->IsVisible() )
		pMapLabel->SetVisible( false );

	SetDialogVariable( "maplabel", "" );

	m_bMapSet = false;
}

//-----------------------------------------------------------------------------
// Purpose: Updates the dialog
//-----------------------------------------------------------------------------
void CRnLLoadingPanel::Update()
{
	if( m_fLastTipTime == -1 || (m_fLastTipTime+5.0f) < Plat_FloatTime() )
		UpdateTip();
	if( !m_bMapSet && (m_fLastContentTime == -1 || (m_fLastContentTime + 15.0f) < Plat_FloatTime()) )
		UpdateBackground();
}

//-----------------------------------------------------------------------------
// Purpose: Updates the tip
//-----------------------------------------------------------------------------
void CRnLLoadingPanel::UpdateTip()
{
	m_fLastTipTime = Plat_FloatTime();
	if( m_TipKeys.Count() > 0 )
	{
		wchar_t wzTipLabel[255] = L"";

		swprintf(wzTipLabel, L"%s %s", g_pVGuiLocalize->Find( "#TipLabel" ), ParseTipForKeys( g_pVGuiLocalize->Find(m_TipKeys[random->RandomInt( 0, m_TipKeys.Count() - 1 )].Get()) ) );
		SetDialogVariable( "tiplabel", wzTipLabel );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Parse console commands from a tip in the %command% format and
//			insert the name of the key that it is bound to.
//			Ex: "Press %duck% to crouch" becomes
//				"Press <CTRL> to crouch"
//-----------------------------------------------------------------------------
wchar_t *CRnLLoadingPanel::ParseTipForKeys( wchar_t* wzStr )
{
	char szStr[255] = "";
	// Convert it for processing.
	g_pVGuiLocalize->ConvertUnicodeToANSI(wzStr, szStr, 255);

	// If there isn't a %, forget it.
	if( !Q_stristr(szStr, "%") )
		return wzStr;

	int iLen = Q_wcslen( wzStr );
	int iStartLocation = -1;
	char szTemp[128] = "";
	char szTemp2[128] = "";

	// For each letter in the tip...
	for( int i = 0; i < iLen - 1; i++ )
	{
		// Only do something when it finds a '%'
		if( szStr[i] == '%' )
		{
			// If we have already found the opening '%',
			// this must be the closing '%'
			if( iStartLocation >= 0 )
			{
				// Cut out what is between the '%'s
				Q_StrSlice(szStr, iStartLocation + 1, i, szTemp, 128);

				// Get the button code for the parsed command
				ButtonCode_t iButton = gameuifuncs->GetButtonCodeForBind( szTemp );

				// Get the first part of the original string
				Q_StrSlice(szStr, 0, iStartLocation, szTemp, 128);

				// If there is anything after this, get that part too.
				if( i < iLen )
					Q_StrSlice(szStr, i + 1, iLen, szTemp2, 128);
				// Clear the second temp buffer so that leftovers aren't used when
				// we are at the end of the string.
				else
					Q_strcpy(szTemp2, "");

				// If the button code is valid and in use
				if( iButton > BUTTON_CODE_NONE )
				{
					// Get the name of it inbetween "<" and ">"
					sprintf(szStr, "%s<%s>%s", szTemp, g_pInputSystem->ButtonCodeToString( iButton ), szTemp2);
					// Update our current location to the end of what we just processed.
					i = iStartLocation + Q_strlen( g_pInputSystem->ButtonCodeToString( iButton ) );
				}
				// Otherwise, it is unbound
				else
				{
					sprintf(szStr, "%s<Unbound>%s", szTemp, szTemp2);
					// Update our current location to the end of what we just processed.
					i = iStartLocation + Q_strlen("<Unbound>");
				}

				// Reset the starting location
				iStartLocation = -1;
				// Update the upper bounds so we check the whole string.
				iLen = Q_strlen(szStr);
			}
			// Otherwise, store the current location
			else
				iStartLocation = i;
		}
	}

	// Convert it back.
	static wchar_t wzTemp[255] = L"";
	g_pVGuiLocalize->ConvertANSIToUnicode(szStr, wzTemp, 255);

	return wzTemp;
}

void CRnLLoadingPanel::UpdateBackground()
{
	m_fLastContentTime = Plat_FloatTime();
	if( m_ContentPanelFiles.Count() > 0 )
	{
		vgui::ImagePanel *pImagePanel = dynamic_cast<vgui::ImagePanel *>( FindChildByName( "MainBackground" ) );
		if ( pImagePanel )
		{
			/*int screenWide, screenTall;
			vgui::surface()->GetScreenSize( screenWide, screenTall );
			float aspectRatio = (float)screenWide/(float)screenTall;
			bool bIsWidescreen = aspectRatio >= 1.6f;*/

			int choice = random->RandomInt( 0, m_ContentPanelFiles.Count()-1);
			pImagePanel->SetImage( /*bIsWidescreen ? 
				m_ContentPanelFiles[choice].m_WidescreenFile.Get() :*/
				m_ContentPanelFiles[choice].m_NormalFile.Get()
				);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Event handler
//-----------------------------------------------------------------------------
void CRnLLoadingPanel::FireGameEvent( IGameEvent *event )
{
	const char *pEventName = event->GetName();

	if ( Q_strcmp( pEventName, "server_spawn" ) ==  0 )
	{
		UpdateTip();

		const char *pMapName = event->GetString( "mapname" );
		if ( pMapName )
		{
			// If we're loading a background map, don't display anything
			// HACK: Client doesn't get gpGlobals->eLoadType, so just do string compare for now.
			if ( Q_stristr( pMapName, "background") )
			{
				ClearMapLabel();
				UpdateTip();
				UpdateBackground();
				m_bMapSet = false;
			}
			else
			{
				vgui::ImagePanel *pBitmap = dynamic_cast<vgui::ImagePanel *>( FindChildByName( "MainBackground" ) );
				if ( pBitmap )
				{
					// determine if we're in widescreen or not and select the appropriate image
					/*int screenWide, screenTall;
					vgui::surface()->GetScreenSize( screenWide, screenTall );
					float aspectRatio = (float)screenWide/(float)screenTall;
					bool bIsWidescreen = aspectRatio >= 1.6f;

					if (bIsWidescreen)
						pBitmap->SetImage(VarArgs("loading/%s_wide", pMapName));
					else*/
						pBitmap->SetImage(VarArgs("loading/%s", pMapName));
					pBitmap->SetVisible(true);
				}

				vgui::Label *pMapLabel = dynamic_cast<vgui::Label *>( FindChildByName( "MapLabel" ) );
				if ( pMapLabel )
					pMapLabel->SetVisible( true );

				// set the map name in the UI
				wchar_t wzMapName[55] = L"";
				g_pVGuiLocalize->ConvertANSIToUnicode( pMapName, wzMapName, sizeof(wzMapName) );
				wchar_t wzMapLabel[255] = L"";
				swprintf( wzMapLabel, L"%s %s", g_pVGuiLocalize->Find( "#LoadingMap" ), wzMapName );

				SetDialogVariable( "maplabel", wzMapLabel );

				m_bMapSet = true;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when we are activated during level load
//-----------------------------------------------------------------------------
void CRnLLoadingPanel::OnActivate()
{
	m_fLastTipTime = -1;
	m_fLastContentTime = -1;

	ClearMapLabel();
	Update();
}

//-----------------------------------------------------------------------------
// Purpose: Called when we are deactivated at end of level load
//-----------------------------------------------------------------------------
void CRnLLoadingPanel::OnDeactivate()
{
	m_fLastTipTime = -1;
	m_fLastContentTime = -1;

	ClearMapLabel();
	Update();
}