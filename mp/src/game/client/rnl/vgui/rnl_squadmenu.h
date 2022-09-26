//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SQUADMENU_H
#define SQUADMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <game/client/iviewport.h>
#include <igameevents.h>
#include "rnl_shareddefs.h"

#define SLOTS_PER_SQUAD 8
#define MAX_SLOTS 16

static const char *sClassNames[CLASSES_MAX] = 
{
	"Unavailable",
	"Rifleman",
	"Machine Gunner",
	"Radioman",
	"Medic",
	"NCO",
	"Grenadier",
	"MG Schuetze",
	"Funker",
	"Sanitaeter",
	"Offizier",
};

//-----------------------------------------------------------------------------
// Purpose: Squad selection panel
//-----------------------------------------------------------------------------
class CRnLSquadMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CRnLSquadMenu, vgui::Frame );

public:
	CRnLSquadMenu( IViewPort *pViewPort );
	virtual ~CRnLSquadMenu();

	virtual const char *GetName( void ) { return PANEL_SQUADS; }
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

	// command callbacks
	void OnCommand( const char *command );

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void Paint( void );
	virtual void PaintBackground( void );

private:
	// elements
	vgui::Panel *m_pSquadPanels[MAX_SQUADS];
	vgui::Button *m_pSlotButtons[MAX_SLOTS];
	vgui::Label *m_pSquadNames[MAX_SQUADS];
	vgui::Label *m_pSlotClasses[MAX_SLOTS];
	vgui::Label *m_pSlotDescriptions[MAX_SLOTS];
	vgui::Label *m_pSlotPlayerNames[MAX_SLOTS];

	// colors
	Color cPlayerName;
	Color cFree;
	Color cDeactivated;
	Color cClass;
	Color cClassMouseOver;

	// others vars
	IViewPort	*m_pViewPort;
	float		m_fNextUpdateTime;
	bool		m_bSubmitting;
	int			m_nTextureID;
	int			iTeam;
	int			iOldSlot;

	// methods
	void UpdateDependancies();
	void UpdatePlayerInfo();
	void SetButtonCommands();
	void SetLabelText(const char *textEntryName, const char *text);
};


#endif // SQUADMENU_H
