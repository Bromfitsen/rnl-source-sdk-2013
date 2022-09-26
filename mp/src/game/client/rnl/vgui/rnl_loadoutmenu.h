//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef LOADOUT_H
#define LOADOUT_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <game/client/iviewport.h>
#include <igameevents.h>
#include <vgui_controls/ProgressBar.h>
#include "ienginevgui.h"
#include "model_types.h"
#include "view.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "viewrender.h"
#include "rnl_shareddefs.h"
#include "game_controls/basemodelpanel.h"
#include "rnl_itemselectionpanel.h"

#if 0
#define MAX_WEAPONPANEL_SLOTS 2
#define MAX_APPEARANCEPANEL_SLOTS 2
#define MAX_EQUIPMENTPANEL_SLOTS 10
#define LOADOUT_MAX_ITEMS 4

// Holds all available appearance values for each class (atm faces & helmets)
// remember to terminate each array with -1
static const int iAlliedAppearance[RNL_CLASS_MAX][MAX_APPEARANCEPANEL_SLOTS][LOADOUT_MAX_ITEMS] = {
	{{ 0,-1 },{ 0,-1 }},			// RNL_CLASS_NONE
	{{ 0,-1 },{ 0,1,-1 }},			// ALLIES_NCO
	{{ 1,2,3,-1 },{ 0,1,-1 }},		// ALLIES_RIFLEMAN
	{{ 1,2,3,-1 },{ 0,1,-1 }},		// ALLIES_MACHINEGUNNER
	{{ 1,2,3,-1 },{ 0,1,-1 }},		// ALLIES_RADIOMAN
	{{ 1,2,3,-1 },{ 0,1,-1 }}		// ALLIES_MEDIC
};

static const int iAxisAppearance[RNL_CLASS_MAX][MAX_APPEARANCEPANEL_SLOTS][LOADOUT_MAX_ITEMS] = {
	{{ 0,-1 },{ 0,-1 }},			// RNL_CLASS_NONE
	{{ 0,-1 },{ 3,-1 }},			// AXIS_OFFIZIER
	{{ 1,2,3,-1 },{ 0,1,2,-1 }},	// AXIS_GRENADIER
	{{ 1,2,3,-1 },{ 0,1,2,-1 }},	// AXIS_MGSCHUTZE
	{{ 1,2,3,-1 },{ 0,1,2,-1 }},	// AXIS_FUNKER
	{{ 1,2,3,-1 },{ 0,1,2,-1 }}	// AXIS_SANITAETER
};


// Holds all available equipments for each class
// no termination needed
static const int iAlliedEquipment[RNL_CLASS_MAX][MAX_EQUIPMENTPANEL_SLOTS] = {
	{ EQUIPMENT_ALLIES_NONE },																	// RNL_CLASS_NONE
	{ EQUIPMENT_ALLIES_EXTRAAMMO,EQUIPMENT_ALLIES_FRAGGRENADES,EQUIPMENT_ALLIES_SMOKEGRENADE },	// ALLIES_NCO
	{ EQUIPMENT_ALLIES_EXTRAAMMO,EQUIPMENT_ALLIES_FRAGGRENADES },								// ALLIES_RIFLEMAN
	{ EQUIPMENT_ALLIES_EXTRAAMMO,EQUIPMENT_ALLIES_FRAGGRENADES },								// ALLIES_MACHINEGUNNER
	{ EQUIPMENT_ALLIES_EXTRAAMMO,EQUIPMENT_ALLIES_FRAGGRENADES },								// ALLIES_RADIOMAN
	{ EQUIPMENT_ALLIES_NONE },																	// ALLIES_MEDIC
};

static const int iAxisEquipment[RNL_CLASS_MAX][MAX_EQUIPMENTPANEL_SLOTS] = {
	{ EQUIPMENT_ALLIES_NONE },	
	{ EQUIPMENT_AXIS_EXTRAAMMO,EQUIPMENT_AXIS_STIELHANDGRANATE,EQUIPMENT_AXIS_SMOKEGRENADE },	// AXIS_OFFIZIER
	{ EQUIPMENT_AXIS_EXTRAAMMO,EQUIPMENT_AXIS_STIELHANDGRANATE },								// AXIS_GRENADIER
	{ EQUIPMENT_AXIS_EXTRAAMMO,EQUIPMENT_AXIS_STIELHANDGRANATE },								// AXIS_MGSCHUTZE
	{ EQUIPMENT_AXIS_EXTRAAMMO,EQUIPMENT_AXIS_STIELHANDGRANATE },								// AXIS_FUNKER
	{ EQUIPMENT_AXIS_NONE },																// AXIS_SANITAETER
};

// Holds the default counts for the equipment above (keep same order)
static const int iAlliedEquipmentDefaults[RNL_CLASS_MAX][MAX_EQUIPMENTPANEL_SLOTS] = {
	{ 0 },			// RNL_CLASS_NONE
	{ 1, 2, 1 },	// ALLIES_NCO
	{ 1, 3 },		// ALLIES_RIFLEMAN
	{ 1, 2 },		// ALLIES_MACHINEGUNNER
	{ 1, 2 },		// ALLIES_RADIOMAN
	{ 0 }			// ALLIES_MEDIC
};

// Holds the default counts for the equipment above (keep same order)
static const int iAxisEquipmentDefaults[RNL_CLASS_MAX][MAX_EQUIPMENTPANEL_SLOTS] = {
	{ 0 },			// RNL_CLASS_NONE
	{ 1, 2, 1 },	// AXIS_OFFIZIER
	{ 1, 3 },		// AXIS_GRENADIER
	{ 1, 2 },		// AXIS_MGSCHUTZE
	{ 1, 2 },		// AXIS_FUNKER
	{ 0 }			// AXIS_SANITAETER
	
};

// Each player got 5 slots to put equipment in
static const int iAlliedMaxClassSlots[RNL_CLASS_MAX] = {
	0,	// RNL_CLASS_NONE
	5,	// ALLIES_NCO
	5,	// ALLIES_RIFLEMAN
	5,	// ALLIES_MACHINEGUNNER
	5,	// ALLIES_RADIOMAN
	5	// ALLIES_MEDIC
};

// Each player got 5 slots to put equipment in
static const int iAxisMaxClassSlots[RNL_CLASS_MAX] = {
	0,	// RNL_CLASS_NONE
	5,	// AXIS_OFFIZIER
	5,	// AXIS_GRENADIER
	5,	// AXIS_MGSCHUTZE
	5,	// AXIS_FUNKER
	5	// AXIS_SANITAETER
};

static const char *sAlliedAppearanceNames[MAX_APPEARANCEPANEL_SLOTS][4] = 
{	// Same order as the iAppearance order
	{	
		"Face 1",
		"Face 2",
		"Face 3",
		"Face 4", 
	},
	{	
		"Helmet",
		"Net Helmet",
	},
};

static const char *sAxisAppearanceNames[MAX_APPEARANCEPANEL_SLOTS][4] = 
{	// Same order as the iAppearance order
	{	
		"Face 1",
		"Face 2",
		"Face 3",
		"Face 4", 
	},
	{	
		"Camo Helmet",
		"Helmet",
		"Wire Helmet",
		"Schirmmuetze",
	},
};

static const char *sAlliedEquipmentNames[EQUIPMENT_ALLIES_MAX] = 
{
	"#RnL_weapon_none",
	"#RnL_weapon_colt_m1911",
	"#RnL_weapon_mk2_grenade",
	"#RnL_weapon_rifle_grenade",
	"#RnL_weapon_m3_knife",
	"#RnL_weapon_extra_ammo",
	"#RnL_weapon_helmet",
	"#RnL_weapon_m9_bayonet",
	"#RnL_weapon_m18_grenade",
};

static const char *sAxisEquipmentNames[EQUIPMENT_ALLIES_MAX] = 
{
	"#RnL_weapon_none",
	"#RnL_weapon_walther",
	"#RnL_weapon_granate24",
	"#RnL_weapon_granate_gewehr",
	"#RnL_weapon_feldmesser",
	"#RnL_weapon_extra_ammo",
	"#RnL_weapon_helmet",
	"#RnL_weapon_bayonet",
	"#RnL_weapon_granate39",
};

static const char *sWeaponNames[27] = 
{
	"#RnL_weapon_none",		// WEAPON_NONE
	"#RnL_weapon_bar",
	"#RnL_weapon_browing",
	"#RnL_weapon_colt",
	"#RnL_weapon_grease",
	"#RnL_weapon_carbine",
	"#RnL_weapon_garand",
	"#RnL_weapon_springfield",
	"#RnL_weapon_thompson",
	"#RnL_weapon_m3_knife",
	"#RnL_weapon_mk2_grenade",
	"#RnL_weapon_m18_grenade",
	"#RnL_weapon_granate24",
	"#RnL_weapon_granate39",
	"#RnL_weapon_mp44",
	"#RnL_weapon_walther",
	"#RnL_weapon_k98k",
	"#RnL_weapon_mp40",
	"#RnL_weapon_mg34",
	"#RnL_weapon_mg42",
	"#RnL_weapon_feldmesser",
	"#RnL_weapon_fg42",
	"#RnL_weapon_k43",
	"#RnL_weapon_k98k_scoped",
	"#RnL_weapon_map",
	"#RnL_weapon_dev_cam",
	"#RnL_weapon_fists"
};

static char *sWeaponModelSequences[27] = // WATCH OUT: TYPO --> CRASH
{
	"sprint_upper_k98",	// WEAPON_NONE
	"sprint_upper_m1", // bar
	"sprint_upper_m1", // browning
	"sprint_upper_pistol", // colt
	"sprint_upper_mp40", // grease
	"sprint_upper_m1", // carbine
	"sprint_upper_m1", // garand
	"sprint_upper_m1", // springfield
	"sprint_upper_thompson", // thompson
	"sprint_upper_m1", // M3 Knife
	"sprint_upper_frag", // MK2 Frag Grenade
	"sprint_upper_frag", // M18 Smoke Grenade
	"sprint_upper_stick", // Stielhandgranate 24
	"sprint_upper_stick", // Stielhandgranate 39 (Nebel)
	"sprint_upper_mp40", // Stg44
	"sprint_upper_pistol", // walther
	"sprint_upper_k98", // K98
	"sprint_upper_mp40", // MP40
	"sprint_upper_k98", // MG34
	"sprint_upper_k98", // MG42
	"sprint_upper_k98", // Feldmesser
	"sprint_upper_k98", // FG42
	"sprint_upper_k98", // G42
	"sprint_upper_k98", // Scoped Kar
	"sprint_upper_k98", // NCO Map
	"sprint_upper_k98", // Devcam
	"sprint_upper_k98", // Fists
};

static char *sWeaponModels[27] = 
{
	"",	// WEAPON_NONE
	EQ_PIR_BAR_MODEL,
	EQ_PIR_BROWNING,
	EQ_PIR_COLT,
	EQ_PIR_GREASEGUN,
	EQ_PIR_M1CARBINE,
	EQ_PIR_M1GARAND,
	EQ_PIR_SPRINGFIELD,
	EQ_PIR_THOMPSON,
	"", // M3 Knife
	"", // MK2 Frag Grenade
	"", // M18 Smoke Grenade
	"", // Stielhandgranate 24
	"", // Stielhandgranate 39 (Nebel)
	EQ_HEER_MP44,
	EQ_HEER_WALTHER,
	EQ_HEER_K98,
	EQ_HEER_MP40,
	EQ_HEER_MG34,
	EQ_HEER_MG42,
	"", // Feldmesser
	EQ_HEER_FG42,
	EQ_HEER_G43,
	EQ_HEER_K98SCOPED,
	"", // NCO Map
	"", // Devcam
	"", // Fists
};
#endif

struct LoadoutMenuData
{
	LoadoutMenuData()
	{
		m_iPrimaryWeapon = 0;
		m_iPrimaryAmmoExtra = 0;
		m_iSecondaryWeapon = 0;
		m_iSecondaryAmmoExtra = 0;
		m_iFragGrenades = 0;
		m_iSmokeGrenades = 0;
	}
	int m_iPrimaryWeapon;
	int m_iPrimaryAmmoExtra;
	int m_iSecondaryWeapon;
	int m_iSecondaryAmmoExtra;
	int m_iFragGrenades;
	int m_iSmokeGrenades;
};

//-----------------------------------------------------------------------------
// Purpose: Squad selection panel
//-----------------------------------------------------------------------------
class CRnLLoadoutMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CRnLLoadoutMenu, vgui::Frame );

public:
	CRnLLoadoutMenu( IViewPort *pViewPort );
	virtual ~CRnLLoadoutMenu();

	virtual const char *GetName( void ) { return PANEL_LOADOUT; }
	virtual void SetData(KeyValues *data);
	virtual void Reset() {}
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

	MESSAGE_FUNC_PARAMS( OnSelectionPanelChanged, "SelectionPanelChanged", data );

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	// virtual void Paint( void );
	// virtual void PaintBackground( void );

private:
	// elements
	CModelPanel *m_pPreviewPanel;
	vgui::ProgressBar *m_pWeightBar;

	ItemSelectionPanel	*m_pPrimaryWeapon;
	ItemSelectionPanel	*m_pSecondaryWeapon;
	ItemSelectionPanel	*m_pGrenades1;
	ItemSelectionPanel	*m_pGrenades2;
	ItemSelectionPanel	*m_pExtraPrimaryAmmo;
	ItemSelectionPanel	*m_pExtraSecondaryAmmo;


	// others vars
	IViewPort	*m_pViewPort;
	int			m_nTextureID;
	float		m_flNextUpdateTime;

	void SendLoadoutString();
	void SetDefaultLoadout( int iClass );	// cjd @add
	void SetupPreviewPanel( void );	// cjd @add
	inline const int ComputeTotalLoadoutWeight( void ) const;	// cjd @add

	LoadoutMenuData	LoadoutData;	// cjd @add
	int m_iDisplayTeam; // cjd @add
	int	m_iWeight;
};


#endif // LOADOUT_H
