//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SDK_HUD_CHAT_H
#define SDK_HUD_CHAT_H
#ifdef _WIN32
#pragma once
#endif

#include <hud_basechat.h>


class CHudChatLine : public CBaseHudChatLine
{
	DECLARE_CLASS_SIMPLE( CHudChatLine, CBaseHudChatLine );

public:

	CHudChatLine( vgui::Panel *parent, const char *panelName );

	void	ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
private:
	CHudChatLine( const CHudChatLine & ); // not defined, not accessible
};

//-----------------------------------------------------------------------------
// Purpose: The prompt and text entry area for chat messages
//-----------------------------------------------------------------------------
class CHudChatInputLine : public CBaseHudChatInputLine
{
	DECLARE_CLASS_SIMPLE( CHudChatInputLine, CBaseHudChatInputLine );
	
public:
	CHudChatInputLine( CBaseHudChat *parent, char const *panelName ) : CBaseHudChatInputLine( parent, panelName ) {}

	void	ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
};

class CHudChat : public CBaseHudChat
{
	DECLARE_CLASS_SIMPLE( CHudChat, CBaseHudChat );

public:
	CHudChat( const char *pElementName );

	void	CreateChatInputLine( void ) OVERRIDE;

	void	Init( void ) OVERRIDE;
	void	Reset( void ) OVERRIDE;
	int				GetChatInputOffset( void ) OVERRIDE;
	void			CreateChatLines( void ) OVERRIDE;

	bool	ShouldDraw( void ) OVERRIDE;

	Color	GetTextColorForClient( TextColor colorNum, int clientIndex ) OVERRIDE;
	Color	GetClientColor( int clientIndex ) OVERRIDE;

	int	GetFilterForString( const char *pString ) OVERRIDE;
	bool IsVisible() OVERRIDE;

};

#endif	//SDK_HUD_CHAT_H