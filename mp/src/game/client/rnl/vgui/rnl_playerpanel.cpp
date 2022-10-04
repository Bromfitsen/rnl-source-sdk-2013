//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"

#include <cdll_client_int.h>
#include <cdll_util.h>
#include <globalvars_base.h>
#include <igameresources.h>
#include "c_playerresource.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>

#include <game/client/iviewport.h>
#include <igameresources.h>

#include "rnl_playerpanel.h"
#include "rnl_mapdialog.h"

#include "c_rnl_player.h"

#include "c_rnl_game_team.h"
#include "rnl_squad.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

///////////////////
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CRnLPlayerPlayerPanel::CRnLPlayerPlayerPanel( CRnLMapPanel *pMapParent, vgui::Panel *pParent, const char *pName ) : 
	BaseClass( pParent, pName )
{
	SetPaintBackgroundEnabled( false );
	m_pNCOParent = pMapParent;

	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( pParent->GetScheme() );
	
	m_hSelfFont = pScheme->GetFont( "DefaultSmall" );

	m_iPlayerIcon = vgui::surface()->CreateNewTextureID();
	vgui::surface()->DrawSetTextureFile( m_iPlayerIcon , ( const char * )"VGUI/overview/player_blue", true, false);
}

void CRnLPlayerPlayerPanel::Paint( void )
{
	C_RnLPlayer *pPlayer = C_RnLPlayer::GetLocalRnLPlayer();
	if( !pPlayer || !m_pNCOParent )
		return;

	C_PlayerResource *pGR = dynamic_cast<C_PlayerResource *>(GameResources());

	if ( !pGR )
		return;

	// PLAYER LOCATOR
	Vector2D panelPos =  m_pNCOParent->WorldToMapCoords( pPlayer->GetAbsOrigin() );

	if( pPlayer->IsAlive() )
	{
		DrawPlayer( panelPos, pPlayer->EyeAngles()[YAW], 24, pPlayer->GetTeamNumber() );

		vgui::surface()->DrawSetTextColor(0, 0, 0, 255);
		vgui::surface()->DrawSetTextPos( panelPos.x - 12, panelPos.y - 36);
		vgui::surface()->DrawSetTextFont( m_hSelfFont );
		vgui::surface()->DrawUnicodeString( L"You" );
	}

	C_RnLGameTeam* pTeam = (C_RnLGameTeam*)GetGlobalRnLTeam( pPlayer->GetTeamNumber() );
	if( !pTeam )
		return;

	CRnLSquad* pSquad = pTeam->GetSquad( pPlayer->GetSquadNumber() );
	if( !pSquad )
		return;

	CRnLPlayer* pOtherPlayer = pSquad->GetSquadLeader();
	if( pOtherPlayer != NULL )
	{
		if( pOtherPlayer == pPlayer )
		{
			for( int i = 0; i < pSquad->GetTotalAvailableKits(); i++ )
			{
				for( int j = 0; j < pSquad->GetMemberCount( i ); j++ )
				{
					pOtherPlayer = pSquad->GetMember( i, j );
					if( pOtherPlayer != NULL && pOtherPlayer != pPlayer )
					{
						if( pOtherPlayer->IsAlive() )
						{
							panelPos =  m_pNCOParent->WorldToMapCoords( pOtherPlayer->GetAbsOrigin() );
							DrawPlayer( panelPos, pOtherPlayer->GetAbsAngles()[YAW], 16, pOtherPlayer->GetTeamNumber());

							wchar_t wcPlayerName[MAX_PLAYER_NAME_LENGTH + 1];
							g_pVGuiLocalize->ConvertANSIToUnicode( pOtherPlayer->GetPlayerName(), wcPlayerName, sizeof( wcPlayerName ) );

							vgui::surface()->DrawSetTextColor(0, 0, 0, 255);
							vgui::surface()->DrawSetTextPos( panelPos.x - 12, panelPos.y - 36);
							vgui::surface()->DrawSetTextFont( m_hSelfFont );
							vgui::surface()->DrawUnicodeString( wcPlayerName );
						}
					}
				}
			}
		}
		else
		{
			if( pOtherPlayer->IsAlive() )
			{
				panelPos =  m_pNCOParent->WorldToMapCoords( pOtherPlayer->GetAbsOrigin() );
				DrawPlayer( panelPos, pOtherPlayer->GetAbsAngles()[YAW], 16, pOtherPlayer->GetTeamNumber() );

				vgui::surface()->DrawSetTextColor(0, 0, 0, 255);
				vgui::surface()->DrawSetTextPos( panelPos.x - 12, panelPos.y - 36);
				vgui::surface()->DrawSetTextFont( m_hSelfFont );
				vgui::surface()->DrawUnicodeString( L"NCO" );
			}
		}
	}
}

void CRnLPlayerPlayerPanel::DrawPlayer( Vector2D panelPos, float angle, float size, int iTeam )
{
	Vector offset;
	offset.z = 0;

	angle = -angle;
	angle -= m_pNCOParent->GetMapAngle( iTeam - 2 );

	offset.x = -size;	offset.y = -size;
	VectorYawRotate( offset, angle, offset );
	Vector2D pos1( panelPos.x + offset.x, panelPos.y + offset.y );

	offset.x = size;	offset.y = -size;
	VectorYawRotate( offset, angle, offset );
	Vector2D pos2( panelPos.x + offset.x, panelPos.y + offset.y );

	offset.x = size;	offset.y = size;
	VectorYawRotate( offset, angle, offset );
	Vector2D pos3( panelPos.x + offset.x, panelPos.y + offset.y );

	offset.x = -size;	offset.y = size;
	VectorYawRotate( offset, angle, offset );
	Vector2D pos4( panelPos.x + offset.x, panelPos.y + offset.y );

	vgui::Vertex_t points[4] =
	{
		vgui::Vertex_t( pos1, Vector2D(0,0) ),
		vgui::Vertex_t( pos2, Vector2D(1,0) ),
		vgui::Vertex_t( pos3, Vector2D(1,1) ),
		vgui::Vertex_t( pos4, Vector2D(0,1) )
	};

	vgui::surface()->DrawSetTexture( m_iPlayerIcon );
	vgui::surface()->DrawTexturedPolygon( 4, points );
}