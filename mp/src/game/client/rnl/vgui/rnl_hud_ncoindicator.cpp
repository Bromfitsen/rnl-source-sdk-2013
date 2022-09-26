//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "hudelement.h"
#include <vgui_controls/Panel.h>
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include "c_playerresource.h"
#include "view_scene.h"

#include "c_rnl_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define NCO_DIST_MAX 2048
#define NCO_DIST_MIN 15

#define ICON_SIZE_MIN 32
#define ICON_SIZE_MAX 48

static bool s_showNCOIndicator = false;

//-----------------------------------------------------------------------------
// Purpose: Shows the flashlight icon
//-----------------------------------------------------------------------------
class CHudNCOIndicator : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudNCOIndicator, vgui::Panel );

public:
	CHudNCOIndicator( const char *pElementName );
	virtual void Init( void );
	// LevelInit's called whenever a new level's starting
	virtual void LevelInit( void );

protected:
	virtual void Paint();

	int m_iAlliesNCOIndicatorIcon;
	int m_iAxisNCOIndicatorIcon;

private:
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
	CPanelAnimationVar( Color, m_TextColor, "TextColor", "FgColor" );
};	

DECLARE_HUDELEMENT( CHudNCOIndicator );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudNCOIndicator::CHudNCOIndicator( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudNCOIndicator" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetVisible( false );
}

//-----------------------------------------------------------------------------
// Purpose: Initialise required textures
//-----------------------------------------------------------------------------
void CHudNCOIndicator::Init()
{
	// Initalise allies indicator
	m_iAlliesNCOIndicatorIcon  = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iAlliesNCOIndicatorIcon, ( const char * )"VGUI/indicators/nco_allies", true, false);

	// Initialise axis indicator
	m_iAxisNCOIndicatorIcon  = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iAxisNCOIndicatorIcon, ( const char * )"VGUI/indicators/nco_axis", true, false);
}

void CHudNCOIndicator::LevelInit( void )
{
	CHudElement::LevelInit();
	SetVisible( false );

	// Reset on level init
	s_showNCOIndicator = false;
}

bool GetVectorInScreenSpace( Vector pos, int& iX, int& iY, Vector *vecOffset );

//-----------------------------------------------------------------------------
// Purpose: draws the name and background icon
//-----------------------------------------------------------------------------
void CHudNCOIndicator::Paint()
{
	// Check the player has the show nco key held down, otherwise exit early
	if( s_showNCOIndicator == false )
		return;

	C_RnLPlayer *pPlayer = C_RnLPlayer::GetLocalRnLPlayer();

	if ( !pPlayer )
		return;

	if( pPlayer->GetClassNumber() == ALLIES_NCO || pPlayer->GetClassNumber() == AXIS_OFFIZIER )
		return;

	bool squadHasNCO = false;

	// Walk all players
	for ( int i = 1; i < gpGlobals->maxClients; i++ )
	{
		// Get the player resource
		C_PlayerResource *pPR = dynamic_cast<C_PlayerResource*>( GameResources() );

		// Check the player is connected
		if ( pPR && pPR->IsConnected( i ) )
		{
			// Check the player is on our team..
			if(	pPR->GetTeam( i ) != pPR->GetTeam( pPlayer->entindex() ) )
				continue;

			// In our squad..
			if( pPR->GetSquad( i ) != pPR->GetSquad( pPlayer->entindex() ) )
				continue;

			// And of the correct class
			if( ( pPR->GetClass( i ) != ALLIES_NCO ) && ( pPR->GetClass( i ) != AXIS_OFFIZIER ) )
				continue;

			// Found the NCO, so don't draw the text
			squadHasNCO = true;

			C_RnLPlayer *pYourNCO = ToRnLPlayer( (cl_entitylist->GetEnt( i ) ) );

			if( pYourNCO == pPlayer || !pYourNCO->IsAlive() )
				continue;

			int helmetAttachmentIndex = pYourNCO->LookupAttachment( "Helmet" );
			Vector helmetOrigin; QAngle helmetAngles;

			if( pYourNCO->GetAttachmentLocal( helmetAttachmentIndex, helmetOrigin, helmetAngles ) == false )
			{
				DevWarning("NCOIndicator: NCO is missing his helmet attachment!\n");
				continue;
			}

			int x = -1, y = -1;
			Vector screen;

			int behind = ScreenTransform( pYourNCO->GetAbsOrigin() + helmetOrigin + Vector( 0, 0, 32 ), screen );
			if( behind )
				continue;

			x =  0.5 * screen[0] * ScreenWidth();
			y = -0.5 * screen[1] * ScreenHeight();
			x += 0.5 * ScreenWidth();
			y += 0.5 * ScreenHeight();

			x = clamp( x, 0, ScreenWidth() );
			y = clamp( y, 0, ScreenHeight() );

			int iconWidth = ICON_SIZE_MAX;
			int iconHeight = ICON_SIZE_MAX;

			float distance = pPR->GetAbsOrigin().DistTo( pPlayer->GetAbsOrigin() );
			float distanceMod = distance / 1024;

			float sizeMod = ( ICON_SIZE_MAX - ICON_SIZE_MIN ) * distanceMod;

			iconHeight -= static_cast<int>(sizeMod);
			iconWidth -= static_cast<int>(sizeMod);

			if( iconHeight < ICON_SIZE_MIN )
				iconHeight = iconWidth = ICON_SIZE_MIN;

			int x0 = x - (int)(iconWidth / 2);
			if( x0 < 0 )
				x0 = 0;

			int y0 = y  - (int)(iconHeight / 2);
			if( y0 < 0 )
				y0 = 0;

			int x1 = x0 + iconWidth;
			int y1 = y0 + iconHeight;

			if( y1 > ScreenHeight() )
			{
				int diff = y1 - ScreenHeight();
				y1 -= diff;
				y0 -= diff;
			}

			if( x1 > ScreenWidth() )
			{
				int diff = x1 - ScreenWidth();
				x0 -= diff;
				x1 -= diff;
			}


			surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );

			if( pPR->GetTeam( i ) == TEAM_ALLIES )
			{
				surface()->DrawSetTexture( m_iAlliesNCOIndicatorIcon );
			}
			else
			{
				surface()->DrawSetTexture( m_iAxisNCOIndicatorIcon );
			}

			surface()->DrawTexturedRect( x0, y0, x1, y1 );
		}
	}

	if( squadHasNCO == false )
	{
		char szNoNCODisplay[128];
		wchar_t wszNoNCODisplay[128];

		const char *key = engine->Key_LookupBinding( "changesquad" );
		if ( !key )
		{
			key = "< not bound >";
		}


		V_snprintf( szNoNCODisplay, sizeof(szNoNCODisplay), "No NCO available, press %s to change class", key );
		vgui::localize()->ConvertANSIToUnicode( szNoNCODisplay, wszNoNCODisplay, sizeof(wszNoNCODisplay) );
		
		int wide, tall;

		vgui::surface()->GetTextSize( m_hTextFont, wszNoNCODisplay, wide, tall );
		vgui::surface()->DrawSetTextFont( m_hTextFont );
		vgui::surface()->DrawSetTextPos( ScreenWidth() - wide - XRES(10), tall / 2 );
		vgui::surface()->DrawSetTextColor( m_TextColor );
		vgui::surface()->DrawPrintText( wszNoNCODisplay, wcslen(wszNoNCODisplay) );
	}
}

void ShowNCOIndicator( void )
{
	CHudNCOIndicator *hudNCOIndicator = GET_HUDELEMENT( CHudNCOIndicator );
	if ( hudNCOIndicator )
	{
		s_showNCOIndicator = true;
	}
}

void HideNCOIndicator( void )
{
	CHudNCOIndicator *hudNCOIndicator = GET_HUDELEMENT( CHudNCOIndicator );
	if ( hudNCOIndicator )
	{
		s_showNCOIndicator = false;
	}
}

static ConCommand hud_showncoindicator( "+shownco", ::ShowNCOIndicator );
static ConCommand hud_hidencoindicator( "-shownco", ::HideNCOIndicator );