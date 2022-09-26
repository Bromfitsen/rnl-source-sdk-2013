#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>

#include "rnl_alliedclassloadoutmenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include "model_types.h"
#include "view.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "viewrender.h"

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/ImagePanel.h>
#include "vgui_BitmapImage.h"

#include "ienginevgui.h"

#include "cdll_util.h"
#include "IGameUIFuncs.h" // for key bindings
#include "animation.h" // for the loadout model anims
extern IGameUIFuncs *gameuifuncs; // for key binding details
#include <game/client/iviewport.h>

#include "engine/IEngineSound.h"

#include <stdlib.h> // MAX_PATH define

float CLASSBUTTON_XPOS[CLASSLOADOUT_ALLIES_MAX] = {
	84,  //CLASSLOADOUT_RIFLEMAN
	160, //CLASSLOADOUT_MACHINEGUNNER
	236, //CLASSLOADOUT_RADIOMAN
	312, //CLASSLOADOUT_MEDIC
	388  //CLASSLOADOUT_NCO
};

#define CLASSBUTTON_YPOS	YRES( 21 )
#define CLASSBUTTON_XSIZE	XRES( 58 )
#define CLASSBUTTON_YSIZE	YRES( 12 )

#include "weapon_rnl_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern g_iLocalPlayerClass;

extern int nSelectionSongGuid;

#define COLUMN_FIXEDSIZE 0x01

#define CLASSPANEL_XSIZE	480
#define CLASSPANEL_YSIZE	420
#define CLASSPANEL_XPOS		80
#define CLASSPANEL_YPOS		40

// camera panel
#define INSET_XSIZE		258
#define INSET_YSIZE		250
#define INSET_XPOS		300
#define INSET_YPOS		54

//combobox
#define PRIMARYWEAP_XPOS		78
#define PRIMARYWEAP_YPOS		15
#define PRIMARYWEAP_XSIZE		133
#define PRIMARYWEAP_YSIZE		15

//labels
#define PRIMARYWEAPL_XPOS		5
#define PRIMARYWEAPL_YPOS		10
#define PRIMARYWEAPL_XSIZE		100
#define PRIMARYWEAPL_YSIZE		20

#define WEIGHTL_XPOS			5
#define WEIGHTL_YPOS			200
#define WEIGHTL_XSIZE			100
#define WEIGHTL_YSIZE			20

#define EQUIPL_XPOS				5
#define EQUIPL_YPOS				45
#define EQUIPL_XSIZE			100
#define EQUIPL_YSIZE			20

#define ERRORLABEL_XPOS			80
#define ERRORLABEL_YPOS			390
#define ERRORLABEL_XSIZE		220
#define ERRORLABEL_YSIZE		20

//listbox equipment
#define AVAILABLEEQUIP_XPOS		8
#define AVAILABLEEQUIP_YPOS		68
#define AVAILABLEEQUIP_XSIZE	202
#define AVAILABLEEQUIP_YSIZE	113

// buttons
#define ADDEQUIP_XPOS			168
#define ADDEQUIP_YPOS			185
#define ADDEQUIP_XSIZE			20
#define ADDEQUIP_YSIZE			20

#define REMEQUIP_XPOS			190
#define REMEQUIP_YPOS			185
#define REMEQUIP_XSIZE			20
#define REMEQUIP_YSIZE			20

#define SUBMIT_XPOS				428
#define SUBMIT_YPOS				397
#define SUBMIT_XSIZE			50
#define SUBMIT_YSIZE			20

#define CANCEL_XPOS				370
#define CANCEL_YPOS				397
#define CANCEL_XSIZE			50
#define CANCEL_YSIZE			20

// weight of equipment
#define WEIGHT_XPOS				30
#define WEIGHT_YPOS				210
#define WEIGHT_XSIZE			180
#define WEIGHT_YSIZE			20

//progress bar
#define WEIGHTPROGRESS_XPOS		8
#define WEIGHTPROGRESS_YPOS		227
#define WEIGHTPROGRESS_XSIZE	204
#define WEIGHTPROGRESS_YSIZE	17

// equipment image panel
#define ITEMIMG_XPOS			1
#define ITEMIMG_YPOS			273
#define ITEMIMG_XSIZE			135
#define ITEMIMG_YSIZE			134

// richtext panel
#define ITEMDESC_XPOS			143
#define ITEMDESC_YPOS			272
#define ITEMDESC_XSIZE			336
#define ITEMDESC_YSIZE			124

//rotation
#define ROTATE_XPOS				370
#define ROTATE_YPOS				5
#define ROTATE_XSIZE			40
#define ROTATE_YSIZE			10

#define ALLIES_PLAYER_PIR_RIFLEMAN "models/player/pir/505_rifleman01.mdl"
#define ALLIES_PLAYER_PIR_NCO "models/player/pir/505_nco01.mdl"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLAlliedClassLoadoutMenu::CRnLAlliedClassLoadoutMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_ALLIEDCLASSLOADOUT)
{
	m_pViewPort = pViewPort;
	m_pFirstButton = NULL;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);
	
	// hide the system buttons
	SetTitleBarVisible( false );
	SetProportional(true);

	SetPos(0, 0);
	SetSize(ScreenWidth(), ScreenHeight());

	m_bViewportModelCreated = false;
	m_flLastModelUpdateTime = 0.0;
	// The player loadout model
	m_pLoadoutModel = NULL;
	// The currently selected weapon and equipment models
	m_pLoadoutWeaponModel = NULL;
	m_pLoadoutModelTemp = NULL;
	m_iNextClass = 0;

	//Set some variables to get the screen size - Stefan
	int m_iSWidth, m_iSHeigth;
	engine->GetScreenSize( m_iSWidth, m_iSHeigth);
	// background image
	m_pBackgroundImage = new ImagePanel(this, "bgimage01");
	m_pBackgroundImage->SetVisible(true);
	m_pBackgroundImage->SetImage("background/loadout01");
	m_pBackgroundImage->SetShouldScaleImage(true);
	m_pBackgroundImage->SetPos( 0, 0 );
	m_pBackgroundImage->SetSize( m_iSWidth, m_iSHeigth );
	
	// Panel that parents all other class controls
	m_pDefaultPanel = new Panel(this, "defaultpanel");
	m_pDefaultPanel->SetPos( XRES( CLASSPANEL_XPOS ), YRES( CLASSPANEL_YPOS ) );
	m_pDefaultPanel->SetSize( XRES( CLASSPANEL_XSIZE ), YRES( CLASSPANEL_YSIZE ));

	m_bIsClassPanelVisible = false;
	
	m_pDefaultPanelLabel = new Label(m_pDefaultPanel, "defpanellabel", "Choose a class from above to begin");
	m_pDefaultPanelLabel->SetPos( 0, 0);
	m_pDefaultPanelLabel->SetSize( XRES( CLASSPANEL_XSIZE ), YRES( CLASSPANEL_YSIZE ));
	m_pDefaultPanelLabel->SetContentAlignment( vgui::Label::a_center );

	m_pInsetBorder = new Panel(this, "insetborder");
	m_pInsetBorder->SetSize( XRES( INSET_XSIZE ), YRES( INSET_YSIZE ) );
	m_pInsetBorder->SetVisible( false );

	// Class choice buttons
	m_pClassButtons[CLASSLOADOUT_RIFLEMAN] = new Button(this, "rifleman", "Rifleman", this, "selectclass 1");
	m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER] = new Button(this, "machinegunner", "MachineGunner", this, "selectclass 2");
	m_pClassButtons[CLASSLOADOUT_RADIOMAN] = new Button(this, "radioman", "Radioman", this, "selectclass 3");
	m_pClassButtons[CLASSLOADOUT_MEDIC] = new Button(this, "medic", "Medic", this, "selectclass 4");
	m_pClassButtons[CLASSLOADOUT_NCO] = new Button(this, "nco", "NCO", this, "selectclass 5");

	m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( CLASSBUTTON_XPOS[CLASSLOADOUT_RIFLEMAN] ), CLASSBUTTON_YPOS);
	m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetSize(CLASSBUTTON_XSIZE, CLASSBUTTON_YSIZE);
	m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetEnabled( false );
	m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetProportional( true );

	m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( CLASSBUTTON_XPOS[CLASSLOADOUT_MACHINEGUNNER] ), CLASSBUTTON_YPOS);
	m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetSize(CLASSBUTTON_XSIZE, CLASSBUTTON_YSIZE);
	m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetEnabled( false );
	m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetProportional( true );

	m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( CLASSBUTTON_XPOS[CLASSLOADOUT_RADIOMAN] ), CLASSBUTTON_YPOS);
	m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetSize(CLASSBUTTON_XSIZE, CLASSBUTTON_YSIZE);
	m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetEnabled( false );
	m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetProportional( true );

	m_pClassButtons[CLASSLOADOUT_MEDIC]->SetPos( XRES( CLASSBUTTON_XPOS[CLASSLOADOUT_MEDIC] ), CLASSBUTTON_YPOS);
	m_pClassButtons[CLASSLOADOUT_MEDIC]->SetSize(CLASSBUTTON_XSIZE, CLASSBUTTON_YSIZE);
	m_pClassButtons[CLASSLOADOUT_MEDIC]->SetEnabled( false );
	m_pClassButtons[CLASSLOADOUT_MEDIC]->SetProportional( true );

	m_pClassButtons[CLASSLOADOUT_NCO]->SetPos( XRES( CLASSBUTTON_XPOS[CLASSLOADOUT_NCO] ), CLASSBUTTON_YPOS);
	m_pClassButtons[CLASSLOADOUT_NCO]->SetSize(CLASSBUTTON_XSIZE, CLASSBUTTON_YSIZE);
	m_pClassButtons[CLASSLOADOUT_NCO]->SetEnabled( false );
	m_pClassButtons[CLASSLOADOUT_NCO]->SetProportional( true );
	
	//initialise our class panels
	InitializeRadiomanPanel();
	InitializeMedicPanel();
	InitializeNCOPanel();
	InitializeRiflemanPanel();
	InitializeMachineGunnerPanel();

	if( m_pClassPanel[CLASSLOADOUT_RIFLEMAN]) 
	{
		m_pClassPanel[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( CLASSPANEL_XPOS ), YRES( CLASSPANEL_YPOS ) );
		m_pClassPanel[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES( CLASSPANEL_XSIZE ), YRES( CLASSPANEL_YSIZE ));
		m_pClassPanel[CLASSLOADOUT_RIFLEMAN]->SetPaintBorderEnabled( true );

		m_pPrimaryWeaponLabel[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( PRIMARYWEAPL_XPOS ), YRES( PRIMARYWEAPL_YPOS ) );
		m_pPrimaryWeaponLabel[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES( PRIMARYWEAPL_XSIZE ), YRES( PRIMARYWEAPL_YSIZE ) );

		m_pWeightLabel[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( WEIGHTL_XPOS ), YRES( WEIGHTL_YPOS ) );
		m_pWeightLabel[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES( WEIGHTL_XSIZE ), YRES( WEIGHTL_YSIZE ) );

		m_pEquipmentLabel[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( EQUIPL_XPOS ), YRES( EQUIPL_YPOS ) );
		m_pEquipmentLabel[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES( EQUIPL_XSIZE ), YRES( EQUIPL_YSIZE ) );

		m_pErrorLabel[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( ERRORLABEL_XPOS ), YRES( ERRORLABEL_YPOS ) );
		m_pErrorLabel[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES( ERRORLABEL_XSIZE ), YRES( ERRORLABEL_YSIZE ) );

		m_pPrimaryWeaponComboBox[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( PRIMARYWEAP_XPOS ), YRES( PRIMARYWEAP_YPOS ) );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES( PRIMARYWEAP_XSIZE ), YRES( PRIMARYWEAP_YSIZE ) );

		m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES(AVAILABLEEQUIP_XPOS), YRES(AVAILABLEEQUIP_YPOS) );
		m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES(AVAILABLEEQUIP_XSIZE), YRES(AVAILABLEEQUIP_YSIZE) );

		m_pAddEquipment[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES(ADDEQUIP_XPOS), YRES(ADDEQUIP_YPOS) );
		m_pAddEquipment[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES(ADDEQUIP_XSIZE), YRES(ADDEQUIP_YSIZE) );

		m_pRemoveEquipment[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES(REMEQUIP_XPOS), YRES(REMEQUIP_YPOS) );
		m_pRemoveEquipment[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES(REMEQUIP_XSIZE), YRES(REMEQUIP_YSIZE) );

		m_pSubmitCommand[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES(SUBMIT_XPOS), YRES(SUBMIT_YPOS) );
		m_pSubmitCommand[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES(SUBMIT_XSIZE), YRES(SUBMIT_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_RIFLEMAN][0]->SetPos( XRES(ROTATE_XPOS), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_RIFLEMAN][0]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_RIFLEMAN][1]->SetPos( XRES(ROTATE_XPOS + (2 * ROTATE_XSIZE)), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_RIFLEMAN][1]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pWeight[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES(WEIGHT_XPOS), YRES(WEIGHT_YPOS) );
		m_pWeight[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES(WEIGHT_XSIZE), YRES(WEIGHT_YSIZE) );

		m_pWeightProgress[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES(WEIGHTPROGRESS_XPOS), YRES(WEIGHTPROGRESS_YPOS) );
		m_pWeightProgress[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES(WEIGHTPROGRESS_XSIZE), YRES(WEIGHTPROGRESS_YSIZE) );

		m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES(ITEMDESC_XPOS), YRES(ITEMDESC_YPOS) );
		m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES(ITEMDESC_XSIZE), YRES(ITEMDESC_YSIZE) );

		m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES(ITEMIMG_XPOS), YRES(ITEMIMG_YPOS) );
		m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES(ITEMIMG_XSIZE), YRES(ITEMIMG_YSIZE) );

		m_pCancelCommand[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES(CANCEL_XPOS), YRES(CANCEL_YPOS) );
		m_pCancelCommand[CLASSLOADOUT_RIFLEMAN]->SetSize( XRES(CANCEL_XSIZE), YRES(CANCEL_YSIZE) );

		m_pInsetBorder->SetSize( XRES(INSET_XSIZE), YRES(INSET_YSIZE) );
		m_pInsetBorder->SetPos( XRES(INSET_XPOS), YRES(INSET_YPOS) );
	}
	if( m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER]) 
	{
		m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( CLASSPANEL_XPOS ), YRES( CLASSPANEL_YPOS ) );
		m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES( CLASSPANEL_XSIZE ), YRES( CLASSPANEL_YSIZE ));
		m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER]->SetPaintBorderEnabled( true );

		m_pPrimaryWeaponLabel[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( PRIMARYWEAPL_XPOS ), YRES( PRIMARYWEAPL_YPOS ) );
		m_pPrimaryWeaponLabel[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES( PRIMARYWEAPL_XSIZE ), YRES( PRIMARYWEAPL_YSIZE ) );

		m_pWeightLabel[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( WEIGHTL_XPOS ), YRES( WEIGHTL_YPOS ) );
		m_pWeightLabel[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES( WEIGHTL_XSIZE ), YRES( WEIGHTL_YSIZE ) );

		m_pEquipmentLabel[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( EQUIPL_XPOS ), YRES( EQUIPL_YPOS ) );
		m_pEquipmentLabel[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES( EQUIPL_XSIZE ), YRES( EQUIPL_YSIZE ) );

		m_pErrorLabel[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( ERRORLABEL_XPOS ), YRES( ERRORLABEL_YPOS ) );
		m_pErrorLabel[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES( ERRORLABEL_XSIZE ), YRES( ERRORLABEL_YSIZE ) );

		m_pPrimaryWeaponComboBox[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( PRIMARYWEAP_XPOS ), YRES( PRIMARYWEAP_YPOS ) );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES( PRIMARYWEAP_XSIZE ), YRES( PRIMARYWEAP_YSIZE ) );

		m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES(AVAILABLEEQUIP_XPOS), YRES(AVAILABLEEQUIP_YPOS) );
		m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES(AVAILABLEEQUIP_XSIZE), YRES(AVAILABLEEQUIP_YSIZE) );

		m_pAddEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES(ADDEQUIP_XPOS), YRES(ADDEQUIP_YPOS) );
		m_pAddEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES(ADDEQUIP_XSIZE), YRES(ADDEQUIP_YSIZE) );

		m_pRemoveEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES(REMEQUIP_XPOS), YRES(REMEQUIP_YPOS) );
		m_pRemoveEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES(REMEQUIP_XSIZE), YRES(REMEQUIP_YSIZE) );

		m_pSubmitCommand[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES(SUBMIT_XPOS), YRES(SUBMIT_YPOS) );
		m_pSubmitCommand[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES(SUBMIT_XSIZE), YRES(SUBMIT_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_MACHINEGUNNER][0]->SetPos( XRES(ROTATE_XPOS), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_MACHINEGUNNER][0]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_MACHINEGUNNER][1]->SetPos( XRES(ROTATE_XPOS + (2 * ROTATE_XSIZE)), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_MACHINEGUNNER][1]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pWeight[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES(WEIGHT_XPOS), YRES(WEIGHT_YPOS) );
		m_pWeight[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES(WEIGHT_XSIZE), YRES(WEIGHT_YSIZE) );

		m_pWeightProgress[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES(WEIGHTPROGRESS_XPOS), YRES(WEIGHTPROGRESS_YPOS) );
		m_pWeightProgress[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES(WEIGHTPROGRESS_XSIZE), YRES(WEIGHTPROGRESS_YSIZE) );

		m_pItemDescription[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES(ITEMDESC_XPOS), YRES(ITEMDESC_YPOS) );
		m_pItemDescription[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES(ITEMDESC_XSIZE), YRES(ITEMDESC_YSIZE) );

		m_pItemImage[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES(ITEMIMG_XPOS), YRES(ITEMIMG_YPOS) );
		m_pItemImage[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES(ITEMIMG_XSIZE), YRES(ITEMIMG_YSIZE) );

		m_pCancelCommand[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES(CANCEL_XPOS), YRES(CANCEL_YPOS) );
		m_pCancelCommand[CLASSLOADOUT_MACHINEGUNNER]->SetSize( XRES(CANCEL_XSIZE), YRES(CANCEL_YSIZE) );

		m_pInsetBorder->SetSize( XRES(INSET_XSIZE), YRES(INSET_YSIZE) );
		m_pInsetBorder->SetPos( XRES(INSET_XPOS), YRES(INSET_YPOS) );
	}
	if( m_pClassPanel[CLASSLOADOUT_RADIOMAN]) 
	{
		m_pClassPanel[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( CLASSPANEL_XPOS ), YRES( CLASSPANEL_YPOS ) );
		m_pClassPanel[CLASSLOADOUT_RADIOMAN]->SetSize( XRES( CLASSPANEL_XSIZE ), YRES( CLASSPANEL_YSIZE ));
		m_pClassPanel[CLASSLOADOUT_RADIOMAN]->SetPaintBorderEnabled( true );

		m_pPrimaryWeaponLabel[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( PRIMARYWEAPL_XPOS ), YRES( PRIMARYWEAPL_YPOS ) );
		m_pPrimaryWeaponLabel[CLASSLOADOUT_RADIOMAN]->SetSize( XRES( PRIMARYWEAPL_XSIZE ), YRES( PRIMARYWEAPL_YSIZE ) );

		m_pWeightLabel[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( WEIGHTL_XPOS ), YRES( WEIGHTL_YPOS ) );
		m_pWeightLabel[CLASSLOADOUT_RADIOMAN]->SetSize( XRES( WEIGHTL_XSIZE ), YRES( WEIGHTL_YSIZE ) );

		m_pEquipmentLabel[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( EQUIPL_XPOS ), YRES( EQUIPL_YPOS ) );
		m_pEquipmentLabel[CLASSLOADOUT_RADIOMAN]->SetSize( XRES( EQUIPL_XSIZE ), YRES( EQUIPL_YSIZE ) );

		m_pErrorLabel[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( ERRORLABEL_XPOS ), YRES( ERRORLABEL_YPOS ) );
		m_pErrorLabel[CLASSLOADOUT_RADIOMAN]->SetSize( XRES( ERRORLABEL_XSIZE ), YRES( ERRORLABEL_YSIZE ) );

		m_pPrimaryWeaponComboBox[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( PRIMARYWEAP_XPOS ), YRES( PRIMARYWEAP_YPOS ) );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_RADIOMAN]->SetSize( XRES( PRIMARYWEAP_XSIZE ), YRES( PRIMARYWEAP_YSIZE ) );

		m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->SetPos( XRES(AVAILABLEEQUIP_XPOS), YRES(AVAILABLEEQUIP_YPOS) );
		m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->SetSize( XRES(AVAILABLEEQUIP_XSIZE), YRES(AVAILABLEEQUIP_YSIZE) );

		m_pAddEquipment[CLASSLOADOUT_RADIOMAN]->SetPos( XRES(ADDEQUIP_XPOS), YRES(ADDEQUIP_YPOS) );
		m_pAddEquipment[CLASSLOADOUT_RADIOMAN]->SetSize( XRES(ADDEQUIP_XSIZE), YRES(ADDEQUIP_YSIZE) );

		m_pRemoveEquipment[CLASSLOADOUT_RADIOMAN]->SetPos( XRES(REMEQUIP_XPOS), YRES(REMEQUIP_YPOS) );
		m_pRemoveEquipment[CLASSLOADOUT_RADIOMAN]->SetSize( XRES(REMEQUIP_XSIZE), YRES(REMEQUIP_YSIZE) );

		m_pSubmitCommand[CLASSLOADOUT_RADIOMAN]->SetPos( XRES(SUBMIT_XPOS), YRES(SUBMIT_YPOS) );
		m_pSubmitCommand[CLASSLOADOUT_RADIOMAN]->SetSize( XRES(SUBMIT_XSIZE), YRES(SUBMIT_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_RADIOMAN][0]->SetPos( XRES(ROTATE_XPOS), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_RADIOMAN][0]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_RADIOMAN][1]->SetPos( XRES(ROTATE_XPOS + (2 * ROTATE_XSIZE)), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_RADIOMAN][1]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pWeight[CLASSLOADOUT_RADIOMAN]->SetPos( XRES(WEIGHT_XPOS), YRES(WEIGHT_YPOS) );
		m_pWeight[CLASSLOADOUT_RADIOMAN]->SetSize( XRES(WEIGHT_XSIZE), YRES(WEIGHT_YSIZE) );

		m_pWeightProgress[CLASSLOADOUT_RADIOMAN]->SetPos( XRES(WEIGHTPROGRESS_XPOS), YRES(WEIGHTPROGRESS_YPOS) );
		m_pWeightProgress[CLASSLOADOUT_RADIOMAN]->SetSize( XRES(WEIGHTPROGRESS_XSIZE), YRES(WEIGHTPROGRESS_YSIZE) );

		m_pItemDescription[CLASSLOADOUT_RADIOMAN]->SetPos( XRES(ITEMDESC_XPOS), YRES(ITEMDESC_YPOS) );
		m_pItemDescription[CLASSLOADOUT_RADIOMAN]->SetSize( XRES(ITEMDESC_XSIZE), YRES(ITEMDESC_YSIZE) );

		m_pItemImage[CLASSLOADOUT_RADIOMAN]->SetPos( XRES(ITEMIMG_XPOS), YRES(ITEMIMG_YPOS) );
		m_pItemImage[CLASSLOADOUT_RADIOMAN]->SetSize( XRES(ITEMIMG_XSIZE), YRES(ITEMIMG_YSIZE) );

		m_pCancelCommand[CLASSLOADOUT_RADIOMAN]->SetPos( XRES(CANCEL_XPOS), YRES(CANCEL_YPOS) );
		m_pCancelCommand[CLASSLOADOUT_RADIOMAN]->SetSize( XRES(CANCEL_XSIZE), YRES(CANCEL_YSIZE) );

		m_pInsetBorder->SetSize( XRES(INSET_XSIZE), YRES(INSET_YSIZE) );
		m_pInsetBorder->SetPos( XRES(INSET_XPOS), YRES(INSET_YPOS) );
	}
	if( m_pClassPanel[CLASSLOADOUT_MEDIC]) 
	{
		m_pClassPanel[CLASSLOADOUT_MEDIC]->SetPos( XRES( CLASSPANEL_XPOS ), YRES( CLASSPANEL_YPOS ) );
		m_pClassPanel[CLASSLOADOUT_MEDIC]->SetSize( XRES( CLASSPANEL_XSIZE ), YRES( CLASSPANEL_YSIZE ));
		m_pClassPanel[CLASSLOADOUT_MEDIC]->SetPaintBorderEnabled( true );

		m_pPrimaryWeaponLabel[CLASSLOADOUT_MEDIC]->SetPos( XRES( PRIMARYWEAPL_XPOS ), YRES( PRIMARYWEAPL_YPOS ) );
		m_pPrimaryWeaponLabel[CLASSLOADOUT_MEDIC]->SetSize( XRES( PRIMARYWEAPL_XSIZE ), YRES( PRIMARYWEAPL_YSIZE ) );

		m_pWeightLabel[CLASSLOADOUT_MEDIC]->SetPos( XRES( WEIGHTL_XPOS ), YRES( WEIGHTL_YPOS ) );
		m_pWeightLabel[CLASSLOADOUT_MEDIC]->SetSize( XRES( WEIGHTL_XSIZE ), YRES( WEIGHTL_YSIZE ) );

		m_pEquipmentLabel[CLASSLOADOUT_MEDIC]->SetPos( XRES( EQUIPL_XPOS ), YRES( EQUIPL_YPOS ) );
		m_pEquipmentLabel[CLASSLOADOUT_MEDIC]->SetSize( XRES( EQUIPL_XSIZE ), YRES( EQUIPL_YSIZE ) );

		m_pErrorLabel[CLASSLOADOUT_MEDIC]->SetPos( XRES( ERRORLABEL_XPOS ), YRES( ERRORLABEL_YPOS ) );
		m_pErrorLabel[CLASSLOADOUT_MEDIC]->SetSize( XRES( ERRORLABEL_XSIZE ), YRES( ERRORLABEL_YSIZE ) );

		m_pPrimaryWeaponComboBox[CLASSLOADOUT_MEDIC]->SetPos( XRES( PRIMARYWEAP_XPOS ), YRES( PRIMARYWEAP_YPOS ) );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_MEDIC]->SetSize( XRES( PRIMARYWEAP_XSIZE ), YRES( PRIMARYWEAP_YSIZE ) );

		m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->SetPos( XRES(AVAILABLEEQUIP_XPOS), YRES(AVAILABLEEQUIP_YPOS) );
		m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->SetSize( XRES(AVAILABLEEQUIP_XSIZE), YRES(AVAILABLEEQUIP_YSIZE) );

		m_pAddEquipment[CLASSLOADOUT_MEDIC]->SetPos( XRES(ADDEQUIP_XPOS), YRES(ADDEQUIP_YPOS) );
		m_pAddEquipment[CLASSLOADOUT_MEDIC]->SetSize( XRES(ADDEQUIP_XSIZE), YRES(ADDEQUIP_YSIZE) );

		m_pRemoveEquipment[CLASSLOADOUT_MEDIC]->SetPos( XRES(REMEQUIP_XPOS), YRES(REMEQUIP_YPOS) );
		m_pRemoveEquipment[CLASSLOADOUT_MEDIC]->SetSize( XRES(REMEQUIP_XSIZE), YRES(REMEQUIP_YSIZE) );

		m_pSubmitCommand[CLASSLOADOUT_MEDIC]->SetPos( XRES(SUBMIT_XPOS), YRES(SUBMIT_YPOS) );
		m_pSubmitCommand[CLASSLOADOUT_MEDIC]->SetSize( XRES(SUBMIT_XSIZE), YRES(SUBMIT_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_MEDIC][0]->SetPos( XRES(ROTATE_XPOS), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_MEDIC][0]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_MEDIC][1]->SetPos( XRES(ROTATE_XPOS + (2 * ROTATE_XSIZE)), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_MEDIC][1]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pWeight[CLASSLOADOUT_MEDIC]->SetPos( XRES(WEIGHT_XPOS), YRES(WEIGHT_YPOS) );
		m_pWeight[CLASSLOADOUT_MEDIC]->SetSize( XRES(WEIGHT_XSIZE), YRES(WEIGHT_YSIZE) );

		m_pWeightProgress[CLASSLOADOUT_MEDIC]->SetPos( XRES(WEIGHTPROGRESS_XPOS), YRES(WEIGHTPROGRESS_YPOS) );
		m_pWeightProgress[CLASSLOADOUT_MEDIC]->SetSize( XRES(WEIGHTPROGRESS_XSIZE), YRES(WEIGHTPROGRESS_YSIZE) );

		m_pItemDescription[CLASSLOADOUT_MEDIC]->SetPos( XRES(ITEMDESC_XPOS), YRES(ITEMDESC_YPOS) );
		m_pItemDescription[CLASSLOADOUT_MEDIC]->SetSize( XRES(ITEMDESC_XSIZE), YRES(ITEMDESC_YSIZE) );

		m_pItemImage[CLASSLOADOUT_MEDIC]->SetPos( XRES(ITEMIMG_XPOS), YRES(ITEMIMG_YPOS) );
		m_pItemImage[CLASSLOADOUT_MEDIC]->SetSize( XRES(ITEMIMG_XSIZE), YRES(ITEMIMG_YSIZE) );

		m_pCancelCommand[CLASSLOADOUT_MEDIC]->SetPos( XRES(CANCEL_XPOS), YRES(CANCEL_YPOS) );
		m_pCancelCommand[CLASSLOADOUT_MEDIC]->SetSize( XRES(CANCEL_XSIZE), YRES(CANCEL_YSIZE) );

		m_pInsetBorder->SetSize( XRES(INSET_XSIZE), YRES(INSET_YSIZE) );
		m_pInsetBorder->SetPos( XRES(INSET_XPOS), YRES(INSET_YPOS) );
	}
	if( m_pClassPanel[CLASSLOADOUT_NCO]) 
	{
		m_pClassPanel[CLASSLOADOUT_NCO]->SetPos( XRES( CLASSPANEL_XPOS ), YRES( CLASSPANEL_YPOS ) );
		m_pClassPanel[CLASSLOADOUT_NCO]->SetSize( XRES( CLASSPANEL_XSIZE ), YRES( CLASSPANEL_YSIZE ));
		m_pClassPanel[CLASSLOADOUT_NCO]->SetPaintBorderEnabled( true );

		m_pPrimaryWeaponLabel[CLASSLOADOUT_NCO]->SetPos( XRES( PRIMARYWEAPL_XPOS ), YRES( PRIMARYWEAPL_YPOS ) );
		m_pPrimaryWeaponLabel[CLASSLOADOUT_NCO]->SetSize( XRES( PRIMARYWEAPL_XSIZE ), YRES( PRIMARYWEAPL_YSIZE ) );

		m_pWeightLabel[CLASSLOADOUT_NCO]->SetPos( XRES( WEIGHTL_XPOS ), YRES( WEIGHTL_YPOS ) );
		m_pWeightLabel[CLASSLOADOUT_NCO]->SetSize( XRES( WEIGHTL_XSIZE ), YRES( WEIGHTL_YSIZE ) );

		m_pEquipmentLabel[CLASSLOADOUT_NCO]->SetPos( XRES( EQUIPL_XPOS ), YRES( EQUIPL_YPOS ) );
		m_pEquipmentLabel[CLASSLOADOUT_NCO]->SetSize( XRES( EQUIPL_XSIZE ), YRES( EQUIPL_YSIZE ) );

		m_pErrorLabel[CLASSLOADOUT_NCO]->SetPos( XRES( ERRORLABEL_XPOS ), YRES( ERRORLABEL_YPOS ) );
		m_pErrorLabel[CLASSLOADOUT_NCO]->SetSize( XRES( ERRORLABEL_XSIZE ), YRES( ERRORLABEL_YSIZE ) );

		m_pPrimaryWeaponComboBox[CLASSLOADOUT_NCO]->SetPos( XRES( PRIMARYWEAP_XPOS ), YRES( PRIMARYWEAP_YPOS ) );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_NCO]->SetSize( XRES( PRIMARYWEAP_XSIZE ), YRES( PRIMARYWEAP_YSIZE ) );

		m_pAvailableEquipment[CLASSLOADOUT_NCO]->SetPos( XRES(AVAILABLEEQUIP_XPOS), YRES(AVAILABLEEQUIP_YPOS) );
		m_pAvailableEquipment[CLASSLOADOUT_NCO]->SetSize( XRES(AVAILABLEEQUIP_XSIZE), YRES(AVAILABLEEQUIP_YSIZE) );

		m_pAddEquipment[CLASSLOADOUT_NCO]->SetPos( XRES(ADDEQUIP_XPOS), YRES(ADDEQUIP_YPOS) );
		m_pAddEquipment[CLASSLOADOUT_NCO]->SetSize( XRES(ADDEQUIP_XSIZE), YRES(ADDEQUIP_YSIZE) );

		m_pRemoveEquipment[CLASSLOADOUT_NCO]->SetPos( XRES(REMEQUIP_XPOS), YRES(REMEQUIP_YPOS) );
		m_pRemoveEquipment[CLASSLOADOUT_NCO]->SetSize( XRES(REMEQUIP_XSIZE), YRES(REMEQUIP_YSIZE) );

		m_pSubmitCommand[CLASSLOADOUT_NCO]->SetPos( XRES(SUBMIT_XPOS), YRES(SUBMIT_YPOS) );
		m_pSubmitCommand[CLASSLOADOUT_NCO]->SetSize( XRES(SUBMIT_XSIZE), YRES(SUBMIT_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_NCO][0]->SetPos( XRES(ROTATE_XPOS), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_NCO][0]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_NCO][1]->SetPos( XRES(ROTATE_XPOS + (2 * ROTATE_XSIZE)), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_NCO][1]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pWeight[CLASSLOADOUT_NCO]->SetPos( XRES(WEIGHT_XPOS), YRES(WEIGHT_YPOS) );
		m_pWeight[CLASSLOADOUT_NCO]->SetSize( XRES(WEIGHT_XSIZE), YRES(WEIGHT_YSIZE) );

		m_pWeightProgress[CLASSLOADOUT_NCO]->SetPos( XRES(WEIGHTPROGRESS_XPOS), YRES(WEIGHTPROGRESS_YPOS) );
		m_pWeightProgress[CLASSLOADOUT_NCO]->SetSize( XRES(WEIGHTPROGRESS_XSIZE), YRES(WEIGHTPROGRESS_YSIZE) );

		m_pItemDescription[CLASSLOADOUT_NCO]->SetPos( XRES(ITEMDESC_XPOS), YRES(ITEMDESC_YPOS) );
		m_pItemDescription[CLASSLOADOUT_NCO]->SetSize( XRES(ITEMDESC_XSIZE), YRES(ITEMDESC_YSIZE) );

		m_pItemImage[CLASSLOADOUT_NCO]->SetPos( XRES(ITEMIMG_XPOS), YRES(ITEMIMG_YPOS) );
		m_pItemImage[CLASSLOADOUT_NCO]->SetSize( XRES(ITEMIMG_XSIZE), YRES(ITEMIMG_YSIZE) );

		m_pCancelCommand[CLASSLOADOUT_NCO]->SetPos( XRES(CANCEL_XPOS), YRES(CANCEL_YPOS) );
		m_pCancelCommand[CLASSLOADOUT_NCO]->SetSize( XRES(CANCEL_XSIZE), YRES(CANCEL_YSIZE) );

		m_pInsetBorder->SetSize( XRES(INSET_XSIZE), YRES(INSET_YSIZE) );
		m_pInsetBorder->SetPos( XRES(INSET_XPOS), YRES(INSET_YPOS) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CRnLAlliedClassLoadoutMenu::~CRnLAlliedClassLoadoutMenu()
{
}

//-----------------------------------------------------------------------------
// Purpose: Called when the user picks a class
//-----------------------------------------------------------------------------
void CRnLAlliedClassLoadoutMenu::OnCommand( const char *command)
{
	//if anything but vguicancel, send it as a client command
	if ( FStrEq( command, "vguicancel" ) )
	{
		Close();
		gViewPortInterface->ShowBackGround( false );
		gViewPortInterface->ShowPanel( PANEL_ALLIEDCLASSLOADOUT, false );
		if ( nSelectionSongGuid != 0 )
		{
			enginesound->StopSoundByGuid( nSelectionSongGuid );
			nSelectionSongGuid = 0;
		}
	}
	else if ( FStrEq( command, "submit" ) )
	{
		if (!m_pPrimaryWeaponComboBox)
		{
			DevMsg("No weapon selected...");
			return;
		}
		
		int iWeaponNumber = 0;

		KeyValues *temp = m_pPrimaryWeaponComboBox[m_iPlayerLoadoutPanel]->GetActiveItemUserData();
		if(temp)
			 iWeaponNumber = temp->GetInt("primaryweapon");

		int iEquipmentAmount[PLAYER_MAX_EQUIPMENTS];

		KeyValues *currentEquipment;
		//the various stuff from the equipment list
		for(int i = 0; i < PLAYER_MAX_EQUIPMENTS; i++)
		{
			currentEquipment = m_pAvailableEquipment[ m_iPlayerLoadoutPanel ]->GetItem( m_pAvailableEquipment[ m_iPlayerLoadoutPanel ]->GetItemIDFromUserData(i) );
			
			if(!currentEquipment)
			{
				iEquipmentAmount[i] = 0;
				continue;
			}

			iEquipmentAmount[i] = currentEquipment->GetInt("carry");
		}

		char szLoadOut[64];
		
		sprintf(szLoadOut, "loadout %i %i %i %i %i %i %i %i %i %i", m_iNextClass, iWeaponNumber, iEquipmentAmount[0], iEquipmentAmount[1], iEquipmentAmount[2], iEquipmentAmount[3], iEquipmentAmount[4], iEquipmentAmount[5], iEquipmentAmount[6], iEquipmentAmount[7]);
		engine->ClientCmd( szLoadOut );
		if ( nSelectionSongGuid != 0 )
		{
			enginesound->StopSoundByGuid( nSelectionSongGuid );
			nSelectionSongGuid = 0;
		}

		//// disable the submit command until loadout is verified
		//m_pSubmitCommand[ m_iPlayerLoadoutPanel ]->SetEnabled( false );
	}
	else if ( FStrEq( command, "add" ) )
	{
		int iCount = m_pAvailableEquipment[ m_iPlayerLoadoutPanel ]->GetSelectedItemsCount();

		KeyValues *itemData;
		for( int i = 0; i < iCount; i++ )
		{
			int j = m_pAvailableEquipment[ m_iPlayerLoadoutPanel ]->GetSelectedItem( i );
			itemData = m_pAvailableEquipment[ m_iPlayerLoadoutPanel ]->GetItem( j );
			
			if( ( m_iCurrentWeight[m_iPlayerLoadoutPanel] + itemData->GetInt("weight") ) <= iMaxAlliedClassSlots[m_iPlayerLoadoutPanel] )
			{
				if(( itemData->GetInt("carry") + 1 )  <= itemData->GetInt("max") )
				{
					// Check whether we got too many grenades, before we add new smoke grenades
					KeyValues *GrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iFragGrenades[m_iPlayerLoadoutPanel]);
					KeyValues *SmokeGrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iSmokeGrenades[m_iPlayerLoadoutPanel]);
					
					// Check whether we got any, the medic doesnt have it - Stefan
					if( SmokeGrenadeItem )
					{
						// If there are too many of that already
						if( GrenadeItem->GetInt("carry") + SmokeGrenadeItem->GetInt("carry") > GrenadeItem->GetInt("max") )
							return;
					}

					// Update the error label, apparantly nothing went wrong
					m_pErrorLabel[ m_iPlayerLoadoutPanel ]->SetText("");
					// Update the the amount of carried items and their weigth
					itemData->SetInt("carry", itemData->GetInt("carry") + 1);
					m_iCurrentWeight[m_iPlayerLoadoutPanel] += itemData->GetInt("weight");
					// and show it as well
					char szWeight[64];
					sprintf(szWeight, "%i / %i", m_iCurrentWeight[m_iPlayerLoadoutPanel], iMaxAlliedClassSlots[m_iPlayerLoadoutPanel]);
					m_pWeight[m_iPlayerLoadoutPanel]->SetText(szWeight);

					//aswell as the label, need to set the progress bar
					m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAlliedClassSlots[m_iPlayerLoadoutPanel] );
				
					// also we need to set the models for the currently selected
					AttachEquipmentToViewPort( j );
					m_pSubmitCommand[ m_iPlayerLoadoutPanel ]->SetEnabled(true);
				}
				// If either that stuff is too heavy or the player is trying to select too many of them
				// Don't add it but warn the player
				else
				{
					m_pErrorLabel[ m_iPlayerLoadoutPanel ]->SetText("Too many of that item.");
				}
			}
			else
			{
				m_pErrorLabel[ m_iPlayerLoadoutPanel ]->SetText("Too much weight.");
			}
		}
	}
	else if ( FStrEq( command, "remove" ) )
	{
		int iCount = m_pAvailableEquipment[ m_iPlayerLoadoutPanel ]->GetSelectedItemsCount();

		KeyValues *itemData;
		for( int i = 0; i < iCount; i++ )
		{
			int j = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetSelectedItem(i);
			itemData = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( j );
			if( itemData->GetInt("carry") > 0 )
			{
				itemData->SetInt("carry", itemData->GetInt("carry") - 1);
				m_iCurrentWeight[m_iPlayerLoadoutPanel] -= itemData->GetInt("weight");
				// Remove the equipment from the loadout model
				RemoveEquipmentFromViewPort( j );
			}
		}

		char szWeight[64];
		sprintf(szWeight, "%i / %i", m_iCurrentWeight[m_iPlayerLoadoutPanel], iMaxAlliedClassSlots[m_iPlayerLoadoutPanel]);
		m_pWeight[m_iPlayerLoadoutPanel]->SetText(szWeight);
		
		//aswell as the label, need to set the progress bar
		m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAlliedClassSlots[m_iPlayerLoadoutPanel] );
		// And reenable the submit button - Stefan
		if( !m_pSubmitCommand[ m_iPlayerLoadoutPanel]->IsEnabled() )
            m_pSubmitCommand[ m_iPlayerLoadoutPanel ]->SetEnabled(false);

		m_pErrorLabel[ m_iPlayerLoadoutPanel ]->SetText("");
	}
	else if( FStrEq( command, "selectclass 1" ) )
	{
		m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetEnabled( false );

		if( m_iNextClass == ALLIES_RIFLEMAN )
			return;

		m_pClassButtons[m_iPlayerLoadoutPanel]->SetEnabled( true );

		m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MEDIC]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_NCO]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

		//hide all panels
		for( int i = 0; i < CLASSLOADOUT_ALLIES_MAX; i++)
		{
			m_pClassPanel[i]->SetVisible( false );
		}
		m_pDefaultPanel->SetVisible( false );

		//this little modification is needed as there are only 4 panels, yet "5" classes (CLASS_NONE) - nuke
		m_iPlayerLoadoutPanel = CLASSLOADOUT_RIFLEMAN;
		m_bIsClassPanelVisible = true;
		m_pInsetBorder->SetVisible( true );
		m_pClassPanel[CLASSLOADOUT_RIFLEMAN]->SetVisible(true);

		if( m_pLoadoutModel )
		{
			m_pLoadoutModel->SetModel( ALLIES_PLAYER_PIR_RIFLEMAN );
			ResetAttachments();
		}

		m_pPrimaryWeaponComboBox[CLASSLOADOUT_RIFLEMAN]->ActivateItemByRow(0);
		ChangeRiflemanDescription( true, WEAPON_M1GARAND );
		m_iNextClass = ALLIES_RIFLEMAN;
		return;
	}
	else if( FStrEq( command, "selectclass 2" ) )
	{
		m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetEnabled( false );

		if( m_iNextClass == ALLIES_MACHINEGUNNER )
			return;

		m_pClassButtons[m_iPlayerLoadoutPanel]->SetEnabled( true );

		m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MEDIC]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_NCO]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

		//hide all panels
		for( int i = 0; i < CLASSLOADOUT_ALLIES_MAX; i++)
		{
			m_pClassPanel[i]->SetVisible( false );
		}
		m_pDefaultPanel->SetVisible( false );

		//this little modification is needed as there are only 4 panels, yet "5" classes (CLASS_NONE) - nuke
		m_iPlayerLoadoutPanel = CLASSLOADOUT_MACHINEGUNNER;
		m_bIsClassPanelVisible = true;
		m_pInsetBorder->SetVisible( true );
		m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER]->SetVisible(true);

		if( m_pLoadoutModel )
		{
			m_pLoadoutModel->SetModel( ALLIES_PLAYER_PIR_RIFLEMAN );
			ResetAttachments();
		}

		m_pPrimaryWeaponComboBox[CLASSLOADOUT_MACHINEGUNNER]->ActivateItemByRow(0);
		ChangeMachineGunnerDescription( true, WEAPON_BROWNING );
		m_iNextClass = ALLIES_MACHINEGUNNER;
		return;
	}
	else if( FStrEq( command, "selectclass 3" ) )
	{
		m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetEnabled( false );

		if( m_iNextClass == ALLIES_RADIOMAN )
			return;

		m_pClassButtons[m_iPlayerLoadoutPanel]->SetEnabled( true );

		m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MEDIC]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_NCO]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

		//hide all panels
		for( int i = 0; i < CLASSLOADOUT_ALLIES_MAX; i++)
		{
			m_pClassPanel[i]->SetVisible( false );
		}
		m_pDefaultPanel->SetVisible( false );

		//this little modification is needed as there are only 4 panels, yet "5" classes (CLASS_NONE) - nuke
		m_iPlayerLoadoutPanel = CLASSLOADOUT_RADIOMAN;
		m_bIsClassPanelVisible = true;
		m_pInsetBorder->SetVisible( true );
		
		m_pClassPanel[CLASSLOADOUT_RADIOMAN]->SetVisible(true);
		if( m_pLoadoutModel )
		{
			m_pLoadoutModel->SetModel( ALLIES_PLAYER_PIR_RIFLEMAN );
			ResetAttachments();
		}

		m_pPrimaryWeaponComboBox[CLASSLOADOUT_RADIOMAN]->ActivateItemByRow(0);
		ChangeRadiomanDescription( true, WEAPON_M1CARBINE );
		m_iNextClass = ALLIES_RADIOMAN;
		return;
	}
	else if( FStrEq( command, "selectclass 4" ) )
	{
		m_pClassButtons[CLASSLOADOUT_MEDIC]->SetEnabled( false );

		if( m_iNextClass == ALLIES_MEDIC )
			return;

		m_pClassButtons[m_iPlayerLoadoutPanel]->SetEnabled( true );

		m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MEDIC]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_NCO]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

		//hide all panels
		for( int i = 0; i < CLASSLOADOUT_ALLIES_MAX; i++)
		{
			m_pClassPanel[i]->SetVisible( false );
		}
		m_pDefaultPanel->SetVisible( false );

		//this little modification is needed as there are only 4 panels, yet "5" classes (CLASS_NONE) - nuke
		m_iPlayerLoadoutPanel = CLASSLOADOUT_MEDIC;
		m_bIsClassPanelVisible = true;
		m_pInsetBorder->SetVisible( true );
		
		m_pClassPanel[CLASSLOADOUT_MEDIC]->SetVisible(true);

		if( m_pLoadoutModel )
		{
			m_pLoadoutModel->SetModel( ALLIES_PLAYER_PIR_RIFLEMAN );
			ResetAttachments();
		}

		m_pPrimaryWeaponComboBox[CLASSLOADOUT_MEDIC]->ActivateItemByRow(0);
		ChangeMedicDescription( false, WEAPON_NONE );
		m_iNextClass = ALLIES_MEDIC;
		return;
	}
	else if( FStrEq( command, "selectclass 5" ) )
	{
		m_pClassButtons[CLASSLOADOUT_NCO]->SetEnabled( false );

		if( m_iNextClass == ALLIES_NCO )
			return;

		m_pClassButtons[m_iPlayerLoadoutPanel]->SetEnabled( true );

		m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MEDIC]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_NCO]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );

		//hide all panels
		for( int i = 0; i < CLASSLOADOUT_ALLIES_MAX; i++)
		{
			m_pClassPanel[i]->SetVisible( false );
		}
		m_pDefaultPanel->SetVisible( false );

		//this little modification is needed as there are only 4 panels, yet "5" classes (CLASS_NONE) - nuke
		m_iPlayerLoadoutPanel = CLASSLOADOUT_NCO;
		m_bIsClassPanelVisible = true;
		m_pInsetBorder->SetVisible( true );
		ResetAttachments();
		
		m_pClassPanel[CLASSLOADOUT_NCO]->SetVisible(true);

		if( m_pLoadoutModel )
		{
			m_pLoadoutModel->SetModel( ALLIES_PLAYER_PIR_NCO );
			ResetAttachments();
		}

		m_pPrimaryWeaponComboBox[CLASSLOADOUT_NCO]->ActivateItemByRow(0);
		ChangeNCODescription( true, WEAPON_THOMPSON );
		m_iNextClass = ALLIES_NCO;
		return;
	}
	else if ( FStrEq( command, "rotateright" ) )
	{
		QAngle newAng = m_pLoadoutModel->GetLocalAngles();
		newAng[YAW] += 45;
		m_pLoadoutModel->SetLocalAngles( newAng );
	}
	else if ( FStrEq( command, "rotateleft" ) )
	{
		QAngle newAng = m_pLoadoutModel->GetLocalAngles();
		newAng[YAW] -= 45;
		m_pLoadoutModel->SetLocalAngles( newAng );
	}
	else
	{	
		engine->ClientCmd( command );
	}
	return;
}

//-----------------------------------------------------------------------------
// Purpose: shows the class menu
//-----------------------------------------------------------------------------
void CRnLAlliedClassLoadoutMenu::ShowPanel(bool bShow)
{
	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( true );

		if( nSelectionSongGuid == 0 )
		{
			enginesound->EmitAmbientSound( "music/Allied_Select.wav", 1.0f, PITCH_NORM, 0, 0.0f );
			nSelectionSongGuid = enginesound->GetGuidForLastSoundEmitted();
		}
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
	}
	
	m_pViewPort->ShowBackGround( bShow );
}


void CRnLAlliedClassLoadoutMenu::SetData(KeyValues *data)
{
	if ( FStrEq( data->GetName(), "loadout" ) )
	{
		//if loadout was a success, hide panel
		if ( data->GetInt("status") & FL_ALL_LOADOUT_SUCCESS )
		{
			if ( nSelectionSongGuid != 0 )
			{
				enginesound->StopSoundByGuid( nSelectionSongGuid );
				nSelectionSongGuid = 0;
			}

			ShowPanel( false );
			//reenable all buttons
			m_pSubmitCommand[m_iPlayerLoadoutPanel]->SetEnabled( true );

			//show/enable the cancel button, the player has chosen their first loadout
			m_pCancelCommand[m_iPlayerLoadoutPanel]->SetVisible( true );
		}
		else
		{
			// even on error, ShowPanel( false );

			//this error string generation should be moved to client side
			m_pErrorLabel[m_iPlayerLoadoutPanel]->SetText( data->GetString("loadout_error"));
			m_pErrorLabel[m_iPlayerLoadoutPanel]->SetPos(50, 400);
			//reenable all buttons
			m_pSubmitCommand[m_iPlayerLoadoutPanel]->SetEnabled( true );
		}
	}
	else if ( FStrEq( data->GetName(), "playerclass" ) ) 
	{
		for( int i = 0; i < CLASSLOADOUT_ALLIES_MAX; i++ )
		{
			m_pClassButtons[i]->SetEnabled( false );
		}

		int avail = data->GetInt( "available" );
		if( avail & FL_RIFLEMAN_AVAILABLE )
		{
			m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetEnabled( true );
		}

		if( avail & FL_MACHINEGUNNER_AVAILABLE )
		{
			m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetEnabled( true );
		}

		if( avail & FL_RADIOMAN_AVAILABLE )
		{
			m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetEnabled( true );
		}

		if( avail & FL_MEDIC_AVAILABLE )
		{
			m_pClassButtons[CLASSLOADOUT_MEDIC]->SetEnabled( true );
		}

		if( avail & FL_NCO_AVAILABLE )
		{
			m_pClassButtons[CLASSLOADOUT_NCO]->SetEnabled( true );
		}

		g_iLocalPlayerClass = data->GetInt( "classnumber", RNL_CLASS_NONE );

		//hide all panels
		for( int i = 0; i < CLASSLOADOUT_ALLIES_MAX; i++)
		{
			m_pClassPanel[i]->SetVisible( false );
		}

		if( data->GetInt( "cancancel", 0 ) )
		{
			for( int i = 0; i < CLASSLOADOUT_ALLIES_MAX; i++ )
			{
				m_pCancelCommand[i]->SetVisible( true );
				m_pCancelCommand[i]->SetEnabled( true );
			}
		}
		else
		{
			for( int i = 0; i < CLASSLOADOUT_ALLIES_MAX; i++ )
			{
				m_pCancelCommand[i]->SetVisible( false );
				m_pCancelCommand[i]->SetEnabled( false );
			}
		}

		m_pDefaultPanel->SetVisible( false );

		//this little modification is needed as there are only 4 panels, yet "5" classes (CLASS_NONE) - nuke
		m_iPlayerLoadoutPanel = g_iLocalPlayerClass - 1;
		m_bIsClassPanelVisible = true;
		m_pInsetBorder->SetVisible( true );

		ResetAttachments();

		m_pClassButtons[m_iPlayerLoadoutPanel]->SetEnabled( false );
		//show only our current panel
		switch( m_iPlayerLoadoutPanel )
		{
			case CLASSLOADOUT_RIFLEMAN:
			{
				m_pClassPanel[CLASSLOADOUT_RIFLEMAN]->SetVisible(true);

				m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MEDIC]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_NCO]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

				if( m_pLoadoutModel )
					m_pLoadoutModel->SetModel( ALLIES_PLAYER_PIR_RIFLEMAN );

				m_pPrimaryWeaponComboBox[CLASSLOADOUT_RIFLEMAN]->ActivateItemByRow(0);
				ChangeRiflemanDescription( true, WEAPON_M1GARAND );
				m_iNextClass = ALLIES_RIFLEMAN;
				break;
			}
			case CLASSLOADOUT_MACHINEGUNNER:
			{
				m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER]->SetVisible(true);

				m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MEDIC]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_NCO]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

				if( m_pLoadoutModel )
					m_pLoadoutModel->SetModel( ALLIES_PLAYER_PIR_RIFLEMAN );

				m_pPrimaryWeaponComboBox[CLASSLOADOUT_MACHINEGUNNER]->ActivateItemByRow(0);
				ChangeMachineGunnerDescription( true, WEAPON_BROWNING );
				m_iNextClass = ALLIES_MACHINEGUNNER;
				break;
			}
			case CLASSLOADOUT_RADIOMAN:
			{
				m_pClassPanel[CLASSLOADOUT_RADIOMAN]->SetVisible(true);

				m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MEDIC]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_NCO]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

				if( m_pLoadoutModel )
					m_pLoadoutModel->SetModel( ALLIES_PLAYER_PIR_RIFLEMAN );

				m_pPrimaryWeaponComboBox[CLASSLOADOUT_RADIOMAN]->ActivateItemByRow(0);
				ChangeRadiomanDescription( true, WEAPON_M1CARBINE );
				m_iNextClass = ALLIES_RADIOMAN;
				break;
			}
			case CLASSLOADOUT_MEDIC:
			{
				m_pClassPanel[CLASSLOADOUT_MEDIC]->SetVisible(true);

				m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MEDIC]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_NCO]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

				if( m_pLoadoutModel )
					m_pLoadoutModel->SetModel( ALLIES_PLAYER_PIR_RIFLEMAN );

				m_pPrimaryWeaponComboBox[CLASSLOADOUT_MEDIC]->ActivateItemByRow(0);
				ChangeMedicDescription( false, WEAPON_NONE );
				m_iNextClass = ALLIES_MEDIC;
				break;
			}
			case CLASSLOADOUT_NCO:
			{
				m_pClassPanel[CLASSLOADOUT_NCO]->SetVisible(true);

				m_pClassButtons[CLASSLOADOUT_RIFLEMAN]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MACHINEGUNNER]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_RADIOMAN]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MEDIC]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_NCO]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );

				if( m_pLoadoutModel )
					m_pLoadoutModel->SetModel( ALLIES_PLAYER_PIR_NCO );

				m_pPrimaryWeaponComboBox[CLASSLOADOUT_NCO]->ActivateItemByRow(0);
				ChangeNCODescription( true, WEAPON_THOMPSON );
				m_iNextClass = ALLIES_NCO;
				break;
			}
		}
	}
}

void CRnLAlliedClassLoadoutMenu::OnKeyCodePressed(KeyCode code)
{
	BaseClass::OnKeyCodePressed( code );
}

void CRnLAlliedClassLoadoutMenu::InitializeRiflemanPanel()
{
	// Their current weight is only the primary weapon. I count the default stuff as zero weight, since it was always there - Stefan
	m_iCurrentWeight[CLASSLOADOUT_RIFLEMAN] = 4;
	m_iPrimaryWeaponWeight[CLASSLOADOUT_RIFLEMAN] = 12;

	m_pClassPanel[CLASSLOADOUT_RIFLEMAN] = new Panel(this, "PanelRifleman");
	m_pClassPanel[CLASSLOADOUT_RIFLEMAN]->SetPos(100, 150);
	m_pClassPanel[CLASSLOADOUT_RIFLEMAN]->SetSize(500, 500);
	m_pClassPanel[CLASSLOADOUT_RIFLEMAN]->SetVisible( false );
	m_pClassPanel[CLASSLOADOUT_RIFLEMAN]->SetPaintBorderEnabled( true );
	
	m_pPrimaryWeaponLabel[CLASSLOADOUT_RIFLEMAN] = new Label(m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "primweaponlbl", "Primary weapon:");

	m_pWeightLabel[CLASSLOADOUT_RIFLEMAN] = new Label(m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "weightlbl", "Weight:");

	m_pEquipmentLabel[CLASSLOADOUT_RIFLEMAN] = new Label(m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "equipmentlbl", "Equipment:");

	m_pErrorLabel[CLASSLOADOUT_RIFLEMAN] = new Label(m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "error", "");
	m_pErrorLabel[CLASSLOADOUT_RIFLEMAN]->SetContentAlignment( vgui::Label::a_east );

	m_pPrimaryWeaponComboBox[CLASSLOADOUT_RIFLEMAN] = new ComboBox(m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "primarycombo", 3, false);
	
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_RIFLEMAN]->AddActionSignalTarget( this );

	//default option is first weapon in list, we set playerloadout here so the TextChange
	// function adds the correct weight for it
	KeyValues *primaryweapon = new KeyValues("primaryweap");
	primaryweapon->SetInt("primaryweapon", WEAPON_M1GARAND);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_M1GARAND]);
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_RIFLEMAN]->AddItem("M1 Garand", primaryweapon);
	
	m_iPlayerLoadoutPanel = CLASSLOADOUT_RIFLEMAN;
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_RIFLEMAN]->ActivateItemByRow(0);

	/*
	primaryweapon->SetInt("primaryweapon", WEAPON_M1CARBINE);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_M1CARBINE]);
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_RIFLEMAN]->AddItem("M1 Carbine", primaryweapon);
	*/

	//primaryweapon->SetInt("primaryweapon", WEAPON_BAR);
	//primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_BAR]);
	//m_pPrimaryWeaponComboBox[CLASSLOADOUT_RIFLEMAN]->AddItem("M1918A2 BAR", primaryweapon);

	m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN] = new ListPanel(m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "availequip", false);
	m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->AddColumnHeader(0, "name", "Equipment name", XRES(100), COLUMN_FIXEDSIZE);
	//m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->AddColumnHeader(1, "weight", "Slots", XRES(30), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->AddColumnHeader(1, "carry", "Carrying", XRES(50), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->AddColumnHeader(2, "max", "Max Carry", XRES(57), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->SetColumnSortable(0, false);
	m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->SetColumnSortable(1, false);
	m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->SetColumnSortable(2, false);
	//m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->SetColumnSortable(3, false);
	m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->AddActionSignalTarget( this );

	KeyValues *equipment = new KeyValues("equipment");
	
	equipment->SetString("name", "1911 Colt");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_COLT]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_COLT]);
	m_iColt[CLASSLOADOUT_RIFLEMAN] = m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->AddItem(equipment, 0, false, false);
	//m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->SetItemVisible(m_iColt[CLASSLOADOUT_RIFLEMAN], false);

	equipment->SetString("name", "Frag grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_FRAGGRENADES]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 2);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_FRAGGRENADES]);
	m_iFragGrenades[CLASSLOADOUT_RIFLEMAN] = m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->AddItem(equipment, 1, false, false);

	equipment->SetString("name", "Rifle grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_RIFLEGRENADES]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_RIFLEGRENADES]);
	m_iRifleGrenades[CLASSLOADOUT_RIFLEMAN]= m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->AddItem(equipment, 2, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->SetItemVisible(m_iRifleGrenades[CLASSLOADOUT_RIFLEMAN], false);

	equipment->SetString("name", "M3 knife");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_MELEE]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_MELEE]);
	m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->AddItem(equipment, 3, false, false);

	equipment->SetString("name", "Extra ammo");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_EXTRAAMMO]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_EXTRAAMMO]);
	m_iExtraAmmo[CLASSLOADOUT_RIFLEMAN] = m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->AddItem(equipment, 4, false, false);

	equipment->SetString("name", "Helmet");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_HELMET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_HELMET]);
	m_iHelmet[CLASSLOADOUT_RIFLEMAN] = m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->AddItem(equipment, 5, false, false);
	
	// The bayonet is shown as carried equipment prop when the garand is selected, but you can neither add nor remove it
	// nor is it being send to the loadout but added by default - Stefan
	equipment->SetString("name", "Bayonet (M1 Garand)");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_GARANDBAYONET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_GARANDBAYONET]);
	m_iGarandBayonet[CLASSLOADOUT_RIFLEMAN] = m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->AddItem(equipment, 6, false, false);

	equipment->SetString("name", "Smoke grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_SMOKEGRENADE]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_SMOKEGRENADE]);
	m_iSmokeGrenades[CLASSLOADOUT_RIFLEMAN] = m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->AddItem(equipment, 7, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->SetItemVisible(m_iSmokeGrenades[CLASSLOADOUT_RIFLEMAN], false);

	m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->SetItemVisible( m_iHelmet[CLASSLOADOUT_RIFLEMAN], false);
	//m_pAvailableEquipment[CLASSLOADOUT_RIFLEMAN]->SetItemVisible( m_iHelmet[CLASSLOADOUT_RIFLEMAN],	false);

	m_pAddEquipment[CLASSLOADOUT_RIFLEMAN] = new Button(m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "add", "+", this, "add");
	m_pAddEquipment[CLASSLOADOUT_RIFLEMAN]->SetContentAlignment( vgui::Label::a_center );

	m_pRemoveEquipment[CLASSLOADOUT_RIFLEMAN] = new Button(m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "remove", "-", this, "remove");
	m_pRemoveEquipment[CLASSLOADOUT_RIFLEMAN]->SetContentAlignment( vgui::Label::a_center );

	m_pSubmitCommand[CLASSLOADOUT_RIFLEMAN] = new Button(m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "cmdsubmit", "Submit", this, "submit");
	m_pSubmitCommand[CLASSLOADOUT_RIFLEMAN]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_RIFLEMAN][0] = new Button(m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "rotateright", ">", this, "rotateright");
	m_pRotateCommand[CLASSLOADOUT_RIFLEMAN][0]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_RIFLEMAN][1] = new Button(m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "rotateleft", "<", this, "rotateleft");
	m_pRotateCommand[CLASSLOADOUT_RIFLEMAN][1]->SetContentAlignment( vgui::Label::a_center );

	m_pCancelCommand[CLASSLOADOUT_RIFLEMAN]= new Button(m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "cmdcancel", "Cancel", this, "vguicancel");
	m_pCancelCommand[CLASSLOADOUT_RIFLEMAN]->SetContentAlignment( vgui::Label::a_center );
	// initialise as false, until player has at least chosen their first loadout
	m_pCancelCommand[CLASSLOADOUT_RIFLEMAN]->SetVisible( false );
	
	char szWeight[64];
	sprintf(szWeight, "%i / %i", m_iCurrentWeight[CLASSLOADOUT_RIFLEMAN], iMaxAlliedClassSlots[CLASSLOADOUT_RIFLEMAN]);
	m_pWeight[CLASSLOADOUT_RIFLEMAN] = new Label(m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "weight", szWeight);
	m_pWeight[CLASSLOADOUT_RIFLEMAN]->MoveToFront();
	m_pWeight[CLASSLOADOUT_RIFLEMAN]->SetContentAlignment( vgui::Label::a_center );

	m_pWeightProgress[CLASSLOADOUT_RIFLEMAN] = new ProgressBar( m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "weightprogress");
	//aswell as the label, need to set the progress bar
	m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAlliedClassSlots[m_iPlayerLoadoutPanel] );
	
	// Item Image Panel
	m_pItemImage[CLASSLOADOUT_RIFLEMAN] = new ImagePanel( m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "itemimage");
	m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetImage( "images/weapons/m1garand" );
	m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetShouldScaleImage( true );
	
	// Description panel
	m_pItemDescription[CLASSLOADOUT_RIFLEMAN] = new RichText( m_pClassPanel[CLASSLOADOUT_RIFLEMAN], "itemdesc");
	m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_default.txt") );
}

void CRnLAlliedClassLoadoutMenu::InitializeMachineGunnerPanel()
{
	// Their current weight is only the primary weapon. I count the default stuff as zero weight, since it was always there - Stefan
	m_iCurrentWeight[CLASSLOADOUT_MACHINEGUNNER] = 4;
	m_iPrimaryWeaponWeight[CLASSLOADOUT_MACHINEGUNNER] = 12;

	m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER] = new Panel(this, "PanelMachineGunner");
	m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER]->SetPos(100, 150);
	m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER]->SetSize(500, 500);
	m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER]->SetVisible( false );
	m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER]->SetPaintBorderEnabled( true );
	
	m_pPrimaryWeaponLabel[CLASSLOADOUT_MACHINEGUNNER] = new Label(m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "primweaponlbl", "Primary weapon:");

	m_pWeightLabel[CLASSLOADOUT_MACHINEGUNNER] = new Label(m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "weightlbl", "Weight:");

	m_pEquipmentLabel[CLASSLOADOUT_MACHINEGUNNER] = new Label(m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "equipmentlbl", "Equipment:");

	m_pErrorLabel[CLASSLOADOUT_MACHINEGUNNER] = new Label(m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "error", "");
	m_pErrorLabel[CLASSLOADOUT_MACHINEGUNNER]->SetContentAlignment( vgui::Label::a_east );

	m_pPrimaryWeaponComboBox[CLASSLOADOUT_MACHINEGUNNER] = new ComboBox(m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "primarycombo", 3, false);
	
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_MACHINEGUNNER]->AddActionSignalTarget( this );

	//default option is first weapon in list, we set playerloadout here so the TextChange
	// function adds the correct weight for it
	KeyValues *primaryweapon = new KeyValues("primaryweap");
	primaryweapon->SetInt("primaryweapon", WEAPON_BROWNING);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_BROWNING]);
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_MACHINEGUNNER]->AddItem("Browning MG", primaryweapon);
	
	m_iPlayerLoadoutPanel = CLASSLOADOUT_MACHINEGUNNER;
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_MACHINEGUNNER]->ActivateItemByRow(0);

	/*
	primaryweapon->SetInt("primaryweapon", WEAPON_M1CARBINE);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_M1CARBINE]);
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_RIFLEMAN]->AddItem("M1 Carbine", primaryweapon);
	*/

	//primaryweapon->SetInt("primaryweapon", WEAPON_BAR);
	//primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_BAR]);
	//m_pPrimaryWeaponComboBox[CLASSLOADOUT_MACHINEGUNNER]->AddItem("M1918A2 BAR", primaryweapon);

	m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER] = new ListPanel(m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "availequip", false);
	m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->AddColumnHeader(0, "name", "Equipment name", XRES(100), COLUMN_FIXEDSIZE);
	//m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->AddColumnHeader(1, "weight", "Slots", XRES(30), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->AddColumnHeader(1, "carry", "Carrying", XRES(50), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->AddColumnHeader(2, "max", "Max Carry", XRES(57), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetColumnSortable(0, false);
	m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetColumnSortable(1, false);
	m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetColumnSortable(2, false);
	//m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetColumnSortable(3, false);
	m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->AddActionSignalTarget( this );

	KeyValues *equipment = new KeyValues("equipment");
	
	equipment->SetString("name", "1911 Colt");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_COLT]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_COLT]);
	m_iColt[CLASSLOADOUT_MACHINEGUNNER] = m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->AddItem(equipment, 0, false, false);
	//m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetItemVisible(m_iColt[CLASSLOADOUT_MACHINEGUNNER], false);

	equipment->SetString("name", "Frag grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_FRAGGRENADES]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 2);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_FRAGGRENADES]);
	m_iFragGrenades[CLASSLOADOUT_MACHINEGUNNER] = m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->AddItem(equipment, 1, false, false);

	equipment->SetString("name", "Rifle grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_RIFLEGRENADES]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_RIFLEGRENADES]);
	m_iRifleGrenades[CLASSLOADOUT_MACHINEGUNNER]= m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->AddItem(equipment, 2, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetItemVisible(m_iRifleGrenades[CLASSLOADOUT_MACHINEGUNNER], false);

	equipment->SetString("name", "M3 knife");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_MELEE]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_MELEE]);
	m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->AddItem(equipment, 3, false, false);

	equipment->SetString("name", "Extra ammo");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_EXTRAAMMO]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_EXTRAAMMO]);
	m_iExtraAmmo[CLASSLOADOUT_MACHINEGUNNER] = m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->AddItem(equipment, 4, false, false);

	equipment->SetString("name", "Helmet");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_HELMET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_HELMET]);
	m_iHelmet[CLASSLOADOUT_MACHINEGUNNER] = m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->AddItem(equipment, 5, false, false);
	
	// The bayonet is shown as carried equipment prop when the garand is selected, but you can neither add nor remove it
	// nor is it being send to the loadout but added by default - Stefan
	equipment->SetString("name", "Bayonet (M1 Garand)");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_GARANDBAYONET]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_GARANDBAYONET]);
	m_iGarandBayonet[CLASSLOADOUT_MACHINEGUNNER] = m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->AddItem(equipment, 6, false, false);

	equipment->SetString("name", "Smoke grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_SMOKEGRENADE]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_SMOKEGRENADE]);
	m_iSmokeGrenades[CLASSLOADOUT_MACHINEGUNNER] = m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->AddItem(equipment, 7, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetItemVisible(m_iSmokeGrenades[CLASSLOADOUT_MACHINEGUNNER], false);

	m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetItemVisible( m_iHelmet[CLASSLOADOUT_MACHINEGUNNER], false);
	//m_pAvailableEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetItemVisible( m_iHelmet[CLASSLOADOUT_MACHINEGUNNER],	false);

	m_pAddEquipment[CLASSLOADOUT_MACHINEGUNNER] = new Button(m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "add", "+", this, "add");
	m_pAddEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetContentAlignment( vgui::Label::a_center );

	m_pRemoveEquipment[CLASSLOADOUT_MACHINEGUNNER] = new Button(m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "remove", "-", this, "remove");
	m_pRemoveEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetContentAlignment( vgui::Label::a_center );

	m_pSubmitCommand[CLASSLOADOUT_MACHINEGUNNER] = new Button(m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "cmdsubmit", "Submit", this, "submit");
	m_pSubmitCommand[CLASSLOADOUT_MACHINEGUNNER]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_MACHINEGUNNER][0] = new Button(m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "rotateright", ">", this, "rotateright");
	m_pRotateCommand[CLASSLOADOUT_MACHINEGUNNER][0]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_MACHINEGUNNER][1] = new Button(m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "rotateleft", "<", this, "rotateleft");
	m_pRotateCommand[CLASSLOADOUT_MACHINEGUNNER][1]->SetContentAlignment( vgui::Label::a_center );

	m_pCancelCommand[CLASSLOADOUT_MACHINEGUNNER]= new Button(m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "cmdcancel", "Cancel", this, "vguicancel");
	m_pCancelCommand[CLASSLOADOUT_MACHINEGUNNER]->SetContentAlignment( vgui::Label::a_center );
	// initialise as false, until player has at least chosen their first loadout
	m_pCancelCommand[CLASSLOADOUT_MACHINEGUNNER]->SetVisible( false );
	
	char szWeight[64];
	sprintf(szWeight, "%i / %i", m_iCurrentWeight[CLASSLOADOUT_MACHINEGUNNER], iMaxAlliedClassSlots[CLASSLOADOUT_MACHINEGUNNER]);
	m_pWeight[CLASSLOADOUT_MACHINEGUNNER] = new Label(m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "weight", szWeight);
	m_pWeight[CLASSLOADOUT_MACHINEGUNNER]->MoveToFront();
	m_pWeight[CLASSLOADOUT_MACHINEGUNNER]->SetContentAlignment( vgui::Label::a_center );

	m_pWeightProgress[CLASSLOADOUT_MACHINEGUNNER] = new ProgressBar( m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "weightprogress");
	//aswell as the label, need to set the progress bar
	m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAlliedClassSlots[m_iPlayerLoadoutPanel] );
	
	// Item Image Panel
	m_pItemImage[CLASSLOADOUT_MACHINEGUNNER] = new ImagePanel( m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "itemimage");
	m_pItemImage[CLASSLOADOUT_MACHINEGUNNER]->SetImage( "images/weapons/browning" );
	m_pItemImage[CLASSLOADOUT_MACHINEGUNNER]->SetShouldScaleImage( true );
	
	// Description panel
	m_pItemDescription[CLASSLOADOUT_MACHINEGUNNER] = new RichText( m_pClassPanel[CLASSLOADOUT_MACHINEGUNNER], "itemdesc");
	m_pItemDescription[CLASSLOADOUT_MACHINEGUNNER]->SetText( LoadItemDescription("resource/ui/descriptions/description_default.txt") );
}

void CRnLAlliedClassLoadoutMenu::InitializeRadiomanPanel()
{
	// add the current weight of the default weapon - Stefan
	m_iCurrentWeight[CLASSLOADOUT_RADIOMAN] = 3;
	m_iPrimaryWeaponWeight[CLASSLOADOUT_RADIOMAN] = 10;

	m_pClassPanel[CLASSLOADOUT_RADIOMAN] = new Panel(this, "PanelRadioman");
	m_pClassPanel[CLASSLOADOUT_RADIOMAN]->SetVisible( false );
	m_pClassPanel[CLASSLOADOUT_RADIOMAN]->SetPaintBorderEnabled( true );
	
	m_pPrimaryWeaponLabel[CLASSLOADOUT_RADIOMAN] = new Label(m_pClassPanel[CLASSLOADOUT_RADIOMAN], "primweaponlbl", "Primary weapon:");

	m_pWeightLabel[CLASSLOADOUT_RADIOMAN] = new Label(m_pClassPanel[CLASSLOADOUT_RADIOMAN], "weightlbl", "Weight:");

	m_pEquipmentLabel[CLASSLOADOUT_RADIOMAN] = new Label(m_pClassPanel[CLASSLOADOUT_RADIOMAN], "equipmentlbl", "Equipment:");

	m_pErrorLabel[CLASSLOADOUT_RADIOMAN] = new Label(m_pClassPanel[CLASSLOADOUT_RADIOMAN], "error", "");
	m_pErrorLabel[CLASSLOADOUT_RADIOMAN]->SetContentAlignment( vgui::Label::a_east );

	m_pPrimaryWeaponComboBox[CLASSLOADOUT_RADIOMAN] = new ComboBox(m_pClassPanel[CLASSLOADOUT_RADIOMAN], "primarycombo", 3, false);

	m_pPrimaryWeaponComboBox[CLASSLOADOUT_RADIOMAN]->AddActionSignalTarget( this );

	KeyValues *primaryweapon = new KeyValues("primaryweap");
	primaryweapon->SetInt("primaryweapon", WEAPON_M1CARBINE);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_M1CARBINE]);
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_RADIOMAN]->AddItem("M1 Carbine", primaryweapon);
	
	//default option is first weapon in list, we set playerloadout here so the TextChange
	// function adds the correct weight for it
	m_iPlayerLoadoutPanel = CLASSLOADOUT_RADIOMAN;
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_RADIOMAN]->ActivateItemByRow(0);

	m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN] = new ListPanel(m_pClassPanel[CLASSLOADOUT_RADIOMAN], "availequip", false);
	m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->AddColumnHeader(0, "name", "Equipment name", XRES(100), COLUMN_FIXEDSIZE);
	//m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->AddColumnHeader(1, "weight", "Slots", XRES(30), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->AddColumnHeader(1, "carry", "Carrying", XRES(50), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->AddColumnHeader(2, "max", "Max Carry", XRES(57), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->SetColumnSortable(0, false);
	m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->SetColumnSortable(1, false);
	m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->SetColumnSortable(2, false);
	m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->AddActionSignalTarget( this );
	
	KeyValues *equipment = new KeyValues("equipment");
	
	equipment->SetString("name", "1911 Colt");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_COLT]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_COLT]);
	m_iColt[CLASSLOADOUT_RADIOMAN] = m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->AddItem(equipment, 0, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->SetItemVisible(m_iColt[CLASSLOADOUT_RADIOMAN], false);

	equipment->SetString("name", "Frag grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_FRAGGRENADES]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 2);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_FRAGGRENADES]);
	m_iFragGrenades[CLASSLOADOUT_RADIOMAN] = m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->AddItem(equipment, 1, false, false);

	equipment->SetString("name", "Rifle grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_RIFLEGRENADES]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_RIFLEGRENADES]);
	m_iRifleGrenades[CLASSLOADOUT_RADIOMAN] = m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->AddItem(equipment, 2, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->SetItemVisible(m_iRifleGrenades[CLASSLOADOUT_RADIOMAN], false);

	equipment->SetString("name", "M3 knife");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_MELEE]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_MELEE]);
	m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->AddItem(equipment, 3, false, false);	

	equipment->SetString("name", "Extra ammo");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_EXTRAAMMO]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_EXTRAAMMO]);
	m_iExtraAmmo[CLASSLOADOUT_RADIOMAN] = m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->AddItem(equipment, 4, false, false);
	
	equipment->SetString("name", "Helmet");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_HELMET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_HELMET]);
	m_iHelmet[CLASSLOADOUT_RADIOMAN] = m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->AddItem(equipment, 5, false, false);

	equipment->SetString("name", "Bayonet (M1 Garand)");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_GARANDBAYONET]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_GARANDBAYONET]);
	m_iGarandBayonet[CLASSLOADOUT_RADIOMAN] = m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->AddItem(equipment, 6, false, false);
	
	m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->SetItemVisible( m_iGarandBayonet[CLASSLOADOUT_RADIOMAN], false);
	m_pAvailableEquipment[CLASSLOADOUT_RADIOMAN]->SetItemVisible( m_iHelmet[CLASSLOADOUT_RADIOMAN], false);
	
	m_pAddEquipment[CLASSLOADOUT_RADIOMAN] = new Button(m_pClassPanel[CLASSLOADOUT_RADIOMAN], "add", "+", this, "add");
	m_pAddEquipment[CLASSLOADOUT_RADIOMAN]->SetContentAlignment( vgui::Label::a_center );

	m_pRemoveEquipment[CLASSLOADOUT_RADIOMAN] = new Button(m_pClassPanel[CLASSLOADOUT_RADIOMAN], "remove", "-", this, "remove");
	m_pRemoveEquipment[CLASSLOADOUT_RADIOMAN]->SetContentAlignment( vgui::Label::a_center );

	m_pSubmitCommand[CLASSLOADOUT_RADIOMAN] = new Button(m_pClassPanel[CLASSLOADOUT_RADIOMAN], "cmdsubmit", "Submit", this, "submit");
	m_pSubmitCommand[CLASSLOADOUT_RADIOMAN]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_RADIOMAN][0] = new Button(m_pClassPanel[CLASSLOADOUT_RADIOMAN], "rotateright", ">", this, "rotateright");
	m_pRotateCommand[CLASSLOADOUT_RADIOMAN][0]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_RADIOMAN][1] = new Button(m_pClassPanel[CLASSLOADOUT_RADIOMAN], "rotateleft", "<", this, "rotateleft");
	m_pRotateCommand[CLASSLOADOUT_RADIOMAN][1]->SetContentAlignment( vgui::Label::a_center );

	m_pCancelCommand[CLASSLOADOUT_RADIOMAN]= new Button(m_pClassPanel[CLASSLOADOUT_RADIOMAN], "cmdcancel", "Cancel", this, "vguicancel");
	m_pCancelCommand[CLASSLOADOUT_RADIOMAN]->SetContentAlignment( vgui::Label::a_center );
	//m_pCancelCommand[CLASSLOADOUT_RADIOMAN]->SetVisible( false );
	
	char szWeight[64];
	sprintf(szWeight, "%i / %i", m_iCurrentWeight[CLASSLOADOUT_RADIOMAN], iMaxAlliedClassSlots[CLASSLOADOUT_RADIOMAN]);
	m_pWeight[CLASSLOADOUT_RADIOMAN] = new Label(m_pClassPanel[CLASSLOADOUT_RADIOMAN], "weight", szWeight);
	m_pWeight[CLASSLOADOUT_RADIOMAN]->MoveToFront();
	m_pWeight[CLASSLOADOUT_RADIOMAN]->SetContentAlignment( vgui::Label::a_center );

	m_pWeightProgress[CLASSLOADOUT_RADIOMAN] = new ProgressBar( m_pClassPanel[CLASSLOADOUT_RADIOMAN], "weightprogress");
	//aswell as the label, need to set the progress bar
	m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAlliedClassSlots[m_iPlayerLoadoutPanel] );

	m_pItemImage[CLASSLOADOUT_RADIOMAN] = new ImagePanel( m_pClassPanel[CLASSLOADOUT_RADIOMAN], "itemimage");
	m_pItemImage[CLASSLOADOUT_RADIOMAN]->SetImage( "images/weapons/m1carbine" );
	m_pItemImage[CLASSLOADOUT_RADIOMAN]->SetShouldScaleImage( true );

	m_pItemDescription[CLASSLOADOUT_RADIOMAN] = new RichText( m_pClassPanel[CLASSLOADOUT_RADIOMAN], "itemdesc");
	m_pItemDescription[CLASSLOADOUT_RADIOMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_default.txt") );
}

void CRnLAlliedClassLoadoutMenu::InitializeMedicPanel()
{
	m_iCurrentWeight[CLASSLOADOUT_MEDIC] = 0;
	m_iPrimaryWeaponWeight[CLASSLOADOUT_MEDIC] = 0;

	m_pClassPanel[CLASSLOADOUT_MEDIC] = new Panel(this, "PanelMedic");
	m_pClassPanel[CLASSLOADOUT_MEDIC]->SetVisible( false );
	m_pClassPanel[CLASSLOADOUT_MEDIC]->SetPaintBorderEnabled( true );
	
	m_pPrimaryWeaponLabel[CLASSLOADOUT_MEDIC] = new Label(m_pClassPanel[CLASSLOADOUT_MEDIC], "primweaponlbl", "Primary weapon:");

	m_pWeightLabel[CLASSLOADOUT_MEDIC] = new Label(m_pClassPanel[CLASSLOADOUT_MEDIC], "weightlbl", "Weight:");

	m_pEquipmentLabel[CLASSLOADOUT_MEDIC] = new Label(m_pClassPanel[CLASSLOADOUT_MEDIC], "equipmentlbl", "Equipment:");
	
	m_pErrorLabel[CLASSLOADOUT_MEDIC] = new Label(m_pClassPanel[CLASSLOADOUT_MEDIC], "error", "");
	m_pErrorLabel[CLASSLOADOUT_MEDIC]->SetContentAlignment( vgui::Label::a_east );

	m_pPrimaryWeaponComboBox[CLASSLOADOUT_MEDIC] = new ComboBox(m_pClassPanel[CLASSLOADOUT_MEDIC], "primarycombo", 3, false);
	
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_MEDIC]->AddActionSignalTarget( this );

	KeyValues *primaryweapon = new KeyValues("primaryweap");
	primaryweapon->SetInt("primaryweapon", WEAPON_NONE);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_NONE]);
	//m_pPrimaryWeaponComboBox[CLASSLOADOUT_MEDIC]->AddItem("unarmed", primaryweapon);
	//default option is first weapon in list, we set playerloadout here so the TextChange
	// function adds the correct weight for it
	m_iPlayerLoadoutPanel = CLASSLOADOUT_MEDIC;
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_MEDIC]->ActivateItemByRow(0);

	m_pAvailableEquipment[CLASSLOADOUT_MEDIC] = new ListPanel(m_pClassPanel[CLASSLOADOUT_MEDIC], "availequip", false);
	m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->AddColumnHeader(0, "name", "Equipment name", XRES(100), COLUMN_FIXEDSIZE);
	//m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->AddColumnHeader(1, "weight", "Slots", XRES(30), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->AddColumnHeader(1, "carry", "Carrying", XRES(50), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->AddColumnHeader(2, "max", "Max Carry", XRES(57), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->SetColumnSortable(0, false);
	m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->SetColumnSortable(1, false);
	m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->SetColumnSortable(2, false);
	// m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->SetColumnSortable(3, false);
	m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->AddActionSignalTarget( this );

	// Equipment
	KeyValues *equipment = new KeyValues("equipment");
	equipment->SetString("name", "M3 knife");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_MELEE]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_MELEE]);
	m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->AddItem(equipment, 0, false, false);

	equipment->SetString("name", "Helmet");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_HELMET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_HELMET]);
	m_iHelmet[CLASSLOADOUT_MEDIC] = m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->AddItem(equipment, 6, false, false);
	
	m_pAvailableEquipment[CLASSLOADOUT_MEDIC]->SetItemVisible( m_iHelmet[CLASSLOADOUT_MEDIC], false);

	m_pAddEquipment[CLASSLOADOUT_MEDIC] = new Button(m_pClassPanel[CLASSLOADOUT_MEDIC], "add", "+", this, "add");
	m_pAddEquipment[CLASSLOADOUT_MEDIC]->SetContentAlignment( vgui::Label::a_center );
	
	m_pRemoveEquipment[CLASSLOADOUT_MEDIC] = new Button(m_pClassPanel[CLASSLOADOUT_MEDIC], "remove", "-", this, "remove");
	m_pRemoveEquipment[CLASSLOADOUT_MEDIC]->SetContentAlignment( vgui::Label::a_center );
	
	m_pSubmitCommand[CLASSLOADOUT_MEDIC] = new Button(m_pClassPanel[CLASSLOADOUT_MEDIC], "cmdsubmit", "Submit", this, "submit");
	m_pSubmitCommand[CLASSLOADOUT_MEDIC]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_MEDIC][0] = new Button(m_pClassPanel[CLASSLOADOUT_MEDIC], "rotateright", ">", this, "rotateright");
	m_pRotateCommand[CLASSLOADOUT_MEDIC][0]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_MEDIC][1] = new Button(m_pClassPanel[CLASSLOADOUT_MEDIC], "rotateleft", "<", this, "rotateleft");
	m_pRotateCommand[CLASSLOADOUT_MEDIC][1]->SetContentAlignment( vgui::Label::a_center );

	m_pCancelCommand[CLASSLOADOUT_MEDIC]= new Button(m_pClassPanel[CLASSLOADOUT_MEDIC], "cmdcancel", "Cancel", this, "vguicancel");
	m_pCancelCommand[CLASSLOADOUT_MEDIC]->SetContentAlignment( vgui::Label::a_center );
	//m_pCancelCommand[CLASSLOADOUT_MEDIC]->SetVisible( false );
	
	char szWeight[64];
	sprintf(szWeight, "%i / %i", m_iCurrentWeight[CLASSLOADOUT_MEDIC], iMaxAlliedClassSlots[CLASSLOADOUT_MEDIC]);
	m_pWeight[CLASSLOADOUT_MEDIC] = new Label(m_pClassPanel[CLASSLOADOUT_MEDIC], "weight", szWeight);
	m_pWeight[CLASSLOADOUT_MEDIC]->MoveToFront();
	m_pWeight[CLASSLOADOUT_MEDIC]->SetContentAlignment( vgui::Label::a_center );

	m_pWeightProgress[CLASSLOADOUT_MEDIC] = new ProgressBar( m_pClassPanel[CLASSLOADOUT_MEDIC], "weightprogress");
	//aswell as the label, need to set the progress bar
	m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAlliedClassSlots[m_iPlayerLoadoutPanel] );
	
	m_pItemImage[CLASSLOADOUT_MEDIC] = new ImagePanel( m_pClassPanel[CLASSLOADOUT_MEDIC], "itemimage");
	m_pItemImage[CLASSLOADOUT_MEDIC]->SetImage( "image_descriptions/default" );
	m_pItemImage[CLASSLOADOUT_MEDIC]->SetShouldScaleImage( true );

	m_pItemDescription[CLASSLOADOUT_MEDIC] = new RichText( m_pClassPanel[CLASSLOADOUT_MEDIC], "itemdesc");
	m_pItemDescription[CLASSLOADOUT_MEDIC]->SetText( LoadItemDescription("resource/ui/descriptions/description_default.txt") );
}

void CRnLAlliedClassLoadoutMenu::InitializeNCOPanel()
{	
	m_iCurrentWeight[CLASSLOADOUT_NCO] = 4;
	m_iPrimaryWeaponWeight[CLASSLOADOUT_NCO] = 12;

	m_pClassPanel[CLASSLOADOUT_NCO] = new Panel(this, "PanelNCO");
	m_pClassPanel[CLASSLOADOUT_NCO]->SetVisible( false );
	m_pClassPanel[CLASSLOADOUT_NCO]->SetPaintBorderEnabled( true );
	
	m_pPrimaryWeaponLabel[CLASSLOADOUT_NCO] = new Label(m_pClassPanel[CLASSLOADOUT_NCO], "primweaponlbl", "Primary weapon:");
	
	m_pWeightLabel[CLASSLOADOUT_NCO] = new Label(m_pClassPanel[CLASSLOADOUT_NCO], "weightlbl", "Weight:");
	
	m_pEquipmentLabel[CLASSLOADOUT_NCO] = new Label(m_pClassPanel[CLASSLOADOUT_NCO], "equipmentlbl", "Equipment:");
	
	m_pErrorLabel[CLASSLOADOUT_NCO] = new Label(m_pClassPanel[CLASSLOADOUT_NCO], "error", "");
	m_pErrorLabel[CLASSLOADOUT_NCO]->SetContentAlignment( vgui::Label::a_east );

	m_pPrimaryWeaponComboBox[CLASSLOADOUT_NCO] = new ComboBox(m_pClassPanel[CLASSLOADOUT_NCO], "primarycombo", 3, false);
	
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_NCO]->AddActionSignalTarget( this );

	KeyValues *primaryweapon = new KeyValues("primaryweap");
	primaryweapon->SetInt("primaryweapon", WEAPON_THOMPSON);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_THOMPSON]);
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_NCO]->AddItem("Thompson SMG", primaryweapon);
	//default option is first weapon in list, we set playerloadout here so the TextChange
	// function adds the correct weight for it
	m_iPlayerLoadoutPanel = CLASSLOADOUT_NCO;
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_NCO]->ActivateItemByRow(0);

	primaryweapon->SetInt("primaryweapon", WEAPON_M1GARAND);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_M1GARAND]);
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_NCO]->AddItem("M1 Garand", primaryweapon);

	//primaryweapon->SetInt("primaryweapon", WEAPON_M1CARBINE);
	//primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_M1CARBINE]);
	//m_pPrimaryWeaponComboBox[CLASSLOADOUT_NCO]->AddItem("M1 Carbine", primaryweapon);

	m_pAvailableEquipment[CLASSLOADOUT_NCO] = new ListPanel(m_pClassPanel[CLASSLOADOUT_NCO], "availequip", false);
	m_pAvailableEquipment[CLASSLOADOUT_NCO]->AddColumnHeader(0, "name", "Equipment name", XRES(100), COLUMN_FIXEDSIZE);
	//m_pAvailableEquipment[CLASSLOADOUT_NCO]->AddColumnHeader(1, "weight", "Slots", XRES(30), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_NCO]->AddColumnHeader(1, "carry", "Carrying", XRES(50), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_NCO]->AddColumnHeader(2, "max", "Max Carry", XRES(50), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_NCO]->SetColumnSortable(0, false);
	m_pAvailableEquipment[CLASSLOADOUT_NCO]->SetColumnSortable(1, false);
	m_pAvailableEquipment[CLASSLOADOUT_NCO]->SetColumnSortable(2, false);
	m_pAvailableEquipment[CLASSLOADOUT_NCO]->AddActionSignalTarget( this );

	KeyValues *equipment = new KeyValues("equipment");
	
	equipment->SetString("name", "1911 Colt");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_COLT]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_COLT]);
	m_iColt[CLASSLOADOUT_NCO] = m_pAvailableEquipment[CLASSLOADOUT_NCO]->AddItem(equipment, 0, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_NCO]->SetItemVisible(m_iColt[CLASSLOADOUT_NCO], false);

	equipment->SetString("name", "Frag grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_FRAGGRENADES]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 2);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_FRAGGRENADES]);
	m_iFragGrenades[CLASSLOADOUT_NCO] = m_pAvailableEquipment[CLASSLOADOUT_NCO]->AddItem(equipment, 1, false, false);

	equipment->SetString("name", "Rifle grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_RIFLEGRENADES]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_RIFLEGRENADES]);
	m_iRifleGrenades[CLASSLOADOUT_NCO] = m_pAvailableEquipment[CLASSLOADOUT_NCO]->AddItem(equipment, 2, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_NCO]->SetItemVisible(m_iRifleGrenades[CLASSLOADOUT_NCO], false);

	equipment->SetString("name", "M3 knife");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_MELEE]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_MELEE]);
	m_pAvailableEquipment[CLASSLOADOUT_NCO]->AddItem(equipment, 3, false, false);

	equipment->SetString("name", "Extra ammo");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_EXTRAAMMO]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_EXTRAAMMO]);
	m_iExtraAmmo[CLASSLOADOUT_NCO] = m_pAvailableEquipment[CLASSLOADOUT_NCO]->AddItem(equipment, 4, false, false);

	equipment->SetString("name", "Helmet");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_HELMET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_HELMET]);
	m_iHelmet[CLASSLOADOUT_NCO] = m_pAvailableEquipment[CLASSLOADOUT_NCO]->AddItem(equipment, 5, false, false);
	
	// The bayonet is shown as carried equipment prop when the garand is selected, but you can neither add nor remove it
	// nor is it being send to the loadout but added by default - Stefan
	equipment->SetString("name", "Bayonet (M1 Garand)");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_GARANDBAYONET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_GARANDBAYONET]);
	m_iGarandBayonet[CLASSLOADOUT_NCO] = m_pAvailableEquipment[CLASSLOADOUT_NCO]->AddItem(equipment, 6, false, false);

	equipment->SetString("name", "Smoke grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_ALLIES_SMOKEGRENADE]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_ALLIES_SMOKEGRENADE]);
	m_iSmokeGrenades[CLASSLOADOUT_NCO] = m_pAvailableEquipment[CLASSLOADOUT_NCO]->AddItem(equipment, 7, false, false);

	m_pAvailableEquipment[CLASSLOADOUT_NCO]->SetItemVisible( m_iGarandBayonet[CLASSLOADOUT_NCO], false);
	m_pAvailableEquipment[CLASSLOADOUT_NCO]->SetItemVisible( m_iHelmet[CLASSLOADOUT_NCO], false);

	m_pAddEquipment[CLASSLOADOUT_NCO] = new Button(m_pClassPanel[CLASSLOADOUT_NCO], "add", "+", this, "add");
	m_pAddEquipment[CLASSLOADOUT_NCO]->SetContentAlignment( vgui::Label::a_center );

	m_pRemoveEquipment[CLASSLOADOUT_NCO] = new Button(m_pClassPanel[CLASSLOADOUT_NCO], "remove", "-", this, "remove");
	m_pRemoveEquipment[CLASSLOADOUT_NCO]->SetContentAlignment( vgui::Label::a_center );

	m_pSubmitCommand[CLASSLOADOUT_NCO] = new Button(m_pClassPanel[CLASSLOADOUT_NCO], "cmdsubmit", "Submit", this, "submit");
	m_pSubmitCommand[CLASSLOADOUT_NCO]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_NCO][0] = new Button(m_pClassPanel[CLASSLOADOUT_NCO], "rotateright", ">", this, "rotateright");
	m_pRotateCommand[CLASSLOADOUT_NCO][0]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_NCO][1] = new Button(m_pClassPanel[CLASSLOADOUT_NCO], "rotateleft", "<", this, "rotateleft");
	m_pRotateCommand[CLASSLOADOUT_NCO][1]->SetContentAlignment( vgui::Label::a_center );

	m_pCancelCommand[CLASSLOADOUT_NCO]= new Button(m_pClassPanel[CLASSLOADOUT_NCO], "cmdcancel", "Cancel", this, "vguicancel");
	m_pCancelCommand[CLASSLOADOUT_NCO]->SetContentAlignment( vgui::Label::a_center );
	m_pCancelCommand[CLASSLOADOUT_NCO]->SetVisible( false );
	
	// Hide the riflegrenades and the garandbayonet by default - Stefan
	m_pAvailableEquipment[CLASSLOADOUT_NCO]->SetItemVisible(m_iGarandBayonet[CLASSLOADOUT_NCO], false);
	m_pAvailableEquipment[CLASSLOADOUT_NCO]->SetItemVisible(m_iRifleGrenades[CLASSLOADOUT_NCO], false);

	char szWeight[64];
	sprintf(szWeight, "%i / %i", m_iCurrentWeight[CLASSLOADOUT_NCO], iMaxAlliedClassSlots[CLASSLOADOUT_NCO]);
	m_pWeight[CLASSLOADOUT_NCO] = new Label(m_pClassPanel[CLASSLOADOUT_NCO], "weight", szWeight);
	m_pWeight[CLASSLOADOUT_NCO]->MoveToFront();
	m_pWeight[CLASSLOADOUT_NCO]->SetContentAlignment( vgui::Label::a_center );

	m_pWeightProgress[CLASSLOADOUT_NCO] = new ProgressBar( m_pClassPanel[CLASSLOADOUT_NCO], "weightprogress");
	//aswell as the label, need to set the progress bar
	m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAlliedClassSlots[m_iPlayerLoadoutPanel] );
	
	m_pItemImage[CLASSLOADOUT_NCO] = new ImagePanel( m_pClassPanel[CLASSLOADOUT_NCO], "itemimage");
	m_pItemImage[CLASSLOADOUT_NCO]->SetImage( "images/weapons/thompson" );
	m_pItemImage[CLASSLOADOUT_NCO]->SetShouldScaleImage( true );

	m_pItemDescription[CLASSLOADOUT_NCO] = new RichText( m_pClassPanel[CLASSLOADOUT_NCO], "itemdesc");
	m_pItemDescription[CLASSLOADOUT_NCO]->SetText( LoadItemDescription("resource/ui/descriptions/description_default.txt") );
}

//this means the combobox has been changed - johny (a weapon has been selected - stefan)
void CRnLAlliedClassLoadoutMenu::OnTextChanged( KeyValues *data )
{
	if(m_iPlayerLoadoutPanel < 0 || m_iPlayerLoadoutPanel >= CLASSLOADOUT_ALLIES_MAX)
		return;
	
	KeyValues *primaryWeapon = m_pPrimaryWeaponComboBox[m_iPlayerLoadoutPanel]->GetActiveItemUserData();
	
	//save off the current weapon
	m_iCurrentPrimaryWeapon[m_iPlayerLoadoutPanel] = m_pPrimaryWeaponComboBox[m_iPlayerLoadoutPanel]->GetActiveItem();
		
	// Set some visibility default values
	m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iGarandBayonet[m_iPlayerLoadoutPanel], false);
	m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iColt[m_iPlayerLoadoutPanel], true);
	// m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iRifleGrenades[m_iPlayerLoadoutPanel], false);

	//if the new weapon is a garand, add the bayonet to the equipment list, else make sure its invisible
	if( primaryWeapon->GetInt("primaryweapon") == WEAPON_M1GARAND )
	{
		KeyValues *BayonetItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iGarandBayonet[m_iPlayerLoadoutPanel] );
		// Show the bayonet
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iGarandBayonet[m_iPlayerLoadoutPanel], true);
		// and add it again
		if( BayonetItem->GetInt("carry") == 0 )
		{
			BayonetItem->SetInt("carry", 1);
		}
	}
	else if( primaryWeapon->GetInt("primaryweapon") == WEAPON_M1CARBINE )
	{
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iRifleGrenades[m_iPlayerLoadoutPanel], false);
		
		// Remove the riflegrenades and hide them
		KeyValues *RifleGrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iRifleGrenades[m_iPlayerLoadoutPanel] );
		KeyValues *BayonetItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iGarandBayonet[m_iPlayerLoadoutPanel] );

		if( RifleGrenadeItem->GetInt("carry") > 0 )
		{
			RifleGrenadeItem->SetInt("carry", 0);
			if( RifleGrenadeItem->GetInt("carry") == 1 )
				m_iCurrentWeight[m_iPlayerLoadoutPanel] -= RifleGrenadeItem->GetInt("weight");
			else if( RifleGrenadeItem->GetInt("carry") == 2 )
				m_iCurrentWeight[m_iPlayerLoadoutPanel] -= 2 * RifleGrenadeItem->GetInt("weight");
		}
		
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible( m_iGarandBayonet[m_iPlayerLoadoutPanel], false);
		// if the player has a bayonet remove the bayonet from his list
		if( BayonetItem->GetInt("carry") > 0 )
		{
			BayonetItem->SetInt("carry", 0);
		}
	}
	else if( primaryWeapon->GetInt("primaryweapon") == WEAPON_BAR )
	{
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iRifleGrenades[m_iPlayerLoadoutPanel], false);
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iColt[m_iPlayerLoadoutPanel], true);

		// Remove the riflegrenades and hide them
		KeyValues *RifleGrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iRifleGrenades[m_iPlayerLoadoutPanel] );
		KeyValues *ColtItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iColt[m_iPlayerLoadoutPanel] );
		KeyValues *BayonetItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iGarandBayonet[m_iPlayerLoadoutPanel] );

		if( RifleGrenadeItem->GetInt("carry") > 0 )
		{
			RifleGrenadeItem->SetInt("carry", 0);
			if( RifleGrenadeItem->GetInt("carry") == 1 )
				m_iCurrentWeight[m_iPlayerLoadoutPanel] -= RifleGrenadeItem->GetInt("weight");
			else if( RifleGrenadeItem->GetInt("carry") == 2 )
				m_iCurrentWeight[m_iPlayerLoadoutPanel] -= 2 * RifleGrenadeItem->GetInt("weight");
		}
		// Remove the Colt as well
		if( ColtItem->GetInt("carry") > 0 )
		{
			ColtItem->SetInt("carry", 0);
			m_iCurrentWeight[m_iPlayerLoadoutPanel] -= ColtItem->GetInt("weight");
		}
		// if the player has a bayonet remove the bayonet from his list
		if( BayonetItem->GetInt("carry") > 0 )
		{
			BayonetItem->SetInt("carry", 0);
		}
	}
	else if( primaryWeapon->GetInt("primaryweapon") == WEAPON_THOMPSON )
	{
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iRifleGrenades[m_iPlayerLoadoutPanel], false);

		// Remove the riflegrenades and hide them
		KeyValues *RifleGrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iRifleGrenades[m_iPlayerLoadoutPanel] );
		KeyValues *BayonetItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iGarandBayonet[m_iPlayerLoadoutPanel] );

		if( RifleGrenadeItem->GetInt("carry") > 0 )
		{
			RifleGrenadeItem->SetInt("carry", 0);
			
			if( RifleGrenadeItem->GetInt("carry") == 1 )
				m_iCurrentWeight[m_iPlayerLoadoutPanel] -= RifleGrenadeItem->GetInt("weight");
			else if( RifleGrenadeItem->GetInt("carry") == 2 )
				m_iCurrentWeight[m_iPlayerLoadoutPanel] -= 2 * RifleGrenadeItem->GetInt("weight");
		}
		// if the player has a bayonet remove the bayonet from his list
		if( BayonetItem->GetInt("carry") > 0 )
		{
			BayonetItem->SetInt("carry", 0);
		}
	}
	else
	{
		KeyValues *BayonetItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iGarandBayonet[m_iPlayerLoadoutPanel] );
		
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible( 
		m_iGarandBayonet[m_iPlayerLoadoutPanel],
		false);
		// if the player has a bayonet remove the bayonet from his list
		if( BayonetItem->GetInt("carry") > 0 )
		{
			BayonetItem->SetInt("carry", 0);
		}
	}
	char szWeight[64];
	sprintf(szWeight, "%i / %i", m_iCurrentWeight[m_iPlayerLoadoutPanel], iMaxAlliedClassSlots[m_iPlayerLoadoutPanel]);
	m_pWeight[m_iPlayerLoadoutPanel]->SetText(szWeight);

	//aswell as the label, need to set the progress bar
	m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAlliedClassSlots[m_iPlayerLoadoutPanel] );	
}

void CRnLAlliedClassLoadoutMenu::SetLoadoutModel(Vector vecOrigin, QAngle angAngles)
{
	// Set the coordinates for the loadout models
	vecModelOrigin = vecOrigin;
	angModelAngles = angAngles;
	
	// Rotate everything 90 degrees
	// angModelAngles.y += 90; ...not necessary at the moment - stefan

	// The player loadout model
	if( m_pLoadoutModel )
	{
		m_pLoadoutModel->SetAbsOrigin( vecModelOrigin );
		m_pLoadoutModel->SetAbsAngles( angModelAngles );
	}

	if( m_pLoadoutWeaponModel )
	{
		m_pLoadoutWeaponModel->SetAbsOrigin( vecModelOrigin );
		m_pLoadoutWeaponModel->SetAbsAngles( angModelAngles );
	}

	if( m_pLoadoutModelTemp )
	{
		m_pLoadoutModelTemp->SetAbsOrigin( vecModelOrigin );
		m_pLoadoutModelTemp->SetAbsAngles( angModelAngles );
	}
}

void CRnLAlliedClassLoadoutMenu::SetLoadoutCamera(Vector vecOrigin, QAngle angAngles)
{
	vecCameraOrigin = vecOrigin;
	angCameraAngles = angAngles;
}

void CRnLAlliedClassLoadoutMenu::CreateViewportEntity()
{
	//if the model has already been created, exit this func - nuke
	if( m_bViewportModelCreated )
		return;

	//create the model
	// the player loadout model
	m_pLoadoutModel = new C_BaseAnimatingOverlay;

	// The currently selected weapon and equipment models
	m_pLoadoutWeaponModel = new C_BaseAnimating;
	m_pLoadoutModelTemp = new C_BaseAnimating;

	if ( m_pLoadoutWeaponModel->InitializeAsClientEntity( EQ_PIR_M1GARAND, RENDER_GROUP_OPAQUE_ENTITY ) == false )
	{
		m_pLoadoutWeaponModel->Release();
		m_pLoadoutWeaponModel = NULL;
	}
	if ( m_pLoadoutModelTemp->InitializeAsClientEntity( EQ_PIR_M1GARAND, RENDER_GROUP_OPAQUE_ENTITY ) == false )
	{
		m_pLoadoutModelTemp->Release();
		m_pLoadoutModelTemp = NULL;
	}
	if ( m_pLoadoutModel->InitializeAsClientEntity( ALLIES_PLAYER_PIR_RIFLEMAN, RENDER_GROUP_OPAQUE_ENTITY ) == false )
	{
		m_pLoadoutModel->Release();
	}
	else
	{
		//start animating here
		SetModel( m_pLoadoutModel, ALLIES_PLAYER_PIR_RIFLEMAN );
		m_pLoadoutModel->ForceClientSideAnimationOn();
		//Andrew : Stuff Stuff Stuff
		m_pLoadoutModel->SetNumAnimOverlays( 2 );
		m_pLoadoutModel->ResetSequence( m_pLoadoutModel->LookupSequence( "idle_lower" ));

		// The weapon model
		if( m_pLoadoutWeaponModel )
			m_pLoadoutWeaponModel->FollowEntity( m_pLoadoutModel, true );

		SetModel( m_pLoadoutModelTemp, NULL );

		//reset this at map recreate, as gpGlobals->curtime is also reset
		m_flLastModelUpdateTime = 0.0;
	}
}

void CRnLAlliedClassLoadoutMenu::Update()
{
	//model hasn't been created yet, let's exit
	if(m_bViewportModelCreated && gpGlobals->curtime > m_flLastModelUpdateTime)
	{
		angModelAngles.y += 0.3;
		m_pLoadoutModel->SetAbsAngles(angModelAngles);
		m_flLastModelUpdateTime = gpGlobals->curtime + 0.01;
	}
}

void CRnLAlliedClassLoadoutMenu::OnComboMenuItemHighlight( KeyValues *data )
{
	int iItemID = -1;

	// if the player highlighted a combo box item
	// then the menu list is out of focus.
	// So there's no more need to display the temporary equipment model - Stefan
	SetModel( m_pLoadoutModelTemp, NULL );

	switch( m_iPlayerLoadoutPanel )
	{
		case CLASSLOADOUT_RIFLEMAN:
		{
			KeyValues *currentItemKeyValues = m_pPrimaryWeaponComboBox[CLASSLOADOUT_RIFLEMAN]->GetItemUserData( data->GetInt("itemID") );
			
			if( currentItemKeyValues != NULL )
			{
				iItemID = currentItemKeyValues->GetInt("primaryweapon");
				//this mess of keyvalues gibberish gets the WEAPON_* value from the combobox selection
				ChangeRiflemanDescription( true, iItemID );
			}
			break;
		}
		case CLASSLOADOUT_MACHINEGUNNER:
		{
			KeyValues *currentItemKeyValues = m_pPrimaryWeaponComboBox[CLASSLOADOUT_MACHINEGUNNER]->GetItemUserData( data->GetInt("itemID") );
			
			if( currentItemKeyValues != NULL )
			{
				iItemID = currentItemKeyValues->GetInt("primaryweapon");
				//this mess of keyvalues gibberish gets the WEAPON_* value from the combobox selection
				ChangeMachineGunnerDescription( true, iItemID );
			}
			break;
		}
		case CLASSLOADOUT_RADIOMAN:
		{
			KeyValues *currentItemKeyValues = currentItemKeyValues = m_pPrimaryWeaponComboBox[CLASSLOADOUT_RADIOMAN]->GetItemUserData( data->GetInt("itemID") );
			
			if( currentItemKeyValues != NULL )
			{
				iItemID = currentItemKeyValues->GetInt("primaryweapon");
				//this mess of keyvalues gibberish gets the WEAPON_* value from the combobox selection
				ChangeRadiomanDescription( true, iItemID );
			}
			break;
		}
		case CLASSLOADOUT_MEDIC:
		{		
			KeyValues *currentItemKeyValues = m_pPrimaryWeaponComboBox[CLASSLOADOUT_MEDIC]->GetItemUserData( data->GetInt("itemID") );
			
			if( currentItemKeyValues != NULL )
			{
				iItemID = currentItemKeyValues->GetInt("primaryweapon");
				//this mess of keyvalues gibberish gets the WEAPON_* value from the combobox selection
				ChangeMedicDescription( true, iItemID );
			}
			break;
		}
		case CLASSLOADOUT_NCO:
		{		
			KeyValues *currentItemKeyValues = m_pPrimaryWeaponComboBox[CLASSLOADOUT_NCO]->GetItemUserData( data->GetInt("itemID") );
			
			if( currentItemKeyValues != NULL )
			{
				iItemID = currentItemKeyValues->GetInt("primaryweapon");
				//this mess of keyvalues gibberish gets the WEAPON_* value from the combobox selection
				ChangeNCODescription( true, iItemID );
			}
			break;
		}
	}
}

void CRnLAlliedClassLoadoutMenu::ChangeRiflemanDescription( bool bWeapon, int iItemNumber )
{
	// Remove all grenades to clear them up if the class has been changed. They will be added afterwards with their proper count
	UpdateGrenadesInViewPort();

	if( bWeapon )
	{
		ResetAttachments();

		//Andrew : Used to get a layer of animation, declared here because visual studio
		// is being gay and has an issue with it inside the switch
		C_AnimationLayer *pLayer = NULL;

		switch( iItemNumber )
		{
		case WEAPON_M1GARAND:

			m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_m1garand.txt") );
			m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetImage( "images/weapons/m1garand" );

			// Set the garand as model now
			SetModel( m_pLoadoutWeaponModel, EQ_PIR_M1GARAND );

			// if you have the garand, you have the bayonet too
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( BAYONET_BODYGROUP ), 1 );

			//Andrew : Animatiing for the hell of it
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_m1");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}
			break;

		case WEAPON_M1CARBINE:

			m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_m1carbine.txt") );
			m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetImage( "images/weapons/m1carbine" );

			// Set the garand as model now
			SetModel( m_pLoadoutWeaponModel, EQ_PIR_M1CARBINE );

			// if you have the garand, you have the bayonet too
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( BAYONET_BODYGROUP ), 0 );

			//Andrew : Animatiing for the hell of it
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				//pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_m1");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}
			break;


		case WEAPON_BAR:

			// ADD LATER

		default:
			Warning("Loadout Rifleman: Something is broken here! - %i \n", iItemNumber);
			break;
		}

		// changed weapon, so update the viewport pouches
		UpdatePouchesInViewPort();
	}
	// If the player changed his equipment, display a text and image for the specified equipment.
	else
	{
		m_pAddEquipment[CLASSLOADOUT_RIFLEMAN]->SetEnabled( true );
		m_pRemoveEquipment[CLASSLOADOUT_RIFLEMAN]->SetEnabled ( true );
		switch( iItemNumber )
		{
		case 0:
			m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_colt.txt") );
			m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetImage( "images/weapons/colt" );
			m_pLoadoutModelTemp->SetModel( NULL );
			break;
		//EQUIPMENT_ALLIES_FRAGGRENADES
		case 1:
			m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_mk2grenade.txt") );
			m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetImage( "images/weapons/mk2grenade" );
			break;
		// rifle grenades
		case 2:
			m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_riflegrenade.txt") );
			m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetImage( "images/weapons/riflegrenade" );
			//m_pLoadoutModelTemp->SetModel( m_szCurrentLoadoutModelRifleGrenade );
			break;
		// Combat knife
		case 3:
			m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_m3knife.txt") );
			m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetImage( "images/weapons/m3knife" );
			//m_pLoadoutModelTemp->SetModel( EQ_PIR_M3KNIFE_MODEL );
			break;
		// extra ammo
		case 4:
			m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_ammo.txt") );
			m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetImage( "images/weapons/ammo" );
			break;
		// helmet
		case 5:
			m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_helmet.txt") );
			m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetImage( "images/weapons/helmet" );
			break;
		// bayonet
		case 6:
			// Disable the add and remove button, so the player cannot remove the bayonet
			m_pAddEquipment[CLASSLOADOUT_RIFLEMAN]->SetEnabled( false );
			m_pRemoveEquipment[CLASSLOADOUT_RIFLEMAN]->SetEnabled ( false );
			m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_garandbayonet.txt") );
			m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetImage( "images/weapons/garandbayonet" );
			break;

		default:
			Warning("Loadout Rifleman: Something is broken here!\n");
			break;
		}
	}
}

void CRnLAlliedClassLoadoutMenu::ChangeMachineGunnerDescription( bool bWeapon, int iItemNumber )
{
	// Remove all grenades to clear them up if the class has been changed. They will be added afterwards with their proper count
	UpdateGrenadesInViewPort();

	if( bWeapon )
	{
		ResetAttachments();

		//Andrew : Used to get a layer of animation, declared here because visual studio
		// is being gay and has an issue with it inside the switch
		C_AnimationLayer *pLayer = NULL;

		switch( iItemNumber )
		{
		case WEAPON_BROWNING:

			m_pItemDescription[CLASSLOADOUT_MACHINEGUNNER]->SetText( LoadItemDescription("resource/ui/descriptions/description_browning.txt") );
			m_pItemImage[CLASSLOADOUT_MACHINEGUNNER]->SetImage( "images/weapons/browning" );

			// Set the garand as model now
			SetModel( m_pLoadoutWeaponModel, EQ_PIR_M1GARAND );

			// if you have the garand, you have the bayonet too
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( BAYONET_BODYGROUP ), 1 );

			//Andrew : Animatiing for the hell of it
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_m1");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}
			break;

		default:
			Warning("Loadout Machine Gunner: Something is broken here! - %i \n", iItemNumber);
			break;
		}

		// changed weapon, so update the viewport pouches
		UpdatePouchesInViewPort();
	}
	// If the player changed his equipment, display a text and image for the specified equipment.
	else
	{
		m_pAddEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetEnabled( true );
		m_pRemoveEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetEnabled ( true );
		switch( iItemNumber )
		{
		case 0:
			m_pItemDescription[CLASSLOADOUT_MACHINEGUNNER]->SetText( LoadItemDescription("resource/ui/descriptions/description_colt.txt") );
			m_pItemImage[CLASSLOADOUT_MACHINEGUNNER]->SetImage( "images/weapons/colt" );
			m_pLoadoutModelTemp->SetModel( NULL );
			break;
		//EQUIPMENT_ALLIES_FRAGGRENADES
		case 1:
			m_pItemDescription[CLASSLOADOUT_MACHINEGUNNER]->SetText( LoadItemDescription("resource/ui/descriptions/description_mk2grenade.txt") );
			m_pItemImage[CLASSLOADOUT_MACHINEGUNNER]->SetImage( "images/weapons/mk2grenade" );
			break;
		// rifle grenades
		case 2:
			m_pItemDescription[CLASSLOADOUT_MACHINEGUNNER]->SetText( LoadItemDescription("resource/ui/descriptions/description_riflegrenade.txt") );
			m_pItemImage[CLASSLOADOUT_MACHINEGUNNER]->SetImage( "images/weapons/riflegrenade" );
			//m_pLoadoutModelTemp->SetModel( m_szCurrentLoadoutModelRifleGrenade );
			break;
		// Combat knife
		case 3:
			m_pItemDescription[CLASSLOADOUT_MACHINEGUNNER]->SetText( LoadItemDescription("resource/ui/descriptions/description_m3knife.txt") );
			m_pItemImage[CLASSLOADOUT_MACHINEGUNNER]->SetImage( "images/weapons/m3knife" );
			//m_pLoadoutModelTemp->SetModel( EQ_PIR_M3KNIFE_MODEL );
			break;
		// extra ammo
		case 4:
			m_pItemDescription[CLASSLOADOUT_MACHINEGUNNER]->SetText( LoadItemDescription("resource/ui/descriptions/description_ammo.txt") );
			m_pItemImage[CLASSLOADOUT_MACHINEGUNNER]->SetImage( "images/weapons/ammo" );
			break;
		// helmet
		case 5:
			m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_helmet.txt") );
			m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetImage( "images/weapons/helmet" );
			break;
		// bayonet
		case 6:
			// Disable the add and remove button, so the player cannot remove the bayonet
			m_pAddEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetEnabled( false );
			m_pRemoveEquipment[CLASSLOADOUT_MACHINEGUNNER]->SetEnabled ( false );
			m_pItemDescription[CLASSLOADOUT_MACHINEGUNNER]->SetText( LoadItemDescription("resource/ui/descriptions/description_garandbayonet.txt") );
			m_pItemImage[CLASSLOADOUT_MACHINEGUNNER]->SetImage( "images/weapons/garandbayonet" );
			break;

		default:
			Warning("Loadout Machine Gunner: Something is broken here!\n");
			break;
		}
	}
}

void CRnLAlliedClassLoadoutMenu::ChangeRadiomanDescription( bool bWeapon, int iItemNumber )
{
	if( bWeapon )
	{
		//Andrew : Used to get a layer of animation
		C_AnimationLayer *pLayer = NULL;

		switch( iItemNumber )
		{
		
		case WEAPON_M1GARAND:
			m_pItemDescription[CLASSLOADOUT_RADIOMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_m1garand.txt") );
			m_pItemImage[CLASSLOADOUT_RADIOMAN]->SetImage( "images/weapons/m1garand" );

			// Set the garand as model now
			SetModel( m_pLoadoutWeaponModel, EQ_PIR_M1GARAND );

			//Andrew : This isn't in yet but it will be
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_m1");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}
			break;

		case WEAPON_M1CARBINE:

			m_pItemDescription[CLASSLOADOUT_RIFLEMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_m1carbine.txt") );
			m_pItemImage[CLASSLOADOUT_RIFLEMAN]->SetImage( "images/weapons/m1carbine" );

			// Set the garand as model now
			//SetModel( m_pLoadoutWeaponModel, EQ_PIR_M1CARBINE );

			// if you have the garand, you have the bayonet too
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( BAYONET_BODYGROUP ), 0 );

			//Andrew : Animatiing for the hell of it
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				//pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_m1");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}
			break;

		case WEAPON_BAR:
			
			// ADD LATER

			break;

		default:
			Warning("Loadout Radioman: Something is broken here!\n");
			break;
		}

	}
	// If the player changed his equipment, display a text and image for the specified equipment.
	else
	{
		m_pAddEquipment[CLASSLOADOUT_RADIOMAN]->SetEnabled( true );
		m_pRemoveEquipment[CLASSLOADOUT_RADIOMAN]->SetEnabled ( true );

		switch( iItemNumber )
		{
		case 0:
			m_pItemDescription[CLASSLOADOUT_RADIOMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_colt.txt") );
			m_pItemImage[CLASSLOADOUT_RADIOMAN]->SetImage( "images/weapons/colt" );

			m_pLoadoutModel->SetSequence( m_pLoadoutModel->LookupSequence( "idle_lower" ) );
			break;
		//EQUIPMENT_ALLIES_FRAGGRENADES
		case 1:
			m_pItemDescription[CLASSLOADOUT_RADIOMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_mk2grenade.txt") );
			m_pItemImage[CLASSLOADOUT_RADIOMAN]->SetImage( "images/weapons/mk2grenade" );
			break;
		// rifle grenades
		case 2:
			m_pItemDescription[CLASSLOADOUT_RADIOMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_riflegrenade.txt") );
			m_pItemImage[CLASSLOADOUT_RADIOMAN]->SetImage( "images/weapons/riflegrenade" );
			break;
		// Combat knife
		case 3:
			m_pItemDescription[CLASSLOADOUT_RADIOMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_m3knife.txt") );
			m_pItemImage[CLASSLOADOUT_RADIOMAN]->SetImage( "images/weapons/m3knife" );
			break;
		// extra ammo
		case 4:
			m_pItemDescription[CLASSLOADOUT_RADIOMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_ammo.txt") );
			m_pItemImage[CLASSLOADOUT_RADIOMAN]->SetImage( "images/weapons/ammo" );
			break;
		// helmet
		case 5:
			m_pItemDescription[CLASSLOADOUT_RADIOMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_helmet.txt") );
			m_pItemImage[CLASSLOADOUT_RADIOMAN]->SetImage( "images/weapons/helmet" );
			break;
		// bayonet
		case 6:
			m_pAddEquipment[CLASSLOADOUT_RADIOMAN]->SetEnabled( false );
			m_pRemoveEquipment[CLASSLOADOUT_RADIOMAN]->SetEnabled ( false );
			m_pItemDescription[CLASSLOADOUT_RADIOMAN]->SetText( LoadItemDescription("resource/ui/descriptions/description_garandbayonet.txt") );
			m_pItemImage[CLASSLOADOUT_RADIOMAN]->SetImage( "images/weapons/garandbayonet" );
			break;

		default:
			Warning("Loadout Radioman: Wrong Equipment Index\n");
			break;
		}
	}
}
void CRnLAlliedClassLoadoutMenu::ChangeMedicDescription( bool bWeapon, int iItemNumber )
{
	if( bWeapon )
	{
		m_pItemDescription[CLASSLOADOUT_MEDIC]->SetText( LoadItemDescription("resource/ui/descriptions/description_unarmed.txt") );
	}
	else
	{
		//Here we need to add a switch for the medics special loadout equipment.
		switch( iItemNumber )
		{
		//EQUIPMENT_ALLIES_FRAGGRENADES
		case 0:
			m_pItemDescription[CLASSLOADOUT_MEDIC]->SetText( LoadItemDescription("resource/ui/descriptions/description_mk2grenade.txt") );
			break;
		// helmet
		case 5:
			m_pItemDescription[CLASSLOADOUT_MEDIC]->SetText( LoadItemDescription("resource/ui/descriptions/description_helmet.txt") );
			m_pItemImage[CLASSLOADOUT_MEDIC]->SetImage( "images/weapons/helmet" );
			break;
		}
	}
}
void CRnLAlliedClassLoadoutMenu::ChangeNCODescription( bool bWeapon, int iItemNumber )
{
	UpdateGrenadesInViewPort();

	if( bWeapon )
	{
		//Andrew : Used to get a layer of animation
		C_AnimationLayer *pLayer = NULL;

		switch( iItemNumber )
		{
		case WEAPON_THOMPSON:
			m_pItemDescription[CLASSLOADOUT_NCO]->SetText( LoadItemDescription("resource/ui/descriptions/description_thompson.txt") );
			m_pItemImage[CLASSLOADOUT_NCO]->SetImage( "images/weapons/thompson" );

			//Andrew : THOMPSON thompson thompson yay
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_thompson");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}

			SetModel( m_pLoadoutWeaponModel, EQ_PIR_THOMPSON );
			SetModel( m_pLoadoutModel, ALLIES_PLAYER_PIR_NCO );

			break;

		case WEAPON_M1GARAND:
			m_pItemDescription[CLASSLOADOUT_NCO]->SetText( LoadItemDescription("resource/ui/descriptions/description_m1garand.txt") );
			m_pItemImage[CLASSLOADOUT_NCO]->SetImage( "images/weapons/m1garand" );
			SetModel( m_pLoadoutWeaponModel, EQ_PIR_M1GARAND );
			//m_pLoadoutModel->SetSequence( m_pLoadoutModel->LookupSequence("idle_lower"));

			//Andrew : Update the upper body
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_m1");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}

			SetModel( m_pLoadoutModel, ALLIES_PLAYER_PIR_NCO );

			break;

		case WEAPON_M1CARBINE:
			
			// And finally set the carbine rifleman model too
			SetModel( m_pLoadoutModel, ALLIES_PLAYER_PIR_NCO );

			m_pItemDescription[CLASSLOADOUT_NCO]->SetText( LoadItemDescription("resource/ui/descriptions/description_m1carbine.txt") );
			m_pItemImage[CLASSLOADOUT_NCO]->SetImage( "images/weapons/m1carbine" );
			if( m_pLoadoutWeaponModel )
				m_pLoadoutWeaponModel->SetModel( EQ_PIR_M1GARAND );
			////m_pLoadoutModel->SetSequence( m_pLoadoutModel->LookupSequence("idle_lower"));

			//Andrew : Update the upper body
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_m1");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}

			break;
		default:
			Warning("Loadout NCO: Wrong weapon index!\n");
			break;
		}

		UpdatePouchesInViewPort();
	}
// If the player changed his equipment, display a text and image for the specified equipment.
	else
	{
		/*m_pAddEquipment[CLASSLOADOUT_NCO]->SetEnabled( true );
		m_pRemoveEquipment[CLASSLOADOUT_NCO]->SetEnabled ( true );*/

		switch( iItemNumber )
		{
		case 0:
			m_pItemDescription[CLASSLOADOUT_NCO]->SetText( LoadItemDescription("resource/ui/descriptions/description_colt.txt") );
			m_pItemImage[CLASSLOADOUT_NCO]->SetImage( "images/weapons/colt" );
			//m_pLoadoutModel->SetSequence( m_pLoadoutModel->LookupSequence( "idle_lower" ) );
			break;
		//EQUIPMENT_ALLIES_FRAGGRENADES
		case 1:
			m_pItemDescription[CLASSLOADOUT_NCO]->SetText( LoadItemDescription("resource/ui/descriptions/description_mk2grenade.txt") );
			m_pItemImage[CLASSLOADOUT_NCO]->SetImage( "images/weapons/mk2grenade" );
			break;
		// rifle grenades
		case 2:
			m_pItemDescription[CLASSLOADOUT_NCO]->SetText( LoadItemDescription("resource/ui/descriptions/description_riflegrenade.txt") );
			m_pItemImage[CLASSLOADOUT_NCO]->SetImage( "images/weapons/riflegrenade" );
			break;
		// Combat knife
		case 3:
			m_pItemDescription[CLASSLOADOUT_NCO]->SetText( LoadItemDescription("resource/ui/descriptions/description_m3knife.txt") );
			m_pItemImage[CLASSLOADOUT_NCO]->SetImage( "images/weapons/m3knife" );
			//m_pLoadoutModelTemp->SetModel( EQ_TRENCHKNIFE_MODEL );
			break;
		// extra ammo
		case 4:
			m_pItemDescription[CLASSLOADOUT_NCO]->SetText( LoadItemDescription("resource/ui/descriptions/description_ammo.txt") );
			m_pItemImage[CLASSLOADOUT_NCO]->SetImage( "images/weapons/ammo" );
			break;
		// helmet
		case 5:
			m_pItemDescription[CLASSLOADOUT_NCO]->SetText( LoadItemDescription("resource/ui/descriptions/description_helmet.txt") );
			m_pItemImage[CLASSLOADOUT_NCO]->SetImage( "images/weapons/helmet" );
			break;
		// bayonet
		case 6:
			//m_pAddEquipment[CLASSLOADOUT_NCO]->SetEnabled( false );
			//m_pRemoveEquipment[CLASSLOADOUT_NCO]->SetEnabled ( false );
			m_pItemDescription[CLASSLOADOUT_NCO]->SetText( LoadItemDescription("resource/ui/descriptions/description_garandbayonet.txt") );
			m_pItemImage[CLASSLOADOUT_NCO]->SetImage( "images/weapons/garandbayonet" );
			break;
		//EQUIPMENT_ALLIES_SMOKEGRENADES
		case 7:
			m_pItemDescription[CLASSLOADOUT_NCO]->SetText( LoadItemDescription("resource/ui/descriptions/description_smokegrenade.txt") );
			m_pItemImage[CLASSLOADOUT_NCO]->SetImage( "images/weapons/smokegrenade" );
			break;
		default:
			Warning("Loadout NCO, Wrong item index\n");
			break;
		}
	}
}

//pretty much ripped from the team menu, why reinvent the wheel ;) - nuke
const char *CRnLAlliedClassLoadoutMenu::LoadItemDescription( const char *szItemDescription )
{
	// if no map specific description exists, load default text
	if( !filesystem->FileExists( szItemDescription ) )
		return "";

	FileHandle_t f = filesystem->Open( szItemDescription, "r" );

	// read into a memory block
	int fileSize = filesystem->Size(f) ;
	wchar_t *memBlock = (wchar_t *)malloc(fileSize + sizeof(wchar_t));
	memset( memBlock, 0x0, fileSize + sizeof(wchar_t));
	filesystem->Read(memBlock, fileSize, f);

	// null-terminate the stream
	memBlock[fileSize / sizeof(wchar_t)] = 0x0000;

	// check the first character, make sure this a little-endian unicode file
	if (memBlock[0] != 0xFEFF)
	{
		// its a ascii char file
		return reinterpret_cast<char *>( memBlock );
	}
	else
	{
		return reinterpret_cast<char *>( memBlock+1 );
	}
	
	filesystem->Close( f );
	free(memBlock);
};

void CRnLAlliedClassLoadoutMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );
	m_pClassPanel[CLASSLOADOUT_RIFLEMAN]->SetBorder( null );//( pScheme->GetBorder("BaseBorder") );
	m_pClassPanel[CLASSLOADOUT_RADIOMAN]->SetBorder( null );//( pScheme->GetBorder("BaseBorder") );
	m_pClassPanel[CLASSLOADOUT_MEDIC]->SetBorder( null );//( pScheme->GetBorder("BaseBorder") );
	m_pClassPanel[CLASSLOADOUT_NCO]->SetBorder( null );//( pScheme->GetBorder("BaseBorder") );
	m_pInsetBorder->SetBorder( pScheme->GetBorder("BaseBorder") );
	m_pDefaultPanelLabel->SetFont( pScheme->GetFont("LoadoutMenu") );
	m_pDefaultPanel->SetBorder( null );//pScheme->GetBorder("BaseBorder") );

	SetBgColor( Color( 0,0,0,0) );
}

void CRnLAlliedClassLoadoutMenu::OnEquipmentItemSelected( KeyValues *data )
{
	//we can only supply description for one item at a time, so only the first
	int iUserDataID = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItemUserData( m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetSelectedItem(0) );
	
	//-1 is failed
	if(iUserDataID != -1)
	{
		switch(m_iPlayerLoadoutPanel)
		{
		case CLASSLOADOUT_RIFLEMAN:
				ChangeRiflemanDescription( false, iUserDataID );
				break;
		case CLASSLOADOUT_RADIOMAN:
				ChangeRadiomanDescription( false, iUserDataID );
				break;
		case CLASSLOADOUT_MEDIC:
				ChangeMedicDescription( false, iUserDataID );
				break;
		case CLASSLOADOUT_NCO:
				ChangeNCODescription( false, iUserDataID );
				break;
		}

	}
}

// Attach the equipment to the loadout player model
// the +1 is necessary to fit the enum which starts with 0 as weapon_none
// while here: weapon_colt == 0 in this class...sort of cheesy - Stefan
// Could be fixed by adding a EQUIPMENT_ALLIES_NONE
void CRnLAlliedClassLoadoutMenu::AttachEquipmentToViewPort( int iItemNumber)
{
	switch(iItemNumber + 1 )
	{
		case EQUIPMENT_ALLIES_FRAGGRENADES:
		case EQUIPMENT_ALLIES_SMOKEGRENADE:
			UpdateGrenadesInViewPort();
			break;
		case EQUIPMENT_ALLIES_COLT:
			// turn on the pistol
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( COLTHOLSTER_BODYGROUP ), 1 );
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( AMMO_POUCHES_BODYGROUP_SECONDARY ), 1 );
			// TODO: Perhaps reenable this when we have it
			//SetModel( m_pLoadoutWeaponModel, EQ_PIR_COLT );
			break;
		case EQUIPMENT_ALLIES_RIFLEGRENADES:
			// TODO: Get bodygroups for this
			break;
		case EQUIPMENT_ALLIES_MELEE:
			// TODO: Get bodygroups for this
			break;
		case EQUIPMENT_ALLIES_EXTRAAMMO:
			UpdatePouchesInViewPort();
			break;
		case EQUIPMENT_ALLIES_GARANDBAYONET:
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( BAYONET_BODYGROUP ), 1 );
			break;
		default:
			DevMsg("Invalid Item Index!\n");
			break;
	}
}
// Remove a piece of equipment
void CRnLAlliedClassLoadoutMenu::RemoveEquipmentFromViewPort( int iItemNumber )
{
	switch( iItemNumber + 1 )
	{
		case EQUIPMENT_ALLIES_FRAGGRENADES:
		case EQUIPMENT_ALLIES_SMOKEGRENADE:
			UpdateGrenadesInViewPort();
			break;
		case EQUIPMENT_ALLIES_COLT:
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( COLTHOLSTER_BODYGROUP ), 0 );
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( AMMO_POUCHES_BODYGROUP_SECONDARY ), 0 );
			SetModel( m_pLoadoutWeaponModel, EQ_PIR_M1GARAND );
			break;
		case EQUIPMENT_ALLIES_RIFLEGRENADES:
			// TODO: Get bodygroups for this
			break;
		case EQUIPMENT_ALLIES_MELEE:
			// TODO: Get bodygroups for this
			break;
		case EQUIPMENT_ALLIES_EXTRAAMMO:
			UpdatePouchesInViewPort();
			break;
		case EQUIPMENT_ALLIES_GARANDBAYONET:
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( BAYONET_BODYGROUP ), 0 );
			break;
		default:
			break;
	}

}

void CRnLAlliedClassLoadoutMenu::UpdateGrenadesInViewPort()
{
	KeyValues *GrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iFragGrenades[m_iPlayerLoadoutPanel]);
	KeyValues *SmokeGrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iSmokeGrenades[m_iPlayerLoadoutPanel]);

	// set relevant submodels
	if( GrenadeItem )
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( GRENADES_BODYGROUP ), GrenadeItem->GetInt("carry") );
	if( SmokeGrenadeItem )
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( SMOKES_BODYGROUP ), SmokeGrenadeItem->GetInt("carry") );
}

void CRnLAlliedClassLoadoutMenu::UpdatePouchesInViewPort()
{
	char szModelName[128];
	strcpy( szModelName, modelinfo->GetModelName( modelinfo->GetModel( m_pLoadoutWeaponModel->GetModelIndex() ) ) );

	if( !Q_strcmp( szModelName, EQ_PIR_M1GARAND ) )
	{
		if( HasExtraAmmo() )
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( AMMO_POUCHES_BODYGROUP ), GARAND_POUCHES_EXTRA );
		else
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( AMMO_POUCHES_BODYGROUP ), GARAND_POUCHES );
	}
	else if( !Q_strcmp( szModelName, EQ_PIR_THOMPSON ) )
	{
		// for now, show the bag which contains 30 round thompson mags
		//  TODO: implement the 30/20 round mags into the thompson weapon code!
		if( HasExtraAmmo() )
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( AMMO_POUCHES_BODYGROUP ), TOMMY_BAG );
		else
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( AMMO_POUCHES_BODYGROUP ), TOMMY_POUCHES );
	}
}

// Check whether we selected extra ammunition
bool CRnLAlliedClassLoadoutMenu::HasExtraAmmo( void )
{
	KeyValues *ExtraAmmo = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iExtraAmmo[m_iPlayerLoadoutPanel]);
	if( ExtraAmmo->GetInt("carry") > 0 )
		return true;
	else
		return false;
}

// call this whenever the class is changed, just sets some default equipment such as helmets and backpacks
void CRnLAlliedClassLoadoutMenu::ResetAttachments()
{
	// Hide the weapon pouches
	m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( AMMO_POUCHES_BODYGROUP ), 0 );

	if( m_pClassPanel[CLASSLOADOUT_NCO]->IsVisible() )
	{
		// show gear and helmet
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( GEAR_BODYGROUP ), 0 );
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( HELMET_BODYGROUP ), RandomInt(3, 4) );
		
		// nco has a map pouch
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( MAP_BODYGROUP ), 1 );

		// show the pistol if carrying it
		KeyValues *ColtItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iColt[m_iPlayerLoadoutPanel] );

		if( ColtItem->GetInt("carry") == 1 )
		{
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( COLTHOLSTER_BODYGROUP ), 1 );
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( AMMO_POUCHES_BODYGROUP_SECONDARY ), 1 );
		}
	}
	else
	{
		// show gear and helmet
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( GEAR_BODYGROUP ), 1 );
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( HELMET_BODYGROUP ), RandomInt(1, 2) );
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( COLTHOLSTER_BODYGROUP ), 0 );
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( MAP_BODYGROUP ), 0 );
	}
}

// Do some checks before we actually set a sequence or a model
// to make sure the model instances we created really exist - Stefan
void CRnLAlliedClassLoadoutMenu::SetModel( C_BaseAnimating *pModel, char cModelName[] )
{
	if( pModel )
		pModel->SetModel( cModelName );
}

void CRnLAlliedClassLoadoutMenu::DrawLoadoutModels( void )
{
	vgui::VPANEL console = 0;

	bool bFound = false;
	vgui::VPANEL panel = enginevgui->GetPanel( PANEL_GAMEUIDLL );

	for( int i = 0; i < vgui::ipanel()->GetChildCount(panel); i++)
	{
		bFound = !strcmp("GameConsole", vgui::ipanel()->GetName( vgui::ipanel()->GetChild( panel, i ) ) );
		
		if(bFound)
		{
				console = vgui::ipanel()->GetChild( panel, i );
				break;
		}
	}

	if( IsVisible() && m_bIsClassPanelVisible && !vgui::ipanel()->IsVisible( vgui::ipanel()->GetParent( console ) ))
	{
		// Now draw it.
		CViewSetup viewSetup;
		// setup the views location, size and fov (amongst others)
		int x, y;
		m_pInsetBorder->GetPos( x, y );
		viewSetup.x = x + (XRES(2));
		viewSetup.y = y + (YRES(2));
		m_pInsetBorder->GetSize( x, y );
		viewSetup.width = x - (XRES(4));
		viewSetup.height = y - (YRES(4));

		viewSetup.m_bOrtho = false;
		viewSetup.fov = 54;

		// make sure that we see all of the player model
		viewSetup.origin = vecCameraOrigin;

		viewSetup.angles = angCameraAngles;
		viewSetup.m_vUnreflectedOrigin = viewSetup.origin;
		viewSetup.zNear = VIEW_NEARZ;
		viewSetup.zFar = 1000;

		// render it out to the new CViewSetup area
		// it's possible that ViewSetup3D will be replaced in future code releases
		Frustum dummyFrustum;
		//view->RenderViewEx( viewSetup, 0, false );
		render->Push3DView( viewSetup, 0/*VIEW_CLEAR_DEPTH | VIEW_CLEAR_COLOR*/, NULL, dummyFrustum );

		if ( m_pLoadoutModel )
		{
			m_pLoadoutModel->DrawModel( STUDIO_RENDER );
		}

		if( m_pLoadoutWeaponModel )
		{
			m_pLoadoutWeaponModel->DrawModel( STUDIO_RENDER );
		}

		render->PopView( dummyFrustum );
	}
}