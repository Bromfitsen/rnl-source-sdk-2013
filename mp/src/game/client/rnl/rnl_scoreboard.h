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
	void Update() OVERRIDE;
	bool NeedsUpdate( void ) OVERRIDE;
	void Reset() OVERRIDE;
	void ShowPanel( bool bShow ) OVERRIDE;


protected:
	// scoreboard overrides
	void InitScoreboardSections() OVERRIDE;
	void UpdateTeamInfo() OVERRIDE;
	bool GetPlayerScoreInfo(int playerIndex, KeyValues *outPlayerInfo) OVERRIDE;
	void UpdatePlayerInfo() OVERRIDE;

	// vgui overrides for rounded corner background
	void PaintBackground() OVERRIDE;
	void PaintBorder() OVERRIDE;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE;

	// sorts players within a section
	static bool StaticPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);

private:
	void AddHeader() OVERRIDE; // add the start header of the scoreboard
	int AddSection(int iSectionType, int iTeamNumber = TEAM_INVALID, const char *szSquadName = ""); // add a new section header for a team
	void FillScoreBoard();

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
