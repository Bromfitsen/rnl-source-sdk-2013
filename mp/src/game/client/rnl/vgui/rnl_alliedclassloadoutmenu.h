//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef ALLIEDCLASSLOADOUT_H
#define ALLIEDCLASSLOADOUT_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>
#include <vgui_controls/ProgressBar.h>
#include <UtlVector.h>
#include <vgui/ILocalize.h>
#include <vgui/KeyCode.h>
#include <game/client/iviewport.h>

#include "c_rnl_player.h"
#include "mouseoverpanelbutton.h"
#include "rnl_shareddefs.h"

class BitmapImage;

namespace vgui
{
	class TextEntry;
}

//-----------------------------------------------------------------------------
// The Allies class and loadout menu
//-----------------------------------------------------------------------------
class CRnLAlliedClassLoadoutMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CRnLAlliedClassLoadoutMenu, vgui::Frame );

public:
	CRnLAlliedClassLoadoutMenu(IViewPort *pViewPort);
	virtual ~CRnLAlliedClassLoadoutMenu();

	virtual const char *GetName( void ) { return PANEL_ALLIEDCLASSLOADOUT; }
	virtual void SetData(KeyValues *data);
	virtual void Reset() {};
	virtual void Update();
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	void SetLoadoutModel( Vector vecOrigin, QAngle angAngles );
	void SetLoadoutCamera( Vector vecOrigin, QAngle angAngles );
	//run this once per map, unless
	void CreateViewportEntity();
	//use this to just change angles/origin/model/equipment etc
	void UpdateViewportEntity(); // not done yet

	float m_flLastModelUpdateTime;

	// The player loadout model
	C_BaseAnimatingOverlay *m_pLoadoutModel;

	// The currently selected weapon and equipment models
	C_BaseAnimating *m_pLoadoutWeaponModel;
	C_BaseAnimating *m_pLoadoutModelTemp;

	//this is for our loadout viewport - nuke
	Vector vecModelOrigin;
	QAngle angModelAngles;
	Vector vecCameraOrigin;
	QAngle angCameraAngles;

	Vector vecHelmet;

	bool m_bViewportModelCreated;

	void InitializeRiflemanPanel();
	void InitializeMachineGunnerPanel();
	void InitializeRadiomanPanel();
	void InitializeMedicPanel();
	void InitializeNCOPanel();

	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", data );

	MESSAGE_FUNC_PARAMS( OnComboMenuItemHighlight, "ComboMenuItemHighlight", data );

	MESSAGE_FUNC_PARAMS( OnEquipmentItemSelected, "ItemSelected", data);

	int m_iPlayerLoadoutPanel;
	int m_iNextClass;

	//for the item/weapon descriptions
	// if bWeapon is true, the item number pertains to RnLWeaponID enum
	// if it isnt, the item pertains e_equiptype
	void ChangeRiflemanDescription( bool bWeapon, int iItemNumber );
	void ChangeMachineGunnerDescription( bool bWeapon, int iItemNumber );
	void ChangeRadiomanDescription( bool bWeapon, int iItemNumber );
	void ChangeMedicDescription( bool bWeapon, int iItemNumber );
	void ChangeNCODescription( bool bWeapon, int iItemNumber );
	
	void AttachEquipmentToViewPort( int iItemNumber );
	void RemoveEquipmentFromViewPort( int iItemNumber );
	void ResetAttachments();

	bool HasExtraAmmo( void );
	void UpdateGrenadesInViewPort();
	void UpdatePouchesInViewPort();

	void SetModel( C_BaseAnimating *pModel, char cModelName[] );

	void DrawLoadoutModels( void );

	const char *LoadItemDescription( const char *szItemDescription );

	//this stores the itemID for the Garand Bayonet.
	// this is messy but I don't know how else to do it. :( - nuke
	// TODO: MAKE IT GEWD.
	int m_iGarandBayonet[CLASSLOADOUT_ALLIES_MAX];
	int m_iHelmet[CLASSLOADOUT_ALLIES_MAX];
	int m_iColt[CLASSLOADOUT_ALLIES_MAX];
	int m_iFragGrenades[CLASSLOADOUT_ALLIES_MAX];
	int m_iRifleGrenades[CLASSLOADOUT_ALLIES_MAX];
	int m_iExtraAmmo[CLASSLOADOUT_ALLIES_MAX];
	int m_iSmokeGrenades[CLASSLOADOUT_ALLIES_MAX];

	//vgui2 overrides
	virtual void OnKeyCodePressed( vgui::KeyCode code );

	// command callbacks
	void OnCommand( const char *command );

	IViewPort	*m_pViewPort;

	//buttons that select our class, need 4
	vgui::Button *m_pClassButtons[CLASSLOADOUT_ALLIES_MAX];

	//seperate panels for each class - nuke
	vgui::Panel *m_pClassPanel[CLASSLOADOUT_ALLIES_MAX];

	vgui::ComboBox *m_pPrimaryWeaponComboBox[CLASSLOADOUT_ALLIES_MAX];
	//this holds the weight of our current primary weapon
	int m_iPrimaryWeaponWeight[CLASSLOADOUT_ALLIES_MAX];
	//we use this so we can switch back when new weapon is too heavy
	int m_iCurrentPrimaryWeapon[CLASSLOADOUT_ALLIES_MAX];

	vgui::Label *m_pPrimaryWeaponLabel[CLASSLOADOUT_ALLIES_MAX];
	vgui::Label *m_pWeightLabel[CLASSLOADOUT_ALLIES_MAX];
	vgui::Label *m_pEquipmentLabel[CLASSLOADOUT_ALLIES_MAX];
	vgui::Label *m_pErrorLabel[CLASSLOADOUT_ALLIES_MAX];

	vgui::RichText *m_pItemDescription[CLASSLOADOUT_ALLIES_MAX];
	vgui::ImagePanel *m_pItemImage[CLASSLOADOUT_ALLIES_MAX];

	vgui::ImagePanel *m_pBackgroundImage;

	//just used to border the inset
	vgui::Panel		*m_pInsetBorder;

	vgui::Button	*m_pSubmitCommand[CLASSLOADOUT_ALLIES_MAX];

	vgui::Button	*m_pRotateCommand[CLASSLOADOUT_ALLIES_MAX][2];

	vgui::ListPanel *m_pAvailableEquipment[CLASSLOADOUT_ALLIES_MAX];

	//this is not a classpanel, it is the panel you see when you first reach the 
	// loadout screen (no class selected)
	vgui::Panel		*m_pDefaultPanel;
	vgui::Label		*m_pDefaultPanelLabel;

	vgui::Button *m_pAddEquipment[CLASSLOADOUT_ALLIES_MAX];
	vgui::Button *m_pRemoveEquipment[CLASSLOADOUT_ALLIES_MAX];

	vgui::Button *m_pCancelCommand[CLASSLOADOUT_ALLIES_MAX];

	vgui::Button *m_pRotateLoadoutModel[CLASSLOADOUT_ALLIES_MAX];

	vgui::ProgressBar *m_pWeightProgress[CLASSLOADOUT_ALLIES_MAX];

	vgui::Label *m_pWeight[CLASSLOADOUT_ALLIES_MAX];
	int m_iCurrentWeight[CLASSLOADOUT_ALLIES_MAX];
	
	MouseOverPanelButton *m_pFirstButton;

	bool	m_bIsClassPanelVisible;
};

#endif // ALLIEDCLASSLOADOUT_H
