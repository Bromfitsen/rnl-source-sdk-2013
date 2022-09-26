//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"

#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Image.h>
#include <vgui_controls/Controls.h>

#include <filesystem.h>

#include "rnl_mappanel.h"
#include "rnl_mapdialog.h"
#include "c_rnl_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CRnLMapPanel::CRnLMapPanel( Panel *pParent, const char *pName ) : Panel( pParent, pName )
{
	// update scoreboard instantly if on of these events occure
	ListenForGameEvent( "game_newmap" );

	SetSize(pParent->GetWide(), pParent->GetTall());
	SetDragEnabled( false );

	// Get the screen size - Stefan
	int iSWidth, iSHeight;
	engine->GetScreenSize( iSWidth, iSHeight);

	int iPanelHeight = iSHeight - iSHeight/4;
	int iPanelWidth = iPanelHeight;

	// Center the map, works for all resolutions
	int iPanelPosX = ((iSWidth/2) - (iPanelWidth/2));
	int iPanelPosY = (iSHeight/8);

	// background image
	m_pMapImage[0] = new ImagePanel(this, "mapimageallies");
	m_pMapImage[1] = new ImagePanel(this, "mapimageaxis");
	
	m_pMapImage[0]->SetVisible(true);
	m_pMapImage[1]->SetVisible(false);

	m_pMapImage[0]->SetShouldScaleImage(true);
	m_pMapImage[1]->SetShouldScaleImage(true);
	
	m_pMapImage[0]->SetSize( iPanelWidth, iPanelHeight );
	m_pMapImage[1]->SetSize( iPanelWidth, iPanelHeight );
	
	m_pMapImage[0]->SetPos( iPanelPosX, iPanelPosY );
	m_pMapImage[1]->SetPos( iPanelPosX, iPanelPosY );

	m_pMapImage[0]->DisableMouseInputForThisPanel( true );
	m_pMapImage[1]->DisableMouseInputForThisPanel( true );

	m_pRnLTaskPanel = new CRnLTaskStatusPanel( this, this, "taskpanel" );
	m_pRnLTaskPanel->SetPos( iPanelPosX, iPanelPosY );
	m_pRnLTaskPanel->SetSize( iPanelWidth, iPanelHeight );
	m_pRnLTaskPanel->SetVisible(true);
	m_pRnLTaskPanel->DisableMouseInputForThisPanel( true );

	m_pRnLPrimaryObjectivePanel = new CRnLObjectiveStatusPanel( true, this, this, "primaryobjectivepanel" );
	m_pRnLPrimaryObjectivePanel->SetPos( iPanelPosX, iPanelPosY );
	m_pRnLPrimaryObjectivePanel->SetSize( iPanelWidth, iPanelHeight );
	m_pRnLPrimaryObjectivePanel->SetVisible(true);
	m_pRnLPrimaryObjectivePanel->DisableMouseInputForThisPanel( true );

	m_pRnLSecondaryObjectivePanel = new CRnLObjectiveStatusPanel( false, this, this, "secondaryobjectivepanel" );
	m_pRnLSecondaryObjectivePanel->SetPos( iPanelPosX, iPanelPosY );
	m_pRnLSecondaryObjectivePanel->SetSize( iPanelWidth, iPanelHeight );
	m_pRnLSecondaryObjectivePanel->SetVisible(true);
	m_pRnLSecondaryObjectivePanel->DisableMouseInputForThisPanel( true );

	m_pPlayerPanel = new CRnLPlayerPlayerPanel( this, this, "playerpanel" );
	m_pPlayerPanel->SetPos( iPanelPosX, iPanelPosY );
	m_pPlayerPanel->SetSize( iPanelWidth, iPanelHeight );
	m_pPlayerPanel->SetVisible(true);
	m_pPlayerPanel->DisableMouseInputForThisPanel( true );

	m_pRnLTaskLabelPanel = new CRnLTaskLabelPanel( m_pRnLTaskPanel, this, "tasklabelpanel" );
	m_pRnLTaskLabelPanel->SetPos( iPanelPosX, iPanelPosY );
	m_pRnLTaskLabelPanel->SetSize( iPanelWidth, iPanelHeight );
	m_pRnLTaskLabelPanel->SetVisible(true);
	m_pRnLTaskLabelPanel->DisableMouseInputForThisPanel( true );

	InvalidateLayout();

}
CRnLMapPanel::~CRnLMapPanel()
{
}

void CRnLMapPanel::FireGameEvent( IGameEvent *event)
{
	const char * type = event->GetName();
	if ( Q_strcmp(type, "game_newmap") == 0 )
	{
		Initialise( event->GetString( "mapname" ) );
	}

	if( IsVisible() )
		Update();

}

void CRnLMapPanel::Paint( void )
{
	BaseClass::Paint();
}

void CRnLMapPanel::Update( void )
{
	if( CBasePlayer::GetLocalPlayer() == NULL )
		return;

	CRnLPlayer* pPlayer = CRnLPlayer::GetLocalRnLPlayer();
	if( !pPlayer )
		return;

	if( pPlayer->GetTeamNumber() == TEAM_AXIS )
	{
		m_pMapImage[1]->SetVisible( true );
		m_pMapImage[0]->SetVisible( false );
	}
	else
	{
		m_pMapImage[0]->SetVisible( true );
		m_pMapImage[1]->SetVisible( false );
	}

	int iTeam = 0;
	if( pPlayer->GetTeamNumber() == TEAM_AXIS || pPlayer->GetTeamNumber() == TEAM_ALLIES )
	{
		iTeam = pPlayer->GetTeamNumber() - TEAM_ALLIES;
	}

	// Set the title
	int width,height;
	GetSize( width, height );
	if( height > 0 )
	{
		m_iMapOffset = ( height / 2 );
		m_iMapFactor = ( 32768 / (height * m_mapToWorldInfo[iTeam].m_flMapScale) );
	}

	m_pRnLTaskPanel->Update();
	m_pRnLPrimaryObjectivePanel->Update();
	m_pRnLSecondaryObjectivePanel->Update();
}

void CRnLMapPanel::Initialise( const char* mapname )
{
	// load values from script file
	KeyValues *mapKeyValues = new KeyValues( "mapvalues" );

	char tempfile[MAX_PATH];
	Q_snprintf( tempfile, sizeof( tempfile ), "resource/ui/overviews/%s.res", mapname );

	if ( !mapKeyValues->LoadFromFile( g_pFullFileSystem, tempfile, "GAME" ) )
	{
		m_mapToWorldInfo[0].m_OffsetMapElements.x	= 0;
		m_mapToWorldInfo[0].m_OffsetMapElements.y	= 0;
		m_mapToWorldInfo[0].m_flMapScale			= 1.0f;
		m_mapToWorldInfo[0].m_bReverseY				= false;
		m_mapToWorldInfo[0].angle					= 0.0;

		m_mapToWorldInfo[1].m_OffsetMapElements.x	= 0;
		m_mapToWorldInfo[1].m_OffsetMapElements.y	= 0;
		m_mapToWorldInfo[1].m_flMapScale			= 1.0f;
		m_mapToWorldInfo[1].m_bReverseY				= false;
		m_mapToWorldInfo[1].angle					= 0.0;

		m_pMapImage[0]->SetImage( "overview/map_back" );
		m_pMapImage[1]->SetImage( "overview/map_back" );
		DevMsg( 1, "CRnLMapPanel::Initialize: couldn't load file %s.\n", tempfile );
		return;
	}

	m_pMapImage[0]->SetImage( "overview/map_back" );
	m_pMapImage[1]->SetImage( "overview/map_back" );

	KeyValues *pOffsetinfo = mapKeyValues->FindKey( "alliesinfo" );

	if( pOffsetinfo != NULL )
	{
		m_pMapImage[0]->SetImage( pOffsetinfo->GetString( "material" ) );
		m_mapToWorldInfo[0].m_OffsetMapElements.x	= pOffsetinfo->GetInt("offset_x", 0);
		m_mapToWorldInfo[0].m_OffsetMapElements.y	= pOffsetinfo->GetInt("offset_y", 0);
		m_mapToWorldInfo[0].m_flMapScale			= pOffsetinfo->GetFloat("scale", 1.0f);
		m_mapToWorldInfo[0].m_bReverseY				= pOffsetinfo->GetInt("reverse_y")!=0;
		m_mapToWorldInfo[0].angle					= pOffsetinfo->GetFloat("angle", 0.0f);
	}

	pOffsetinfo = mapKeyValues->FindKey( "axisinfo" );

	if( pOffsetinfo != NULL )
	{
		m_pMapImage[1]->SetImage( pOffsetinfo->GetString( "material" ) );
		m_mapToWorldInfo[1].m_OffsetMapElements.x	= pOffsetinfo->GetInt("offset_x", 0);
		m_mapToWorldInfo[1].m_OffsetMapElements.y	= pOffsetinfo->GetInt("offset_y", 0);
		m_mapToWorldInfo[1].m_flMapScale			= pOffsetinfo->GetFloat("scale", 1.0f);
		m_mapToWorldInfo[1].m_bReverseY				= pOffsetinfo->GetInt("reverse_y")!=0;
		m_mapToWorldInfo[1].angle					= pOffsetinfo->GetFloat("angle", 0.0f);
	}

	//m_pCompassPanel->m_bReverse = m_bReverseY;

	m_pRnLTaskPanel->Initialise( mapKeyValues->FindKey( "tasks" ) );
	m_pRnLPrimaryObjectivePanel->Initialise( mapKeyValues->FindKey( "objectives" ) );
	m_pRnLSecondaryObjectivePanel->Initialise( mapKeyValues->FindKey( "objectives" ) );

	mapKeyValues->deleteThis();
}

// handy helper function, converts world vector positions to a 2d vector (x,y) within the map image
Vector2D CRnLMapPanel::WorldToMapCoords( Vector worldPos )
{
	int iTeam = 0;
	CRnLPlayer* pLocal = CRnLPlayer::GetLocalRnLPlayer();

	if( pLocal && ( pLocal->GetTeamNumber() == TEAM_AXIS || pLocal->GetTeamNumber() == TEAM_ALLIES ) )
	{
		iTeam = pLocal->GetTeamNumber() - TEAM_ALLIES;
	}

	Vector2D panelPos;

	if( m_mapToWorldInfo[iTeam].angle != 0.0 )
	{
		float rad = DEG2RAD( m_mapToWorldInfo[iTeam].angle );
		Vector2D old( worldPos.x, worldPos.y );
		worldPos.x = cos( rad ) * old.x - sin( rad ) * old.y;
		worldPos.y = sin( rad ) * old.x + cos( rad ) * old.y;
	}

	// reverse the maps y axis
	if( m_mapToWorldInfo[iTeam].m_bReverseY )
	{
		panelPos.x = (int)((worldPos.x / m_iMapFactor) + m_iMapOffset );
		panelPos.y = ScreenHeight() - (int)(( worldPos.y / m_iMapFactor) + m_iMapOffset);
	}
	else
	{
		panelPos.x = (int)((worldPos.y / m_iMapFactor) + m_iMapOffset );
		panelPos.y = (int)(( worldPos.x / m_iMapFactor) + m_iMapOffset);	
	}

	panelPos.x += YRES(m_mapToWorldInfo[iTeam].m_OffsetMapElements.x);
	panelPos.y += YRES(m_mapToWorldInfo[iTeam].m_OffsetMapElements.y);

	return panelPos;
}

// Handy helper function that draws a textured arrow from panelPos to endPos
void CRnLMapPanel::DrawArrow( Vector2D panelPos, Vector2D endPos, int iArrowSize )
{
	vgui::Vertex_t ArrowPoint[4];
	vgui::Vertex_t ArrowBase[3];
	Vector2D dir = panelPos - endPos;
	float dist = dir.Length();
	dir.NormalizeInPlace();

	Vector2D right( dir.y, dir.x );
	right.x *= -1;


	// (3)--------(2)
	//  |          |
	// (0)--------(1)
	ArrowPoint[0].m_Position = endPos - ( right * XRES( iArrowSize / 2.0f ) );
	ArrowPoint[1].m_Position = endPos + (dir * (dist * 0.75f)) - ( right * XRES( iArrowSize / 2.0f ) );
	ArrowPoint[2].m_Position = endPos + (dir * (dist * 0.75f)) + ( right * XRES( iArrowSize / 2.0f ) );
	ArrowPoint[3].m_Position = endPos + ( right * XRES( iArrowSize / 2.0f ) );

	surface()->DrawTexturedPolygon( 4, ArrowPoint );

	// (0)
	// | \
	// |  \
	// |  (2)
	// |  /
	// | /
	// (1)
	ArrowBase[0].m_Position = endPos + (dir * (dist * 0.75f)) + ( right * XRES( iArrowSize ) );
	ArrowBase[1].m_Position = endPos + (dir * (dist * 0.75f)) - ( right * XRES( iArrowSize ) );
	ArrowBase[2].m_Position = endPos + (dir * dist);

	surface()->DrawTexturedPolygon( 3, ArrowBase );
}

float CRnLMapPanel::GetMapAngle( int iTeam )
{
	return m_mapToWorldInfo[iTeam].angle;
}