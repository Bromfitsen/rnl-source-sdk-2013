//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>
#include <cdll_util.h>
#include <globalvars_base.h>
#include <igameresources.h>
#include "IGameUIFuncs.h" // for key bindings
#include "inputsystem/iinputsystem.h"
#include "rnl_mapdialog.h"
#include <voice_status.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vstdlib/IKeyValuesSystem.h>

#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/SectionedListPanel.h>

#include <game/client/iviewport.h>
#include <igameresources.h>
#include <filesystem.h>

#include "vgui_avatarimage.h"

#include "c_rnl_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern bool AvatarIndexLessFunc( const int &lhs, const int &rhs );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLMapDialog::CRnLMapDialog(IViewPort *pViewPort) : EditablePanel( NULL, PANEL_MAP )
{
	//memset(s_VoiceImage, 0x0, sizeof( s_VoiceImage ));
	m_pViewPort = pViewPort;

	// initialize dialog
	SetProportional(true);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	// set the scheme before any child control is created
	SetScheme("ClientScheme");

	LoadControlSettings("Resource/UI/rnl_map.res");
	SetSize(ScreenWidth(), ScreenHeight());
	SetDragEnabled( false );

	m_pMapPanel = new CRnLMapPanel( this, "mappanel" );

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLMapDialog::~CRnLMapDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: clears everything in the scoreboard and all it's state
//-----------------------------------------------------------------------------
void CRnLMapDialog::Reset()
{
	m_fNextUpdateTime = 0;
}


//-----------------------------------------------------------------------------
// Purpose: sets up screen
//-----------------------------------------------------------------------------
void CRnLMapDialog::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	PostApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: Does dialog-specific customization after applying scheme settings.
//-----------------------------------------------------------------------------
void CRnLMapDialog::PostApplySchemeSettings( vgui::IScheme *pScheme )
{
	// light up scoreboard a bit
	SetBgColor( Color( 0,0,0,0) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLMapDialog::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		Update();
		SetVisible( true );
		MoveToFront();
	}
	else
	{
		BaseClass::SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}
}

bool CRnLMapDialog::NeedsUpdate( void )
{
	return (m_fNextUpdateTime < gpGlobals->curtime);	
}

//-----------------------------------------------------------------------------
// Purpose: Recalculate the internal scoreboard data
//-----------------------------------------------------------------------------
void CRnLMapDialog::Update( void )
{
	if (CBasePlayer::GetLocalPlayer() == NULL)
		return;

	CRnLPlayer* pPlayer = CRnLPlayer::GetLocalRnLPlayer();
	if (!pPlayer)
		return;

	// Reset();
	MoveToCenterOfScreen();

	m_pMapPanel->Update();

	if (pPlayer->GetTeamNumber() <= TEAM_SPECTATOR ||
		!pPlayer->IsAlive() ||
		(pPlayer->GetWaterLevel() >= WL_Waist))
	{
		ShowPanel(false);
	}

	// update every second
	m_fNextUpdateTime = gpGlobals->curtime + 1.0f; 
}

//-----------------------------------------------------------------------------
// Purpose: Center the dialog on the screen.  (vgui has this method on
//			Frame, but we're an EditablePanel, need to roll our own.)
//-----------------------------------------------------------------------------
void CRnLMapDialog::MoveToCenterOfScreen()
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);
	SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}
