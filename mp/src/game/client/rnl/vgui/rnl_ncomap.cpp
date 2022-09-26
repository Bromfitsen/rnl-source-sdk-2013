//
// This is the RnL overview map cpp file, here everything concerning the map image, the capture points
// and button input for hiding it is handled. - Stefan
//

#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>

#include "rnl_ncomap.h"

//#include <vgui/IScheme.h>
//#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
//#include <KeyValues.h>
//#include <vgui_controls/ImageList.h>
//#include <FileSystem.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/ImagePanel.h>
#include "vgui_BitmapImage.h"

//#include "cdll_util.h"
#include "IGameUIFuncs.h" // for key bindings
//#include "animation.h" // for the loadout model anims
//extern IGameUIFuncs *gameuifuncs; // for key binding details
//#include <game/client/iviewport.h>
#include "weapon_selection.h"

#include "c_rnl_capture_node.h"
#include "c_rnl_spawn_area.h"

//#include <stdlib.h> // MAX_PATH define

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

// map specific
//#define MAP_OFFSET 512 // This is the offset which will be used to make a range from -512 to 512 suitable for 0 - 1024 uhh..well..need to change that to match all resolutions TODO: here
//#define MAP_FACTOR 32.5 // The factor by which the coordinates will be divided - Stefan

static CRnLNCOMap *s_pRnLNCOMap = NULL; // we assume only one overview is created

void CC_DevNCOMap()
{
	if ( !s_pRnLNCOMap )
		return;

	CBasePlayer* pLocal = CBasePlayer::GetLocalPlayer();

	if( pLocal && ( pLocal->GetTeamNumber() == TEAM_AXIS || pLocal->GetTeamNumber() == TEAM_ALLIES ) )
	{
		int iTeam = pLocal->GetTeamNumber() - TEAM_ALLIES;
		if( engine->Cmd_Argc() < 5 )
		{
			DevMsg("Current settings: x_shift: %f y_shift: %f scale: %f reverse_y: %i angle: %f\n",
				s_pRnLNCOMap->m_mapToWorldInfo[iTeam].m_OffsetMapElements.x,
				s_pRnLNCOMap->m_mapToWorldInfo[iTeam].m_OffsetMapElements.y,
				s_pRnLNCOMap->m_mapToWorldInfo[iTeam].m_flMapScale,
				s_pRnLNCOMap->m_mapToWorldInfo[iTeam].m_bReverseY,
				s_pRnLNCOMap->m_mapToWorldInfo[iTeam].angle
				);
		}
		else if ( engine->Cmd_Argc() == 6)
		{
			s_pRnLNCOMap->m_mapToWorldInfo[iTeam].m_OffsetMapElements.x = atoi(engine->Cmd_Argv(1));
			s_pRnLNCOMap->m_mapToWorldInfo[iTeam].m_OffsetMapElements.y = atoi(engine->Cmd_Argv(2));
			s_pRnLNCOMap->m_mapToWorldInfo[iTeam].m_flMapScale = atof(engine->Cmd_Argv(3));
			s_pRnLNCOMap->m_mapToWorldInfo[iTeam].m_bReverseY = atoi(engine->Cmd_Argv(4));
			s_pRnLNCOMap->m_mapToWorldInfo[iTeam].angle = atof(engine->Cmd_Argv(5));
		}
	}
}

ConCommand rnl_ncomap( "rnl_devncomap", CC_DevNCOMap, "Manually configure NCO map. Usage: rnl_devncomap <x_shift> <y_shift> <scale> <reverse_y>", FCVAR_CLIENTDLL | FCVAR_CHEAT );

//static float infoTimer = 0.0f;
CapZonePanel::CapZonePanel( CRnLNCOMap *pMapParent, vgui::Panel *pParent, const char *pName ) : 
	BaseClass( pParent, pName )
{
	SetPaintBackgroundEnabled( false );
	m_pNCOParent = pMapParent;
}

void CapZonePanel::Paint( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if( m_pNCOParent )
	{
		for( int i = 0; i < m_pNCOParent->m_hNodes.Size(); i++ )
		{
			C_TriggerCapturePoint* pPoint = dynamic_cast<C_TriggerCapturePoint*>(m_pNCOParent->m_hNodes.Element(i));

			if( pPoint && pPoint->IsObjectiveVisibleOnNCOMap() )
			{
				// FALLBACK CIRCLE
				if ( pPlayer->GetTeamNumber() == pPoint->GetOccupyingTeam() && pPoint->GetFallbackRadius() > 0 /*&& developer.GetInt() >= 1*/ )
				{
					if ( !m_pNCOParent->isFallbackActive(pPoint) )
						surface()->DrawSetColor( 128, 128, 128, 200 );
					else
						surface()->DrawSetColor( 255, 0, 0, 200 );
					Vector worldOrigin = pPoint->GetObjectiveOrigin();
					Vector2D origin = m_pNCOParent->WorldToMapCoords( worldOrigin );
					surface()->DrawOutlinedCircle ( origin.x, origin.y, pPoint->GetFallbackRadius() / m_pNCOParent->m_iMapFactor, 20 );
				}

				// ZONE COLOR
				switch( pPoint->GetObjectiveState() )
				{
				case RNL_OBJECTIVE_ALLIED_CONTROLLED_AXIS_CAPTURING:
				case RNL_OBJECTIVE_NEUTRAL_AXIS_CAPTURING:
					surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[0] );
					break;

				case RNL_OBJECTIVE_AXIS_CONTROLLED_ALLIED_CAPTURING:
				case RNL_OBJECTIVE_NEUTRAL_ALLIED_CAPTURING:
					surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[1] );
					break;
				case RNL_OBJECTIVE_AXIS_CONTROLLED:
					surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[2] );
					break;
				case RNL_OBJECTIVE_ALLIED_CONTROLLED:
					surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[3] );
					break;
				default:
					surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[4] );
					break;
				}

				// POLY ZONE
				if( i < MAX_TACT_ENG_NODES &&  m_pNCOParent->m_CapZones[i].polyCount[0] > 0 )
				{
					for( int area = 0; area < MAX_SUB_AREAS; area++ )
					{
						if( m_pNCOParent->m_CapZones[i].polies[area] > 0 )
						{
							surface()->DrawTexturedPolygon( m_pNCOParent->m_CapZones[i].polyCount[area], m_pNCOParent->m_CapZones[i].polies[area] );
						}
					}

					surface()->DrawSetColor( 0, 0, 0, 255 );

					for( int area = 0; area < MAX_SUB_AREAS; area++ )
					{
						int max = m_pNCOParent->m_CapZones[i].polyCount[area]-1;
						for( int j = 0; j < max; j++ )
						{
							surface()->DrawLine( m_pNCOParent->m_CapZones[i].polies[area][j].m_Position.x,  m_pNCOParent->m_CapZones[i].polies[area][j].m_Position.y,
								m_pNCOParent->m_CapZones[i].polies[area][j+1].m_Position.x, m_pNCOParent->m_CapZones[i].polies[area][j+1].m_Position.y );
						}

						if( max > 2 )
						{
							surface()->DrawLine( m_pNCOParent->m_CapZones[i].polies[area][max].m_Position.x,  m_pNCOParent->m_CapZones[i].polies[area][max].m_Position.y,
								m_pNCOParent->m_CapZones[i].polies[area][0].m_Position.x, m_pNCOParent->m_CapZones[i].polies[area][0].m_Position.y );
						}
					}
				}
				// RECT ZONE
				else
				{
					Vector worldMax = pPoint->GetObjectiveMax();
					Vector worldMin = pPoint->GetObjectiveMin();
					Vector2D panelPos1 = m_pNCOParent->WorldToMapCoords( worldMin );
					Vector2D panelPos2 = m_pNCOParent->WorldToMapCoords( worldMax );

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

					surface()->DrawTexturedRect( panelPos1.x, panelPos1.y, panelPos2.x, panelPos2.y );
					surface()->DrawSetColor( 0, 0, 0, 255 );
					surface()->DrawLine( panelPos1.x, panelPos1.y, panelPos1.x, panelPos2.y );
					surface()->DrawLine( panelPos1.x, panelPos2.y, panelPos2.x, panelPos2.y );
					surface()->DrawLine( panelPos2.x, panelPos2.y, panelPos2.x, panelPos1.y );
					surface()->DrawLine( panelPos1.x, panelPos1.y, panelPos2.x, panelPos1.y );
				}
			}
		}
	}
}

/////////////////////
////-----------------------------------------------------------------------------
//// Constructor
////-----------------------------------------------------------------------------
//CompassPanel::CompassPanel( ) :	BaseClass( NULL, "CompassPanel" )
//{
//	SetPaintBackgroundEnabled( false );
//	m_iCompass = surface()->CreateNewTextureID();
//	surface()->DrawSetTextureFile( m_iCompass , ( const char * )"VGUI/watch/gerhour", true, false);
//	m_iCompassBackground  = surface()->CreateNewTextureID();
//	surface()->DrawSetTextureFile( m_iCompassBackground , ( const char * )"VGUI/overview/circle", true, false);
//	m_bReverse = false;
//}
//
//CompassPanel::CompassPanel( vgui::Panel *pParent, const char *pName ) : 
//	BaseClass( pParent, pName )
//{
//	SetPaintBackgroundEnabled( false );
//	m_iCompass = surface()->CreateNewTextureID();
//	surface()->DrawSetTextureFile( m_iCompass , ( const char * )"VGUI/watch/gerhour", true, false);
//	m_iCompassBackground  = surface()->CreateNewTextureID();
//	surface()->DrawSetTextureFile( m_iCompassBackground , ( const char * )"VGUI/overview/circle", true, false);
//	m_bReverse = false;
//}
//
//void CompassPanel::Paint( void )
//{
//
//	C_RnLPlayer *pPlayer = C_RnLPlayer::GetLocalRnLPlayer();
//	if( pPlayer )
//	{
//		float theta = pPlayer->EyeAngles()[YAW];
//
//		if( !m_bReverse )
//		{
//			theta *= -1;
//		}
//
//		float scale = 32;
//
//		Vector offset; offset.z = 0;
//		Vector pos( (GetWide() / 2), (GetTall() / 2), 0 );
//
//		offset.x = -scale/8;	offset.y = scale;
//		VectorYawRotate( offset, theta, offset );
//		Vector2D pos1( XRES(pos.x + offset.x), YRES(pos.y + offset.y) );
//
//		offset.x = scale/8;	offset.y = scale;
//		VectorYawRotate( offset, theta, offset );
//		Vector2D pos2( XRES(pos.x + offset.x), YRES(pos.y + offset.y) );
//
//		offset.x = scale/8;	offset.y = -scale;
//		VectorYawRotate( offset, theta, offset );
//		Vector2D pos3( XRES(pos.x + offset.x), YRES(pos.y + offset.y) );
//
//		offset.x = -scale/8;	offset.y = -scale;
//		VectorYawRotate( offset, theta, offset );
//		Vector2D pos4( XRES(pos.x + offset.x), YRES(pos.y + offset.y) );
//
//		Vertex_t points[4] =
//		{
//				Vertex_t( pos4, Vector2D(0,0) ),
//				Vertex_t( pos3, Vector2D(1,0) ),
//				Vertex_t( pos2, Vector2D(1,1) ),
//				Vertex_t( pos1, Vector2D(0,1) )
//		};
//
//		surface()->DrawSetTexture( m_iCompassBackground );
//		surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );
//
//		surface()->DrawSetTexture( m_iCompass );
//		surface()->DrawTexturedPolygon( 4, points );
//	}
//}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLNCOMap::CRnLNCOMap(IViewPort *pViewPort) : Frame(NULL, PANEL_NCOMAP )
{
	SetPos(0, 0);
	SetSize(ScreenWidth(), ScreenHeight());

	// The button alignment needs some serious work- Stefan
	m_pViewPort = pViewPort;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	SetPaintBackgroundEnabled( false );
	
	// hide the system buttons
	SetTitleBarVisible( false );
	SetProportional(true);

	// Get the screen size - Stefan
	int m_iSWidth, m_iSHeight;
	engine->GetScreenSize( m_iSWidth, m_iSHeight);

	// background image
	m_pMapImage = new ImagePanel(this, "mapimage01");
	m_pMapImage->SetVisible(true);
	m_pMapImage->SetShouldScaleImage(true);
	m_pMapImage->SetSize( m_iSHeight, m_iSHeight );

	m_pZoneIndicator = new CapZonePanel( this, m_pMapImage, "ZoneIndicator" );
	m_pZoneIndicator->SetEnabled( true );
	m_pZoneIndicator->SetVisible(true);
	//m_pZoneIndicator->SetProportional( true );
	m_pZoneIndicator->SetZPos( -2 );
	m_pZoneIndicator->SetSize( m_iSHeight, m_iSHeight );

	// Center the map, works for all resolutions
	m_pMapImage->SetPos(  ((m_iSWidth/2) - (m_iSHeight/2)), 0 );

	m_iCapZoneState[0] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iCapZoneState[0], ( const char * )"VGUI/overview/node_axis_capturing", true, false);

	m_iCapZoneState[1] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iCapZoneState[1], ( const char * )"VGUI/overview/node_allies_capturing", true, false);

	m_iCapZoneState[2] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iCapZoneState[2], ( const char * )"VGUI/overview/node_axis_control", true, false);

	m_iCapZoneState[3] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iCapZoneState[3], ( const char * )"VGUI/overview/node_allies_control", true, false);

	m_iCapZoneState[4] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iCapZoneState[4], ( const char * )"VGUI/overview/node_grey", true, false);

	m_iCapZoneState[5] = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iCapZoneState[5], ( const char * )"VGUI/overview/node_arrow", true, false);

	for( int i = 0; i < MAX_TACT_ENG_NODES; i++ )
	{
		m_pCaptureZone[i] = NULL;
		m_pCaptureZoneButton[i] = NULL;
		m_pCaptureZoneCaption[i] = NULL;

		for ( int u = 0; u < MAX_SUB_AREAS; u++ )
		{
			m_CapZones[i].polyCount[u] = -1;
			m_CapZones[i].polies[u] = NULL;
		}
	}

	/*for( i = 0; i < MAX_SPAWN_AREAS; i++ )
	{
		m_pSpawnAreas[i] = NULL;
	}*/
	
	m_pLocalPlayer = new ImagePanel( m_pMapImage, "LocalPlayer" );
	m_pLocalPlayer->SetImage( ICON_ALLIES_LOCALPLAYER );
	m_pLocalPlayer->SetVisible( true );
	m_pLocalPlayer->SetSize( PLAYER_ICON_SIZE_X, PLAYER_ICON_SIZE_Y );
	m_pLocalPlayer->SetShouldScaleImage( true );

	/*
	m_pCompassPanel = new CompassPanel( this, "CompassPanel" );
	m_pCompassPanel->SetProportional( true );
	m_pCompassPanel->SetSize( 50, 50 );
	m_pCompassPanel->SetPos( 10, 400 );
	m_pCompassPanel->SetVisible( true );
	m_pCompassPanel->SetZPos( 50 );
	*/

	gameeventmanager->AddListener( this, "game_newmap", false );

	s_pRnLNCOMap = this;

	m_bLoadOverViewData = false;
	m_bCreateElements = false;
	m_flNextUpdateTime = 0.0f;

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CRnLNCOMap::~CRnLNCOMap()
{
	for( int i = 0; i < MAX_TACT_ENG_NODES; i++ )
	{
		for ( int u = 0; u < MAX_SUB_AREAS; u++ )
		{
			if( m_CapZones[i].polies[u] )
				delete[] (m_CapZones[i].polies[u]);

			m_CapZones[i].polies[u] = NULL;
			m_CapZones[i].polyCount[u] = 0;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get the map name and set the nco maps name
//-----------------------------------------------------------------------------
void CRnLNCOMap::InitializeMap()
{
	// load up the level name
	Q_FileBase( engine->GetLevelName(), mapname, sizeof(mapname) );
	m_bLoadOverViewData = true;

	DeleteElements();
	m_bCreateElements = true;
	
	// load values from script file
	KeyValues *mapKeyValues = new KeyValues( "mapvalues" );

	char tempfile[MAX_PATH];
	Q_snprintf( tempfile, sizeof( tempfile ), "resource/ui/overviews/%s.txt", mapname );

	if ( !mapKeyValues->LoadFromFile( vgui::filesystem(), tempfile, "GAME" ) )
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

		m_pMapImage->SetImage( "overview/map_back" );
		DevMsg( 1, "CRnLNCOMap::InitializeMap: couldn't load file %s.\n", tempfile );
		return;
	}

	m_iTeam = 0;

	m_pMapImage->SetImage( "overview/map_back" );

	KeyValues *pOffsetinfo = mapKeyValues->FindKey( "alliesinfo" );

	if( pOffsetinfo != NULL )
	{
		strcpy( m_szMapMaterial[0], pOffsetinfo->GetString( "material" ) );
		m_mapToWorldInfo[0].m_OffsetMapElements.x	= pOffsetinfo->GetInt("offset_x", 0);
		m_mapToWorldInfo[0].m_OffsetMapElements.y	= pOffsetinfo->GetInt("offset_y", 0);
		m_mapToWorldInfo[0].m_flMapScale			= pOffsetinfo->GetFloat("scale", 1.0f);
		m_mapToWorldInfo[0].m_bReverseY				= pOffsetinfo->GetInt("reverse_y")!=0;
		m_mapToWorldInfo[0].angle					= pOffsetinfo->GetFloat("angle", 0.0f);
	}

	pOffsetinfo = mapKeyValues->FindKey( "axisinfo" );

	if( pOffsetinfo != NULL )
	{
		strcpy( m_szMapMaterial[1], pOffsetinfo->GetString( "material" ) );
		m_mapToWorldInfo[1].m_OffsetMapElements.x	= pOffsetinfo->GetInt("offset_x", 0);
		m_mapToWorldInfo[1].m_OffsetMapElements.y	= pOffsetinfo->GetInt("offset_y", 0);
		m_mapToWorldInfo[1].m_flMapScale			= pOffsetinfo->GetFloat("scale", 1.0f);
		m_mapToWorldInfo[1].m_bReverseY				= pOffsetinfo->GetInt("reverse_y")!=0;
		m_mapToWorldInfo[1].angle					= pOffsetinfo->GetFloat("angle", 0.0f);
	}

	//m_pCompassPanel->m_bReverse = m_bReverseY;

	mapKeyValues->deleteThis();

	m_flNextUpdateTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: shows the map
//-----------------------------------------------------------------------------
void CRnLNCOMap::ShowPanel( bool bShow )
{
	int iTeam = 0;
	CBasePlayer* pLocal = CBasePlayer::GetLocalPlayer();

	if( pLocal && ( pLocal->GetTeamNumber() == TEAM_AXIS || pLocal->GetTeamNumber() == TEAM_ALLIES ) )
	{
		iTeam = pLocal->GetTeamNumber() - TEAM_ALLIES;
	}

	if ( bShow )
	{
		m_flDisplayDelay = gpGlobals->curtime + 2.5f;
		Activate();
		SetKeyBoardInputEnabled( true );
		SetMouseInputEnabled( true );

		//Set some variables to get the screen size - Stefan
		int m_iSWidth, m_iSHeight;
		engine->GetScreenSize( m_iSWidth, m_iSHeight);

		// here comes the tricky part
		// we need to set some factors that set the capture zones right, independently from the resolution - Stefan
		m_iMapOffset = ( m_iSHeight / 2 );

		if( m_iSHeight != 0 )
			m_iMapFactor = ( 32768 / (m_iSHeight * m_mapToWorldInfo[iTeam].m_flMapScale) );

		if( m_bCreateElements )
			CreateElements();

		//if( m_bLoadOverViewData ) POLY ZONES DONT UPDATE THEIR OFFSET ON TEAM CHANGE AND I'M TOO LAZY TO FIX NOW
			LoadOverViewData();

		Update();
		//UpdateCapZones();
	}
	else
	{
		SetVisible( false );
		SetKeyBoardInputEnabled( false );
		SetMouseInputEnabled( false );
	}
	
	m_pViewPort->ShowBackGround( bShow );
}

void CRnLNCOMap::Update()
{
	C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if( pLocalPlayer )
	{
		if( pLocalPlayer->GetTeamNumber() != m_iTeam )
		{
			m_iTeam = pLocalPlayer->GetTeamNumber();

			if( m_iTeam == TEAM_ALLIES || m_iTeam == TEAM_AXIS )
			{
				
				m_pMapImage->SetImage( m_szMapMaterial[m_iTeam - TEAM_ALLIES]);

				if( !m_pMapImage->GetImage() )
					m_pMapImage->SetImage( "overview/map_back" );
			}
			else
			{
				m_pMapImage->SetImage( "overview/map_back" );
			}
		}
	}

	UpdateCapZones();
	m_flNextUpdateTime = gpGlobals->curtime + 0.25f;
}

void CRnLNCOMap::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

void CRnLNCOMap::SetData(KeyValues *data)
{	
}

bool CRnLNCOMap::IsVisible( void )
{
	if( BaseClass::IsVisible() )
	{
		
		return (gpGlobals->curtime > m_flDisplayDelay );
	}
	else
	{
		return false;
	}
}

// Here we first grab the capture zones from the player, when this panel is being initialized for the first time
// Then we scale the position down by 32(32768? / 1024), that's the relation between world map and overview map at the moment
// And finally we set the image positions that are supposed to show the capture zones to these 
void CRnLNCOMap::UpdateCapZones()
{
	Vector2D panelPos;
	Vector2D panelMin;
	Vector2D panelMax;
	C_TriggerCapturePoint *pPoint = NULL;
	//C_RnLSpawnArea *pSpawnArea = NULL;
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	for( int i = 0; i < m_hNodes.Count(); i++)
	{
		if( m_hNodes[i] && (m_hNodes[i]->GetObjectiveType() == RNL_OBJECTIVE_TYPE_CAPTURE) )
		{
			pPoint = dynamic_cast<C_TriggerCapturePoint*>(m_hNodes[i]);

			if( pPoint->IsObjectiveVisibleOnNCOMap() )
			{
				panelPos = WorldToMapCoords( pPoint->GetObjectiveOrigin() );
				panelMin = WorldToMapCoords( pPoint->GetObjectiveMin() );
				panelMax = WorldToMapCoords( pPoint->GetObjectiveMax() );
			
				// NODE BUTTON SETTINGS
				m_pCaptureZoneButton[i]->SetPos( panelMin.x, panelMin.y );
				m_pCaptureZoneButton[i]->SetSize( panelMax.x-panelMin.x, panelMax.y-panelMin.y );

				m_pCaptureZoneButton[i]->SetVisible( true );
				m_pCaptureZoneButton[i]->SetParent( m_pMapImage );

				// NODE ICON SETTINGS
				if ( m_CapZones[i].iconPos <= 0 ) // above
					m_pCaptureZone[i]->SetPos( panelPos.x - (NODE_ICON_SIZE_X/2), panelMin.y - NODE_ICON_SIZE_Y - YRES(3) );
				else if ( m_CapZones[i].iconPos == 1 ) // centered
					m_pCaptureZone[i]->SetPos( panelPos.x - (NODE_ICON_SIZE_X/2), panelPos.y - (NODE_ICON_SIZE_Y/2) );
				else if ( m_CapZones[i].iconPos == 2 ) // below
					m_pCaptureZone[i]->SetPos( panelPos.x - (NODE_ICON_SIZE_X/2), panelMax.y + NODE_ICON_SIZE_Y + YRES(3) );
				else if ( m_CapZones[i].iconPos == 3 ) // left
					m_pCaptureZone[i]->SetPos( panelMin.x - NODE_ICON_SIZE_X - XRES(3), panelPos.y - (NODE_ICON_SIZE_Y/2) );
				else // right
					m_pCaptureZone[i]->SetPos( panelMax.x + XRES(3), panelPos.y - (NODE_ICON_SIZE_Y/2) );

				m_pCaptureZone[i]->SetVisible( true );
				m_pCaptureZone[i]->SetParent( m_pMapImage );
				
				// NODE TEXT SETTINGS
				int textWidth = 0;
				int textHeight = 0;
				int iNeeded = 0;
				char szCaption[80] = "";
				strcat( szCaption, m_CapZones[i].szCaption );

				if ( pPoint->GetObjectiveState() == RNL_OBJECTIVE_ALLIED_CONTROLLED )
					iNeeded = pPoint->GetPlayersRequired() - pPoint->GetAxisTouching();
				else if ( pPoint->GetObjectiveState() == RNL_OBJECTIVE_AXIS_CONTROLLED )
					iNeeded = pPoint->GetPlayersRequired() - pPoint->GetAlliesTouching();
				
				if ( iNeeded && pPoint->IsEnabled() && pPoint->GetOccupyingTeam() != pPlayer->GetTeamNumber() )
				{
					strcat(szCaption, " ");
					for( int j = 0; j < iNeeded; j++ )
						strcat(szCaption, "#");
				}

				for( int j = 0; j < Q_strlen( szCaption ); j++ )
					textWidth += surface()->GetCharacterWidth( m_pCaptureZoneCaption[i]->GetFont(), szCaption[j]);
				textHeight = surface()->GetFontTall( m_pCaptureZoneCaption[i]->GetFont() );

				m_pCaptureZoneCaption[i]->SetText( szCaption );
				m_pCaptureZoneCaption[i]->SetSize( textWidth, surface()->GetFontTall( m_pCaptureZoneCaption[i]->GetFont() ) );
				m_pCaptureZoneCaption[i]->SetVisible( true );

				if ( m_CapZones[i].textPos <= 0 ) // above
					m_pCaptureZoneCaption[i]->SetPos( panelPos.x - (textWidth/2), panelMin.y - textHeight );
				else if ( m_CapZones[i].textPos == 1 ) // centered
					m_pCaptureZoneCaption[i]->SetPos( panelPos.x - (textWidth/2), panelPos.y - (textHeight/2) );
				else if ( m_CapZones[i].textPos == 2 ) // below
					m_pCaptureZoneCaption[i]->SetPos( panelPos.x - (textWidth/2), panelMax.y );
				else if ( m_CapZones[i].textPos == 3 ) // left
					m_pCaptureZoneCaption[i]->SetPos( panelMin.x - textWidth - XRES(8), panelPos.y - (textHeight/2) );
				else // right
					m_pCaptureZoneCaption[i]->SetPos( panelMax.x + XRES(8), panelPos.y - (textHeight/2) );
								
				if ( pPoint->GetObjectiveState() == RNL_OBJECTIVE_ALLIED_CONTROLLED )
                    m_pCaptureZoneCaption[i]->SetFgColor( Color(0, 140, 0, 255) );
				else if ( pPoint->GetObjectiveState() == RNL_OBJECTIVE_AXIS_CONTROLLED )
                    m_pCaptureZoneCaption[i]->SetFgColor( Color(180, 0, 0, 255) );
				else
                    m_pCaptureZoneCaption[i]->SetFgColor( Color(120, 120, 120, 255) );

				// Disable button if node has no attached spawns
				if ( hasEnabledSpawns(pPoint) )
				{
					m_pCaptureZoneButton[i]->SetEnabled( true );
					m_pCaptureZone[i]->SetEnabled( true );
				}
				else
				{
					m_pCaptureZoneButton[i]->SetEnabled( false );
					m_pCaptureZone[i]->SetEnabled( false );
				}

				// AXIS
				if( pPlayer->GetTeamNumber() == TEAM_AXIS )
				{
					if( pPoint->GetObjectiveState() != RNL_OBJECTIVE_AXIS_CONTROLLED )
					{							
						// The zone is enemy controlled or neutral
						if( pPoint->GetObjectiveState() == RNL_OBJECTIVE_ALLIED_CONTROLLED )
							m_pCaptureZone[i]->SetImage( ICON_AXIS_UNCAPTURED_NODE );
						else
							m_pCaptureZone[i]->SetImage( ICON_AXIS_NEUTRAL_NODE );
					}
					else 
					{
						// It's ours (axis)
						m_pCaptureZone[i]->SetImage( ICON_AXIS_CAPTURED_NODE );

						// Check if it's our selected spawn
						C_RnLSpawnArea *pSpawn = (C_RnLSpawnArea*)pPlayer->m_Local.m_hCurrentSpawnArea.Get();
						C_TriggerCapturePoint *pOwnerNode = (C_TriggerCapturePoint*)pSpawn->m_hOwnerNode.Get();
						if ( pOwnerNode->GetCapturePointID() == pPoint->GetCapturePointID() )
						{
							if ( isFallbackActive(pPoint) )
								m_pCaptureZone[i]->SetImage( ICON_AXIS_BLOCKED_NODE );
							else
								m_pCaptureZone[i]->SetImage( ICON_AXIS_SELECTED_NODE );
						}
					}
				}
				// ALLIES
				else
				{
					if( pPoint->GetObjectiveState() != RNL_OBJECTIVE_ALLIED_CONTROLLED )
					{
						// The zone is enemy controlled or neutral
						if( pPoint->GetObjectiveState() == RNL_OBJECTIVE_AXIS_CONTROLLED )
							m_pCaptureZone[i]->SetImage( ICON_ALLIES_UNCAPTURED_NODE );
						else
							m_pCaptureZone[i]->SetImage( ICON_ALLIES_NEUTRAL_NODE );
					}
					else // It's ours
					{
						m_pCaptureZone[i]->SetImage( ICON_ALLIES_CAPTURED_NODE );

						// Check if it's our selected spawn
						C_RnLSpawnArea *pSpawn = (C_RnLSpawnArea*)pPlayer->m_Local.m_hCurrentSpawnArea.Get();
						C_TriggerCapturePoint *pOwnerNode = (C_TriggerCapturePoint*)pSpawn->m_hOwnerNode.Get();
						if ( pOwnerNode->GetCapturePointID() == pPoint->GetCapturePointID() )
						{
							if ( isFallbackActive(pPoint) )
								m_pCaptureZone[i]->SetImage( ICON_ALLIES_BLOCKED_NODE );
							else
								m_pCaptureZone[i]->SetImage( ICON_ALLIES_SELECTED_NODE );
						}
					}
				}
			}
			else
			{
				if( m_pCaptureZone[i] != NULL )
				{
					m_pCaptureZone[i]->SetVisible( false );
					m_pCaptureZoneCaption[i]->SetVisible( false );
				}
			}
		}
		else
		{
			if( m_pCaptureZone[i] != NULL )
			{
				m_pCaptureZone[i]->SetVisible( false );
				m_pCaptureZoneCaption[i]->SetVisible( false );
			}
		}
	}

	/*for( i = 0; i < m_hSpawnAreas.Count(); i++ )
	{
		pSpawnArea = (C_RnLSpawnArea*)m_hSpawnAreas[i].Get();
		pPoint = (C_TriggerCapturePoint*)pSpawnArea->m_hOwnerNode.Get();

		// if not a valid spawn area, hide it and skip it
		if( pPoint == NULL || !pPoint->IsObjectiveVisibleOnNCOMap() )
		{
			continue;
		}

		if( pPlayer->GetTeamNumber() == pPoint->GetOccupyingTeam() )
		{
			color32 buttonColour;

			if( pPlayer->GetTeamNumber() == TEAM_AXIS )
			{
				if( m_hSpawnAreas[i] == pPlayer->m_Local.m_hCurrentSpawnArea )
				{
					buttonColour.r = 255; buttonColour.g = 255; buttonColour.b = 255; buttonColour.a = 250;
					m_pSpawnAreas[i]->SetImage( CBitmapButton::BUTTON_ENABLED, ICON_AXIS_SELECTED_SPAWN, buttonColour );
				}
				else
				{
					buttonColour.r = 255; buttonColour.g = 255; buttonColour.b = 255; buttonColour.a = 250;
					m_pSpawnAreas[i]->SetImage( CBitmapButton::BUTTON_ENABLED, ICON_AXIS_AVAILABLE_SPAWN, buttonColour );
				}

				buttonColour.r = 255; buttonColour.g = 125; buttonColour.b = 125; buttonColour.a = 100;
				m_pSpawnAreas[i]->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, ICON_AXIS_SELECTED_SPAWN, buttonColour );

				buttonColour.r = 255; buttonColour.g = 255; buttonColour.b = 255; buttonColour.a = 0;
				m_pSpawnAreas[i]->SetImage( CBitmapButton::BUTTON_PRESSED, ICON_AXIS_SELECTED_SPAWN, buttonColour );
			}
			else
			{
				if( m_hSpawnAreas[i] == pPlayer->m_Local.m_hCurrentSpawnArea )
				{
					buttonColour.r = 255; buttonColour.g = 255; buttonColour.b = 255; buttonColour.a = 250;
					m_pSpawnAreas[i]->SetImage( CBitmapButton::BUTTON_ENABLED, ICON_ALLIES_SELECTED_SPAWN, buttonColour );
				}
				else
				{
					buttonColour.r = 255; buttonColour.g = 255; buttonColour.b = 255; buttonColour.a = 250;
					m_pSpawnAreas[i]->SetImage( CBitmapButton::BUTTON_ENABLED, ICON_ALLIES_AVAILABLE_SPAWN, buttonColour );
				}


				buttonColour.r = 255; buttonColour.g = 125; buttonColour.b = 125; buttonColour.a = 100;
				m_pSpawnAreas[i]->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, ICON_ALLIES_SELECTED_SPAWN, buttonColour );

				buttonColour.r = 255; buttonColour.g = 255; buttonColour.b = 255; buttonColour.a = 0;
				m_pSpawnAreas[i]->SetImage( CBitmapButton::BUTTON_PRESSED, ICON_ALLIES_AVAILABLE_SPAWN, buttonColour );
			}

			m_pSpawnAreas[i]->SetBorder( NULL );
			m_pSpawnAreas[i]->SetVisible( true );
			m_pSpawnAreas[i]->SetEnabled( true );
		}
		else
		{
			m_pSpawnAreas[i]->SetVisible( false );
			m_pSpawnAreas[i]->SetEnabled( false );
			m_pSpawnAreas[i]->SetArmed( false );
		}

		panelPos = WorldToMapCoords( pSpawnArea->m_vecCenter );
		m_pSpawnAreas[i]->SetPos( panelPos.x - (m_pSpawnAreas[i]->GetWide()/2), panelPos.y - (m_pSpawnAreas[i]->GetTall()/2) );
	}*/

	// PLAYER LOCATOR
	panelPos = WorldToMapCoords( pPlayer->GetAbsOrigin() );
	m_pLocalPlayer->SetPos( panelPos.x - 8, panelPos.y - 8 );
	m_pLocalPlayer->SetSize( 16, 16 );
	m_pLocalPlayer->SetVisible( true );	// m_pLocalPlayer->SetVisible( developer.GetBool() );


	//m_pCompassPanel->SetSize( 50, 50 );
	//m_pCompassPanel->SetPos( 10, 400 );
	//m_pCompassPanel->SetVisible( true );

}

// Grab the button input from the player
void CRnLNCOMap::OnCommand( const char *command )
{
	engine->ClientCmd( command );
}

void CRnLNCOMap::OnKeyCodePressed(KeyCode code)
{
	// handle "ncomap" key --> close map
	if( engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind( "ncomap" ))
	{	
		// switch to last active weapon, if there's none, just holster the map				
		C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
		if ( player ) 
		{
			CWeaponRnLBase* pWep = dynamic_cast<CWeaponRnLBase*>( player->GetLastWeapon() );
			if ( pWep && pWep->GetWeaponID() != WEAPON_NONE && pWep->GetWeaponID() != WEAPON_NCOMAP )
			{
				CBaseHudWeaponSelection *pHudSelection = GetHudWeaponSelection();
				if ( pHudSelection )
					pHudSelection->SwitchToLastWeapon();
			}
			else
			{
				pWep = dynamic_cast<CWeaponRnLBase*>( player->GetActiveWeapon() );
				if (pWep)
					pWep->Holster();
			}
		}
		ShowPanel( false );
	}
	else
		BaseClass::OnKeyCodePressed( code );
}

void CRnLNCOMap::FireGameEvent( IGameEvent *event )
{
	const char * type = event->GetName();

	if ( Q_strcmp(type, "game_newmap") == 0 )
	{
		InitializeMap();
	}
}

// handy helper function, converts world vector positions to a 2d vector (x,y) within the map image
Vector2D CRnLNCOMap::WorldToMapCoords( Vector worldPos )
{
	int iTeam = 0;
	CBasePlayer* pLocal = CBasePlayer::GetLocalPlayer();

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

void CRnLNCOMap::LoadOverViewData( )
{
	// load values from script file
	KeyValues *mapKeyValues = new KeyValues( "mapvalues" );

	char tempfile[MAX_PATH];
	Q_snprintf( tempfile, sizeof( tempfile ), "resource/ui/overviews/%s.txt", mapname );

	if ( !mapKeyValues->LoadFromFile( vgui::filesystem(), tempfile, "GAME" ) )
	{
		DevMsg( 1, "CRnLNCOMap::LoadOverViewData: couldn't load file %s.\n", tempfile );
		mapKeyValues->deleteThis();
		return;
	}

	/*C_TriggerCapturePoint*/IRnLObjective *pPoint = NULL;

	if (m_pBrotherArrows.Count() > 0)
		ClearBrotherArrows();

	for(int i = 0; i < m_hNodes.Count(); i++ )
	{
		pPoint = /*(C_TriggerCapturePoint*)*/m_hNodes[i];

		//Grab the brother nodes if it's a tactical engagement node
		if( pPoint )
		{
			CBrotherLinkArrow* pArrow = NULL;
			C_RnLTacticalEngagementNode* pTacNode = dynamic_cast<C_RnLTacticalEngagementNode*>( pPoint );
			if( pTacNode )
			{
				Vector pos = pTacNode->GetObjectiveOrigin();

				DevMsg( "Found a tactical node %s at location %f %f\n", pTacNode->GetClassname(), pos.x, pos.y );
				for( int i = 0; i < MAX_BROTHER_NODES; i++ )
				{
					if( pTacNode->m_hAlliesBrotherNodes[i] )
					{
						pArrow = new CBrotherLinkArrow( this, m_pMapImage, TEAM_ALLIES, pTacNode, pTacNode->m_hAlliesBrotherNodes[i] );
						pArrow->SetZPos( -1 );
						Vector pos2 = pTacNode->m_hAlliesBrotherNodes[i]->GetObjectiveOrigin();
						float xDiff = 0;
						float yDiff = 0;

						if( pos.x > pos2.x )
							xDiff = pos2.x - pos.x;
						else
							xDiff -= pos.x - pos2.x;

						if( pos.y > pos2.y )
							yDiff = pos2.y - pos.y;
						else
							yDiff -= pos.y - pos2.y;

						pArrow->AddPoint( pos2.x, pos2.y );
						pArrow->AddPoint( pos.x + ( xDiff / 2 ), pos.y + ( yDiff / 2 ) );
						pArrow->AddPoint( pos.x + ( xDiff / 4), pos.y + ( yDiff / 4 ) );
						pArrow->AddPoint( pos.x, pos.y );

						pArrow->SetEnabled( true );
						pArrow->SetVisible( true );
						m_pBrotherArrows.AddToTail( pArrow );
						pArrow = NULL;
					}

					if( pTacNode->m_hAxisBrotherNodes[i] )
					{
						pArrow = new CBrotherLinkArrow( this, m_pMapImage, TEAM_AXIS, pTacNode, pTacNode->m_hAlliesBrotherNodes[i] );
						Vector pos2 = pTacNode->m_hAxisBrotherNodes[i]->GetObjectiveOrigin();
						float xDiff = 0;
						float yDiff = 0;

						if( pos.x > pos2.x )
							xDiff = pos2.x - pos.x;
						else
							xDiff -= pos.x - pos2.x;

						if( pos.y > pos2.y )
							yDiff = pos2.y - pos.y;
						else
							yDiff -= pos.y - pos2.y;

						pArrow->AddPoint( pos2.x, pos2.y );
						pArrow->AddPoint( pos.x + ( xDiff / 2 ), pos.y + ( yDiff / 2 ) );
						pArrow->AddPoint( pos.x + ( xDiff / 4), pos.y + ( yDiff / 4 ) );
						pArrow->AddPoint( pos.x, pos.y );

						pArrow->SetEnabled( true );
						pArrow->SetVisible( true );
						m_pBrotherArrows.AddToTail( pArrow );
						pArrow = NULL;
					}
				}
			}
		}

		// this loads the data for this point from the overview script
		//  only needs to be done once per map
		char szCaptureZoneData[32];
		Q_snprintf( szCaptureZoneData, sizeof(szCaptureZoneData), "node%i", pPoint->GetNCOMapIndex() );

		KeyValues *nodeKeyValues = mapKeyValues->FindKey( szCaptureZoneData );

		if(!nodeKeyValues)
		{
			continue;
		}

		KeyValues *vertsKeyValues = nodeKeyValues->FindKey( "area", false );
		int areaNr = 0;
        while ( vertsKeyValues && areaNr < MAX_SUB_AREAS )
		{
			if ( stricmp(vertsKeyValues->GetName(),"area") != 0 )
			{
				vertsKeyValues = vertsKeyValues->GetNextKey();
				continue;
			}

			//DevMsg( "Found 'Area' Key Values\n" );
			m_CapZones[i].polyCount[areaNr] = 0;
			int iTotal = 0;
			KeyValues *vert = vertsKeyValues->GetFirstSubKey();
			
			while( vert )
			{
				iTotal++;
				//DevMsg( "SubKey %s for total %d\n", vert->GetString(), iTotal );
				vert = vert->GetNextKey();
			}

			if( iTotal > 0 )
			{
				m_CapZones[i].polyCount[areaNr] = iTotal;
				m_CapZones[i].polies[areaNr] = new Vertex_t[iTotal];

				Vector2D panelPos;
				Vector worldPos;
				vert = vertsKeyValues->GetFirstSubKey();
				iTotal = 0;
				while( vert )
				{
					sscanf( vert->GetString(), "%f %f %f", &(worldPos.x), &(worldPos.y), &(worldPos.z) );
					panelPos = WorldToMapCoords( worldPos );

					m_CapZones[i].polies[areaNr][iTotal].Init( panelPos );

					iTotal++;

					vert = vert->GetNextKey();
				}

				iTotal++;
				Vector2D min, max;
				for( int polyLoop = 0; polyLoop < iTotal; polyLoop++ )
				{
					if( m_CapZones[i].polies[areaNr][polyLoop].m_Position.x < min.x )
					{
						min.x = m_CapZones[i].polies[areaNr][polyLoop].m_Position.x;
					}
					else if( m_CapZones[i].polies[areaNr][polyLoop].m_Position.x > max.x )
					{
						max.x = m_CapZones[i].polies[areaNr][polyLoop].m_Position.x;
					}

					if( m_CapZones[i].polies[areaNr][polyLoop].m_Position.y < min.y )
					{
						min.y = m_CapZones[i].polies[areaNr][polyLoop].m_Position.y;
					}
					else if( m_CapZones[i].polies[areaNr][polyLoop].m_Position.y > max.y )
					{
						max.y = m_CapZones[i].polies[areaNr][polyLoop].m_Position.y;
					}
				}

				max.x = max.x - min.x;
				max.y = max.y - min.y;

				for( int polyLoop = 0; polyLoop < iTotal; polyLoop++ )
				{
					m_CapZones[i].polies[areaNr][polyLoop].m_TexCoord.x = (m_CapZones[i].polies[areaNr][polyLoop].m_Position.x - min.x) / max.x;
					m_CapZones[i].polies[areaNr][polyLoop].m_TexCoord.x *= 10;
					m_CapZones[i].polies[areaNr][polyLoop].m_TexCoord.y = (m_CapZones[i].polies[areaNr][polyLoop].m_Position.y - min.y) / max.y;
					m_CapZones[i].polies[areaNr][polyLoop].m_TexCoord.y *= 10;
				}
			}

			areaNr++;
			vertsKeyValues = vertsKeyValues->GetNextKey();
		}

		strcpy( m_CapZones[i].szCaption, nodeKeyValues->GetString("displayname") );
		m_CapZones[i].textPos = nodeKeyValues->GetInt("textposition", 2);
		m_CapZones[i].iconPos = nodeKeyValues->GetInt("iconposition", 0);

		if( m_pCaptureZoneCaption[i] )
			m_pCaptureZoneCaption[i]->SetText( m_CapZones[i].szCaption );
	}

	mapKeyValues->deleteThis();
	

	// loaded, don't load again until next map
	m_bLoadOverViewData = false;
}

void CRnLNCOMap::CreateElements()
{
	// Get our scheme and font information
	vgui::HScheme scheme = vgui::scheme()->GetScheme( "ClientScheme" );
	vgui::HFont hFont = vgui::scheme()->GetIScheme(scheme)->GetFont( "joeHand" );
	color32 normalColor;
	normalColor.a = normalColor.r = normalColor.g = normalColor.b = 255;

	//C_RnLSpawnArea* pSpawnArea = NULL;
	/*C_TriggerCapturePoint*/IRnLObjective *pNode = NULL;

	for( int i = 0; i < m_hNodes.Count(); i++ )
	{
		pNode = /*(C_TriggerCapturePoint*)*/m_hNodes[i];

		char szCommand[32];
		Q_snprintf( szCommand, sizeof(szCommand), "selectspawn %i", dynamic_cast<C_TriggerCapturePoint*>(pNode)->GetCapturePointID() );

		m_pCaptureZoneButton[i] = new CRnLNCOMapSpawnAreaButton(m_pMapImage, "CaptureZone", "", this, szCommand );
		m_pCaptureZoneButton[i]->SetVisible(true);
		
		m_pCaptureZone[i] = new CRnLNCOMapSpawnAreaButton(m_pMapImage, "CaptureZone", "", this, szCommand );
		m_pCaptureZone[i]->SetImage( ICON_ALLIES_UNCAPTURED_NODE );
		m_pCaptureZone[i]->SetVisible(true);
		m_pCaptureZone[i]->SetSize( NODE_ICON_SIZE_X, NODE_ICON_SIZE_Y );

		m_pCaptureZoneCaption[i] = new Label(m_pMapImage, "CaptureZoneCaption", "");
		m_pCaptureZoneCaption[i]->SetVisible( true );
		m_pCaptureZoneCaption[i]->SetFont( hFont );
		m_pCaptureZoneCaption[i]->SetMouseInputEnabled( false );
	}

	/*for( i = 0; i < m_hSpawnAreas.Count(); i++ )
	{
		pSpawnArea = (C_RnLSpawnArea*)m_hSpawnAreas[i].Get();

		char szCommand[32];
		Q_snprintf( szCommand, sizeof(szCommand), "selectspawn %i", pSpawnArea->m_iSpawnAreaID );

		m_pSpawnAreas[i] = new CRnLNCOMapSpawnAreaButton( m_pMapImage, "SpawnArea", "", this, szCommand );

		color32 buttonColour;
		buttonColour.r = 255; buttonColour.g = 125; buttonColour.b = 125; buttonColour.a = 100;
		m_pSpawnAreas[i]->SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, ICON_ALLIES_AVAILABLE_SPAWN, buttonColour );
		buttonColour.r = 255; buttonColour.g = 255; buttonColour.b = 255; buttonColour.a = 250;
		m_pSpawnAreas[i]->SetImage( CBitmapButton::BUTTON_ENABLED, ICON_ALLIES_AVAILABLE_SPAWN, buttonColour );
		buttonColour.r = 255; buttonColour.g = 255; buttonColour.b = 255; buttonColour.a = 250;
		m_pSpawnAreas[i]->SetImage( CBitmapButton::BUTTON_PRESSED, ICON_ALLIES_SELECTED_SPAWN, buttonColour );
		m_pSpawnAreas[i]->SetSize(SPAWN_ICON_SIZE_X, SPAWN_ICON_SIZE_Y);
		m_pSpawnAreas[i]->AddActionSignalTarget( this );
		m_pSpawnAreas[i]->SetDefaultBorder( NULL );
		m_pSpawnAreas[i]->SetButtonBorderEnabled( false );
		m_pSpawnAreas[i]->DrawFocusBox( false );
		// dc_hand
		m_pSpawnAreas[i]->SetCursor( 14 );
		m_pSpawnAreas[i]->SetVisible( false );
	}*/

	m_bCreateElements = false;
}

void CRnLNCOMap::DeleteElements()
{
	ClearBrotherArrows();

	for( int i = 0; i < MAX_TACT_ENG_NODES; i++ )
	{
		for ( int u = 0; u < MAX_SUB_AREAS; u++ )
		{
			delete[] m_CapZones[i].polies[u];

			m_CapZones[i].polies[u] = NULL;
			m_CapZones[i].polyCount[u] = 0;
		}
	}

	// clear the vectors of nodes and spawns
	//m_hSpawnAreas.Purge();
	m_hNodes.Purge();

	for( int i = 0; i < MAX_TACT_ENG_NODES; i++ )
	{
		if( m_pCaptureZoneButton[i] )
		{
			m_pCaptureZoneButton[i]->MarkForDeletion();
			m_pCaptureZoneButton[i] = NULL;
		}

		if( m_pCaptureZone[i] )
		{
			m_pCaptureZone[i]->MarkForDeletion();
			m_pCaptureZone[i] = NULL;
		}

		if( m_pCaptureZoneCaption[i] )
		{
			m_pCaptureZoneCaption[i]->MarkForDeletion();
			m_pCaptureZoneCaption[i] = NULL;
		}
	}

	/*for( i = 0; i < MAX_SPAWN_AREAS; i++ )
	{
		if( m_pSpawnAreas[i] )
		{
			m_pSpawnAreas[i]->MarkForDeletion();
			m_pSpawnAreas[i] = NULL;
		}
	}*/
}

void CRnLNCOMap::AddNodeToMap( IRnLObjective* pNode )
{
	if( m_hNodes.HasElement( pNode ) )
	{
		DevMsg("Adding node twice to map? I DONT THINK SO!\n");
		return;
	}

	m_hNodes.AddToTail( pNode );
}

void CRnLNCOMap::AddSpawnAreaToMap( EHANDLE pSpawn )
{
	if( m_hSpawnAreas.HasElement( pSpawn ) )
	{
		DevMsg("Adding spawn area twice to map? I DONT THINK SO!\n");
		return;
	}

	m_hSpawnAreas.AddToTail( pSpawn );
}

void CRnLNCOMap::RemoveNodeFromMap( IRnLObjective* pNode )
{
	if( m_hNodes.HasElement( pNode ) )
	{
		m_hNodes.FindAndRemove( pNode );
	}
}

void CRnLNCOMap::RemoveSpawnAreaFromMap( EHANDLE pSpawn )
{
	if( m_hSpawnAreas.HasElement( pSpawn ) )
	{
		m_hSpawnAreas.FindAndRemove( pSpawn );
	}
}

void CRnLNCOMap::ClearBrotherArrows( void )
{
	for( int i = 0; i < m_pBrotherArrows.Count(); i++ )
	{
		m_pBrotherArrows[i]->SetParent( (VPANEL)NULL ); 
	}

	m_pBrotherArrows.PurgeAndDeleteElements();
}

bool CRnLNCOMap::hasEnabledSpawns( C_TriggerCapturePoint* pPoint )
{
	C_RnLSpawnArea* pSpawnArea = NULL;
	C_TriggerCapturePoint *pOwnerNode = NULL;

	for( int i = 0; i < m_hSpawnAreas.Count(); i++ )
	{
		pSpawnArea = (C_RnLSpawnArea*)m_hSpawnAreas[i].Get();
		if ( pSpawnArea )
			pOwnerNode = (C_TriggerCapturePoint*)pSpawnArea->m_hOwnerNode.Get();
		if ( pOwnerNode && pOwnerNode->GetCapturePointID() == pPoint->GetCapturePointID() && (!pSpawnArea->m_bFallback /* meaning it's enabled */ || pSpawnArea->m_bForcefallback /* meaning it's enabled but with fallback */ ) )
			return true;
	}

	return false;
}

bool CRnLNCOMap::isFallbackActive( C_TriggerCapturePoint* pPoint )
{
	C_RnLSpawnArea* pSpawnArea = NULL;
	C_TriggerCapturePoint *pOwnerNode = NULL;

	for( int i = 0; i < m_hSpawnAreas.Count(); i++ )
	{
		pSpawnArea = (C_RnLSpawnArea*)m_hSpawnAreas[i].Get();
		if ( pSpawnArea )
			pOwnerNode = (C_TriggerCapturePoint*)pSpawnArea->m_hOwnerNode.Get();
		if ( pOwnerNode && pOwnerNode->GetCapturePointID() == pPoint->GetCapturePointID() && pSpawnArea->m_bForcefallback )
			return true;
	}

	CBaseEntity *ent = NULL;
	int iEnemy = TEAM_NONE;

	if ( pPoint->GetOccupyingTeam() == TEAM_AXIS )
		iEnemy = TEAM_ALLIES;
	else if ( pPoint->GetOccupyingTeam() == TEAM_ALLIES )
		iEnemy = TEAM_AXIS;
					
	if ( iEnemy != TEAM_NONE && pPoint->GetFallbackRadius() > 0 )
	{
		for (CEntitySphereQuery sphere( pPoint->GetObjectiveOrigin(), pPoint->GetFallbackRadius() ); (ent = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity())
		{
			CBaseEntity *plent = ent;

			if ( plent && plent->IsPlayer() && plent->GetTeamNumber() == iEnemy )
				return true;
		}
	}

	return false;
}
