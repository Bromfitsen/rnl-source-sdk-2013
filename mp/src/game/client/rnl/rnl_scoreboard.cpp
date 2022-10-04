//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "rnl_scoreboard.h"
#include "c_rnl_team.h"
#include "c_playerresource.h"
#include "rnl_gamerules.h"
#include "rnl_backgroundpanel.h"
#include "c_rnl_player.h"
#include "c_rnl_game_team.h"
#include "rnl_squad.h"

#include <KeyValues.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVgui.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/SectionedListPanel.h>

using namespace vgui;

extern ConVar sv_showenemyclass;
extern ConVar sv_showenemytickets;
extern ConVar sv_showenemysquads;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLScoreboard::CRnLScoreboard(IViewPort *pViewPort)
	: CClientScoreBoardDialog(pViewPort)
{
	m_pAxisPlayerList = dynamic_cast<vgui::SectionedListPanel*>(FindChildByName("AxisPlayerList", true));
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CRnLScoreboard::~CRnLScoreboard()
{
}

//-----------------------------------------------------------------------------
// Purpose: clears everything in the scoreboard and all it's state
//-----------------------------------------------------------------------------
void CRnLScoreboard::Reset()
{
	// clear Allies
	m_pPlayerList->DeleteAllItems();
	m_pPlayerList->RemoveAllSections();

	// clear Axis
	m_pAxisPlayerList->DeleteAllItems();
	m_pAxisPlayerList->RemoveAllSections();

	m_iSectionId = 0;
	m_fNextUpdateTime = 0;
	// add all the sections
	InitScoreboardSections();
}

//-----------------------------------------------------------------------------
// Purpose: Paint background with rounded corners
//-----------------------------------------------------------------------------
void CRnLScoreboard::PaintBackground()
{
	m_pPlayerList->SetBgColor( Color(0, 0, 0, 0) );
	m_pPlayerList->SetBorder(NULL);
	m_pAxisPlayerList->SetBgColor( Color(0, 0, 0, 0) );
	m_pAxisPlayerList->SetBorder(NULL);

	int iWide, iTall;
	GetSize( iWide, iTall );

	DrawRoundedBackground( m_bgColor, iWide, iTall );
}

//-----------------------------------------------------------------------------
// Purpose: Paint border with rounded corners
//-----------------------------------------------------------------------------
void CRnLScoreboard::PaintBorder()
{
	int iWide, iTall;
	GetSize( iWide, iTall );

	DrawRoundedBorder( m_borderColor, iWide, iTall );
}

//-----------------------------------------------------------------------------
// Purpose: Apply scheme settings
//-----------------------------------------------------------------------------
void CRnLScoreboard::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_bgColor = GetSchemeColor("SectionedListPanel.BgColor", GetBgColor(), pScheme);
	m_borderColor = pScheme->GetColor( "FgColor", Color( 0, 0, 0, 0 ) );

	SetBorder( pScheme->GetBorder( "BaseBorder" ) );
	PostApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLScoreboard::ShowPanel(bool bShow)
{
	if ( bShow )
		Update();

	BaseClass::ShowPanel(bShow);
}

//-----------------------------------------------------------------------------
// Purpose: sets up base sections
//-----------------------------------------------------------------------------
void CRnLScoreboard::InitScoreboardSections()
{
	m_pPlayerList->SetBgColor( Color(0, 0, 0, 0) );
	m_pPlayerList->SetBorder(NULL);
	m_pAxisPlayerList->SetBgColor( Color(0, 0, 0, 0) );
	m_pAxisPlayerList->SetBorder(NULL);

	// fill out the structure of the scoreboard
	AddHeader();

	if ( RnLGameRules() && RnLGameRules()->IsTeamplay() )
	{
		m_iSectionId = 2;

		iTeamSections[TEAM_AXIS]   = AddSection( TYPE_TEAM );
		iTeamSections[TEAM_ALLIES] = iTeamSections[TEAM_AXIS];

		m_pAxisPlayerList->SetSectionAlwaysVisible( iTeamSections[TEAM_AXIS] );
		m_pPlayerList->SetSectionAlwaysVisible( iTeamSections[TEAM_ALLIES] );

		C_RnLPlayer *pPlayer = C_RnLPlayer::GetLocalRnLPlayer();
		char szSquadName[32];
		wchar_t wzSquadName[32];
		
		// Only show the axis squads if you aren't on a team, are on the
		// Axis team, or the server is set to show enemy squads.
		if( sv_showenemysquads.GetBool() || (pPlayer && (pPlayer->GetTeamNumber() < TEAM_ALLIES || pPlayer->GetTeamNumber() == TEAM_AXIS)) )
		{
			g_pVGuiLocalize->ConstructString( wzSquadName, sizeof( wzSquadName ), g_pVGuiLocalize->Find("#RnL_Axis_Squad1"), 0 );
			g_pVGuiLocalize->ConvertUnicodeToANSI( wzSquadName, szSquadName, 32 );
			m_pAxisPlayerList->SetSectionAlwaysVisible( AddSection( TYPE_SQUAD, TEAM_AXIS, szSquadName ) );

			g_pVGuiLocalize->ConstructString( wzSquadName, sizeof( wzSquadName ), g_pVGuiLocalize->Find("#RnL_Axis_Squad1_Fireteam1"), 0 );
			g_pVGuiLocalize->ConvertUnicodeToANSI( wzSquadName, szSquadName, 32 );
			m_pAxisPlayerList->SetSectionAlwaysVisible( AddSection( TYPE_SQUAD, TEAM_AXIS, szSquadName ) );

			g_pVGuiLocalize->ConstructString( wzSquadName, sizeof( wzSquadName ), g_pVGuiLocalize->Find("#RnL_Axis_Squad1_Fireteam2"), 0 );
			g_pVGuiLocalize->ConvertUnicodeToANSI( wzSquadName, szSquadName, 32 );
			m_pAxisPlayerList->SetSectionAlwaysVisible( AddSection( TYPE_SQUAD, TEAM_AXIS, szSquadName ) );

			g_pVGuiLocalize->ConstructString( wzSquadName, sizeof( wzSquadName ), g_pVGuiLocalize->Find("#RnL_Axis_Squad2"), 0 );
			g_pVGuiLocalize->ConvertUnicodeToANSI( wzSquadName, szSquadName, 32 );
			m_pAxisPlayerList->SetSectionAlwaysVisible( AddSection( TYPE_SQUAD, TEAM_AXIS, szSquadName ) );

			g_pVGuiLocalize->ConstructString( wzSquadName, sizeof( wzSquadName ), g_pVGuiLocalize->Find("#RnL_Axis_Squad2_Fireteam1"), 0 );
			g_pVGuiLocalize->ConvertUnicodeToANSI( wzSquadName, szSquadName, 32 );
			m_pAxisPlayerList->SetSectionAlwaysVisible( AddSection( TYPE_SQUAD, TEAM_AXIS, szSquadName ) );

			g_pVGuiLocalize->ConstructString( wzSquadName, sizeof( wzSquadName ), g_pVGuiLocalize->Find("#RnL_Axis_Squad2_Fireteam2"), 0 );
			g_pVGuiLocalize->ConvertUnicodeToANSI( wzSquadName, szSquadName, 32 );
			m_pAxisPlayerList->SetSectionAlwaysVisible( AddSection( TYPE_SQUAD, TEAM_AXIS, szSquadName ) );

			m_iSectionId -= 6;
		}

		// Only show the axis squads if you aren't on a team, are on the
		// Allies team, or the server is set to show enemy squads.
		if( sv_showenemysquads.GetBool() || (pPlayer && pPlayer->GetTeamNumber() <= TEAM_ALLIES) )
		{
			g_pVGuiLocalize->ConstructString( wzSquadName, sizeof( wzSquadName ), g_pVGuiLocalize->Find("#RnL_Allied_Squad1"), 0 );
			g_pVGuiLocalize->ConvertUnicodeToANSI( wzSquadName, szSquadName, 32 );
			m_pPlayerList->SetSectionAlwaysVisible( AddSection( TYPE_SQUAD, TEAM_ALLIES, szSquadName ) );

			g_pVGuiLocalize->ConstructString( wzSquadName, sizeof( wzSquadName ), g_pVGuiLocalize->Find("#RnL_Allied_Squad1_Fireteam1"), 0 );
			g_pVGuiLocalize->ConvertUnicodeToANSI( wzSquadName, szSquadName, 32 );
			m_pPlayerList->SetSectionAlwaysVisible( AddSection( TYPE_SQUAD, TEAM_ALLIES, szSquadName ) );

			g_pVGuiLocalize->ConstructString( wzSquadName, sizeof( wzSquadName ), g_pVGuiLocalize->Find("#RnL_Allied_Squad1_Fireteam2"), 0 );
			g_pVGuiLocalize->ConvertUnicodeToANSI( wzSquadName, szSquadName, 32 );
			m_pPlayerList->SetSectionAlwaysVisible( AddSection( TYPE_SQUAD, TEAM_ALLIES, szSquadName ) );

			g_pVGuiLocalize->ConstructString( wzSquadName, sizeof( wzSquadName ), g_pVGuiLocalize->Find("#RnL_Allied_Squad2"), 0 );
			g_pVGuiLocalize->ConvertUnicodeToANSI( wzSquadName, szSquadName, 32 );
			m_pPlayerList->SetSectionAlwaysVisible( AddSection( TYPE_SQUAD, TEAM_ALLIES, szSquadName ) );

			g_pVGuiLocalize->ConstructString( wzSquadName, sizeof( wzSquadName ), g_pVGuiLocalize->Find("#RnL_Allied_Squad2_Fireteam1"), 0 );
			g_pVGuiLocalize->ConvertUnicodeToANSI( wzSquadName, szSquadName, 32 );
			m_pPlayerList->SetSectionAlwaysVisible( AddSection( TYPE_SQUAD, TEAM_ALLIES, szSquadName ) );

			g_pVGuiLocalize->ConstructString( wzSquadName, sizeof( wzSquadName ), g_pVGuiLocalize->Find("#RnL_Allied_Squad2_Fireteam2"), 0 );
			g_pVGuiLocalize->ConvertUnicodeToANSI( wzSquadName, szSquadName, 32 );
			m_pPlayerList->SetSectionAlwaysVisible( AddSection( TYPE_SQUAD, TEAM_ALLIES, szSquadName ) );
		}
	}
	else
		iTeamSections[TEAM_UNASSIGNED] = AddSection(TYPE_NOTEAM);

	iTeamSections[TEAM_SPECTATOR] = AddSection( TYPE_SPECTATORS );
}

bool CRnLScoreboard::NeedsUpdate( void )
{
	return (m_fNextUpdateTime < gpGlobals->curtime);	
}

//-----------------------------------------------------------------------------
// Purpose: Recalculate the internal scoreboard data
//-----------------------------------------------------------------------------
void CRnLScoreboard::Update( void )
{
	// Reset();
	m_pPlayerList->DeleteAllItems();
	m_pAxisPlayerList->DeleteAllItems();
	
	FillScoreBoard();

	// Grow the scoreboard to fit all the players
	int iWide = GetWide(), iTall = GetTall();

	SetSize(iWide, iTall);
	m_pPlayerList->SetSize(iWide, iTall);
	m_pAxisPlayerList->SetSize(iWide, iTall);

	MoveToCenterOfScreen();

	// update every second
	m_fNextUpdateTime = gpGlobals->curtime + 1.0f; 
}

//-----------------------------------------------------------------------------
// Purpose: resets the scoreboard team info
//-----------------------------------------------------------------------------
void CRnLScoreboard::UpdateTeamInfo()
{
	int iSectionId;
	char chString[256];
	char chPlural[2];
	wchar_t szTeamName[256];
	IGameResources *pGR = GameResources();

	if ( !pGR )
		return;

	wchar_t sz[32];

	for (int i = TEAM_SPECTATOR; i < TEAMS_COUNT; i++)
	{
		iSectionId = iTeamSections[i];

		if (iNumPlayersOnTeam[i] == 1)
			sprintf(chPlural, "");
		else
			sprintf(chPlural, "s");

		sprintf(chString, "%s - (%i player%s)", pGR->GetTeamName(i), iNumPlayersOnTeam[i], chPlural);
		g_pVGuiLocalize->ConvertANSIToUnicode(chString, szTeamName, sizeof(szTeamName));
		if( i == TEAM_AXIS )
			m_pAxisPlayerList->ModifyColumn(iSectionId, "name", szTeamName);
		else
			m_pPlayerList->ModifyColumn(iSectionId, "name", szTeamName);

		if ( iNumPlayersOnTeam[i] > 0 )
			iTeamLatency[i] /= iNumPlayersOnTeam[i];
		else
			iTeamLatency[i] = 0;

		C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

		if( !pLocalPlayer )
			return;

		/*If the local player is on this team
		  or the server allows the player to see
		  enemy tickets, put the amount of remaining
		  tickets on the scoreboard*/
		if( sv_showenemytickets.GetBool() || (i == pLocalPlayer->GetTeamNumber()) )
		{
			int iTeamScore = pGR->GetTeamScore(i);

			if( RnLGameRules() && RnLGameRules()->GetGameManager() )
				iTeamScore = RnLGameRules()->GetGameManager()->GetTicketsRemaining( i );
		
			//just run a sanity check here, i thought it was crashing 
			// the code before but i was wrong. doesnt hurt :p - nuke
			if( iTeamScore > 9999 )
				iTeamScore = 9999;

			swprintf_s(sz, sizeof(sz)-1, L"%i", iTeamScore);
		}
		else
			swprintf_s(sz, sizeof(sz)-1, L"");

		if( i == TEAM_AXIS )
			m_pAxisPlayerList->ModifyColumn(iSectionId, "score", sz);
		else
			m_pPlayerList->ModifyColumn(iSectionId, "score", sz);

		if (iTeamLatency[i] < 1)
		{
			m_pPlayerList->ModifyColumn(iSectionId, "ping", L"");
			m_pAxisPlayerList->ModifyColumn(iSectionId, "ping", L"");
		}
		else
		{
			//limiting this prevents crash if teamLatancy is too large for sz
			if(iTeamLatency[i] > 9999)
				iTeamLatency[i] = 9999;

			swprintf_s(sz, sizeof(sz)-1,  L"%i", iTeamLatency[i]);
			if( i == TEAM_AXIS )
				m_pAxisPlayerList->ModifyColumn(iSectionId, "ping", sz);
			else
				m_pPlayerList->ModifyColumn(iSectionId, "ping", sz);
		}

		iTeamLatency[i] = 0;

		if( i == TEAM_AXIS )
			m_pAxisPlayerList->SetSectionFgColor( iSectionId, pGR->GetTeamColor( i ) );
		else
			m_pPlayerList->SetSectionFgColor( iSectionId, pGR->GetTeamColor( i ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: adds the top header of the scoreboars
//-----------------------------------------------------------------------------
void CRnLScoreboard::AddHeader()
{
	m_iSectionId = 0;
	m_pPlayerList->AddSection( m_iSectionId, "" );
	m_pPlayerList->SetSectionAlwaysVisible( m_iSectionId );
	m_pPlayerList->AddColumnToSection( m_iSectionId, "name", "", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH) );

	m_pAxisPlayerList->AddSection( m_iSectionId, "" );
	m_pAxisPlayerList->SetSectionAlwaysVisible( m_iSectionId );
	m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "name", "", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH) );

	m_iSectionId = 1;
	m_pPlayerList->AddSection( m_iSectionId, "" );
	m_pPlayerList->SetSectionAlwaysVisible( m_iSectionId );

	if ( ShowAvatars() )
		m_pPlayerList->AddColumnToSection( m_iSectionId, "avatar", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_RIGHT, m_iAvatarWidth );

	m_pPlayerList->AddColumnToSection( m_iSectionId, "name", "#PlayerName", 0, scheme()->GetProportionalScaledValueEx( GetScheme(), NAME_WIDTH ) - m_iAvatarWidth );
	m_pPlayerList->AddColumnToSection( m_iSectionId, "score", "#RnL_Tickets", 0, scheme()->GetProportionalScaledValue( SCORE_WIDTH ) );
	m_pPlayerList->AddColumnToSection( m_iSectionId, "class", "#PlayerClass", 0, scheme()->GetProportionalScaledValue( CLASS_WIDTH ) );
	m_pPlayerList->AddColumnToSection( m_iSectionId, "ping", "#PlayerPing", 0, scheme()->GetProportionalScaledValue( PING_WIDTH ) );

	m_pAxisPlayerList->AddSection( m_iSectionId, "" );
	m_pAxisPlayerList->SetSectionAlwaysVisible( m_iSectionId );

	if ( ShowAvatars() )
		m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "avatar", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_RIGHT, m_iAvatarWidth );

	m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "name", "#PlayerName", 0, scheme()->GetProportionalScaledValueEx( GetScheme(), NAME_WIDTH ) - m_iAvatarWidth );
	m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "score", "#RnL_Tickets", 0, scheme()->GetProportionalScaledValue( SCORE_WIDTH ) );
	m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "class", "#PlayerClass", 0, scheme()->GetProportionalScaledValue( CLASS_WIDTH ) );
	m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "ping", "#PlayerPing", 0, scheme()->GetProportionalScaledValue( PING_WIDTH ) );
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new section to the scoreboard (i.e the team header)
//-----------------------------------------------------------------------------
int CRnLScoreboard::AddSection(int iSectionType, int iTeamNumber, const char *szSquadName)
{
	if ( iSectionType == TYPE_TEAM )
	{
 		m_pPlayerList->AddSection( m_iSectionId, "", StaticPlayerSortFunc );

		if ( ShowAvatars() )
			m_pPlayerList->AddColumnToSection( m_iSectionId, "avatar", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_RIGHT, m_iAvatarWidth );

		m_pPlayerList->AddColumnToSection( m_iSectionId, "name", "", 0, scheme()->GetProportionalScaledValueEx( GetScheme(), NAME_WIDTH ) - m_iAvatarWidth );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "score", "", 0, scheme()->GetProportionalScaledValue( SCORE_WIDTH ) );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "class", "", 0, scheme()->GetProportionalScaledValue( CLASS_WIDTH ) );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "ping", "", 0, scheme()->GetProportionalScaledValue( PING_WIDTH ) );

		// set the section to have the team color
		if ( GameResources() )
			m_pPlayerList->SetSectionFgColor( m_iSectionId, GameResources()->GetTeamColor( TEAM_ALLIES ) );

		m_pPlayerList->SetSectionAlwaysVisible( m_iSectionId );

 		m_pAxisPlayerList->AddSection( m_iSectionId, "", StaticPlayerSortFunc );

		if ( ShowAvatars() )
			m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "avatar", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_RIGHT, m_iAvatarWidth );

		m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "name", "", 0, scheme()->GetProportionalScaledValueEx( GetScheme(), NAME_WIDTH ) - m_iAvatarWidth );
		m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "score", "", 0, scheme()->GetProportionalScaledValue( SCORE_WIDTH ) );
		m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "class", "", 0, scheme()->GetProportionalScaledValue( CLASS_WIDTH ) );
		m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "ping", "", 0, scheme()->GetProportionalScaledValue( PING_WIDTH ) );

		// set the section to have the team color
		if ( GameResources() )
			m_pAxisPlayerList->SetSectionFgColor( m_iSectionId, GameResources()->GetTeamColor( TEAM_AXIS ) );

		m_pAxisPlayerList->SetSectionAlwaysVisible( m_iSectionId );
	}
	else if ( iSectionType == TYPE_SQUAD )
	{
		if( iTeamNumber != TEAM_AXIS )
		{
			m_pPlayerList->AddSection( m_iSectionId, "", StaticPlayerSortFunc );

			if ( ShowAvatars() )
				m_pPlayerList->AddColumnToSection( m_iSectionId, "avatar", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_RIGHT, m_iAvatarWidth );
			
			m_pPlayerList->AddColumnToSection( m_iSectionId, "name", szSquadName, 0, scheme()->GetProportionalScaledValueEx( GetScheme(), NAME_WIDTH ) - m_iAvatarWidth );
			m_pPlayerList->AddColumnToSection( m_iSectionId, "score", "", 0, scheme()->GetProportionalScaledValue( SCORE_WIDTH ) );
			m_pPlayerList->AddColumnToSection( m_iSectionId, "class", "", 0, scheme()->GetProportionalScaledValue( CLASS_WIDTH ) );
			m_pPlayerList->AddColumnToSection( m_iSectionId, "ping", "", 0, scheme()->GetProportionalScaledValue( PING_WIDTH ) );

			// set the section to have the team color
			if ( GameResources() )
				m_pPlayerList->SetSectionFgColor( m_iSectionId,  GameResources()->GetTeamColor( iTeamNumber ) );
		}
		else
		{
			m_pAxisPlayerList->AddSection( m_iSectionId, "", StaticPlayerSortFunc );

			if ( ShowAvatars() )
				m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "avatar", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_RIGHT, m_iAvatarWidth );

			m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "name", szSquadName, 0, scheme()->GetProportionalScaledValueEx( GetScheme(), NAME_WIDTH ) - m_iAvatarWidth );
			m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "score", "", 0, scheme()->GetProportionalScaledValue( SCORE_WIDTH ) );
			m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "class", "", 0, scheme()->GetProportionalScaledValue( CLASS_WIDTH ) );
			m_pAxisPlayerList->AddColumnToSection( m_iSectionId, "ping", "", 0, scheme()->GetProportionalScaledValue( PING_WIDTH ) );

			// set the section to have the team color
			if ( GameResources() )
				m_pAxisPlayerList->SetSectionFgColor( m_iSectionId,  GameResources()->GetTeamColor( iTeamNumber ) );
		}
	}

	m_iSectionId++;
	return m_iSectionId - 1;
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new row to the scoreboard, from the playerinfo structure
//-----------------------------------------------------------------------------
bool CRnLScoreboard::GetPlayerScoreInfo(int playerIndex, KeyValues *kv)
{
	IGameResources *pGR = GameResources();

	if (!pGR )
		return false;

	char chName[128];
	sprintf(chName, "      %s", pGR->GetPlayerName( playerIndex ) );

	kv->SetInt("deaths", pGR->GetDeaths( playerIndex ) );
	kv->SetInt("frags", pGR->GetFrags( playerIndex ) );
	kv->SetString("name", chName );
	kv->SetInt("playerIndex", playerIndex );
	kv->SetString( "class", "" );

	C_RnLPlayer *pPlayer = dynamic_cast< C_RnLPlayer* >( UTIL_PlayerByIndex( playerIndex ) );
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	if( !pPlayer || !pLocalPlayer )
		return false;

	C_RnLGameTeam *pTeam = dynamic_cast< C_RnLGameTeam* >( pPlayer->GetTeam() );

	if( !pTeam )
		return false;

	CRnLSquad*pSquad = pTeam->GetSquad( pPlayer->GetSquadNumber() );

	if( !pSquad )
		return false;

	/*If the server wants to show everyone's class,
	or the local player is on the same team as the
	player we are adding, get their kit information*/
	if( sv_showenemyclass.GetBool() || pLocalPlayer->GetTeamNumber() == pTeam->m_iTeamNum )
	{
		if( pTeam->GetKitDescription( pSquad->GetKitDescription( pPlayer->GetKitNumber() ) ).title )
			kv->SetString( "class", pTeam->GetKitDescription( pSquad->GetKitDescription( pPlayer->GetKitNumber() ) ).title );
	}
		

	/*Store the player's squad number, but
	only if they are on a valid team*/
	if( pTeam->m_iTeamNum < TEAM_ALLIES )
		kv->SetInt( "squad", 0 );
	else
		kv->SetInt( "squad", pPlayer->GetSquadNumber() );

	if (pGR->GetPing( playerIndex ) < 1)
	{
		if ( pGR->IsFakePlayer( playerIndex ) )
			kv->SetString("ping", "BOT");
		else
			kv->SetString("ping", "1");
	}
	else
		kv->SetInt("ping", pGR->GetPing( playerIndex ));

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRnLScoreboard::UpdatePlayerInfo()
{
	m_iSectionId = 0; // 0'th row is a header
	int iSelectedRow = -1;

	// walk all the players and make sure they're in the scoreboard
	for( int i = 1; i < gpGlobals->maxClients; i++ )
	{
		IGameResources *pGR = GameResources();

		if( pGR && pGR->IsConnected( i ) )
		{
			// add the player to the list
			KeyValues *playerData = new KeyValues("data");
			GetPlayerScoreInfo( i, playerData );
			UpdatePlayerAvatar( i, playerData );

			const char *oldName = playerData->GetString("name","");
			int bufsize = strlen(oldName) * 2;
			char *newName = (char *)_alloca( bufsize );

			UTIL_MakeSafeName( oldName, newName, bufsize );

			playerData->SetString("name", newName);

			int iPlayerTeam = pGR->GetTeam(i);
			int iItemID = FindItemIDForPlayerIndex( i );

			// put them in the correct scoreboard display
			int iSectionID = iTeamSections[iPlayerTeam];
			C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

			if( sv_showenemysquads.GetBool() || ( pPlayer && ((pPlayer->GetTeamNumber() < TEAM_ALLIES) || (pPlayer->GetTeamNumber() == iPlayerTeam)) ) )
			{
				iSectionID += playerData->GetInt("squad") + 2;

				if( playerData->GetInt("squad") > 1 )
					iSectionID += 1;
			}

			iNumPlayersOnTeam[iPlayerTeam]++;
			iTeamLatency[iPlayerTeam] += playerData->GetInt("ping");
           
			if( pGR->IsLocalPlayer( i ) )
				iSelectedRow = iItemID;

			switch ( iPlayerTeam )
			{
				case TEAM_AXIS:
					if (iItemID == -1)
						// add a new row
						iItemID = m_pAxisPlayerList->AddItem( iSectionID, playerData );
					else
						// modify the current row
						m_pAxisPlayerList->ModifyItem( iItemID, iSectionID, playerData );

					// set the row color based on the players team
					m_pAxisPlayerList->SetItemFgColor( iItemID, pGR->GetTeamColor( iPlayerTeam ) );
					break;
				default:
					if (iItemID == -1)
						// add a new row
						iItemID = m_pPlayerList->AddItem( iSectionID, playerData );
					else
						// modify the current row
						m_pPlayerList->ModifyItem( iItemID, iSectionID, playerData );

					// set the row color based on the players team
					m_pPlayerList->SetItemFgColor( iItemID, pGR->GetTeamColor( iPlayerTeam ) );
					break;
			}

			playerData->deleteThis();
		}
		else if( pGR )
		{
			// remove the player
			int iItemID = FindItemIDForPlayerIndex( i );
			if (iItemID != -1)
			{
				switch ( pGR->GetTeam( i ) )
				{
					case TEAM_AXIS:
						m_pAxisPlayerList->RemoveItem(iItemID);
						break;
					default:
						m_pPlayerList->RemoveItem(iItemID);
						break;
				}
			}
		}
	}

	if ( iSelectedRow != -1 )
	{
		m_pPlayerList->SetSelectedItem( iSelectedRow );
		m_pAxisPlayerList->SetSelectedItem( iSelectedRow );
	}
}

//-----------------------------------------------------------------------------
// Purpose: reload the player list on the scoreboard
//-----------------------------------------------------------------------------
void CRnLScoreboard::FillScoreBoard()
{
    for (int i = TEAM_UNASSIGNED; i < TEAMS_COUNT; i++)
        iNumPlayersOnTeam[i] = 0;

    UpdatePlayerInfo();
    UpdateTeamInfo();
}

//-----------------------------------------------------------------------------
// Purpose: Used for sorting players
//-----------------------------------------------------------------------------
bool CRnLScoreboard::StaticPlayerSortFunc(vgui::SectionedListPanel *list, int iItemID1, int iItemID2)
{
	KeyValues *it1 = list->GetItemData(iItemID1);
	KeyValues *it2 = list->GetItemData(iItemID2);
	Assert(it1 && it2);

	// sort by class
	int v1 = it1->GetInt("class");
	int v2 = it2->GetInt("class");
	if (v1 > v2)
		return true;
	else if (v1 < v2)
		return false;

	// the same, so compare iItemID's (as a sentinel value to get deterministic sorts)
	return iItemID1 < iItemID2;
}