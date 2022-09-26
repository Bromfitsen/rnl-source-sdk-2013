//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTeam class
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_rnl_base_squad.h"
#include "c_rnl_game_team.h"
#include "dt_utlvector_recv.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

int g_iInsertPositions6[][2] = { {20, 20}, {340, 20}, 
								{20, 160}, {340, 160}, 
								{20, 300}, {340, 300 } };

int g_iInsertPositions4[][2] = { {170, 180}, {-10, 180}, 
								{170, 300}, {-10, 300} };

int g_iInsertPositions2[][2] = { {170, 180}, {-10, 180} };

int g_iInsertSize[][2] = { {160,220}, {160,100}, {160,100} };

void SquadGUIGetIdealProportions( int iCurrSquad, int iTotalSquads, int &x, int &y, int &w, int &h )
{
	if( iTotalSquads <= 2 )
	{
		iCurrSquad = min( iCurrSquad, 1 );
		x = g_iInsertPositions2[iCurrSquad][0];
		y = g_iInsertPositions2[iCurrSquad][1];
		w = g_iInsertSize[0][0];
		h = g_iInsertSize[0][1];
	}
	else if( iTotalSquads <= 4 )
	{
		iCurrSquad = min( iCurrSquad, 3 );
		x = g_iInsertPositions4[iCurrSquad][0];
		y = g_iInsertPositions4[iCurrSquad][1];
		w = g_iInsertSize[1][0];
		h = g_iInsertSize[1][1];
	}
	else
	{
		iCurrSquad = min( iCurrSquad, 5 );
		x = g_iInsertPositions6[iCurrSquad][0];
		y = g_iInsertPositions6[iCurrSquad][1];
		w = g_iInsertSize[2][0];
		h = g_iInsertSize[2][1];
	}
}


BEGIN_RECV_TABLE_NOBASE( CSquadSlotInfo, DT_RnLSquadSlotData )
	RecvPropInt( RECVINFO( iKitDesc ) ),
	RecvPropInt( RECVINFO( iMaxCount ) ),
	RecvPropUtlVector( 
		RECVINFO_UTLVECTOR( pMembers ), 
		RNL_SQUAD_SLOTS_MAX,
		RecvPropEHandle(NULL, 0, 0)),
END_RECV_TABLE()


IMPLEMENT_CLIENTCLASS_DT(C_RnLBaseSquad, DT_RnLBaseSquad, CRnLBaseSquad)
	RecvPropEHandle( RECVINFO( m_hParentTeam ) ),
	RecvPropEHandle( RECVINFO( m_hSquadLeader ) ),
	RecvPropUtlVectorDataTable( m_aSlotInfo, RNL_SQUAD_SLOTS_MAX, DT_RnLSquadSlotData ),
END_RECV_TABLE()


BEGIN_PREDICTION_DATA( C_RnLBaseSquad )
END_PREDICTION_DATA();

int C_RnLBaseSquad::GetMemberCount( int iKit )
{
	if( iKit < 0 || iKit >= m_aSlotInfo.Count())
		return 0;

	return m_aSlotInfo[iKit].pMembers.Count();
}

CRnLPlayer* C_RnLBaseSquad::GetMember( int iKit, int idx )
{
	if( iKit < 0 || iKit >= m_aSlotInfo.Count())
		return NULL;

	if( idx < 0 || idx >= m_aSlotInfo[iKit].pMembers.Count() )
		return NULL;

	return m_aSlotInfo[iKit].pMembers[idx];
}

int C_RnLBaseSquad::GetKitMaxCount( int iKit )
{
	if( iKit < 0 || iKit >= m_aSlotInfo.Count())
		return NULL;

	return m_aSlotInfo[iKit].iMaxCount;
}