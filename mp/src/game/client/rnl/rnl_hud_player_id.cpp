//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "c_rnl_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar hud_showplayerid( "hud_showplayerid", "1" );

//-----------------------------------------------------------------------------
// Purpose: Shows the player nametag
//-----------------------------------------------------------------------------
class CHudPlayerID : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudPlayerID, vgui::Panel );

public:
	CHudPlayerID( const char *pElementName );
	virtual void Init( void );

protected:
	virtual void Paint();

private:
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
	//CPanelAnimationVar( Color, m_TextColor, "TextColor", "FgColor" );
	Color m_TextColor;
};	

DECLARE_HUDELEMENT( CHudPlayerID );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudPlayerID::CHudPlayerID( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudPlayerID" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );
	m_TextColor = COLOR_WHITE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudPlayerID::Init()
{
}

//-----------------------------------------------------------------------------
// Purpose: draws the name and background icon
//-----------------------------------------------------------------------------
void CHudPlayerID::Paint()
{
	if( hud_showplayerid.GetInt() == 0 )
		return;

	C_RnLPlayer *pPlayer = C_RnLPlayer::GetLocalRnLPlayer();

	if ( !pPlayer )
		return;

	int iEntIndex = pPlayer->GetPlayerTargetID();

	if ( iEntIndex )
	{
		if ( IsPlayerIndex( iEntIndex ) )
		{
			C_RnLPlayer *pPlayer = ToRnLPlayer((cl_entitylist->GetEnt( iEntIndex )));

			if( pPlayer )
			{
				wchar_t wszPlayerName[ MAX_PLAYER_NAME_LENGTH ];

				g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(),  wszPlayerName, sizeof(wszPlayerName) );

				if ( wszPlayerName[0] )
				{
					int wide, tall;
					int ypos = YRES(260);
					int xpos = XRES(10);

					vgui::surface()->GetTextSize( m_hTextFont, wszPlayerName, wide, tall );

					if( hud_showplayerid.GetInt() == 1 )
					{
						ypos = YRES(420);
					}
					else
					{
						GetVectorInScreenSpace( pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset(), xpos, ypos, NULL );
						xpos -= (wide / 2);
						ypos -= tall;
					}

					vgui::surface()->DrawSetTextFont( m_hTextFont );
					vgui::surface()->DrawSetTextPos( xpos, ypos );
					vgui::surface()->DrawSetTextColor( m_TextColor );
					vgui::surface()->DrawPrintText( wszPlayerName, wcslen(wszPlayerName) );
				}
			}
		}
	}
}