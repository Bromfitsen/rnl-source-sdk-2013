//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSDK_SCOREBOARD_H
#define CSDK_SCOREBOARD_H
#ifdef _WIN32
#pragma once
#endif

#include <clientscoreboarddialog.h>
#include "rnl_shareddefs.h"

// defined for squad header
#define TYPE_SQUAD			6

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CRnLScoreboard : public CClientScoreBoardDialog
{
private:
	DECLARE_CLASS_SIMPLE(CRnLScoreboard, CClientScoreBoardDialog);
	
public:
	// column widths at 640
	enum { CLASS_WIDTH = 65};
	// total = 340

	CRnLScoreboard(IViewPort *pViewPort);
	~CRnLScoreboard();
	virtual void Update();
	virtual bool NeedsUpdate( void );
	virtual void Reset();
	virtual void ShowPanel( bool bShow );


protected:
	// scoreboard overrides
	virtual void InitScoreboardSections();
	virtual void UpdateTeamInfo();
	virtual bool GetPlayerScoreInfo(int playerIndex, KeyValues *outPlayerInfo);
	virtual void UpdatePlayerInfo();

	// vgui overrides for rounded corner background
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	// sorts players within a section
	static bool StaticPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);

private:
	virtual void AddHeader(); // add the start header of the scoreboard
	virtual int AddSection(int iSectionType, int iTeamNumber = TEAM_INVALID, const char *szSquadName = ""); // add a new section header for a team
	virtual void FillScoreBoard();

	/* Michael Lebson
	Keep track of the team sections, number of players
	on a team, and the team latency for the scoreboard.
	*/
	int			iTeamSections[TEAMS_COUNT];
	int			iNumPlayersOnTeam[TEAMS_COUNT];
	int			iTeamLatency[TEAMS_COUNT];

	vgui::SectionedListPanel* m_pAxisPlayerList;

	// rounded corners
	Color					 m_bgColor;
	Color					 m_borderColor;
};


#endif // CSDK_SCOREBOARD_H
