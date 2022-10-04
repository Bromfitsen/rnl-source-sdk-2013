//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/isurface.h>
#include "rnl_ammodef.h"
#include "rnl_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudAmmo : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudAmmo, vgui::Panel );

public:
	CHudAmmo( const char *pElementName );
	void Init( void );
	void VidInit( void );
	void LevelInit( void );
	void ApplySchemeSettings( vgui::IScheme *scheme );
	void Reset();
	bool ShouldDraw();	
	void Paint();

private:
	CHudTexture *m_pAmmoIcons[MAX_AMMO_TYPES][2];
	CHudTexture *m_pBeltTextures[2];

	CPanelAnimationVarAliasType( float, primary_ypos, "primary_ypos", "416", "proportional_float" );
	CPanelAnimationVarAliasType( float, primary_xpos, "primary_xpos", "40", "proportional_float" );

	CPanelAnimationVarAliasType( float, secondary_ypos, "secondary_ypos", "416", "proportional_float" );
	CPanelAnimationVarAliasType( float, secondary_xpos, "secondary_xpos", "440", "proportional_float" );

	CPanelAnimationVarAliasType( float, grenade_ypos, "grenade_ypos", "416", "proportional_float" );
	CPanelAnimationVarAliasType( float, grenade_xpos, "grenade_xpos", "270", "proportional_float" );

	Color	m_clrIcon;
};

extern bool s_showNCOIndicator;
static bool s_showAmmoIndicator = false;

void ShowAmmoIndicator( void )
{
	/*s_showNCOIndicator = */s_showAmmoIndicator = true;
}

void HideAmmoIndicator( void )
{
	/*s_showNCOIndicator = */s_showAmmoIndicator = false;
}

static ConCommand hud_showammoindicator( "+ammobelt", ::ShowAmmoIndicator );
static ConCommand hud_hideammoindicator( "-ammobelt", ::HideAmmoIndicator );

DECLARE_HUDELEMENT( CHudAmmo );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAmmo::CHudAmmo( const char *pElementName ) : BaseClass(NULL, "HudAmmo"), CHudElement( pElementName )
{
	SetParent( g_pClientMode->GetViewport() );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );
	m_clrIcon = Color(255,255,255,255);

	for( int i = 0; i < MAX_AMMO_TYPES; i++ )
	{
		m_pAmmoIcons[i][0] = NULL;
		m_pAmmoIcons[i][1] = NULL;
	}

	m_pBeltTextures[0] = NULL;
	m_pBeltTextures[1] = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmo::Init( void )
{
	s_showAmmoIndicator = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmo::VidInit( void )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmo::LevelInit( void )
{
	CHudElement::LevelInit();

	// Reset on level init
	s_showAmmoIndicator = false;
}

void CHudAmmo::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	int iScreenWide, iScreenTall;
	vgui::surface()->GetScreenSize(iScreenWide, iScreenTall);
	SetBounds(0, 0, iScreenWide, iScreenTall);

	char szTemp[64];
	for( int i = 0; i < MAX_AMMO_TYPES; i++ )
	{
		Ammo_t* pAmmo = GetAmmoDef()->GetAmmoOfIndex( i );
		if( pAmmo )
		{
			sprintf( szTemp, "%s_full", pAmmo->pName );
			m_pAmmoIcons[i][0] = gHUD.GetIcon( szTemp );
			
			sprintf( szTemp, "%s_part", pAmmo->pName );
			m_pAmmoIcons[i][1] = gHUD.GetIcon( szTemp );
		}
		else
		{
			m_pAmmoIcons[i][0] = NULL;
			m_pAmmoIcons[i][1] = NULL;
		}
	}

	m_pBeltTextures[0] = gHUD.GetIcon( "american_belt" );
	m_pBeltTextures[1] = gHUD.GetIcon( "german_belt" );
}

//-----------------------------------------------------------------------------
// Purpose: Resets hud after save/restore
//-----------------------------------------------------------------------------
void CHudAmmo::Reset()
{
	CHudElement::Reset();
	s_showAmmoIndicator = false;
}

//-----------------------------------------------------------------------------
// See if the player is holding down the ammo key
//-----------------------------------------------------------------------------
bool CHudAmmo::ShouldDraw()
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if( pPlayer && pPlayer->IsAlive() )
		return s_showAmmoIndicator;
	else
		return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmo::Paint()
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	CRnLAmmoDef* AmmoDef = GetRnLAmmoDef();
	if( !pPlayer || !AmmoDef)
		return;

	if( pPlayer->GetTeamNumber() != TEAM_ALLIES && pPlayer->GetTeamNumber() != TEAM_AXIS )
		return;

	int iBeltXPos = XRES( 40 );
	int iBeltYPos = YRES( 450 );

	int iBeltWidth = XRES( 560 );
	int iBeltHeight = YRES( 30 );

	m_pBeltTextures[pPlayer->GetTeamNumber() - TEAM_ALLIES]->DrawSelf( iBeltXPos, iBeltYPos, iBeltWidth, iBeltHeight, m_clrIcon );

	int iIconSizeX = XRES( 15 );
	int iIconSpacingX = XRES( 20 );
	int iIconSizeY = YRES( 20 );

	int iAmmo, iClipSize, iGrenadeCount = 0, iPrimaryCount = 0, iSecondaryCount = 0;

	for( int i = 0; i < MAX_AMMO_TYPES; i++ )
	{
		iAmmo = pPlayer->GetAmmoCount( i );

		if( iAmmo > 0 )
		{
			Ammo_t* pAmmo = AmmoDef->GetAmmoOfIndex( i );
			if( pAmmo )
			{
				iClipSize = AmmoDef->GetMagazineSize( i );
				if( pAmmo->nFlags & AMMO_RNL_PRIMARY_AMMO )
				{
					int iLim = (int)(iAmmo / iClipSize );
					for( int j = 0; j < iLim; j++ )
					{
						if( m_pAmmoIcons[i][0] )
							m_pAmmoIcons[i][0]->DrawSelf( primary_xpos + (iIconSpacingX * iPrimaryCount), primary_ypos, m_clrIcon );
						else
						{
							vgui::surface()->DrawSetColor( 100, 0, 0, 255 );
							vgui::surface()->DrawFilledRect( primary_xpos + (iIconSpacingX * iPrimaryCount), primary_ypos, primary_xpos + (iIconSpacingX * iPrimaryCount) + iIconSizeX, primary_ypos + iIconSizeY );
						}
						iPrimaryCount++;
					}

					if( (iAmmo % iClipSize) > 0 )
					{
						if( m_pAmmoIcons[i][1] )
							m_pAmmoIcons[i][1]->DrawSelf( primary_xpos + (iIconSpacingX * iPrimaryCount), primary_ypos + (iIconSizeY / 2), m_clrIcon );
						else
						{
							vgui::surface()->DrawSetColor( 100, 0, 0, 200 );
							vgui::surface()->DrawFilledRect( primary_xpos + (iIconSpacingX * iPrimaryCount), primary_ypos + (iIconSizeY / 2), primary_xpos + (iIconSpacingX * iPrimaryCount) + iIconSizeX, primary_ypos + iIconSizeY );
						}
						iPrimaryCount++;
					}
				}
				else if( pAmmo->nFlags & AMMO_RNL_SECONDARY_AMMO )
				{
					int iLim = (int)(iAmmo / iClipSize );
					for( int j = 0; j < iLim; j++ )
					{
						if( m_pAmmoIcons[i][0] )
							m_pAmmoIcons[i][0]->DrawSelf( secondary_xpos + (iIconSpacingX * iSecondaryCount), secondary_ypos, m_clrIcon );
						else
						{
							vgui::surface()->DrawSetColor( 0, 200, 0, 255 );
							vgui::surface()->DrawFilledRect( secondary_xpos + (iIconSpacingX * iSecondaryCount), secondary_ypos, secondary_xpos + (iIconSpacingX * iSecondaryCount) + iIconSizeX, secondary_ypos + iIconSizeY );
						}
						iSecondaryCount++;
					}

					if( (iAmmo % iClipSize) > 0 )
					{
						if( m_pAmmoIcons[i][1] )
							m_pAmmoIcons[i][1]->DrawSelf( secondary_xpos + (iIconSpacingX * iSecondaryCount), secondary_ypos + (iIconSizeY / 2), m_clrIcon );
						else
						{
							vgui::surface()->DrawSetColor( 0, 200, 0, 200 );
							vgui::surface()->DrawFilledRect( secondary_xpos + (iIconSpacingX * iSecondaryCount), secondary_ypos + (iIconSizeY / 2), secondary_xpos + (iIconSpacingX * iSecondaryCount) + iIconSizeX, secondary_ypos + iIconSizeY );
						}
						iSecondaryCount++;
					}
				}
				else if( pAmmo->nFlags & AMMO_RNL_GRENADE_AMMO )
				{
					int iLim = (int)(iAmmo / iClipSize );
					for( int j = 0; j < iLim; j++ )
					{
						if( m_pAmmoIcons[i][0] )
							m_pAmmoIcons[i][0]->DrawSelf( grenade_xpos + (iIconSpacingX * iGrenadeCount), grenade_ypos, m_clrIcon );
						else
						{
							vgui::surface()->DrawSetColor( 0, 0, 0, 255 );
							vgui::surface()->DrawFilledRect( grenade_xpos + (iIconSpacingX * iGrenadeCount), grenade_ypos, grenade_xpos + (iIconSpacingX * iGrenadeCount) + iIconSizeX, grenade_ypos + iIconSizeY );
						}
						iGrenadeCount++;

						if( iGrenadeCount == 2 )
							iGrenadeCount++;
					}

					if( (iAmmo % iClipSize) > 0 )
					{
						if( m_pAmmoIcons[i][1] )
							m_pAmmoIcons[i][1]->DrawSelf( grenade_xpos + (iIconSpacingX * iGrenadeCount), grenade_ypos + (iIconSizeY / 2), m_clrIcon );
						else
						{
							vgui::surface()->DrawSetColor( 0, 0, 0, 200 );
							vgui::surface()->DrawFilledRect( grenade_xpos + (iIconSpacingX * iGrenadeCount), grenade_ypos + (iIconSizeY / 2), grenade_xpos + (iIconSpacingX * iGrenadeCount) + iIconSizeX, grenade_ypos + iIconSizeY );
						}
						iGrenadeCount++;

						if( iGrenadeCount == 2 )
							iGrenadeCount++;
					}
				}
			}
		}
	}
}