//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"

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

#include "rnl_objectivestatuspanel.h"
#include "rnl_mapdialog.h"
#include "rnl_gamerules.h"
#include "rnl_objective_base.h"

#include "c_rnl_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLObjectiveStatusPanel::CRnLObjectiveStatusPanel( bool primaryObjectives, CRnLMapPanel *pMapParent, vgui::Panel *pParent, const char *pName )
	: Panel(pParent,pName)
{
	m_PrimaryObjectives = primaryObjectives;

	m_pMapParent = pMapParent;

	m_iObjectiveStateTextures[0] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iObjectiveStateTextures[0], ICON_NEUTRAL_OBJECTIVE, true, false);

	m_iObjectiveStateTextures[1] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iObjectiveStateTextures[1], ICON_ALLIES_OBJECTIVE, true, false);

	m_iObjectiveStateTextures[2] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iObjectiveStateTextures[2], ICON_AXIS_OBJECTIVE, true, false);

	m_iObjectiveStateTextures[3] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iObjectiveStateTextures[3], ICON_ALLIES_ARROW, true, false);

	m_iObjectiveStateTextures[4] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iObjectiveStateTextures[4], ICON_AXIS_ARROW, true, false);

	m_iArrowTexture = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iArrowTexture, ARROW_TEXTURE, true, false);

	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( pParent->GetScheme() );
	m_hSelfFont = pScheme->GetFont( "QuicksandBold-Regular" );

	m_iArrowSize = 16;
}

CRnLObjectiveStatusPanel::~CRnLObjectiveStatusPanel()
{
}

void CRnLObjectiveStatusPanel::Paint( void )
{
	CRnLPlayer* pLocalPlayer = CRnLPlayer::GetLocalRnLPlayer();
	if( !pLocalPlayer )
		return;

	int iPrevStatus = pLocalPlayer->GetTeamNumber() > TEAM_SPECTATOR ? RNL_OBJECTIVE_NEUTRAL : pLocalPlayer->GetTeamNumber() - 1;

	for( int i = 0; i < m_ObjectiveInfo.Count(); i++ )
	{
		if( !m_ObjectiveInfo[i].visible )
			continue;

		if( m_PrimaryObjectives != m_ObjectiveInfo[i].primary )
			continue;

		surface()->DrawSetColor( 0, 0, 0, 200 );
		Vector2D panelPos = m_pMapParent->WorldToMapCoords( m_ObjectiveInfo[i].displayPos );

		// ARROW COLOR
		switch( iPrevStatus )
		{
		case RNL_OBJECTIVE_ALLIED_CONTROLLED:
			if( pLocalPlayer->GetTeamNumber() == TEAM_ALLIES )
				surface()->DrawSetTexture( m_iObjectiveStateTextures[3] );
			else
				surface()->DrawSetTexture( m_iObjectiveStateTextures[0] );
			break;
		case RNL_OBJECTIVE_AXIS_CONTROLLED:
			if( pLocalPlayer->GetTeamNumber() == TEAM_AXIS )
				surface()->DrawSetTexture( m_iObjectiveStateTextures[4] );
			else
				surface()->DrawSetTexture( m_iObjectiveStateTextures[0] );
			break;
		case RNL_OBJECTIVE_NEUTRAL:
		default:
			surface()->DrawSetTexture( m_iObjectiveStateTextures[0] );
			break;
		}

		iPrevStatus = m_ObjectiveInfo[i].objStatus;

		// Only draw lines to tasks if there is more than 1 task.
		if( m_ObjectiveInfo[i].taskPositions.Count() > 1 )
		{
			for( int j = 0; j < m_ObjectiveInfo[i].taskPositions.Count(); j++ )
			{
				Vector2D taskPos = m_pMapParent->WorldToMapCoords( m_ObjectiveInfo[i].taskPositions[j] );
				surface()->DrawLine( panelPos.x, panelPos.y, taskPos.x, taskPos.y );
			}
		}

		// Only draw arrows if the size is greater than 0
		if( m_iArrowSize > 0 )
		{
			for( int j = 0; j < m_ObjectiveInfo[i].requirementPositions.Count(); j++ )
			{
				Vector2D reqPos = m_pMapParent->WorldToMapCoords( m_ObjectiveInfo[i].requirementPositions[j] );
				m_pMapParent->DrawArrow( panelPos, reqPos, m_iArrowSize );
			}
		}

		// Only draw the icon and text if the mapper wants to...
		if( m_ObjectiveInfo[i].iconVisible )
		{
			// ZONE COLOR
			switch( m_ObjectiveInfo[i].objStatus )
			{
			case RNL_OBJECTIVE_ALLIED_CONTROLLED:
				surface()->DrawSetTexture( m_iObjectiveStateTextures[1] );
				break;
			case RNL_OBJECTIVE_AXIS_CONTROLLED:
				surface()->DrawSetTexture( m_iObjectiveStateTextures[2] );
				break;
			case RNL_OBJECTIVE_NEUTRAL:
			default:
				surface()->DrawSetTexture( m_iObjectiveStateTextures[0] );
				break;
			}
			
			Vector2D panelMin( panelPos.x - XRES( 8 ), panelPos.y - YRES( 8 ) );
			Vector2D panelMax( panelPos.x + XRES( 8 ), panelPos.y + YRES( 8 ) );

			surface()->DrawTexturedRect( panelMin.x, panelMin.y, panelMax.x, panelMax.y );
			surface()->DrawLine( panelMin.x, panelMin.y, panelMin.x, panelMax.y );
			surface()->DrawLine( panelMin.x, panelMax.y, panelMax.x, panelMax.y );
			surface()->DrawLine( panelMax.x, panelMax.y, panelMax.x, panelMin.y );
			surface()->DrawLine( panelMin.x, panelMin.y, panelMax.x, panelMin.y );

			wchar_t wcObjName[OBJECTIVE_CAPTION_MAX + 1];
			g_pVGuiLocalize->ConvertANSIToUnicode( V_strupr(m_ObjectiveInfo[i].szCaption), wcObjName, sizeof( wcObjName ) );

			int iTextWidth = 0, iTextHeight = 0, iXPos, iYPos;
			surface()->GetTextSize( m_hSelfFont, wcObjName, iTextWidth, iTextHeight );

			switch( m_ObjectiveInfo[i].textPos )
			{
				case 1: // Centered
					iXPos = panelPos.x - (iTextWidth / 2);
					iYPos = panelPos.y - (iTextHeight / 2);
					break;
				case 2: // Below
					iXPos = panelPos.x - (iTextWidth / 2);
					iYPos = panelMax.y;
					break;
				case 3: // Left
					iXPos = panelMin.x - iTextWidth - XRES(8);
					iYPos = panelPos.y - (iTextHeight / 2);
					break;
				case 4: // Right
					iXPos = panelMax.x + XRES(8);
					iYPos = panelPos.y - (iTextHeight / 2);
					break;
				default: // Above
					iXPos = panelPos.x - (iTextWidth / 2);
					iYPos = panelMin.y - iTextHeight;
					break;
			}

			surface()->SetFontGlyphSet(m_hSelfFont, "QuicksandBold-Regular", 16, 700, 0, 0, surface()->FONTFLAG_ANTIALIAS|surface()->FONTFLAG_CUSTOM);
			Color cTextColor = COLOR_BLACK;

			switch (m_ObjectiveInfo[i].objStatus)
			{
				case RNL_OBJECTIVE_ALLIED_CONTROLLED:
					cTextColor = COLOR_DARKGREEN;
					break;

				case RNL_OBJECTIVE_AXIS_CONTROLLED:
					cTextColor = COLOR_DARKRED;
					break;

				default:
					cTextColor = COLOR_BLACK;
			}

			surface()->DrawSetTextColor( cTextColor );
			surface()->DrawSetTextFont( m_hSelfFont );
			surface()->DrawSetTextPos( iXPos, iYPos );
			surface()->DrawUnicodeString( wcObjName );
		}
	}
}

void CRnLObjectiveStatusPanel::Update( void )
{
	if( !RnLGameRules() )
		return;

	CRnLPlayer* pLocalPlayer = CRnLPlayer::GetLocalRnLPlayer();
	if( !pLocalPlayer )
		return;

	for( int i = 0; i < m_ObjectiveInfo.Count(); i++ )
	{
		m_ObjectiveInfo[i].visible = false;

		CRnLObjectiveBase* pObj = (CRnLObjectiveBase*)(RnLGameRules()->GetObjective( m_ObjectiveInfo[i].szObjectiveName ));
		if( pObj )
		{
			m_ObjectiveInfo[i].primary = pObj->IsPrimary();
			pObj->SetDisplayPosition( m_ObjectiveInfo[i].displayPos );
			m_ObjectiveInfo[i].objStatus = pObj->GetObjectiveState();
			m_ObjectiveInfo[i].visible = pObj->IsObjectiveVisibleOnMap();
			m_ObjectiveInfo[i].iconVisible = pObj->IsObjectiveIconVisibleOnMap();

			m_ObjectiveInfo[i].taskPositions.Purge();
			IRnLTask* pTask = NULL;
			for( int j = 0; j < pObj->GetTaskCount(); j++ )
			{
				pTask = pObj->GetTask( j );
				if( pTask )
					m_ObjectiveInfo[i].taskPositions.AddToTail( pTask->GetTaskOrigin() );
			}

			m_ObjectiveInfo[i].requirementPositions.Purge();

			CRnLObjectiveOrderFilter* pFilter = pObj->GetOrderFilter(pLocalPlayer->GetTeamNumber() );
			if( pFilter )
			{
				for( int j = 0; j < pFilter->m_vNodes.Count(); j++ )
				{
					m_ObjectiveInfo[i].requirementPositions.AddToTail( pFilter->m_vNodes[j]->GetDisplayPosition() );
				}
			}
		}
	}
}

void CRnLObjectiveStatusPanel::Initialise( KeyValues* pMapInfo )
{
	m_ObjectiveInfo.Purge();

	if( pMapInfo == NULL )
		return;

	KeyValues* pNodeInfo = pMapInfo->GetFirstSubKey();
	while( pNodeInfo )
	{
		if( FStrEq(pNodeInfo->GetName(), "arrow") )
			sscanf( pNodeInfo->GetString("size", "16"), "%i", &m_iArrowSize );
		else
		{
			int pos = m_ObjectiveInfo.AddToTail();
			if( m_ObjectiveInfo.IsValidIndex( pos ) )
			{
				Q_strncpy( m_ObjectiveInfo[pos].szObjectiveName, pNodeInfo->GetName(), OBJECTIVE_NAME_MAX );
				Q_strncpy( m_ObjectiveInfo[pos].szCaption, pNodeInfo->GetString( "displayname", m_ObjectiveInfo[pos].szObjectiveName ), OBJECTIVE_CAPTION_MAX );
				sscanf( pNodeInfo->GetString("displayposition", "0.0 0.0 0.0"), "%f %f %f", &(m_ObjectiveInfo[pos].displayPos.x), &(m_ObjectiveInfo[pos].displayPos.y), &(m_ObjectiveInfo[pos].displayPos.z) );
				m_ObjectiveInfo[pos].textPos = pNodeInfo->GetInt( "textposition" );
			}
		}
		pNodeInfo = pNodeInfo->GetNextKey();
	}
}