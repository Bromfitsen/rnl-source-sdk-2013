//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: NCO map class. Contains all the details for the map specific nco map
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_NCOMAP_H
#define RNL_NCOMAP_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_bitmapbutton.h>
#include <vgui_controls/HTML.h>
#include <vgui_controls/ProgressBar.h>
#include <UtlVector.h>
#include <vgui/ILocalize.h>
#include <vgui/KeyCode.h>
#include <game/client/iviewport.h>
#include <vgui/ISurface.h>
#include <igameevents.h>

#include <vgui/IBorder.h>

#include "c_rnl_player.h"
#include "mouseoverpanelbutton.h"
#include "rnl_shareddefs.h"
#include "rnl_iobjective.h"

#include "c_rnl_spawn_area.h"

#define MAX_SUB_AREAS				16

// Capture zones and spawn icon stuff
#define ICON_ALLIES_LOCALPLAYER		"overview/player_blue"
#define ICON_AXIS_LOCALPLAYER		"overview/player_blue"

//#define ICON_ALLIES_AVAILABLE_SPAWN "VGUI/overview/airborne_available_spawn"
//#define ICON_AXIS_AVAILABLE_SPAWN	"VGUI/overview/axis_available_spawn"

#define ICON_ALLIES_SELECTED_NODE	"VGUI/overview/us_selected_spawn"
#define ICON_AXIS_SELECTED_NODE		"VGUI/overview/ger_selected_spawn"

#define ICON_ALLIES_UNCAPTURED_NODE	"VGUI/overview/us_enemy_node"
#define ICON_AXIS_UNCAPTURED_NODE	"VGUI/overview/ger_enemy_node"

#define ICON_ALLIES_CAPTURED_NODE	"VGUI/overview/us_captured_node"
#define ICON_AXIS_CAPTURED_NODE		"VGUI/overview/ger_captured_node"

#define ICON_ALLIES_BLOCKED_NODE	"VGUI/overview/us_blocked_spawn"
#define ICON_AXIS_BLOCKED_NODE		"VGUI/overview/ger_blocked_spawn"

#define ICON_ALLIES_NEUTRAL_NODE	"VGUI/overview/us_neutral_node"
#define ICON_AXIS_NEUTRAL_NODE		"VGUI/overview/ger_neutral_node"

//#define ICON_ALLIES_CAPTURING_NODE	"VGUI/overview/airborne_capturing_node"
//#define ICON_AXIS_CAPTURING_NODE	"VGUI/overview/axis_capturing_node"

#define PLAYER_ICON_SIZE_X 32
#define PLAYER_ICON_SIZE_Y 32

#define	NODE_ICON_SIZE_X 32
#define NODE_ICON_SIZE_Y 32

#define SPAWN_ICON_SIZE_X 32
#define SPAWN_ICON_SIZE_Y 32

struct capZoneData {
	char szCaption[64];
	int textPos;
	int iconPos;
	vgui::Vertex_t *polies[MAX_SUB_AREAS];
	int polyCount[MAX_SUB_AREAS];
};

class BitmapImage;
class CRnLNCOMap;

namespace vgui
{
	class TextEntry;
}

class CapZonePanel : public vgui::Panel
{
	typedef vgui::Panel BaseClass;
public:
	// constructor
	CapZonePanel( CRnLNCOMap *pMapParent, vgui::Panel *pParent, const char *pName );

	virtual void Paint( void );
	virtual void PaintBackground( void ) {}
private:
	CRnLNCOMap *m_pNCOParent;
};

//Thanks To The Plan of Attack team, could have written it myself but theirs was here and free and worked!
//Originally CStrategicArrow

#define MAX_POINTS 12
#define MIN_WIDTH 25
#define MAX_WIDTH 50
#define DEFAULT_ARROW_Z_VERTEX 0.0

enum eArrowType
{
	ARROW_PUSH,
	ARROW_HOLD,
	ARROW_NONE,
	ARROW_MAX
};

#define INITIAL_ARROW_WIDTH XRES(20)

eArrowType AliasToArrowType( const char *alias );

class CBrotherLinkArrow : public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE(CBrotherLinkArrow, vgui::Panel);

	CBrotherLinkArrow( CRnLNCOMap *pMapParent, vgui::Panel *pParent, int team, IRnLObjective* pNode, IRnLObjective* pParentNode );
	~CBrotherLinkArrow();

	/**
	* Struct declaration for a arrow point
	**/
	struct SStrategicArrowPoint
	{
		Vector m_vecPos;
		int m_iWidth;
	};

	// overwritten
	void SetPos(int x, int y);
	void SetSize(int wide, int tall);
	void SetZ(float z) { m_Z = z; m_bFinalized = false; }
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnMouseReleased(vgui::MouseCode code);
	virtual void OnMousePressed(vgui::MouseCode code);
	virtual void OnCursorEntered();
	virtual void OnCursorExited();

	// color of this arrow
	void SetColor(Color &color) { m_color = color; }
	// what kind of arrow head you want on it
	void SetArrowType(eArrowType type) { m_eArrowType = type; }

	// size
	void ModifyWidth(int iIncrement);
	int GetArrowWidth(void) { return m_iInitialWidth; }

	// doing these will force a re-calculation of the tri-points.
	// adds a control point given a vector
	SStrategicArrowPoint *AddPoint(Vector &point, int width = 10);
	// adds a control point as (x,y)
	SStrategicArrowPoint *AddPoint(float x, float y, int width = 10);
	void RemovePoint(const SStrategicArrowPoint *pPoint);
	void SetFinalized(bool bFinalized) { m_bFinalized = bFinalized; }
	
	// accessors
	CUtlVector<SStrategicArrowPoint *> &GetPoints(void) { return m_aPoints; }
	bool IsComplete(void) { return m_aPoints.Count() > 3; }
	virtual void PanelToMap(int &iXPos, int &iYPos);

	void Paint();
	virtual void PaintBackground( void ) {}

private:
	// finalizes data before painting
	void Finalize();
	// calculates the triangular points from the control points
	void CalculateTriPoints();

	Color m_color;
	bool m_bFinalized;
	int m_iInitialWidth;

	// mutates map coordiates [0, 1024] into the position and size specified for this object
	void MapToScreen(Vector &point, bool bDoParent = true);
	void MapToScreen(Vector2D &point, bool bDoParent = true);

	// control points (not too many of these)
	CUtlVector<SStrategicArrowPoint *> m_aPoints;
	// our triangle strip points (quite a few of these)
	CUtlVector<Vector> m_aTriPoints;
	// what type of arrow are we?
	eArrowType m_eArrowType;

	vgui::Vertex_t		m_vertArrowBase[3];
	vgui::Vertex_t		m_vertArrowHead[3];
	vgui::Vertex_t		m_vertArrow[4];

	Vector m_vEndPoint;
	Vector m_vEndTangent;
	CRnLNCOMap *m_pNCOParent;
	int m_iTeam;
	IRnLObjective* m_pMainNode;
	IRnLObjective* m_pParentNode;
	float m_Z;
};

//class CompassPanel : public vgui::Panel
//{
//	typedef vgui::Panel BaseClass;
//public:
//	// constructor
//	CompassPanel( );
//	CompassPanel( vgui::Panel *pParent, const char *pName );
//
//	virtual void Paint( void );
//	virtual void PaintBackground( void ) {}
//
//	bool m_bReverse;
//
//private:
//	int m_iCompass;
//	int m_iCompassBackground;
//};

class CRnLNCOMapSpawnAreaButton : public CBitmapButton
{
public:
	CRnLNCOMapSpawnAreaButton( vgui::Panel *pParent, const char *pName, const char *pText, Panel *pActionSignalTarget, char *pCmd ): CBitmapButton(pParent, pName, pText, pActionSignalTarget, pCmd	) {}

	void SetImage( const char *pMaterialName ) 
	{
		color32 normalColor;
		normalColor.a = normalColor.r = normalColor.g = normalColor.b = 255;
		CBitmapButton::SetImage( CBitmapButton::BUTTON_DISABLED, pMaterialName, normalColor );
		CBitmapButton::SetImage( CBitmapButton::BUTTON_PRESSED, pMaterialName, normalColor );
		CBitmapButton::SetImage( CBitmapButton::BUTTON_ENABLED_MOUSE_OVER, pMaterialName, normalColor );
		CBitmapButton::SetImage( CBitmapButton::BUTTON_ENABLED, pMaterialName, normalColor );
	}

protected:
	virtual vgui::IBorder *GetBorder(bool depressed, bool armed, bool selected, bool keyfocus) { return NULL; }
};

typedef struct MapToWorldInfo_t
{
	Vector2D m_OffsetMapElements;
	bool m_bReverseY;
	float m_flMapScale;
	float angle;
}MapToWorldInfo;

//-----------------------------------------------------------------------------
// Purpose: Team Manager
//-----------------------------------------------------------------------------
class CRnLNCOMap : public vgui::Frame, public IViewPortPanel, public IGameEventListener2
{
private:
	DECLARE_CLASS_SIMPLE( CRnLNCOMap, vgui::Frame );

public:

	CRnLNCOMap(IViewPort *pViewPort);
	virtual ~CRnLNCOMap();

	IViewPort	*m_pViewPort;

	virtual const char *GetName( void ) { return PANEL_NCOMAP; }
	virtual void SetData(KeyValues *data);
	virtual void Reset() {};
	virtual void Update();
	virtual bool NeedsUpdate( void ) { return (gpGlobals->curtime > m_flNextUpdateTime ); }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow);//, Vector vCapZones[], int iCapZonesState[] );

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible();
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	// Initialization
	void InitializeMap();
	void LoadOverViewData();
	void CreateElements();
	void UpdateCapZones();

	// reset/destruction
	void DeleteElements();

	//vgui2 overrides
	virtual void OnKeyCodePressed( vgui::KeyCode code );

	// for map load changes
	virtual void FireGameEvent( IGameEvent *event);

	// command callbacks
	void OnCommand( const char *command );

	Vector2D WorldToMapCoords( Vector worldPos );

	void AddNodeToMap( IRnLObjective* pNode );
	void AddSpawnAreaToMap( EHANDLE pSpawn );

	void RemoveNodeFromMap( IRnLObjective* pNode );
	void RemoveSpawnAreaFromMap( EHANDLE pSpawn );
	void ClearBrotherArrows( void );

	bool hasEnabledSpawns( C_TriggerCapturePoint* pPoint );
	bool isFallbackActive( C_TriggerCapturePoint* pPoint );

	vgui::ImagePanel	*m_pMapImage;
	CRnLNCOMapSpawnAreaButton	*m_pCaptureZone[MAX_TACT_ENG_NODES];
	CRnLNCOMapSpawnAreaButton	*m_pCaptureZoneButton[MAX_TACT_ENG_NODES];
	vgui::Label			*m_pCaptureZoneCaption[MAX_TACT_ENG_NODES];

	//CompassPanel*		m_pCompassPanel;

	//CRnLNCOMapSpawnAreaButton 	*m_pSpawnAreas[MAX_SPAWN_AREAS];
	CapZonePanel				*m_pZoneIndicator;
	int m_iCapZoneState[6];

	vgui::ImagePanel	*m_pLocalPlayer;

	// Get the map name and set the overview this way
	char mapname[MAX_MAP_NAME];

	char m_szMapMaterial[2][128];
	
	int m_iMapOffset;
	int m_iMapFactor;

	MapToWorldInfo m_mapToWorldInfo[2];

	bool	m_bLoadOverViewData;
	bool	m_bCreateElements;

	CUtlVector<IRnLObjective*> m_hNodes;
	CUtlVector<EHANDLE> m_hSpawnAreas;
	CUtlVector< CBrotherLinkArrow* >m_pBrotherArrows; 

	capZoneData m_CapZones[MAX_TACT_ENG_NODES];

	float m_flNextUpdateTime;
	float m_flDisplayDelay;
	int m_iTeam;
};


#endif // TF_TEAM_H
