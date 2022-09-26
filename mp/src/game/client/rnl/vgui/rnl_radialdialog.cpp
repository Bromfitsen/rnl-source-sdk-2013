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
#include "rnl_radialdialog.h"
#include <voice_status.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vstdlib/IKeyValuesSystem.h>

#include <game/client/iviewport.h>
#include <igameresources.h>
#include <filesystem.h>

#include "vgui_avatarimage.h"

#include "c_rnl_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLRadialDialog::CRnLRadialDialog(IViewPort *pViewPort) : Frame( NULL, PANEL_RADIAL, false )
{
	//memset(s_VoiceImage, 0x0, sizeof( s_VoiceImage ));
	m_pViewPort = pViewPort;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );
	SetProportional(true);
	SetDragEnabled( false );

	SetPaintBackgroundEnabled( false );

	LoadControlSettings("Resource/UI/rnl_radial.res");
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLRadialDialog::~CRnLRadialDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: clears everything in the scoreboard and all it's state
//-----------------------------------------------------------------------------
void CRnLRadialDialog::Reset()
{
	m_fNextUpdateTime = 0;
}


//-----------------------------------------------------------------------------
// Purpose: sets up screen
//-----------------------------------------------------------------------------
void CRnLRadialDialog::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	PostApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: Does dialog-specific customization after applying scheme settings.
//-----------------------------------------------------------------------------
void CRnLRadialDialog::PostApplySchemeSettings( vgui::IScheme *pScheme )
{
	// light up scoreboard a bit
	SetBgColor( Color( 0,0,0,0) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLRadialDialog::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( true );
		Update();
		SetVisible( true );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled( false );
	}
}

bool CRnLRadialDialog::NeedsUpdate( void )
{
	return (m_fNextUpdateTime < gpGlobals->curtime);	
}

//-----------------------------------------------------------------------------
// Purpose: Recalculate the internal scoreboard data
//-----------------------------------------------------------------------------
void CRnLRadialDialog::Update( void )
{	
	// Reset();
	//MoveToCenterOfScreen();

	// update every second
	m_fNextUpdateTime = gpGlobals->curtime + 1.0f; 
}
