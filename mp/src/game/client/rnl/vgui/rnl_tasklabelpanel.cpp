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

#include "rnl_tasklabelpanel.h"
#include "rnl_taskstatuspanel.h"
#include "rnl_mapdialog.h"
#include "rnl_task_base.h"
#include "utlvector.h"
#include "igameresources.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLTaskLabelPanel::CRnLTaskLabelPanel( CRnLTaskStatusPanel *pStatusPanelParent, vgui::Panel *pParent, const char *pName ) : Panel(pParent,pName)
{
	m_pStatusPanelParent = pStatusPanelParent;

	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( pParent->GetScheme() );
	m_hSelfFont = pScheme->GetFont( "QuicksandBold-Regular" );
}

CRnLTaskLabelPanel::~CRnLTaskLabelPanel()
{
}

void CRnLTaskLabelPanel::Paint( void )
{
	CUtlVector<RnLTaskStatusInfo> &taskList = m_pStatusPanelParent->GetTaskInfo();

	for( int i = 0; i < taskList.Count(); i++ )
	{
		if( !taskList[i].visible )
			continue;

		Vector2D panelPos = m_pStatusPanelParent->GetMapParent()->WorldToMapCoords( taskList[i].taskOrigin );
		Vector2D panelMin( panelPos.x - XRES( 8 ), panelPos.y - YRES( 4 ) );
		Vector2D panelMax( panelPos.x + XRES( 8 ), panelPos.y + YRES( 4 ) );

		// Handle the task names, including the capture ٪.
		// Note: A special Unicode ٪ had to be used because, for some reason,
		//		 whenever *char was converted to wchar_t, all % were stripped.
		wchar_t wcTaskTitle[OBJECTIVE_CAPTION_MAX + 1];
		g_pVGuiLocalize->ConvertANSIToUnicode( V_strupr(taskList[i].szCaption), wcTaskTitle, sizeof( wcTaskTitle ) );

		if( taskList[i].taskType != RNL_TASK_TYPE_DESTROY )
		{
			C_RnLTaskBase *pTask = FindClientTaskByName(taskList[i].szTaskName);

			if( pTask && pTask->GetCapturePercent() > 0 && pTask->GetCapturePercent() < 100)
			{
				wchar_t wcTaskName[OBJECTIVE_CAPTION_MAX + 1];
				Q_wcsncpy(wcTaskName, wcTaskTitle, sizeof( wcTaskName ));

				_swprintf( wcTaskTitle, L"%s - %i٪", wcTaskName, pTask->GetCapturePercent());
			}
		}

		int iTextWidth = 0, iTextHeight = 0, iXPos, iYPos;
		surface()->GetTextSize( m_hSelfFont, wcTaskTitle, iTextWidth, iTextHeight );

		switch( taskList[i].textPos )
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

		surface()->SetFontGlyphSet(m_hSelfFont, "QuicksandBold-Regular", 16, 700, 0, 0, surface()->FONTFLAG_ANTIALIAS);
		
		Color cTextColor = COLOR_BLACK;

		switch (taskList[i].taskStatus)
		{
			case RNL_TASK_ALLIED_CONTROLLED:
			case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED:
			case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED_BLOCKED:
				cTextColor = COLOR_DARKGREEN;
				break;

			case RNL_TASK_AXIS_CONTROLLED:
			case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS:
			case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS_BLOCKED:
				cTextColor = COLOR_DARKRED;
				break;

			default:
				cTextColor = COLOR_BLACK;
		}

		surface()->DrawSetTextColor( cTextColor );
		surface()->DrawSetTextFont( m_hSelfFont );
		surface()->DrawSetTextPos( iXPos, iYPos );
		surface()->DrawUnicodeString( wcTaskTitle );
	}
}