#include "cbase.h"
#include "rnl_ncomap.h"
#include "igameresources.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//#define DEBUG_STRATEGIES 1

#define ALLIES_MATERIAL "VGUI/overview/node_allies_control"
#define AXIS_MATERIAL "VGUI/overview/node_axis_control"
#define NEUTRAL_MATERIAL "VGUI/overview/node_grey"

static const char * s_ArrowTypeInfo[] = 
{
	"push",		// ARROW_PUSH,
	"hold",		// ARROW_HOLD,
	"none",		// ARROW_NONE,
	NULL
};

eArrowType AliasToArrowType( const char *alias )
{
	if (alias)
	{
		for( int i=0; s_ArrowTypeInfo[i] != NULL; ++i )
			if (!Q_stricmp( s_ArrowTypeInfo[i], alias ))
				return (eArrowType)i;
	}

	return ARROW_NONE;
}

CBrotherLinkArrow::CBrotherLinkArrow( CRnLNCOMap *pMapParent, vgui::Panel *pParent, int team,  IRnLObjective* pNode, IRnLObjective* pParentNode ) 
	: BaseClass( pParent )
{
	m_pNCOParent = pMapParent;
	m_iTeam = team;
	m_pMainNode = pNode;
	m_pParentNode = pParentNode;

	m_bFinalized = false;
	m_eArrowType = ARROW_PUSH;

	SetProportional( false );
	SetPos( 0, 0 );
	SetSize(ScreenWidth(), ScreenHeight());

	m_Z = 0.0f;

	SetColor(Color(255, 0, 0));
	m_color[3] = 255;
	SetAlpha( 255 );
	SetZ(DEFAULT_ARROW_Z_VERTEX);

	// set the intial arrow size
	m_iInitialWidth = INITIAL_ARROW_WIDTH;
	
	SetMouseInputEnabled( false );
	SetPaintBackgroundEnabled( false );
}

CBrotherLinkArrow::~CBrotherLinkArrow()
{
	// kill teh list of points
	m_aPoints.PurgeAndDeleteElements();
}

void CBrotherLinkArrow::SetPos(int x, int y) 
{ 
	m_bFinalized = false; 
	BaseClass::SetPos(x, y);
}

void CBrotherLinkArrow::SetSize(int wide, int tall)
{ 
	m_bFinalized = false; 
	BaseClass::SetSize(wide, tall);
}

/*
* Converts a point in the panel space to the map space
*
* @param int &iXPos The x coordinate to convert
* @param int &iYPos The y coordinate to convert
* @return void
**/

void CBrotherLinkArrow::PanelToMap(int &iXPos, int &iYPos)
{
	// map is always 1024 so scale up by the fraction of our panel size
	iXPos /= ((float)GetWide() / 1024.0f);
	iYPos /= ((float)GetTall() / 1024.0f);
}

void CBrotherLinkArrow::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);
}	

CBrotherLinkArrow::SStrategicArrowPoint *CBrotherLinkArrow::AddPoint(float x, float y, int width/* = 10*/)
{
	if (m_aPoints.Count() >= MAX_POINTS)
		return NULL;

	SStrategicArrowPoint *pPoint;

	width = clamp(width, MIN_WIDTH, MAX_WIDTH);

	m_bFinalized = false;

	Vector v(x, y, 0.0);
	MapToScreen( v, false );
	pPoint = new SStrategicArrowPoint();
	pPoint->m_iWidth = width;
	pPoint->m_vecPos = v;
	m_aPoints.AddToTail(pPoint);

	return pPoint;
}

CBrotherLinkArrow::SStrategicArrowPoint *CBrotherLinkArrow::AddPoint(Vector &point, int width/* = 10*/)
{
	if (m_aPoints.Count() >= MAX_POINTS)
		return NULL;

	SStrategicArrowPoint *pPoint;

	width = clamp(width, MIN_WIDTH, MAX_WIDTH);

	m_bFinalized = false;

	MapToScreen( point, false );
	pPoint = new SStrategicArrowPoint();
	pPoint->m_vecPos = point;
	pPoint->m_iWidth = width;
	m_aPoints.AddToTail(pPoint);

	return pPoint;
}

/**
* Removes a point from the arrow
*
* @param CBrotherLinkArrow::SStrategicArrowPoint *pPoint The point to remove
* @return void
**/
void CBrotherLinkArrow::RemovePoint(const CBrotherLinkArrow::SStrategicArrowPoint *pPoint)
{
	// see if we can find it
	for(int i = 0; i < m_aPoints.Count(); ++i)
	{
		// is this it?
		if(m_aPoints[i] == pPoint)
		{
			// kill teh point
			delete m_aPoints[i];
			m_aPoints.Remove(i);

			// need to figure out points again
			m_bFinalized = false;

			break;
		}
	}
}

void CBrotherLinkArrow::Finalize()
{
	m_bFinalized = true;
	CalculateTriPoints();
}

void CBrotherLinkArrow::Paint()
{
	if( C_BasePlayer::GetLocalPlayer() && C_BasePlayer::GetLocalPlayer()->GetTeamNumber() != m_iTeam )
		return;

	switch(m_iTeam) {
			case TEAM_ALLIES:
				SetColor(Color(0, 255, 0, 50));
				break;
			case TEAM_AXIS:
			default:
				SetColor(Color(255, 0, 0, 50));
				break;
		}

	if (!m_bFinalized)
		Finalize();

	if( developer.GetBool() && m_aPoints.Count() > 1 )
	{
		Vector pos;
		Vector pos2;
		for( int i = 0; i < m_aPoints.Count() / 2; i++ )
		{
			pos = m_aPoints[i + 1]->m_vecPos;
			pos2 = m_aPoints[i]->m_vecPos;
			surface()->DrawLine( pos.x, pos.y, pos2.x, pos2.y );
		}

		for( int i = 1; i < m_aTriPoints.Count(); i++ )
		{
			vgui::surface()->DrawLine( m_vertArrow[i-1].m_Position.x, m_vertArrow[i-1].m_Position.y, 
				m_vertArrow[i].m_Position.x, m_vertArrow[i].m_Position.y );
		}


		/*for( int i = 1; i < m_aTriPoints.Count(); i += 2 )
		{
			surface()->DrawLine( m_aTriPoints[i-1].x, m_aTriPoints[i-1].y, m_aTriPoints[i].x, m_aTriPoints[i].y );
		}*/
	}

	if (m_aTriPoints.Count() < 3)
		return;

	if( m_iTeam == TEAM_ALLIES )
	{
		if( m_pMainNode && m_pParentNode )
		{
			if( m_pParentNode->GetObjectiveState() == RNL_OBJECTIVE_ALLIED_CONTROLLED )
			{
				surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[3] );
				surface()->DrawSetColor( 0, 255, 0, 255 );
			}
			else
			{
				surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[4] );
				surface()->DrawSetColor( 150, 150, 150, 255 );
			}
			if( m_pParentNode->GetObjectiveState() == RNL_OBJECTIVE_ALLIED_CONTROLLED && m_pMainNode->GetObjectiveState() == RNL_OBJECTIVE_ALLIED_CONTROLLED )
			{
				surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[5] );
				surface()->DrawSetColor( 0, 0, 0, 255 );
			}
		}
		else
		{
			surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[3] );
			surface()->DrawSetColor( 0, 255, 0, 255 );
		}
	}
	else
	{
		if( m_pMainNode && m_pParentNode )
		{
			if( m_pParentNode->GetObjectiveState() == RNL_OBJECTIVE_AXIS_CONTROLLED )
			{
				surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[2] );
				surface()->DrawSetColor( 255, 0, 0, 255 );
			}
			else
			{
				surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[4] );
				surface()->DrawSetColor( 150, 150, 150, 255 );
			}
			if( m_pParentNode->GetObjectiveState() == RNL_OBJECTIVE_AXIS_CONTROLLED && m_pMainNode->GetObjectiveState() == RNL_OBJECTIVE_AXIS_CONTROLLED )
			{
				surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[5] );
				surface()->DrawSetColor( 0, 0, 0, 255 );
			}
		}
		else
		{
			surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[2] );
			surface()->DrawSetColor( 255, 0, 0, 255 );
		}
	}

	vgui::surface()->DrawTexturedPolygon( 3, m_vertArrowBase );
	vgui::surface()->DrawTexturedPolygon( 4, m_vertArrow );
	vgui::surface()->DrawTexturedPolygon( 3, m_vertArrowHead );
}

//void CBrotherLinkArrow::DrawTriPoints()
//{
//	IMaterial *pMaterial = NULL;
//	
//	if( m_iTeam == TEAM_ALLIES )
//	{
//		if( m_pMainNode && m_pParentNode )
//		{
//			if( m_pParentNode->GetObjectiveState() == RNL_OBJECTIVE_ALLIED_CONTROLLED )
//			{
//				pMaterial = materials->FindMaterial( ALLIES_MATERIAL, TEXTURE_GROUP_VGUI );
//				surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[3] );
//				surface()->DrawSetColor( 0, 255, 0, 255 );
//			}
//			else
//			{
//				pMaterial = materials->FindMaterial( NEUTRAL_MATERIAL, TEXTURE_GROUP_VGUI );
//				surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[4] );
//				surface()->DrawSetColor( 150, 150, 150, 255 );
//			}
//		}
//		else
//		{
//			pMaterial = materials->FindMaterial( ALLIES_MATERIAL, TEXTURE_GROUP_VGUI );
//			surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[3] );
//			surface()->DrawSetColor( 0, 255, 0, 255 );
//		}
//	}
//	else
//	{
//		if( m_pMainNode && m_pParentNode )
//		{
//			if( m_pParentNode->GetObjectiveState() == RNL_OBJECTIVE_AXIS_CONTROLLED )
//			{
//				pMaterial = materials->FindMaterial( AXIS_MATERIAL, TEXTURE_GROUP_VGUI );
//				surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[2] );
//				surface()->DrawSetColor( 255, 0, 0, 255 );
//			}
//			else
//			{
//				pMaterial = materials->FindMaterial( NEUTRAL_MATERIAL, TEXTURE_GROUP_VGUI );
//				surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[4] );
//				surface()->DrawSetColor( 150, 150, 150, 255 );
//			}
//		}
//		else
//		{
//			pMaterial = materials->FindMaterial( AXIS_MATERIAL, TEXTURE_GROUP_VGUI );
//			surface()->DrawSetTexture( m_pNCOParent->m_iCapZoneState[2] );
//			surface()->DrawSetColor( 255, 0, 0, 255 );
//		}
//	}
//
//	vgui::Vertex_t points[3];
//	int xOffset, yOffset;
//	GetParent()->GetPos( xOffset, yOffset );
//
//	
//
//#ifdef DEBUG_STRATEGIES
//	vgui::ISurface *s = vgui::surface();
//	s->DrawSetColor(0, 0, 255, 255);
//	for(int i = 0; i < 3; ++i) {
//		Vector v = points[i];
//		//MapToScreen(v, false);
//		s->DrawOutlinedCircle(v.x, v.y, 4, 6);
//	}
//#endif
//
//	vgui::surface()->DrawTexturedPolygon( 3, m_vertArrowBase );
//
////	IMesh *pMesh = materials->GetDynamicMesh(true, NULL, NULL, pMaterial);
////
////	m_MeshBuilder.Begin( pMesh, MATERIAL_TRIANGLE_STRIP, m_aTriPoints.Count() - 2 );
////
////	for(int i = 0; i < m_aTriPoints.Count(); ++i) {
////		m_MeshBuilder.Color4ubv( ucColor );
////		m_MeshBuilder.TexCoord2f(0, i % 2, (i / 2) % 2);	// [0, 0], [1, 0], [0, 1], [1, 1], ...
////		m_MeshBuilder.Position3fv( m_aTriPoints[i].Base() );
////		m_MeshBuilder.AdvanceVertex();
////	}
////
////	m_MeshBuilder.End();
////	pMesh->Draw();
////
////#ifdef DEBUG_STRATEGIES
////	for(int i = 0; i < m_aTriPoints.Count(); i++ ) {
////		vgui::ISurface *s = vgui::surface();
////		Vector v = m_aTriPoints[i];
////		//MapToScreen(v, false);
////		s->DrawSetColor(0, 255, 0, 255);
////		s->DrawOutlinedCircle(v.x, v.y, 4, 6);
////	}
////#endif
//}

//void CBrotherLinkArrow::DrawArrowHead()
//{
//	int xOffset, yOffset;
//	GetParent()->GetPos( xOffset, yOffset );
//
//	Vector points[3];
//	int width = m_aPoints[m_aPoints.Count() - 1]->m_iWidth;
//
//	// perpendicular point 1
//	points[0].x = m_vEndPoint.x + (m_vEndTangent.y * width);
//	points[0].y = m_vEndPoint.y - (m_vEndTangent.x * width);
//	
//	// end of the arrow head
//	points[1].x = m_vEndPoint.x + (m_vEndTangent.x * width);
//	points[1].y = m_vEndPoint.y + (m_vEndTangent.y * width);
//
//	// perpendicular point 2
//	points[2].x = m_vEndPoint.x - (m_vEndTangent.y * width);
//	points[2].y = m_vEndPoint.y + (m_vEndTangent.x * width);
//
//#ifdef DEBUG_STRATEGIES
//	vgui::ISurface *s = vgui::surface();
//	s->DrawSetColor(0, 0, 255, 255);
//	for(int i = 0; i < 3; ++i) {
//		Vector v = points[i];
//		//MapToScreen(v, false);
//		s->DrawOutlinedCircle(v.x, v.y, 4, 6);
//	}
//#endif
//
//	// translate the points to screen coords
//	for(int i = 0; i < 3; ++i) {
//		//MapToScreen(points[i]);
//		points[i].z = m_Z;
//	}
//
//	IMaterial *pMaterial = NULL;
//	if( m_iTeam == TEAM_ALLIES )
//	{
//		if( m_pMainNode && m_pParentNode )
//		{
//			if( m_pParentNode->GetObjectiveState() == RNL_OBJECTIVE_ALLIED_CONTROLLED )
//			{
//				pMaterial = materials->FindMaterial( ALLIES_MATERIAL, TEXTURE_GROUP_VGUI );
//			}
//			else
//			{
//				pMaterial = materials->FindMaterial( NEUTRAL_MATERIAL, TEXTURE_GROUP_VGUI );
//			}
//		}
//		else
//		{
//			pMaterial = materials->FindMaterial( ALLIES_MATERIAL, TEXTURE_GROUP_VGUI );
//		}
//	}
//	else
//	{
//		if( m_pMainNode && m_pParentNode )
//		{
//			if( m_pParentNode->GetObjectiveState() == RNL_OBJECTIVE_AXIS_CONTROLLED )
//			{
//				pMaterial = materials->FindMaterial( AXIS_MATERIAL, TEXTURE_GROUP_VGUI );
//			}
//			else
//			{
//				pMaterial = materials->FindMaterial( NEUTRAL_MATERIAL, TEXTURE_GROUP_VGUI );
//			}
//		}
//		else
//		{
//			pMaterial = materials->FindMaterial( AXIS_MATERIAL, TEXTURE_GROUP_VGUI );
//		}
//	}
//
//	IMesh *pMesh = materials->GetDynamicMesh(true, NULL, NULL, pMaterial);
//
//	m_MeshBuilder.Begin( pMesh, MATERIAL_TRIANGLES, 1 );
//
//	for(int i = 0; i < 3; ++i) {
//		m_MeshBuilder.Color4f( m_color.r() / 255.0f, m_color.g() / 255.0f, m_color.b() / 255.0f, m_color.a() / 255.f );
//		m_MeshBuilder.TexCoord2f(0, i % 2, (i / 2) % 2);
//		m_MeshBuilder.Position3fv( points[i].Base() );
//		m_MeshBuilder.AdvanceVertex();
//	}
//
//	m_MeshBuilder.End();
//	pMesh->Draw();
//}

void CBrotherLinkArrow::CalculateTriPoints()
{
	m_aTriPoints.Purge();

    Vector linePoint, tangentVector, triPoint1, triPoint2;

	for(int i = 0; i < m_aPoints.Count() - 3; i++ ) {
		DevMsg( "Calculating for Vector %f %f %f\n", m_aPoints[i]->m_vecPos.x, m_aPoints[i]->m_vecPos.y, m_aPoints[i]->m_vecPos.z ); 
		//float tStepSize = PIXEL_STEP_SIZE / (aControlPoints[i+2] - aControlPoints[i+1]).Length();
		float tStepSize = 0.1;
		for(float t = 0; t < 1.0; t += tStepSize) {
			// the trinary operator here should save us a few cycles on 
			// lines that do not change width.
            int width = (m_aPoints[i+2]->m_iWidth == m_aPoints[i+1]->m_iWidth ? m_aPoints[i+1]->m_iWidth : ceil(m_aPoints[i+2]->m_iWidth * t + m_aPoints[i+1]->m_iWidth * (1.0 - t)));
			
			// where on the line are we?
			Catmull_Rom_Spline(
				m_aPoints[i]->m_vecPos, 
				m_aPoints[i+1]->m_vecPos, 
				m_aPoints[i+2]->m_vecPos, 
				m_aPoints[i+3]->m_vecPos,
				t, 
				linePoint);

			// what is our tangent at this point?
			Catmull_Rom_Spline_Tangent(
				m_aPoints[i]->m_vecPos, 
				m_aPoints[i+1]->m_vecPos, 
				m_aPoints[i+2]->m_vecPos, 
				m_aPoints[i+3]->m_vecPos, 
				t, 
				tangentVector);

			// normalize tangent
			VectorNormalize(tangentVector);

			// set the two perpendicular points
			triPoint1.Init();
			triPoint2.Init();

			triPoint1.x = linePoint.x - (tangentVector.y * (width / 2.0));
			triPoint1.y = linePoint.y + (tangentVector.x * (width / 2.0));
			triPoint1.z = m_Z;
			
			triPoint2.x = linePoint.x + (tangentVector.y * (width / 2.0));
			triPoint2.y = linePoint.y - (tangentVector.x * (width / 2.0));
			triPoint2.z = m_Z;

			/*DevMsg("--- World Positions: %3f %3f | %3f %3f\n", triPoint1.x, triPoint1.y, triPoint2.x, triPoint2.y);

			MapToScreen(triPoint1);
			MapToScreen(triPoint2);

			DevMsg("--- Map Positions:  %3f %3f | %3f %3f\n", triPoint1.x, triPoint1.y, triPoint2.x, triPoint2.y);*/

			m_aTriPoints.AddToTail(triPoint1);
			m_aTriPoints.AddToTail(triPoint2);
		}
	}

	m_vEndPoint = linePoint;
	m_vEndTangent = tangentVector;

	// perpendicular point 1
	m_vertArrowBase[0].m_TexCoord.x = 0;
	m_vertArrowBase[0].m_TexCoord.y = 0;
	m_vertArrowBase[0].m_Position.x = m_aTriPoints[0].x;
	m_vertArrowBase[0].m_Position.y = m_aTriPoints[0].y;
	
	// end of the arrow head
	m_vertArrowBase[0].m_TexCoord.x = 1;
	m_vertArrowBase[0].m_TexCoord.y = 0;
	m_vertArrowBase[1].m_Position.x =  m_aPoints[0]->m_vecPos.x;
	m_vertArrowBase[1].m_Position.y =  m_aPoints[0]->m_vecPos.y;

	// perpendicular point 2
	m_vertArrowBase[2].m_TexCoord.x = 0;
	m_vertArrowBase[2].m_TexCoord.y = 1;
	m_vertArrowBase[2].m_Position.x = m_aTriPoints[1].x;
	m_vertArrowBase[2].m_Position.y = m_aTriPoints[1].y;

	int width = m_aPoints[m_aPoints.Count() - 1]->m_iWidth;

	// perpendicular point 1
	m_vertArrowHead[0].m_TexCoord.x = 0;
	m_vertArrowHead[0].m_TexCoord.y = 0;
	m_vertArrowHead[0].m_Position.x = m_vEndPoint.x + (m_vEndTangent.y * width);
	m_vertArrowHead[0].m_Position.y = m_vEndPoint.y - (m_vEndTangent.x * width);
	
	// end of the arrow head
	m_vertArrowHead[1].m_TexCoord.x = 0;
	m_vertArrowHead[1].m_TexCoord.y = 1;
	m_vertArrowHead[1].m_Position.x = m_vEndPoint.x + (m_vEndTangent.x * width);
	m_vertArrowHead[1].m_Position.y = m_vEndPoint.y + (m_vEndTangent.y * width);

	// perpendicular point 2
	m_vertArrowHead[2].m_TexCoord.x = 1;
	m_vertArrowHead[2].m_TexCoord.y = 1;
	m_vertArrowHead[2].m_Position.x = m_vEndPoint.x - (m_vEndTangent.y * width);
	m_vertArrowHead[2].m_Position.y = m_vEndPoint.y + (m_vEndTangent.x * width);

	if( m_aTriPoints.Count() > 0 )
	{
		m_vertArrow[0].m_TexCoord.y = 1;
		m_vertArrow[0].m_TexCoord.x = 1;
		m_vertArrow[0].m_Position.x = m_aTriPoints[0].x;
		m_vertArrow[0].m_Position.y = m_aTriPoints[0].y;

		m_vertArrow[1].m_TexCoord.y = 1;
		m_vertArrow[1].m_TexCoord.x = 0;
		m_vertArrow[1].m_Position.x = m_aTriPoints[1].x;
		m_vertArrow[1].m_Position.y = m_aTriPoints[1].y;

		m_vertArrow[2].m_TexCoord.y = 0;
		m_vertArrow[2].m_TexCoord.x = 1;
		m_vertArrow[2].m_Position.x = m_aTriPoints[m_aTriPoints.Count() - 1].x;
		m_vertArrow[2].m_Position.y = m_aTriPoints[m_aTriPoints.Count() - 1].y;

		m_vertArrow[3].m_TexCoord.y = 1;
		m_vertArrow[3].m_TexCoord.x = 0;
		m_vertArrow[3].m_Position.x = m_aTriPoints[m_aTriPoints.Count() - 2].x;
		m_vertArrow[3].m_Position.y = m_aTriPoints[m_aTriPoints.Count() - 2].y;
	}
}

void CBrotherLinkArrow::MapToScreen(Vector &point, bool bDoParent /*= true*/)
{
	Vector2D pos = m_pNCOParent->WorldToMapCoords( point );
	point.x = pos.x;
	point.y = pos.y;
}

void CBrotherLinkArrow::MapToScreen(Vector2D &point, bool bDoParent /*= true*/)
{
	Vector2D pos = m_pNCOParent->WorldToMapCoords( Vector( point.x, point.y, 0 ) );
	point.x = pos.x;
	point.y = pos.y;
}

void CBrotherLinkArrow::OnMousePressed(MouseCode code)
{
	GetParent()->OnMousePressed(code);
}

void CBrotherLinkArrow::OnMouseReleased(MouseCode code)
{
	GetParent()->OnMouseReleased(code);
}

void CBrotherLinkArrow::OnCursorEntered()
{
	GetParent()->OnCursorEntered();
}

void CBrotherLinkArrow::OnCursorExited()
{
	GetParent()->OnCursorExited();
}

/**
* Modifies the width of every point on teh arrow
*
* @param int iIncrement The amount to increment by
* @return void
**/
void CBrotherLinkArrow::ModifyWidth(int iIncrement)
{
	// run through all the points
	for(int i = 0; i < m_aPoints.Count(); ++i)
	{
		// adjust the point
		m_aPoints[i]->m_iWidth = clamp(m_aPoints[i]->m_iWidth + iIncrement, MIN_WIDTH, MAX_WIDTH);
	}

	// increment our initial width
	m_iInitialWidth = clamp(m_iInitialWidth + iIncrement, MIN_WIDTH, MAX_WIDTH);

	// not finalized
	m_bFinalized = false;
}