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

#include "rnl_taskstatuspanel.h"
#include "rnl_mapdialog.h"
#include "rnl_task_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern bool AvatarIndexLessFunc( const int &lhs, const int &rhs );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLTaskStatusPanel::CRnLTaskStatusPanel( CRnLMapPanel *pMapParent, vgui::Panel *pParent, const char *pName ) : Panel(pParent,pName)
{
	m_pMapParent = pMapParent;

	m_iTaskStateTextures[RNL_TASK_TYPE_CAPTURE][0] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iTaskStateTextures[RNL_TASK_TYPE_CAPTURE][0], ( const char * )"VGUI/overview/task_axis_capturing", true, false);

	m_iTaskStateTextures[RNL_TASK_TYPE_CAPTURE][1] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iTaskStateTextures[RNL_TASK_TYPE_CAPTURE][1], ( const char * )"VGUI/overview/task_allies_capturing", true, false);

	m_iTaskStateTextures[RNL_TASK_TYPE_CAPTURE][2] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iTaskStateTextures[RNL_TASK_TYPE_CAPTURE][2], ( const char * )"VGUI/overview/task_axis_control", true, false);

	m_iTaskStateTextures[RNL_TASK_TYPE_CAPTURE][3] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iTaskStateTextures[RNL_TASK_TYPE_CAPTURE][3], ( const char * )"VGUI/overview/task_allies_control", true, false);

	m_iTaskStateTextures[RNL_TASK_TYPE_CAPTURE][4] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iTaskStateTextures[RNL_TASK_TYPE_CAPTURE][4], ( const char * )"VGUI/overview/task_grey", true, false);

	m_iTaskStateTextures[RNL_TASK_TYPE_CAPTURE][5] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iTaskStateTextures[RNL_TASK_TYPE_CAPTURE][5], ( const char * )"VGUI/overview/objective_arrow", true, false);

	m_iTaskStateTextures[RNL_TASK_TYPE_DESTROY][0] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iTaskStateTextures[RNL_TASK_TYPE_DESTROY][0], ICON_AXIS_NEUTRAL_DESTROY, true, false);

	m_iTaskStateTextures[RNL_TASK_TYPE_DESTROY][1] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iTaskStateTextures[RNL_TASK_TYPE_DESTROY][1], ICON_ALLIES_NEUTRAL_DESTROY, true, false);

	m_iTaskStateTextures[RNL_TASK_TYPE_DESTROY][2] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iTaskStateTextures[RNL_TASK_TYPE_DESTROY][2], ICON_AXIS_NEUTRAL_DESTROY, true, false);

	m_iTaskStateTextures[RNL_TASK_TYPE_DESTROY][3] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iTaskStateTextures[RNL_TASK_TYPE_DESTROY][3], ICON_ALLIES_NEUTRAL_DESTROY, true, false);

	m_iTaskStateTextures[RNL_TASK_TYPE_DESTROY][4] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iTaskStateTextures[RNL_TASK_TYPE_DESTROY][4], ICON_AXIS_NEUTRAL_DESTROY, true, false);

	m_iTaskStateTextures[RNL_TASK_TYPE_DESTROY][5] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iTaskStateTextures[RNL_TASK_TYPE_DESTROY][5], ICON_ALLIES_NEUTRAL_DESTROY, true, false);
}

CRnLTaskStatusPanel::~CRnLTaskStatusPanel()
{
}

void CRnLTaskStatusPanel::Paint( void )
{
	for( int i = 0; i < m_TaskInfo.Count(); i++ )
	{
		if( !m_TaskInfo[i].visible )
			continue;

		// ZONE COLOR
		switch( m_TaskInfo[i].taskStatus )
		{
		case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED:
		case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED_BLOCKED:
		case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL:
		case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL_BLOCKED:
			surface()->DrawSetTexture( m_iTaskStateTextures[m_TaskInfo[i].taskType][0] );
			break;

		case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS:
		case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS_BLOCKED:
		case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL:
		case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL_BLOCKED:
			surface()->DrawSetTexture( m_iTaskStateTextures[m_TaskInfo[i].taskType][1] );
			break;
		case RNL_TASK_AXIS_CONTROLLED:
			surface()->DrawSetTexture( m_iTaskStateTextures[m_TaskInfo[i].taskType][2] );
			break;
		case RNL_TASK_ALLIED_CONTROLLED:
			surface()->DrawSetTexture( m_iTaskStateTextures[m_TaskInfo[i].taskType][3] );
			break;
		default:
			surface()->DrawSetTexture( m_iTaskStateTextures[m_TaskInfo[i].taskType][4] );
			break;
		}

		int alpha = 160;
		switch( m_TaskInfo[i].taskStatus )
		{
		case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED_BLOCKED:
		case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL_BLOCKED:
		case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS_BLOCKED:
		case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL_BLOCKED:
			alpha = 80 + (70 * sin( gpGlobals->curtime * 4 ));
			break;
		}

		// POLY ZONE
		if( m_TaskInfo[i].mapPolies.Count() > 0 )
		{
			surface()->DrawSetColor( 0, 0, 0, alpha );

			for( int area = 0; area < m_TaskInfo[i].mapPolies.Count(); area++ )
			{
				if( m_TaskInfo[i].mapPolies[area].Count() > 0 )
					surface()->DrawTexturedPolygon( m_TaskInfo[i].mapPolies[area].Count(), m_TaskInfo[i].mapPolies[area].Base() );

				for( int j = 1; j < m_TaskInfo[i].mapPolies[area].Count(); j++ )
				{
					surface()->DrawLine( m_TaskInfo[i].mapPolies[area][j-1].m_Position.x,  m_TaskInfo[i].mapPolies[area][j-1].m_Position.y,
						m_TaskInfo[i].mapPolies[area][j].m_Position.x, m_TaskInfo[i].mapPolies[area][j].m_Position.y );
				}

				if( m_TaskInfo[i].mapPolies[area].Count() > 2 )
				{
					surface()->DrawLine( m_TaskInfo[i].mapPolies[area][m_TaskInfo[i].mapPolies[area].Count()-1].m_Position.x,  m_TaskInfo[i].mapPolies[area][m_TaskInfo[i].mapPolies[area].Count()-1].m_Position.y,
						m_TaskInfo[i].mapPolies[area][0].m_Position.x, m_TaskInfo[i].mapPolies[area][0].m_Position.y );
				}
			}
		}
		// RECT ZONE
		else
		{
			Vector2D panelPos1 = m_pMapParent->WorldToMapCoords( m_TaskInfo[i].taskExtentMin );
			Vector2D panelPos2 = m_pMapParent->WorldToMapCoords( m_TaskInfo[i].taskExtentMax );

			if( m_TaskInfo[i].taskType == RNL_TASK_TYPE_DESTROY )
			{
				Vector2D basePos = m_pMapParent->WorldToMapCoords( m_TaskInfo[i].taskOrigin );
				panelPos1.Init( basePos.x - XRES( 4 ), basePos.y - YRES( 4 ) );
				panelPos2.Init( basePos.x + XRES( 4 ), basePos.y + YRES( 4 ) );
			}

			if( panelPos1.x > panelPos2.x )
			{
				float temp = panelPos1.x;
				panelPos1.x = panelPos2.x;
				panelPos2.x = temp;
			}

			if( panelPos1.y > panelPos2.y )
			{
				float temp = panelPos1.y;
				panelPos1.y = panelPos2.y;
				panelPos2.y = temp;
			}

			surface()->DrawSetColor( 0, 0, 0, alpha );
			surface()->DrawTexturedRect( panelPos1.x, panelPos1.y, panelPos2.x, panelPos2.y );

			if( m_TaskInfo[i].taskType != RNL_TASK_TYPE_DESTROY )
			{
				surface()->DrawLine( panelPos1.x, panelPos1.y, panelPos1.x, panelPos2.y );
				surface()->DrawLine( panelPos1.x, panelPos2.y, panelPos2.x, panelPos2.y );
				surface()->DrawLine( panelPos2.x, panelPos2.y, panelPos2.x, panelPos1.y );
				surface()->DrawLine( panelPos1.x, panelPos1.y, panelPos2.x, panelPos1.y );
			}
		}
	}
}

void CRnLTaskStatusPanel::Update( void )
{
	for( int i = 0; i < m_TaskInfo.Count(); i++ )
	{
		m_TaskInfo[i].visible = false;

		CRnLTaskBase* pTask = FindClientTaskByName( m_TaskInfo[i].szTaskName );
		if( pTask )
		{
			m_TaskInfo[i].taskStatus = pTask->GetTaskState();
			m_TaskInfo[i].taskType = pTask->GetTaskType();
			m_TaskInfo[i].taskOrigin = pTask->GetTaskOrigin();
			pTask->GetTaskExtents( m_TaskInfo[i].taskExtentMin, m_TaskInfo[i].taskExtentMax );
			m_TaskInfo[i].visible = pTask->IsTaskVisibleOnMap();
		}

		for( int j = 0; j < m_TaskInfo[i].mapPolies.Count() && j < m_TaskInfo[i].worldPolies.Count(); j++ )
		{
			for( int k = 0; k < m_TaskInfo[i].mapPolies[j].Count() && k < m_TaskInfo[i].worldPolies[j].Count(); k++ )
			{
				Vector2D vec = m_pMapParent->WorldToMapCoords( m_TaskInfo[i].worldPolies[j][k] );
				m_TaskInfo[i].mapPolies[j][k].Init( vec );
			}
		}
	}
}

void CRnLTaskStatusPanel::Initialise( KeyValues* pMapInfo )
{
	m_TaskInfo.Purge();

	if( pMapInfo == NULL )
		return;

	KeyValues* pNodeInfo = pMapInfo->GetFirstSubKey();
	while( pNodeInfo )
	{
		int pos = m_TaskInfo.AddToTail();
		if( m_TaskInfo.IsValidIndex( pos ) )
		{
			Q_strncpy( m_TaskInfo[pos].szTaskName, pNodeInfo->GetName(), TASK_NAME_MAX );
			Q_strncpy( m_TaskInfo[pos].szCaption, pNodeInfo->GetString( "displayname", m_TaskInfo[pos].szTaskName ), TASK_NAME_MAX );
			m_TaskInfo[pos].textPos = pNodeInfo->GetInt( "textposition" );
			m_TaskInfo[pos].iconPos = pNodeInfo->GetInt( "iconposition" );

			KeyValues* pAreaInfo = pNodeInfo->GetFirstSubKey();
			while( pAreaInfo )
			{
				if( Q_stricmp( pAreaInfo->GetName(), "area" ) == 0 )
				{
					int polyPos = m_TaskInfo[pos].worldPolies.AddToTail();
					int mappolyPos = m_TaskInfo[pos].mapPolies.AddToTail();
					if( m_TaskInfo[pos].worldPolies.IsValidIndex( polyPos ) && m_TaskInfo[pos].mapPolies.IsValidIndex( mappolyPos ) )
					{
						Vector2D panelPos;
						Vector worldPos;
						KeyValues* vert = pAreaInfo->GetFirstSubKey();
						while( vert )
						{
							sscanf( vert->GetString(), "%f %f %f", &(worldPos.x), &(worldPos.y), &(worldPos.z) );
							m_TaskInfo[pos].worldPolies[polyPos].AddToTail( worldPos );
							m_TaskInfo[pos].mapPolies[mappolyPos].AddToTail();
							vert = vert->GetNextKey();
						}
					}
				}

				pAreaInfo = pAreaInfo->GetNextKey();
			}
		}
		pNodeInfo = pNodeInfo->GetNextKey();
	}
}