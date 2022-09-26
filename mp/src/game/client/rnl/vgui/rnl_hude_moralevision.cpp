//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "c_rnl_player.h"
#include "vguimatsurface/IMatSystemSurface.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMesh.h"
#include "materialsystem/imaterialvar.h"
#include "ScreenSpaceEffects.h"

#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/AnimationController.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Draws the zoom screen
//-----------------------------------------------------------------------------
class CHudMoraleVision : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudMoraleVision, vgui::Panel );

public:
			CHudMoraleVision( const char *pElementName );
	
	void	Init( void );
	void	LevelInit( void );
	bool	ShouldDraw( void );

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void Paint( void );

private:

	CMaterialReference m_ZoomMaterial;

};

DECLARE_HUDELEMENT( CHudMoraleVision );

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudMoraleVision::CHudMoraleVision( const char *pElementName ) : CHudElement(pElementName), BaseClass(NULL, "HudMoraleVision")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	
	SetHiddenBits( HIDEHUD_PLAYERDEAD );
}

//-----------------------------------------------------------------------------
// Purpose: standard hud element init function
//-----------------------------------------------------------------------------
void CHudMoraleVision::Init( void )
{
	m_ZoomMaterial.Init( "vgui/zoom", TEXTURE_GROUP_VGUI );
}

//-----------------------------------------------------------------------------
// Purpose: standard hud element init function
//-----------------------------------------------------------------------------
void CHudMoraleVision::LevelInit( void )
{
	Init();
}

bool CHudMoraleVision::ShouldDraw( void )
{
	if( C_BasePlayer::GetLocalPlayer() == NULL )
		return false;

	// see if we're zoomed any
	C_RnLPlayer *pPlayer = ToRnLPlayer(C_BasePlayer::GetLocalPlayer());
	
	if ( pPlayer == NULL )
		return false;
	if( !pPlayer->IsAlive() || pPlayer->IsObserver() )
		return false;
	//if ( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS || pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS  )
	//	return true;
	if ( pPlayer->GetMoraleLevel() < 50 )
		return true;

	return ( CHudElement::ShouldDraw() );
}

//-----------------------------------------------------------------------------
// Purpose: sets scheme colors
//-----------------------------------------------------------------------------
void CHudMoraleVision::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);
	SetFgColor(scheme->GetColor("ZoomReticleColor", GetFgColor()));

	int screenWide, screenTall;
	surface()->GetScreenSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);
}

//-----------------------------------------------------------------------------
// Purpose: draws the zoom effect
//-----------------------------------------------------------------------------
void CHudMoraleVision::Paint( void )
{
	// see if we're zoomed any
	C_RnLPlayer *pPlayer = ToRnLPlayer(C_BasePlayer::GetLocalPlayer());
	
	if ( pPlayer == NULL )
		return;

	// draw the appropriately scaled zoom animation
	float morale = 1.0f - (pPlayer->GetMoraleLevel() / 75.0f);
	float scale = clamp( morale, 0.0f, 1.0f );

	if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_IRONSIGHTS )
	{
		scale = clamp( scale + 0.35, 0.0f, 1.0f );
	}
	else if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
	{
		scale = clamp( scale + 0.60, 0.0f, 1.0f );
	}

	float alpha = scale;

	Color col = GetFgColor();
	col[3] = alpha * 64;

	surface()->DrawSetColor( col );

	// draw the darkened edges, with a rotated texture in the four corners
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( m_ZoomMaterial );
	IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, NULL );

	int wide, tall;
	GetSize(wide, tall);

	float x0 = 0.0f, x1 = wide / 2.0f, x2 = wide;
	float y0 = 0.0f, y1 = tall / 2.0f, y2 = tall;

	float uv1 = 1.0f - (1.0f / 255.0f);
	float uv2 = 0.0f + (1.0f / 255.0f);

	struct coord_t
	{
		float x, y;
		float u, v;
	};
	coord_t coords[16] = 
	{
		// top-left
		{ x0, y0, uv1, uv2 },
		{ x1, y0, uv2, uv2 },
		{ x1, y1, uv2, uv1 },
		{ x0, y1, uv1, uv1 },

		// top-right
		{ x1, y0, uv2, uv2 },
		{ x2, y0, uv1, uv2 },
		{ x2, y1, uv1, uv1 },
		{ x1, y1, uv2, uv1 },

		// bottom-right
		{ x1, y1, uv2, uv1 },
		{ x2, y1, uv1, uv1 },
		{ x2, y2, uv1, uv2 },
		{ x1, y2, uv2, uv2 },

		// bottom-left
		{ x0, y1, uv1, uv1 },
		{ x1, y1, uv2, uv1 },
		{ x1, y2, uv2, uv2 },
		{ x0, y2, uv1, uv2 },
	};

	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 4 );

	for (int i = 0; i < 16; i++)
	{
		meshBuilder.Color4f( 0.0, 0.0, 0.0, alpha );
		meshBuilder.TexCoord2f( 0, coords[i].u, coords[i].v );
		meshBuilder.Position3f( coords[i].x, coords[i].y, 0.0f );
		meshBuilder.AdvanceVertex();
	}

	meshBuilder.End();
	pMesh->Draw();

	if( pPlayer->GetMoraleLevel() < 25 )
	{
		// Create a keyvalue block to set these params
		KeyValues *pKeys = new KeyValues( "keys" );
		if ( pKeys == NULL )
			return;

		float flMorale = morale / 25.0f;

		if( pPlayer->GetWeaponPosture() == WEAPON_POSTURE_SUPERSIGHTS )
			flMorale *= 2.0f;

		// Set our keys
		pKeys->SetFloat( "bluriness", 0.045f );
		pKeys->SetFloat( "duration", 0.04f );
		pKeys->SetFloat( "scale", flMorale );
		pKeys->SetFloat( "speed", 1.0f );

		g_pScreenSpaceEffects->SetScreenSpaceEffectParams( "rnl_visionblur", pKeys );
		g_pScreenSpaceEffects->EnableScreenSpaceEffect( "rnl_visionblur" );
	}
}