/*********************************
* Resistance and Liberation Loadout UI
*
* Copyright (c) 2008, Blackened Interactive
* Author: Cale "Mazor" Dunlap (cale@blackenedinteractive.com)
* Date: 8/24/2008
*
* Purpose: The squad selection menu (sub-menu from the loadout room)
**********************************/

#ifndef SQUADSELECT_H
#define SQUADSELECT_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <game/client/iviewport.h>
#include <igameevents.h>

class CRnLSquadButton : public vgui::Button
{
public:
	DECLARE_CLASS_SIMPLE( CRnLSquadButton, vgui::Button );

	CRnLSquadButton( vgui::Panel *parent, const char *panelName, const char *text, int squad, Panel *pActionSignalTarget=NULL, const char *pCmd=NULL)
		: vgui::Button( parent, panelName, text, pActionSignalTarget, pCmd )
	{
		m_iSquad = squad;
	}
	CRnLSquadButton( vgui::Panel *parent, const char *panelName, const wchar_t *text, int squad, Panel *pActionSignalTarget=NULL, const char *pCmd=NULL)
		: vgui::Button( parent, panelName, text, pActionSignalTarget, pCmd )
	{
		m_iSquad = squad;
	}

	void OnMousePressed( vgui::MouseCode code )
	{
		KeyValues *data = new KeyValues("SquadButtonPressed");
		data->SetInt( "SquadNumber", m_iSquad );

		PostActionSignal( data );

		BaseClass::OnMousePressed( code );
	}

	int	GetSquadNumber() { return m_iSquad; }

private:
	int	m_iSquad;

};

//-----------------------------------------------------------------------------
// Purpose: Squad selection panel
//-----------------------------------------------------------------------------
class CRnLSquadSelectMenu : public vgui::Frame, public IViewPortPanel, public IGameEventListener2
{
private:
	DECLARE_CLASS_SIMPLE( CRnLSquadSelectMenu, vgui::Frame );

protected:
	// column widths at 640
	enum { NAME_WIDTH = 160, CLASS_WIDTH = 100 };
	// total = 260

public:
	CRnLSquadSelectMenu( IViewPort *pViewPort );
	virtual ~CRnLSquadSelectMenu();

	virtual const char *GetName( void ) { return PANEL_SQUAD_SELECT; }
	virtual void SetData(KeyValues *data);
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate( void );
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	// IGameEventListener interface:
	virtual void FireGameEvent( IGameEvent *event);

	MESSAGE_FUNC_PARAMS( OnSquadListItemSelected, "ItemSelected", data);
	MESSAGE_FUNC_PARAMS( OnSquadButtonPressed, "SquadButtonPressed", data );

	// command callbacks
	void OnCommand( const char *command );

protected:

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	// the two lists, broken into two fireteams
	CUtlVector< vgui::ListPanel*> m_pSquadList;
	CUtlVector< CRnLSquadButton *> m_pSquadButtons;

private:
	inline void	ClearSquadButtonsAndLists();

	IViewPort	*m_pViewPort;
	float		m_fNextUpdateTime;

};


#endif // SQUADSELECT_H
