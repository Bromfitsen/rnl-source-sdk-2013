#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>

#include "rnl_axisclassloadoutmenu.h"

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
extern IGameUIFuncs *gameuifuncs; // for key binding details
#include <game/client/iviewport.h>

#include "engine/IEngineSound.h"

#include <stdlib.h> // MAX_PATH define

extern float CLASSBUTTON_XPOS[CLASSLOADOUT_AXIS_MAX];
//float CLASSBUTTON_XPOS[CLASSLOADOUT_AXIS_MAX] = {
//	84,  //CLASSLOADOUT_GRENADIER
//	160, //CLASSLOADOUT_MGSCHUTZE
//	236, //CLASSLOADOUT_FUNKER
//	312, //CLASSLOADOUT_SANITAETER
//	388  //CLASSLOADOUT_OFFIZIER
//};

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

#define CLASSPANEL_XSIZE		480
#define CLASSPANEL_YSIZE		420
#define CLASSPANEL_XPOS			80
#define AXIS_CLASSPANEL_YPOS	40

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

#define AXIS_PLAYER_HEER_RIFLEMANLOADOUT "models/player/heer/heer_rifleman01.mdl"
#define AXIS_PLAYER_HEER_RIFLEMAN2LOADOUT "models/player/heer/heer_rifleman02.mdl" //"models/player/heer/heer_rifleman02.mdl"
#define AXIS_PLAYER_HEER_NCOLOADOUT "models/player/heer/heer_nco01.mdl"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLAxisClassLoadoutMenu::CRnLAxisClassLoadoutMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_AXISCLASSLOADOUT)
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
	m_pDefaultPanel->SetPos( XRES( CLASSPANEL_XPOS ), YRES( AXIS_CLASSPANEL_YPOS ) );
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
	m_pClassButtons[CLASSLOADOUT_GRENADIER] = new Button(this, "grenadier", "Grenadier", this, "selectclass 5");
	m_pClassButtons[CLASSLOADOUT_MGSCHUTZE] = new Button(this, "mgschutze", "MG Schutze", this, "selectclass 6");
	m_pClassButtons[CLASSLOADOUT_FUNKER] = new Button(this, "funker", "Funker", this, "selectclass 7");
	m_pClassButtons[CLASSLOADOUT_SANITAETER] = new Button(this, "sanitaeter", "Sanitaeter", this, "selectclass 8");
	m_pClassButtons[CLASSLOADOUT_OFFIZIER] = new Button(this, "offizier", "Offizier", this, "selectclass 9");

	m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetPos(XRES(CLASSBUTTON_XPOS[CLASSLOADOUT_GRENADIER]), CLASSBUTTON_YPOS);
	m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetSize( CLASSBUTTON_XSIZE, CLASSBUTTON_YSIZE);
	m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetEnabled( false );
	m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetProportional( true );

	m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetPos(XRES(CLASSBUTTON_XPOS[CLASSLOADOUT_MGSCHUTZE]), CLASSBUTTON_YPOS);
	m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetSize( CLASSBUTTON_XSIZE, CLASSBUTTON_YSIZE);
	m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetEnabled( false );
	m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetProportional( true );

	m_pClassButtons[CLASSLOADOUT_FUNKER]->SetPos(XRES(CLASSBUTTON_XPOS[CLASSLOADOUT_FUNKER]), CLASSBUTTON_YPOS);
	m_pClassButtons[CLASSLOADOUT_FUNKER]->SetSize(CLASSBUTTON_XSIZE, CLASSBUTTON_YSIZE);
	m_pClassButtons[CLASSLOADOUT_FUNKER]->SetEnabled( false );
	m_pClassButtons[CLASSLOADOUT_FUNKER]->SetProportional( true );

	m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetPos(XRES(CLASSBUTTON_XPOS[CLASSLOADOUT_SANITAETER]), CLASSBUTTON_YPOS);
	m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetSize(CLASSBUTTON_XSIZE, CLASSBUTTON_YSIZE);
	m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetEnabled( false );
	m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetProportional( true );

	m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetPos(XRES(CLASSBUTTON_XPOS[CLASSLOADOUT_OFFIZIER]), CLASSBUTTON_YPOS);
	m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetSize(CLASSBUTTON_XSIZE, CLASSBUTTON_YSIZE);
	m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetEnabled( false );
	m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetProportional( true );

	//initialise our class panels
	InitializeFunkerPanel();
	InitializeSanitaeterPanel();
	InitializeOffizierPanel();
	InitializeGrenadierPanel();
	InitializeMGSchutzePanel();

	if( m_pClassPanel[CLASSLOADOUT_GRENADIER]) 
	{
		m_pClassPanel[CLASSLOADOUT_GRENADIER]->SetPos( XRES( CLASSPANEL_XPOS ), YRES( AXIS_CLASSPANEL_YPOS ) );
		m_pClassPanel[CLASSLOADOUT_GRENADIER]->SetSize( XRES( CLASSPANEL_XSIZE ), YRES( CLASSPANEL_YSIZE ));
		m_pClassPanel[CLASSLOADOUT_GRENADIER]->SetPaintBorderEnabled( true );
		
		m_pPrimaryWeaponLabel[CLASSLOADOUT_GRENADIER]->SetPos( XRES( PRIMARYWEAPL_XPOS ), YRES( PRIMARYWEAPL_YPOS ) );
		m_pPrimaryWeaponLabel[CLASSLOADOUT_GRENADIER]->SetSize( XRES( PRIMARYWEAPL_XSIZE ), YRES( PRIMARYWEAPL_YSIZE ) );

		m_pWeightLabel[CLASSLOADOUT_GRENADIER]->SetPos( XRES( WEIGHTL_XPOS ), YRES( WEIGHTL_YPOS ) );
		m_pWeightLabel[CLASSLOADOUT_GRENADIER]->SetSize( XRES( WEIGHTL_XSIZE ), YRES( WEIGHTL_YSIZE ) );

		m_pEquipmentLabel[CLASSLOADOUT_GRENADIER]->SetPos( XRES( EQUIPL_XPOS ), YRES( EQUIPL_YPOS ) );
		m_pEquipmentLabel[CLASSLOADOUT_GRENADIER]->SetSize( XRES( EQUIPL_XSIZE ), YRES( EQUIPL_YSIZE ) );

		m_pErrorLabel[CLASSLOADOUT_GRENADIER]->SetPos( XRES( ERRORLABEL_XPOS ), YRES( ERRORLABEL_YPOS ) );
		m_pErrorLabel[CLASSLOADOUT_GRENADIER]->SetSize( XRES( ERRORLABEL_XSIZE ), YRES( ERRORLABEL_YSIZE ) );
													  
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_GRENADIER]->SetPos( XRES( PRIMARYWEAP_XPOS ), YRES( PRIMARYWEAP_YPOS ) );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_GRENADIER]->SetSize( XRES( PRIMARYWEAP_XSIZE ), YRES( PRIMARYWEAP_YSIZE ) );
		
		m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->SetPos( XRES(AVAILABLEEQUIP_XPOS), YRES(AVAILABLEEQUIP_YPOS) );
		m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->SetSize( XRES(AVAILABLEEQUIP_XSIZE), YRES(AVAILABLEEQUIP_YSIZE) );

		m_pAddEquipment[CLASSLOADOUT_GRENADIER]->SetPos( XRES(ADDEQUIP_XPOS), YRES(ADDEQUIP_YPOS) );
		m_pAddEquipment[CLASSLOADOUT_GRENADIER]->SetSize( XRES(ADDEQUIP_XSIZE), YRES(ADDEQUIP_YSIZE) );

		m_pRemoveEquipment[CLASSLOADOUT_GRENADIER]->SetPos( XRES(REMEQUIP_XPOS), YRES(REMEQUIP_YPOS) );
		m_pRemoveEquipment[CLASSLOADOUT_GRENADIER]->SetSize( XRES(REMEQUIP_XSIZE), YRES(REMEQUIP_YSIZE) );

		m_pSubmitCommand[CLASSLOADOUT_GRENADIER]->SetPos( XRES(SUBMIT_XPOS), YRES(SUBMIT_YPOS) );
		m_pSubmitCommand[CLASSLOADOUT_GRENADIER]->SetSize( XRES(SUBMIT_XSIZE), YRES(SUBMIT_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_GRENADIER][0]->SetPos( XRES(ROTATE_XPOS), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_GRENADIER][0]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_GRENADIER][1]->SetPos( XRES(ROTATE_XPOS + (2 * ROTATE_XSIZE)), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_GRENADIER][1]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pWeight[CLASSLOADOUT_GRENADIER]->SetPos( XRES(WEIGHT_XPOS), YRES(WEIGHT_YPOS) );
		m_pWeight[CLASSLOADOUT_GRENADIER]->SetSize( XRES(WEIGHT_XSIZE), YRES(WEIGHT_YSIZE) );

		m_pWeightProgress[CLASSLOADOUT_GRENADIER]->SetPos( XRES(WEIGHTPROGRESS_XPOS), YRES(WEIGHTPROGRESS_YPOS) );
		m_pWeightProgress[CLASSLOADOUT_GRENADIER]->SetSize( XRES(WEIGHTPROGRESS_XSIZE), YRES(WEIGHTPROGRESS_YSIZE) );
	
		m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetPos( XRES(ITEMDESC_XPOS), YRES(ITEMDESC_YPOS) );
		m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetSize( XRES(ITEMDESC_XSIZE), YRES(ITEMDESC_YSIZE) );

		m_pItemImage[CLASSLOADOUT_GRENADIER]->SetPos( XRES(ITEMIMG_XPOS), YRES(ITEMIMG_YPOS) );
		m_pItemImage[CLASSLOADOUT_GRENADIER]->SetSize( XRES(ITEMIMG_XSIZE), YRES(ITEMIMG_YSIZE) );

		m_pCancelCommand[CLASSLOADOUT_GRENADIER]->SetPos( XRES(CANCEL_XPOS), YRES(CANCEL_YPOS) );
		m_pCancelCommand[CLASSLOADOUT_GRENADIER]->SetSize( XRES(CANCEL_XSIZE), YRES(CANCEL_YSIZE) );

		m_pInsetBorder->SetSize( XRES(INSET_XSIZE), YRES(INSET_YSIZE) );
		m_pInsetBorder->SetPos( XRES(INSET_XPOS), YRES(INSET_YPOS) );
	}
	if( m_pClassPanel[CLASSLOADOUT_MGSCHUTZE]) 
	{
		m_pClassPanel[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( CLASSPANEL_XPOS ), YRES( AXIS_CLASSPANEL_YPOS ) );
		m_pClassPanel[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES( CLASSPANEL_XSIZE ), YRES( CLASSPANEL_YSIZE ));
		m_pClassPanel[CLASSLOADOUT_MGSCHUTZE]->SetPaintBorderEnabled( true );
		
		m_pPrimaryWeaponLabel[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( PRIMARYWEAPL_XPOS ), YRES( PRIMARYWEAPL_YPOS ) );
		m_pPrimaryWeaponLabel[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES( PRIMARYWEAPL_XSIZE ), YRES( PRIMARYWEAPL_YSIZE ) );

		m_pWeightLabel[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( WEIGHTL_XPOS ), YRES( WEIGHTL_YPOS ) );
		m_pWeightLabel[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES( WEIGHTL_XSIZE ), YRES( WEIGHTL_YSIZE ) );

		m_pEquipmentLabel[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( EQUIPL_XPOS ), YRES( EQUIPL_YPOS ) );
		m_pEquipmentLabel[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES( EQUIPL_XSIZE ), YRES( EQUIPL_YSIZE ) );

		m_pErrorLabel[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( ERRORLABEL_XPOS ), YRES( ERRORLABEL_YPOS ) );
		m_pErrorLabel[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES( ERRORLABEL_XSIZE ), YRES( ERRORLABEL_YSIZE ) );
													  
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( PRIMARYWEAP_XPOS ), YRES( PRIMARYWEAP_YPOS ) );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES( PRIMARYWEAP_XSIZE ), YRES( PRIMARYWEAP_YSIZE ) );
		
		m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES(AVAILABLEEQUIP_XPOS), YRES(AVAILABLEEQUIP_YPOS) );
		m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES(AVAILABLEEQUIP_XSIZE), YRES(AVAILABLEEQUIP_YSIZE) );

		m_pAddEquipment[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES(ADDEQUIP_XPOS), YRES(ADDEQUIP_YPOS) );
		m_pAddEquipment[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES(ADDEQUIP_XSIZE), YRES(ADDEQUIP_YSIZE) );

		m_pRemoveEquipment[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES(REMEQUIP_XPOS), YRES(REMEQUIP_YPOS) );
		m_pRemoveEquipment[CLASSLOADOUT_GRENADIER]->SetSize( XRES(REMEQUIP_XSIZE), YRES(REMEQUIP_YSIZE) );

		m_pSubmitCommand[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES(SUBMIT_XPOS), YRES(SUBMIT_YPOS) );
		m_pSubmitCommand[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES(SUBMIT_XSIZE), YRES(SUBMIT_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_MGSCHUTZE][0]->SetPos( XRES(ROTATE_XPOS), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_MGSCHUTZE][0]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_MGSCHUTZE][1]->SetPos( XRES(ROTATE_XPOS + (2 * ROTATE_XSIZE)), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_MGSCHUTZE][1]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pWeight[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES(WEIGHT_XPOS), YRES(WEIGHT_YPOS) );
		m_pWeight[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES(WEIGHT_XSIZE), YRES(WEIGHT_YSIZE) );

		m_pWeightProgress[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES(WEIGHTPROGRESS_XPOS), YRES(WEIGHTPROGRESS_YPOS) );
		m_pWeightProgress[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES(WEIGHTPROGRESS_XSIZE), YRES(WEIGHTPROGRESS_YSIZE) );
	
		m_pItemDescription[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES(ITEMDESC_XPOS), YRES(ITEMDESC_YPOS) );
		m_pItemDescription[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES(ITEMDESC_XSIZE), YRES(ITEMDESC_YSIZE) );

		m_pItemImage[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES(ITEMIMG_XPOS), YRES(ITEMIMG_YPOS) );
		m_pItemImage[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES(ITEMIMG_XSIZE), YRES(ITEMIMG_YSIZE) );

		m_pCancelCommand[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES(CANCEL_XPOS), YRES(CANCEL_YPOS) );
		m_pCancelCommand[CLASSLOADOUT_MGSCHUTZE]->SetSize( XRES(CANCEL_XSIZE), YRES(CANCEL_YSIZE) );

		m_pInsetBorder->SetSize( XRES(INSET_XSIZE), YRES(INSET_YSIZE) );
		m_pInsetBorder->SetPos( XRES(INSET_XPOS), YRES(INSET_YPOS) );
	}
	if( m_pClassPanel[CLASSLOADOUT_FUNKER]) 
	{
		m_pClassPanel[CLASSLOADOUT_FUNKER]->SetPos( XRES( CLASSPANEL_XPOS ), YRES( AXIS_CLASSPANEL_YPOS ) );
		m_pClassPanel[CLASSLOADOUT_FUNKER]->SetSize( XRES( CLASSPANEL_XSIZE ), YRES( CLASSPANEL_YSIZE ));
		m_pClassPanel[CLASSLOADOUT_FUNKER]->SetPaintBorderEnabled( true );
		
		m_pPrimaryWeaponLabel[CLASSLOADOUT_FUNKER]->SetPos( XRES( PRIMARYWEAPL_XPOS ), YRES( PRIMARYWEAPL_YPOS ) );
		m_pPrimaryWeaponLabel[CLASSLOADOUT_FUNKER]->SetSize( XRES( PRIMARYWEAPL_XSIZE ), YRES( PRIMARYWEAPL_YSIZE ) );

		m_pWeightLabel[CLASSLOADOUT_FUNKER]->SetPos( XRES( WEIGHTL_XPOS ), YRES( WEIGHTL_YPOS ) );
		m_pWeightLabel[CLASSLOADOUT_FUNKER]->SetSize( XRES( WEIGHTL_XSIZE ), YRES( WEIGHTL_YSIZE ) );

		m_pEquipmentLabel[CLASSLOADOUT_FUNKER]->SetPos( XRES( EQUIPL_XPOS ), YRES( EQUIPL_YPOS ) );
		m_pEquipmentLabel[CLASSLOADOUT_FUNKER]->SetSize( XRES( EQUIPL_XSIZE ), YRES( EQUIPL_YSIZE ) );

		m_pErrorLabel[CLASSLOADOUT_FUNKER]->SetPos( XRES( ERRORLABEL_XPOS ), YRES( ERRORLABEL_YPOS ) );
		m_pErrorLabel[CLASSLOADOUT_FUNKER]->SetSize( XRES( ERRORLABEL_XSIZE ), YRES( ERRORLABEL_YSIZE ) );
													  
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_FUNKER]->SetPos( XRES( PRIMARYWEAP_XPOS ), YRES( PRIMARYWEAP_YPOS ) );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_FUNKER]->SetSize( XRES( PRIMARYWEAP_XSIZE ), YRES( PRIMARYWEAP_YSIZE ) );
		
		m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->SetPos( XRES(AVAILABLEEQUIP_XPOS), YRES(AVAILABLEEQUIP_YPOS) );
		m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->SetSize( XRES(AVAILABLEEQUIP_XSIZE), YRES(AVAILABLEEQUIP_YSIZE) );

		m_pAddEquipment[CLASSLOADOUT_FUNKER]->SetPos( XRES(ADDEQUIP_XPOS), YRES(ADDEQUIP_YPOS) );
		m_pAddEquipment[CLASSLOADOUT_FUNKER]->SetSize( XRES(ADDEQUIP_XSIZE), YRES(ADDEQUIP_YSIZE) );

		m_pRemoveEquipment[CLASSLOADOUT_FUNKER]->SetPos( XRES(REMEQUIP_XPOS), YRES(REMEQUIP_YPOS) );
		m_pRemoveEquipment[CLASSLOADOUT_FUNKER]->SetSize( XRES(REMEQUIP_XSIZE), YRES(REMEQUIP_YSIZE) );

		m_pSubmitCommand[CLASSLOADOUT_FUNKER]->SetPos( XRES(SUBMIT_XPOS), YRES(SUBMIT_YPOS) );
		m_pSubmitCommand[CLASSLOADOUT_FUNKER]->SetSize( XRES(SUBMIT_XSIZE), YRES(SUBMIT_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_FUNKER][0]->SetPos( XRES(ROTATE_XPOS), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_FUNKER][0]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_FUNKER][1]->SetPos( XRES(ROTATE_XPOS + (2 * ROTATE_XSIZE)), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_FUNKER][1]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pWeight[CLASSLOADOUT_FUNKER]->SetPos( XRES(WEIGHT_XPOS), YRES(WEIGHT_YPOS) );
		m_pWeight[CLASSLOADOUT_FUNKER]->SetSize( XRES(WEIGHT_XSIZE), YRES(WEIGHT_YSIZE) );

		m_pWeightProgress[CLASSLOADOUT_FUNKER]->SetPos( XRES(WEIGHTPROGRESS_XPOS), YRES(WEIGHTPROGRESS_YPOS) );
		m_pWeightProgress[CLASSLOADOUT_FUNKER]->SetSize( XRES(WEIGHTPROGRESS_XSIZE), YRES(WEIGHTPROGRESS_YSIZE) );
	
		m_pItemDescription[CLASSLOADOUT_FUNKER]->SetPos( XRES(ITEMDESC_XPOS), YRES(ITEMDESC_YPOS) );
		m_pItemDescription[CLASSLOADOUT_FUNKER]->SetSize( XRES(ITEMDESC_XSIZE), YRES(ITEMDESC_YSIZE) );

		m_pItemImage[CLASSLOADOUT_FUNKER]->SetPos( XRES(ITEMIMG_XPOS), YRES(ITEMIMG_YPOS) );
		m_pItemImage[CLASSLOADOUT_FUNKER]->SetSize( XRES(ITEMIMG_XSIZE), YRES(ITEMIMG_YSIZE) );

		m_pCancelCommand[CLASSLOADOUT_FUNKER]->SetPos( XRES(CANCEL_XPOS), YRES(CANCEL_YPOS) );
		m_pCancelCommand[CLASSLOADOUT_FUNKER]->SetSize( XRES(CANCEL_XSIZE), YRES(CANCEL_YSIZE) );

		m_pInsetBorder->SetSize( XRES(INSET_XSIZE), YRES(INSET_YSIZE) );
		m_pInsetBorder->SetPos( XRES(INSET_XPOS), YRES(INSET_YPOS) );
	}
	if( m_pClassPanel[CLASSLOADOUT_SANITAETER]) 
	{
		m_pClassPanel[CLASSLOADOUT_SANITAETER]->SetPos( XRES( CLASSPANEL_XPOS ), YRES( AXIS_CLASSPANEL_YPOS ) );
		m_pClassPanel[CLASSLOADOUT_SANITAETER]->SetSize( XRES( CLASSPANEL_XSIZE ), YRES( CLASSPANEL_YSIZE ));
		m_pClassPanel[CLASSLOADOUT_SANITAETER]->SetPaintBorderEnabled( true );
		
		m_pPrimaryWeaponLabel[CLASSLOADOUT_SANITAETER]->SetPos( XRES( PRIMARYWEAPL_XPOS ), YRES( PRIMARYWEAPL_YPOS ) );
		m_pPrimaryWeaponLabel[CLASSLOADOUT_SANITAETER]->SetSize( XRES( PRIMARYWEAPL_XSIZE ), YRES( PRIMARYWEAPL_YSIZE ) );

		m_pWeightLabel[CLASSLOADOUT_SANITAETER]->SetPos( XRES( WEIGHTL_XPOS ), YRES( WEIGHTL_YPOS ) );
		m_pWeightLabel[CLASSLOADOUT_SANITAETER]->SetSize( XRES( WEIGHTL_XSIZE ), YRES( WEIGHTL_YSIZE ) );

		m_pEquipmentLabel[CLASSLOADOUT_SANITAETER]->SetPos( XRES( EQUIPL_XPOS ), YRES( EQUIPL_YPOS ) );
		m_pEquipmentLabel[CLASSLOADOUT_SANITAETER]->SetSize( XRES( EQUIPL_XSIZE ), YRES( EQUIPL_YSIZE ) );

		m_pErrorLabel[CLASSLOADOUT_SANITAETER]->SetPos( XRES( ERRORLABEL_XPOS ), YRES( ERRORLABEL_YPOS ) );
		m_pErrorLabel[CLASSLOADOUT_SANITAETER]->SetSize( XRES( ERRORLABEL_XSIZE ), YRES( ERRORLABEL_YSIZE ) );
													  
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_SANITAETER]->SetPos( XRES( PRIMARYWEAP_XPOS ), YRES( PRIMARYWEAP_YPOS ) );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_SANITAETER]->SetSize( XRES( PRIMARYWEAP_XSIZE ), YRES( PRIMARYWEAP_YSIZE ) );
		
		m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->SetPos( XRES(AVAILABLEEQUIP_XPOS), YRES(AVAILABLEEQUIP_YPOS) );
		m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->SetSize( XRES(AVAILABLEEQUIP_XSIZE), YRES(AVAILABLEEQUIP_YSIZE) );

		m_pAddEquipment[CLASSLOADOUT_SANITAETER]->SetPos( XRES(ADDEQUIP_XPOS), YRES(ADDEQUIP_YPOS) );
		m_pAddEquipment[CLASSLOADOUT_SANITAETER]->SetSize( XRES(ADDEQUIP_XSIZE), YRES(ADDEQUIP_YSIZE) );

		m_pRemoveEquipment[CLASSLOADOUT_SANITAETER]->SetPos( XRES(REMEQUIP_XPOS), YRES(REMEQUIP_YPOS) );
		m_pRemoveEquipment[CLASSLOADOUT_SANITAETER]->SetSize( XRES(REMEQUIP_XSIZE), YRES(REMEQUIP_YSIZE) );

		m_pSubmitCommand[CLASSLOADOUT_SANITAETER]->SetPos( XRES(SUBMIT_XPOS), YRES(SUBMIT_YPOS) );
		m_pSubmitCommand[CLASSLOADOUT_SANITAETER]->SetSize( XRES(SUBMIT_XSIZE), YRES(SUBMIT_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_SANITAETER][0]->SetPos( XRES(ROTATE_XPOS), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_SANITAETER][0]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_SANITAETER][1]->SetPos( XRES(ROTATE_XPOS + (2 * ROTATE_XSIZE)), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_SANITAETER][1]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pWeight[CLASSLOADOUT_SANITAETER]->SetPos( XRES(WEIGHT_XPOS), YRES(WEIGHT_YPOS) );
		m_pWeight[CLASSLOADOUT_SANITAETER]->SetSize( XRES(WEIGHT_XSIZE), YRES(WEIGHT_YSIZE) );

		m_pWeightProgress[CLASSLOADOUT_SANITAETER]->SetPos( XRES(WEIGHTPROGRESS_XPOS), YRES(WEIGHTPROGRESS_YPOS) );
		m_pWeightProgress[CLASSLOADOUT_SANITAETER]->SetSize( XRES(WEIGHTPROGRESS_XSIZE), YRES(WEIGHTPROGRESS_YSIZE) );
	
		m_pItemDescription[CLASSLOADOUT_SANITAETER]->SetPos( XRES(ITEMDESC_XPOS), YRES(ITEMDESC_YPOS) );
		m_pItemDescription[CLASSLOADOUT_SANITAETER]->SetSize( XRES(ITEMDESC_XSIZE), YRES(ITEMDESC_YSIZE) );

		m_pItemImage[CLASSLOADOUT_SANITAETER]->SetPos( XRES(ITEMIMG_XPOS), YRES(ITEMIMG_YPOS) );
		m_pItemImage[CLASSLOADOUT_SANITAETER]->SetSize( XRES(ITEMIMG_XSIZE), YRES(ITEMIMG_YSIZE) );

		m_pCancelCommand[CLASSLOADOUT_SANITAETER]->SetPos( XRES(CANCEL_XPOS), YRES(CANCEL_YPOS) );
		m_pCancelCommand[CLASSLOADOUT_SANITAETER]->SetSize( XRES(CANCEL_XSIZE), YRES(CANCEL_YSIZE) );

		m_pInsetBorder->SetSize( XRES(INSET_XSIZE), YRES(INSET_YSIZE) );
		m_pInsetBorder->SetPos( XRES(INSET_XPOS), YRES(INSET_YPOS) );
	}
	if( m_pClassPanel[CLASSLOADOUT_OFFIZIER]) 
	{
		m_pClassPanel[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( CLASSPANEL_XPOS ), YRES( AXIS_CLASSPANEL_YPOS ) );
		m_pClassPanel[CLASSLOADOUT_OFFIZIER]->SetSize( XRES( CLASSPANEL_XSIZE ), YRES( CLASSPANEL_YSIZE ));
		m_pClassPanel[CLASSLOADOUT_OFFIZIER]->SetPaintBorderEnabled( true );
		
		m_pPrimaryWeaponLabel[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( PRIMARYWEAPL_XPOS ), YRES( PRIMARYWEAPL_YPOS ) );
		m_pPrimaryWeaponLabel[CLASSLOADOUT_OFFIZIER]->SetSize( XRES( PRIMARYWEAPL_XSIZE ), YRES( PRIMARYWEAPL_YSIZE ) );

		m_pWeightLabel[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( WEIGHTL_XPOS ), YRES( WEIGHTL_YPOS ) );
		m_pWeightLabel[CLASSLOADOUT_OFFIZIER]->SetSize( XRES( WEIGHTL_XSIZE ), YRES( WEIGHTL_YSIZE ) );

		m_pEquipmentLabel[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( EQUIPL_XPOS ), YRES( EQUIPL_YPOS ) );
		m_pEquipmentLabel[CLASSLOADOUT_OFFIZIER]->SetSize( XRES( EQUIPL_XSIZE ), YRES( EQUIPL_YSIZE ) );

		m_pErrorLabel[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( ERRORLABEL_XPOS ), YRES( ERRORLABEL_YPOS ) );
		m_pErrorLabel[CLASSLOADOUT_OFFIZIER]->SetSize( XRES( ERRORLABEL_XSIZE ), YRES( ERRORLABEL_YSIZE ) );
													  
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( PRIMARYWEAP_XPOS ), YRES( PRIMARYWEAP_YPOS ) );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_OFFIZIER]->SetSize( XRES( PRIMARYWEAP_XSIZE ), YRES( PRIMARYWEAP_YSIZE ) );
		
		m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->SetPos( XRES(AVAILABLEEQUIP_XPOS), YRES(AVAILABLEEQUIP_YPOS) );
		m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->SetSize( XRES(AVAILABLEEQUIP_XSIZE), YRES(AVAILABLEEQUIP_YSIZE) );

		m_pAddEquipment[CLASSLOADOUT_OFFIZIER]->SetPos( XRES(ADDEQUIP_XPOS), YRES(ADDEQUIP_YPOS) );
		m_pAddEquipment[CLASSLOADOUT_OFFIZIER]->SetSize( XRES(ADDEQUIP_XSIZE), YRES(ADDEQUIP_YSIZE) );

		m_pRemoveEquipment[CLASSLOADOUT_OFFIZIER]->SetPos( XRES(REMEQUIP_XPOS), YRES(REMEQUIP_YPOS) );
		m_pRemoveEquipment[CLASSLOADOUT_OFFIZIER]->SetSize( XRES(REMEQUIP_XSIZE), YRES(REMEQUIP_YSIZE) );

		m_pSubmitCommand[CLASSLOADOUT_OFFIZIER]->SetPos( XRES(SUBMIT_XPOS), YRES(SUBMIT_YPOS) );
		m_pSubmitCommand[CLASSLOADOUT_OFFIZIER]->SetSize( XRES(SUBMIT_XSIZE), YRES(SUBMIT_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_OFFIZIER][0]->SetPos( XRES(ROTATE_XPOS), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_OFFIZIER][0]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pRotateCommand[CLASSLOADOUT_OFFIZIER][1]->SetPos( XRES(ROTATE_XPOS + (2 * ROTATE_XSIZE)), YRES(ROTATE_YPOS) );
		m_pRotateCommand[CLASSLOADOUT_OFFIZIER][1]->SetSize( XRES(ROTATE_XSIZE), YRES(ROTATE_YSIZE) );

		m_pWeight[CLASSLOADOUT_OFFIZIER]->SetPos( XRES(WEIGHT_XPOS), YRES(WEIGHT_YPOS) );
		m_pWeight[CLASSLOADOUT_OFFIZIER]->SetSize( XRES(WEIGHT_XSIZE), YRES(WEIGHT_YSIZE) );

		m_pWeightProgress[CLASSLOADOUT_OFFIZIER]->SetPos( XRES(WEIGHTPROGRESS_XPOS), YRES(WEIGHTPROGRESS_YPOS) );
		m_pWeightProgress[CLASSLOADOUT_OFFIZIER]->SetSize( XRES(WEIGHTPROGRESS_XSIZE), YRES(WEIGHTPROGRESS_YSIZE) );
	
		m_pItemDescription[CLASSLOADOUT_OFFIZIER]->SetPos( XRES(ITEMDESC_XPOS), YRES(ITEMDESC_YPOS) );
		m_pItemDescription[CLASSLOADOUT_OFFIZIER]->SetSize( XRES(ITEMDESC_XSIZE), YRES(ITEMDESC_YSIZE) );

		m_pItemImage[CLASSLOADOUT_OFFIZIER]->SetPos( XRES(ITEMIMG_XPOS), YRES(ITEMIMG_YPOS) );
		m_pItemImage[CLASSLOADOUT_OFFIZIER]->SetSize( XRES(ITEMIMG_XSIZE), YRES(ITEMIMG_YSIZE) );

		m_pCancelCommand[CLASSLOADOUT_OFFIZIER]->SetPos( XRES(CANCEL_XPOS), YRES(CANCEL_YPOS) );
		m_pCancelCommand[CLASSLOADOUT_OFFIZIER]->SetSize( XRES(CANCEL_XSIZE), YRES(CANCEL_YSIZE) );

		m_pInsetBorder->SetSize( XRES(INSET_XSIZE), YRES(INSET_YSIZE) );
		m_pInsetBorder->SetPos( XRES(INSET_XPOS), YRES(INSET_YPOS) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CRnLAxisClassLoadoutMenu::~CRnLAxisClassLoadoutMenu()
{
}

//-----------------------------------------------------------------------------
// Purpose: Called when the user picks a class
//-----------------------------------------------------------------------------
void CRnLAxisClassLoadoutMenu::OnCommand( const char *command)
{
	//if anything but vguicancel, send it as a client command
	if ( FStrEq( command, "vguicancel" ) )
	{
		Close();
		gViewPortInterface->ShowBackGround( false );
		gViewPortInterface->ShowPanel( PANEL_TEAM, true );
		gViewPortInterface->ShowPanel( PANEL_AXISCLASSLOADOUT, false );
		char szTeam[64];
		strcpy(szTeam, "jointeam");
		engine->ClientCmd( szTeam );
		if ( nSelectionSongGuid != 0 )
		{
			enginesound->StopSoundByGuid( nSelectionSongGuid );
			nSelectionSongGuid = 0;
		}
		// TODO: the player joins the Axis team regardless what panel it being chosen 
		
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
		DevMsg( szLoadOut );
		engine->ClientCmd( szLoadOut );
		if ( nSelectionSongGuid != 0 )
		{
			enginesound->StopSoundByGuid( nSelectionSongGuid );
			nSelectionSongGuid = 0;
		}

		//disable the submit button so it can't be pressed multiple times
		/*m_pSubmitCommand[ m_iPlayerLoadoutPanel ]->SetEnabled(false);
		EnableClassButtons( false );*/
		// Don't disable it yet...first., it's being hidden anyway and second, the menu does not know whether the loadout succeded or not - Stefan		
	}
	else if ( FStrEq( command, "add" ) )
	{
		int iCount = m_pAvailableEquipment[ m_iPlayerLoadoutPanel ]->GetSelectedItemsCount();

		KeyValues *itemData;
		for( int i = 0; i < iCount; i++ )
		{
			int j = m_pAvailableEquipment[ m_iPlayerLoadoutPanel ]->GetSelectedItem( i );
			itemData = m_pAvailableEquipment[ m_iPlayerLoadoutPanel ]->GetItem( j );
			
			if( ( m_iCurrentWeight[m_iPlayerLoadoutPanel] + itemData->GetInt("weight") ) <= iMaxAxisClassSlots[m_iPlayerLoadoutPanel] )
			{	
				// The Heer NCO cannot have more than 2 grenades attached. so check whether there are two already - Stefan
				/*if( m_pClassPanel[CLASSLOADOUT_OFFIZIER]->IsVisible() && j == EQUIPMENT_AXIS_STIELHANDGRANATE)
				{
					if(itemData->GetInt("Carry") => 2)
						return;
				}*/
				
				// Check whether we got too many grenades, before we add new smoke grenades
				KeyValues *GrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iFragGrenades[m_iPlayerLoadoutPanel]);
				KeyValues *SmokeGrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iSmokeGrenades[m_iPlayerLoadoutPanel]);

				// Only the NCO has them, so it would crash for any other class - Stefan
				if( SmokeGrenadeItem )
				{
					// If there are too many of that already
					if( GrenadeItem->GetInt("carry") + SmokeGrenadeItem->GetInt("carry") >= 4 )
						return;
				}

				if(( itemData->GetInt("carry") + 1 )  <= itemData->GetInt("max") )
				{
					// Update the error label, apparantly nothing went wrong
					m_pErrorLabel[ m_iPlayerLoadoutPanel ]->SetText("");
					// Update the the amount of carried items and their weigth
					itemData->SetInt("carry", itemData->GetInt("carry") + 1);
					m_iCurrentWeight[m_iPlayerLoadoutPanel] += itemData->GetInt("weight");
					// and show it as well
					char szWeight[64];
					sprintf(szWeight, "%i / %i", m_iCurrentWeight[m_iPlayerLoadoutPanel], iMaxAxisClassSlots[m_iPlayerLoadoutPanel]);
					m_pWeight[m_iPlayerLoadoutPanel]->SetText(szWeight);

					//aswell as the label, need to set the progress bar
					m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAxisClassSlots[m_iPlayerLoadoutPanel] );
				
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
		sprintf(szWeight, "%i / %i", m_iCurrentWeight[m_iPlayerLoadoutPanel], iMaxAxisClassSlots[m_iPlayerLoadoutPanel]);
		m_pWeight[m_iPlayerLoadoutPanel]->SetText(szWeight);
		
		//aswell as the label, need to set the progress bar
		m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAxisClassSlots[m_iPlayerLoadoutPanel] );
		// And reenable the submit button - Stefan
		if( !m_pSubmitCommand[ m_iPlayerLoadoutPanel]->IsEnabled() )
            m_pSubmitCommand[ m_iPlayerLoadoutPanel ]->SetEnabled(false);

		m_pErrorLabel[ m_iPlayerLoadoutPanel ]->SetText("");
	}
	else if( FStrEq( command, "selectclass 5" ) )
	{
		m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetEnabled( false );

		if( m_iNextClass == AXIS_GRENADIER )
			return;

		m_pClassButtons[m_iPlayerLoadoutPanel]->SetEnabled( true );

		m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_FUNKER]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

		//hide all panels
		for( int i = 0; i < CLASSLOADOUT_AXIS_MAX; i++)
		{
			m_pClassPanel[i]->SetVisible( false );
		}
		m_pDefaultPanel->SetVisible( false );

		//this little modification is needed as there are only 4 panels, yet "5" classes (CLASS_NONE) - nuke
		m_iPlayerLoadoutPanel = CLASSLOADOUT_GRENADIER;
		m_bIsClassPanelVisible = true;
		m_pInsetBorder->SetVisible( true );
		m_pClassPanel[CLASSLOADOUT_GRENADIER]->SetVisible(true);

		//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_RIFLEMANLOADOUT );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_GRENADIER]->ActivateItemByRow(0);
		ChangeGrenadierDescription( true, WEAPON_K98K );
		m_iNextClass = AXIS_GRENADIER;
		return;
	}
	else if( FStrEq( command, "selectclass 6" ) )
	{
		m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetEnabled( false );

		if( m_iNextClass == AXIS_MGSCHUTZE )
			return;

		m_pClassButtons[m_iPlayerLoadoutPanel]->SetEnabled( true );

		m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_FUNKER]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

		//hide all panels
		for( int i = 0; i < CLASSLOADOUT_AXIS_MAX; i++)
		{
			m_pClassPanel[i]->SetVisible( false );
		}
		m_pDefaultPanel->SetVisible( false );

		//this little modification is needed as there are only 4 panels, yet "5" classes (CLASS_NONE) - nuke
		m_iPlayerLoadoutPanel = CLASSLOADOUT_MGSCHUTZE;
		m_bIsClassPanelVisible = true;
		m_pInsetBorder->SetVisible( true );
		m_pClassPanel[CLASSLOADOUT_MGSCHUTZE]->SetVisible(true);

		//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_RIFLEMANLOADOUT );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_MGSCHUTZE]->ActivateItemByRow(0);
		ChangeMGSchutzeDescription( true, WEAPON_MG42 );
		m_iNextClass = AXIS_MGSCHUTZE;
		return;
	}
	else if( FStrEq( command, "selectclass 7" ) )
	{
		m_pClassButtons[CLASSLOADOUT_FUNKER]->SetEnabled( false );

		if( m_iNextClass == AXIS_FUNKER )
			return;

		m_pClassButtons[m_iPlayerLoadoutPanel]->SetEnabled( true );

		m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_FUNKER]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

		//hide all panels
		for( int i = 0; i < CLASSLOADOUT_AXIS_MAX; i++)
		{
			m_pClassPanel[i]->SetVisible( false );
		}
		m_pDefaultPanel->SetVisible( false );

		//this little modification is needed as there are only 4 panels, yet "5" classes (CLASS_NONE) - nuke
		m_iPlayerLoadoutPanel = CLASSLOADOUT_FUNKER;
		m_bIsClassPanelVisible = true;
		m_pInsetBorder->SetVisible( true );
		m_pClassPanel[CLASSLOADOUT_FUNKER]->SetVisible(true);

		//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_RIFLEMANLOADOUT );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_FUNKER]->ActivateItemByRow(0);
		ChangeFunkerDescription( true, WEAPON_G43 );
		m_iNextClass = AXIS_FUNKER;
		return;
	}
	else if( FStrEq( command, "selectclass 8" ) )
	{
		m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetEnabled( false );

		if( m_iNextClass == AXIS_SANITAETER )
			return;

		m_pClassButtons[m_iPlayerLoadoutPanel]->SetEnabled( true );

		m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_FUNKER]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

		//hide all panels
		for( int i = 0; i < CLASSLOADOUT_AXIS_MAX; i++)
		{
			m_pClassPanel[i]->SetVisible( false );
		}
		m_pDefaultPanel->SetVisible( false );

		//this little modification is needed as there are only 4 panels, yet "5" classes (CLASS_NONE) - nuke
		m_iPlayerLoadoutPanel = CLASSLOADOUT_SANITAETER;
		m_bIsClassPanelVisible = true;
		m_pInsetBorder->SetVisible( true );
		m_pClassPanel[CLASSLOADOUT_SANITAETER]->SetVisible(true);

		//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_RIFLEMANLOADOUT );
		m_pPrimaryWeaponComboBox[AXIS_SANITAETER]->ActivateItemByRow(0);
		ChangeSanitaeterDescription( true, WEAPON_NONE );
		m_iNextClass = AXIS_SANITAETER;
		return;
	}
	else if( FStrEq( command, "selectclass 9" ) )
	{
		m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetEnabled( false );

		if( m_iNextClass == AXIS_OFFIZIER )
			return;

		m_pClassButtons[m_iPlayerLoadoutPanel]->SetEnabled( true );

		m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_FUNKER]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );
		m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );

		//hide all panels
		for( int i = 0; i < CLASSLOADOUT_AXIS_MAX; i++)
		{
			m_pClassPanel[i]->SetVisible( false );
		}
		m_pDefaultPanel->SetVisible( false );

		//this little modification is needed as there are only 4 panels, yet "5" classes (CLASS_NONE) - nuke
		m_iPlayerLoadoutPanel = CLASSLOADOUT_OFFIZIER;
		m_bIsClassPanelVisible = true;
		m_pInsetBorder->SetVisible( true );
		m_pClassPanel[CLASSLOADOUT_OFFIZIER]->SetVisible(true);

		//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_RIFLEMANLOADOUT );
		m_pPrimaryWeaponComboBox[CLASSLOADOUT_OFFIZIER]->ActivateItemByRow(0);
		ChangeOffizierDescription( true, WEAPON_MP40 );
		m_iNextClass = AXIS_OFFIZIER;
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
void CRnLAxisClassLoadoutMenu::ShowPanel(bool bShow)
{
	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( true );

		if( nSelectionSongGuid == 0 )
		{
			enginesound->EmitAmbientSound( "music/Axis_Select.wav", 1.0f, PITCH_NORM, 0, 0.0f );
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


void CRnLAxisClassLoadoutMenu::SetData(KeyValues *data)
{
	if ( FStrEq( data->GetName(), "loadout" ) )
	{
		//if loadout was a success, hide panel
		if ( data->GetInt("status") & FL_PRIMARYWEAPON_SUCCESS )
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
			//this error string generation should be moved to client side
			m_pErrorLabel[m_iPlayerLoadoutPanel]->SetText( data->GetString("loadout_error"));
			m_pErrorLabel[m_iPlayerLoadoutPanel]->SetPos(50, 400);
			//reenable all buttons
			m_pSubmitCommand[m_iPlayerLoadoutPanel]->SetEnabled( true );
		}
	}
	else if ( FStrEq( data->GetName(), "playerclass" ) ) 
	{
		for( int i = 0; i < CLASSLOADOUT_AXIS_MAX; i++ )
		{
			m_pClassButtons[i]->SetEnabled( false );
		}

		int avail = data->GetInt( "available" );
		if( avail & FL_RIFLEMAN_AVAILABLE )
			m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetEnabled( true );

		if( avail & FL_MACHINEGUNNER_AVAILABLE )
			m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetEnabled( true );

		if( avail & FL_RADIOMAN_AVAILABLE )
			m_pClassButtons[CLASSLOADOUT_FUNKER]->SetEnabled( true );

		if( avail & FL_MEDIC_AVAILABLE )
			m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetEnabled( true );

		if( avail & FL_NCO_AVAILABLE )
			m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetEnabled( true );

		if( data->GetInt( "cancancel", 0 ) )
		{
			for( int i = 0; i < CLASSLOADOUT_AXIS_MAX; i++ )
			{
				m_pCancelCommand[i]->SetVisible( true );
				m_pCancelCommand[i]->SetEnabled( true );
			}
		}
		else
		{
			for( int i = 0; i < CLASSLOADOUT_AXIS_MAX; i++ )
			{
				m_pCancelCommand[i]->SetVisible( false );
				m_pCancelCommand[i]->SetEnabled( false );
			}
		}


		//hide all panels
		for( int i = 0; i < CLASSLOADOUT_AXIS_MAX; i++)
		{
			m_pClassPanel[i]->SetVisible( false );
		}
		m_pDefaultPanel->SetVisible( false );

		//this little modification is needed as there are only 4 panels, yet "5" classes (CLASS_NONE) - nuke
		// here it's -5 since the value thats provided here is like 5 or 8 for the grenadier and higher
		// while the classloadout is expecting something between 0 and 3 - Stefan
		m_iPlayerLoadoutPanel = data->GetInt( "classnumber", RNL_CLASS_NONE ) - AXIS_GRENADIER;

		m_bIsClassPanelVisible = true;

		m_pInsetBorder->SetVisible( true );

		ResetAttachments();

		m_pClassButtons[m_iPlayerLoadoutPanel]->SetEnabled( false );
			
		//show only our current panel
		switch( m_iPlayerLoadoutPanel )
		{
			case CLASSLOADOUT_GRENADIER:
			{
				m_pClassPanel[CLASSLOADOUT_GRENADIER]->SetVisible(true);

				m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_FUNKER]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

				//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_RIFLEMANLOADOUT );
				m_pPrimaryWeaponComboBox[CLASSLOADOUT_GRENADIER]->ActivateItemByRow(0);
				ChangeGrenadierDescription( true, WEAPON_K98K );
				m_iNextClass = AXIS_GRENADIER;
				break;
			}
			case CLASSLOADOUT_MGSCHUTZE:
			{
				m_pClassPanel[CLASSLOADOUT_MGSCHUTZE]->SetVisible(true);

				m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_FUNKER]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

				//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_RIFLEMANLOADOUT );
				m_pPrimaryWeaponComboBox[CLASSLOADOUT_MGSCHUTZE]->ActivateItemByRow(0);
				ChangeMGSchutzeDescription( true, WEAPON_MG42 );
				m_iNextClass = AXIS_MGSCHUTZE;
				break;
			}
			case CLASSLOADOUT_FUNKER:
			{	
				m_pClassPanel[CLASSLOADOUT_FUNKER]->SetVisible(true);

				m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_FUNKER]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

				//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_RIFLEMANLOADOUT );
				m_pPrimaryWeaponComboBox[CLASSLOADOUT_FUNKER]->ActivateItemByRow(0);
				ChangeFunkerDescription( true, WEAPON_G43 );
				m_iNextClass = AXIS_FUNKER;
				break;
			}
			case CLASSLOADOUT_SANITAETER:
			{
				m_pClassPanel[CLASSLOADOUT_SANITAETER]->SetVisible(true);

				m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_FUNKER]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );

				//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_RIFLEMANLOADOUT );
				m_pPrimaryWeaponComboBox[CLASSLOADOUT_SANITAETER]->ActivateItemByRow(0);
				ChangeSanitaeterDescription( false, WEAPON_NONE );

				m_iNextClass = AXIS_SANITAETER;
				break;
			}
			case CLASSLOADOUT_OFFIZIER:
			{
				m_pClassPanel[CLASSLOADOUT_OFFIZIER]->SetVisible(true);

				m_pClassButtons[CLASSLOADOUT_GRENADIER]->SetPos( XRES( CLASSBUTTON_XPOS[1] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_MGSCHUTZE]->SetPos( XRES( CLASSBUTTON_XPOS[2] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_FUNKER]->SetPos( XRES( CLASSBUTTON_XPOS[3] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_SANITAETER]->SetPos( XRES( CLASSBUTTON_XPOS[4] ), CLASSBUTTON_YPOS );
				m_pClassButtons[CLASSLOADOUT_OFFIZIER]->SetPos( XRES( CLASSBUTTON_XPOS[0] ), CLASSBUTTON_YPOS );
				
				//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_NCOLOADOUT );
				m_pPrimaryWeaponComboBox[CLASSLOADOUT_OFFIZIER]->ActivateItemByRow(0);
				ChangeOffizierDescription( true, WEAPON_MP40 );

				m_iNextClass = AXIS_OFFIZIER;
				break;
			}
		}
	}
}

void CRnLAxisClassLoadoutMenu::OnKeyCodePressed(KeyCode code)
{
	BaseClass::OnKeyCodePressed( code );
}

void CRnLAxisClassLoadoutMenu::InitializeGrenadierPanel()
{
	// Their current weight is only the primary weapon. I count the default stuff as zero weight, since it was always there - Stefan
	m_iCurrentWeight[CLASSLOADOUT_GRENADIER] = 4;
	m_iPrimaryWeaponWeight[CLASSLOADOUT_GRENADIER] = 12;

	m_pClassPanel[CLASSLOADOUT_GRENADIER] = new Panel(this, "PanelGrenadier");
	m_pClassPanel[CLASSLOADOUT_GRENADIER]->SetPos(100, 150);
	m_pClassPanel[CLASSLOADOUT_GRENADIER]->SetSize(500, 500);
	//m_pClassPanel[CLASSLOADOUT_GRENADIER]->SetVisible( false );
	m_pClassPanel[CLASSLOADOUT_GRENADIER]->SetPaintBorderEnabled( true );
	
	m_pPrimaryWeaponLabel[CLASSLOADOUT_GRENADIER] = new Label(m_pClassPanel[CLASSLOADOUT_GRENADIER], "primweaponlbl", "Primary weapon:");

	m_pWeightLabel[CLASSLOADOUT_GRENADIER] = new Label(m_pClassPanel[CLASSLOADOUT_GRENADIER], "weightlbl", "Weight:");

	m_pEquipmentLabel[CLASSLOADOUT_GRENADIER] = new Label(m_pClassPanel[CLASSLOADOUT_GRENADIER], "equipmentlbl", "Equipment:");

	m_pErrorLabel[CLASSLOADOUT_GRENADIER] = new Label(m_pClassPanel[CLASSLOADOUT_GRENADIER], "error", "");
	m_pErrorLabel[CLASSLOADOUT_GRENADIER]->SetContentAlignment( vgui::Label::a_east );

	m_pPrimaryWeaponComboBox[CLASSLOADOUT_GRENADIER] = new ComboBox(m_pClassPanel[CLASSLOADOUT_GRENADIER], "primarycombo", 3, false);
	
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_GRENADIER]->AddActionSignalTarget( this );

	KeyValues *primaryweapon = new KeyValues("primaryweap");
	primaryweapon->SetInt("primaryweapon", WEAPON_K98K);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_K98K]);
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_GRENADIER]->AddItem("K98K", primaryweapon);
	//default option is first weapon in list, we set playerloadout here so the TextChange
	// function adds the correct weight for it
	m_iPlayerLoadoutPanel = CLASSLOADOUT_GRENADIER;
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_GRENADIER]->ActivateItemByRow(0);

	/*
	primaryweapon->SetInt("primaryweapon", WEAPON_G43);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_G43]);
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_GRENADIER]->AddItem("G43", primaryweapon);
	*/

	m_pAvailableEquipment[CLASSLOADOUT_GRENADIER] = new ListPanel(m_pClassPanel[CLASSLOADOUT_GRENADIER], "availequip", false);
	m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->AddColumnHeader(0, "name", "Equipment name", XRES(100), COLUMN_FIXEDSIZE);
	//m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->AddColumnHeader(1, "weight", "Slots", XRES(30), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->AddColumnHeader(1, "carry", "Carrying", XRES(50), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->AddColumnHeader(2, "max", "Max Carry", XRES(57), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->SetColumnSortable(0, false);
	m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->SetColumnSortable(1, false);
	m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->SetColumnSortable(2, false);
	//m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->SetColumnSortable(3, false);
	m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->AddActionSignalTarget( this );

	KeyValues *equipment = new KeyValues("equipment");
	
	equipment->SetString("name", "Walther P38");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_WALTHER]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_WALTHER]);
	m_iWalther[CLASSLOADOUT_GRENADIER] = m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->AddItem(equipment, 0, false, false);
	//m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->SetItemVisible( m_iWalther[CLASSLOADOUT_GRENADIER], false);

	equipment->SetString("name", "Frag grenade");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_STIELHANDGRANATE]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 2);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_STIELHANDGRANATE]);
	m_iFragGrenades[CLASSLOADOUT_GRENADIER] = m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->AddItem(equipment, 1, false, false);

	equipment->SetString("name", "Rifle grenade");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_RIFLEGRENADES]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_RIFLEGRENADES]);
	m_iRifleGrenades[CLASSLOADOUT_GRENADIER]= m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->AddItem(equipment, 2, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->SetItemVisible(m_iRifleGrenades[CLASSLOADOUT_GRENADIER], false);

	equipment->SetString("name", "Trench Knife");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_MELEE]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_MELEE]);
	m_iTrenchknife[CLASSLOADOUT_GRENADIER] = m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->AddItem(equipment, 3, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->SetItemVisible(m_iTrenchknife[CLASSLOADOUT_GRENADIER], false);

	equipment->SetString("name", "Extra ammo");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_EXTRAAMMO]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_EXTRAAMMO]);
	m_iExtraAmmo[CLASSLOADOUT_GRENADIER] = m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->AddItem(equipment, 4, false, false);

	equipment->SetString("name", "Helmet");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_HELMET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_HELMET]);
	m_iHelmet[CLASSLOADOUT_GRENADIER] = m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->AddItem(equipment, 5, false, false);
	
	// The bayonet is shown as carried equipment prop when the k98 is selected, but you can neither add nor remove it
	// nor is it being send to the loadout but added by default - Stefan
	equipment->SetString("name", "Bayonet ( K98K )");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_K98KBAYONET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_K98KBAYONET]);
	m_iK98KBayonet[CLASSLOADOUT_GRENADIER] = m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->AddItem(equipment, 6, false, false);

	equipment->SetString("name", "Smoke grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_AXIS_SMOKEGRENADE]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_AXIS_SMOKEGRENADE]);
	m_iSmokeGrenades[CLASSLOADOUT_GRENADIER] = m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->AddItem(equipment, 7, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->SetItemVisible(m_iSmokeGrenades[CLASSLOADOUT_GRENADIER], false);

	m_pAvailableEquipment[CLASSLOADOUT_GRENADIER]->SetItemVisible( m_iHelmet[CLASSLOADOUT_GRENADIER], false);

	m_pAddEquipment[CLASSLOADOUT_GRENADIER] = new Button(m_pClassPanel[CLASSLOADOUT_GRENADIER], "add", "+", this, "add");
	m_pAddEquipment[CLASSLOADOUT_GRENADIER]->SetContentAlignment( vgui::Label::a_center );

	m_pRemoveEquipment[CLASSLOADOUT_GRENADIER] = new Button(m_pClassPanel[CLASSLOADOUT_GRENADIER], "remove", "-", this, "remove");
	m_pRemoveEquipment[CLASSLOADOUT_GRENADIER]->SetContentAlignment( vgui::Label::a_center );

	m_pSubmitCommand[CLASSLOADOUT_GRENADIER] = new Button(m_pClassPanel[CLASSLOADOUT_GRENADIER], "cmdsubmit", "Submit", this, "submit");
	m_pSubmitCommand[CLASSLOADOUT_GRENADIER]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_GRENADIER][0] = new Button(m_pClassPanel[CLASSLOADOUT_GRENADIER], "rotateright", ">", this, "rotateright");
	m_pRotateCommand[CLASSLOADOUT_GRENADIER][0]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_GRENADIER][1] = new Button(m_pClassPanel[CLASSLOADOUT_GRENADIER], "rotateleft", "<", this, "rotateleft");
	m_pRotateCommand[CLASSLOADOUT_GRENADIER][1]->SetContentAlignment( vgui::Label::a_center );

	m_pCancelCommand[CLASSLOADOUT_GRENADIER]= new Button(m_pClassPanel[CLASSLOADOUT_GRENADIER], "cmdcancel", "Cancel", this, "vguicancel");
	m_pCancelCommand[CLASSLOADOUT_GRENADIER]->SetContentAlignment( vgui::Label::a_center );
	m_pCancelCommand[CLASSLOADOUT_GRENADIER]->SetVisible( false );
	
	char szWeight[64];
	sprintf(szWeight, "%i / %i", m_iCurrentWeight[CLASSLOADOUT_GRENADIER], iMaxAxisClassSlots[CLASSLOADOUT_GRENADIER]);
	m_pWeight[CLASSLOADOUT_GRENADIER] = new Label(m_pClassPanel[CLASSLOADOUT_GRENADIER], "weight", szWeight);
	m_pWeight[CLASSLOADOUT_GRENADIER]->MoveToFront();
	m_pWeight[CLASSLOADOUT_GRENADIER]->SetContentAlignment( vgui::Label::a_center );

	m_pWeightProgress[CLASSLOADOUT_GRENADIER] = new ProgressBar( m_pClassPanel[CLASSLOADOUT_GRENADIER], "weightprogress");
	//aswell as the label, need to set the progress bar
	m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAxisClassSlots[m_iPlayerLoadoutPanel] );
	
	// Item Image Panel
	m_pItemImage[CLASSLOADOUT_GRENADIER] = new ImagePanel( m_pClassPanel[CLASSLOADOUT_GRENADIER], "itemimage");
	m_pItemImage[CLASSLOADOUT_GRENADIER]->SetImage( "images/weapons/k98k" );
	m_pItemImage[CLASSLOADOUT_GRENADIER]->SetShouldScaleImage( true );
	
	// Description panel
	m_pItemDescription[CLASSLOADOUT_GRENADIER] = new RichText( m_pClassPanel[CLASSLOADOUT_GRENADIER], "itemdesc");
	m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_default.txt") );
}

void CRnLAxisClassLoadoutMenu::InitializeMGSchutzePanel()
{
	// Their current weight is only the primary weapon. I count the default stuff as zero weight, since it was always there - Stefan
	m_iCurrentWeight[CLASSLOADOUT_MGSCHUTZE] = 4;
	m_iPrimaryWeaponWeight[CLASSLOADOUT_MGSCHUTZE] = 12;

	m_pClassPanel[CLASSLOADOUT_MGSCHUTZE] = new Panel(this, "MGSchutze");
	m_pClassPanel[CLASSLOADOUT_MGSCHUTZE]->SetPos(100, 150);
	m_pClassPanel[CLASSLOADOUT_MGSCHUTZE]->SetSize(500, 500);
	//m_pClassPanel[CLASSLOADOUT_MGSCHUTZE]->SetVisible( false );
	m_pClassPanel[CLASSLOADOUT_MGSCHUTZE]->SetPaintBorderEnabled( true );
	
	m_pPrimaryWeaponLabel[CLASSLOADOUT_MGSCHUTZE] = new Label(m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "primweaponlbl", "Primary weapon:");

	m_pWeightLabel[CLASSLOADOUT_MGSCHUTZE] = new Label(m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "weightlbl", "Weight:");

	m_pEquipmentLabel[CLASSLOADOUT_MGSCHUTZE] = new Label(m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "equipmentlbl", "Equipment:");

	m_pErrorLabel[CLASSLOADOUT_MGSCHUTZE] = new Label(m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "error", "");
	m_pErrorLabel[CLASSLOADOUT_MGSCHUTZE]->SetContentAlignment( vgui::Label::a_east );

	m_pPrimaryWeaponComboBox[CLASSLOADOUT_MGSCHUTZE] = new ComboBox(m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "primarycombo", 3, false);
	
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_MGSCHUTZE]->AddActionSignalTarget( this );

	KeyValues *primaryweapon = new KeyValues("primaryweap");
	primaryweapon->SetInt("primaryweapon", WEAPON_MG42);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_MG42]);
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_MGSCHUTZE]->AddItem("MG42", primaryweapon);
	//default option is first weapon in list, we set playerloadout here so the TextChange
	// function adds the correct weight for it
	m_iPlayerLoadoutPanel = CLASSLOADOUT_MGSCHUTZE;
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_MGSCHUTZE]->ActivateItemByRow(0);

	/*
	primaryweapon->SetInt("primaryweapon", WEAPON_G43);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_G43]);
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_MGSCHUTZE]->AddItem("G43", primaryweapon);
	*/

	m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE] = new ListPanel(m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "availequip", false);
	m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->AddColumnHeader(0, "name", "Equipment name", XRES(100), COLUMN_FIXEDSIZE);
	//m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->AddColumnHeader(1, "weight", "Slots", XRES(30), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->AddColumnHeader(1, "carry", "Carrying", XRES(50), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->AddColumnHeader(2, "max", "Max Carry", XRES(57), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->SetColumnSortable(0, false);
	m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->SetColumnSortable(1, false);
	m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->SetColumnSortable(2, false);
	//m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->SetColumnSortable(3, false);
	m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->AddActionSignalTarget( this );

	KeyValues *equipment = new KeyValues("equipment");
	
	equipment->SetString("name", "Walther P38");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_WALTHER]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_WALTHER]);
	m_iWalther[CLASSLOADOUT_MGSCHUTZE] = m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->AddItem(equipment, 0, false, false);
	//m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->SetItemVisible( m_iWalther[CLASSLOADOUT_MGSCHUTZE], false);

	equipment->SetString("name", "Frag grenade");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_STIELHANDGRANATE]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 2);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_STIELHANDGRANATE]);
	m_iFragGrenades[CLASSLOADOUT_MGSCHUTZE] = m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->AddItem(equipment, 1, false, false);

	equipment->SetString("name", "Rifle grenade");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_RIFLEGRENADES]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_RIFLEGRENADES]);
	m_iRifleGrenades[CLASSLOADOUT_MGSCHUTZE]= m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->AddItem(equipment, 2, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->SetItemVisible(m_iRifleGrenades[CLASSLOADOUT_MGSCHUTZE], false);

	equipment->SetString("name", "Trench Knife");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_MELEE]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_MELEE]);
	m_iTrenchknife[CLASSLOADOUT_MGSCHUTZE] = m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->AddItem(equipment, 3, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->SetItemVisible(m_iTrenchknife[CLASSLOADOUT_MGSCHUTZE], false);

	equipment->SetString("name", "Extra ammo");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_EXTRAAMMO]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_EXTRAAMMO]);
	m_iExtraAmmo[CLASSLOADOUT_MGSCHUTZE] = m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->AddItem(equipment, 4, false, false);

	equipment->SetString("name", "Helmet");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_HELMET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_HELMET]);
	m_iHelmet[CLASSLOADOUT_MGSCHUTZE] = m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->AddItem(equipment, 5, false, false);
	
	// The bayonet is shown as carried equipment prop when the k98 is selected, but you can neither add nor remove it
	// nor is it being send to the loadout but added by default - Stefan
	equipment->SetString("name", "Bayonet ( K98K )");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_K98KBAYONET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_K98KBAYONET]);
	m_iK98KBayonet[CLASSLOADOUT_MGSCHUTZE] = m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->AddItem(equipment, 6, false, false);

	equipment->SetString("name", "Smoke grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_AXIS_SMOKEGRENADE]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_AXIS_SMOKEGRENADE]);
	m_iSmokeGrenades[CLASSLOADOUT_MGSCHUTZE] = m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->AddItem(equipment, 7, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->SetItemVisible(m_iSmokeGrenades[CLASSLOADOUT_MGSCHUTZE], false);

	m_pAvailableEquipment[CLASSLOADOUT_MGSCHUTZE]->SetItemVisible( m_iHelmet[CLASSLOADOUT_MGSCHUTZE], false);

	m_pAddEquipment[CLASSLOADOUT_MGSCHUTZE] = new Button(m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "add", "+", this, "add");
	m_pAddEquipment[CLASSLOADOUT_MGSCHUTZE]->SetContentAlignment( vgui::Label::a_center );

	m_pRemoveEquipment[CLASSLOADOUT_MGSCHUTZE] = new Button(m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "remove", "-", this, "remove");
	m_pRemoveEquipment[CLASSLOADOUT_MGSCHUTZE]->SetContentAlignment( vgui::Label::a_center );

	m_pSubmitCommand[CLASSLOADOUT_MGSCHUTZE] = new Button(m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "cmdsubmit", "Submit", this, "submit");
	m_pSubmitCommand[CLASSLOADOUT_MGSCHUTZE]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_MGSCHUTZE][0] = new Button(m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "rotateright", ">", this, "rotateright");
	m_pRotateCommand[CLASSLOADOUT_MGSCHUTZE][0]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_MGSCHUTZE][1] = new Button(m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "rotateleft", "<", this, "rotateleft");
	m_pRotateCommand[CLASSLOADOUT_MGSCHUTZE][1]->SetContentAlignment( vgui::Label::a_center );

	m_pCancelCommand[CLASSLOADOUT_MGSCHUTZE]= new Button(m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "cmdcancel", "Cancel", this, "vguicancel");
	m_pCancelCommand[CLASSLOADOUT_MGSCHUTZE]->SetContentAlignment( vgui::Label::a_center );
	m_pCancelCommand[CLASSLOADOUT_MGSCHUTZE]->SetVisible( false );
	
	char szWeight[64];
	sprintf(szWeight, "%i / %i", m_iCurrentWeight[CLASSLOADOUT_MGSCHUTZE], iMaxAxisClassSlots[CLASSLOADOUT_MGSCHUTZE]);
	m_pWeight[CLASSLOADOUT_MGSCHUTZE] = new Label(m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "weight", szWeight);
	m_pWeight[CLASSLOADOUT_MGSCHUTZE]->MoveToFront();
	m_pWeight[CLASSLOADOUT_MGSCHUTZE]->SetContentAlignment( vgui::Label::a_center );

	m_pWeightProgress[CLASSLOADOUT_MGSCHUTZE] = new ProgressBar( m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "weightprogress");
	//aswell as the label, need to set the progress bar
	m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAxisClassSlots[m_iPlayerLoadoutPanel] );
	
	// Item Image Panel
	m_pItemImage[CLASSLOADOUT_MGSCHUTZE] = new ImagePanel( m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "itemimage");
	m_pItemImage[CLASSLOADOUT_MGSCHUTZE]->SetImage( "images/weapons/mg42" );
	m_pItemImage[CLASSLOADOUT_MGSCHUTZE]->SetShouldScaleImage( true );
	
	// Description panel
	m_pItemDescription[CLASSLOADOUT_MGSCHUTZE] = new RichText( m_pClassPanel[CLASSLOADOUT_MGSCHUTZE], "itemdesc");
	m_pItemDescription[CLASSLOADOUT_MGSCHUTZE]->SetText( LoadItemDescription("resource/ui/descriptions/description_default.txt") );
}

void CRnLAxisClassLoadoutMenu::InitializeFunkerPanel()
{
	// add the current weight of the default weapon - Stefan
	m_iCurrentWeight[CLASSLOADOUT_FUNKER] = 3;
	m_iPrimaryWeaponWeight[CLASSLOADOUT_FUNKER] = 10;

	m_pClassPanel[CLASSLOADOUT_FUNKER] = new Panel(this, "PanelFunker");
	m_pClassPanel[CLASSLOADOUT_FUNKER]->SetVisible( false );
	m_pClassPanel[CLASSLOADOUT_FUNKER]->SetPaintBorderEnabled( true );
	
	m_pPrimaryWeaponLabel[CLASSLOADOUT_FUNKER] = new Label(m_pClassPanel[CLASSLOADOUT_FUNKER], "primweaponlbl", "Primary weapon:");

	m_pWeightLabel[CLASSLOADOUT_FUNKER] = new Label(m_pClassPanel[CLASSLOADOUT_FUNKER], "weightlbl", "Weight:");

	m_pEquipmentLabel[CLASSLOADOUT_FUNKER] = new Label(m_pClassPanel[CLASSLOADOUT_FUNKER], "equipmentlbl", "Equipment:");

	m_pErrorLabel[CLASSLOADOUT_FUNKER] = new Label(m_pClassPanel[CLASSLOADOUT_FUNKER], "error", "");
	m_pErrorLabel[CLASSLOADOUT_FUNKER]->SetContentAlignment( vgui::Label::a_east );

	m_pPrimaryWeaponComboBox[CLASSLOADOUT_FUNKER] = new ComboBox(m_pClassPanel[CLASSLOADOUT_FUNKER], "primarycombo", 3, false);

	m_pPrimaryWeaponComboBox[CLASSLOADOUT_FUNKER]->AddActionSignalTarget( this );

	KeyValues *primaryweapon = new KeyValues("primaryweap");
	primaryweapon->SetInt("primaryweapon", WEAPON_G43);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_G43]);
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_FUNKER]->AddItem("G43", primaryweapon);
	//default option is first weapon in list, we set playerloadout here so the TextChange
	// function adds the correct weight for it
	m_iPlayerLoadoutPanel = CLASSLOADOUT_FUNKER;
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_FUNKER]->ActivateItemByRow(0);

	m_pAvailableEquipment[CLASSLOADOUT_FUNKER] = new ListPanel(m_pClassPanel[CLASSLOADOUT_FUNKER], "availequip", false);
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->AddColumnHeader(0, "name", "Equipment name", XRES(100), COLUMN_FIXEDSIZE);
	//m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->AddColumnHeader(1, "weight", "Slots", XRES(30), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->AddColumnHeader(1, "carry", "Carrying", XRES(50), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->AddColumnHeader(2, "max", "Max Carry", XRES(57), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->SetColumnSortable(0, false);
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->SetColumnSortable(1, false);
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->SetColumnSortable(2, false);
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->AddActionSignalTarget( this );
	
	KeyValues *equipment = new KeyValues("equipment");
	
	equipment->SetString("name", "Walther P38");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_WALTHER]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_WALTHER]);
	m_iWalther[CLASSLOADOUT_FUNKER] = m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->AddItem(equipment, 0, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->SetItemVisible( m_iWalther[CLASSLOADOUT_FUNKER], false);

	equipment->SetString("name", "Frag grenade");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_STIELHANDGRANATE]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_STIELHANDGRANATE]);
	m_iFragGrenades[CLASSLOADOUT_FUNKER] = m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->AddItem(equipment, 1, false, false);

	equipment->SetString("name", "Rifle grenade");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_RIFLEGRENADES]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_RIFLEGRENADES]);
	m_iRifleGrenades[CLASSLOADOUT_FUNKER] = m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->AddItem(equipment, 2, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->SetItemVisible(m_iRifleGrenades[CLASSLOADOUT_FUNKER], false);

	equipment->SetString("name", "Trench Knife");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_MELEE]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_MELEE]);
	m_iTrenchknife[CLASSLOADOUT_FUNKER] = m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->AddItem(equipment, 3, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->SetItemVisible(m_iTrenchknife[CLASSLOADOUT_FUNKER], false);

	equipment->SetString("name", "Extra ammo");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_EXTRAAMMO]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_EXTRAAMMO]);
	m_iExtraAmmo[CLASSLOADOUT_FUNKER] = m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->AddItem(equipment, 4, false, false);
	
	equipment->SetString("name", "Helmet");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_HELMET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_HELMET]);
	m_iHelmet[CLASSLOADOUT_FUNKER] = m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->AddItem(equipment, 5, false, false);

	equipment->SetString("name", "Bayonet (K98K)");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_K98KBAYONET]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_K98KBAYONET]);
	m_iK98KBayonet[CLASSLOADOUT_FUNKER] = m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->AddItem(equipment, 6, false, false);
	
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->SetItemVisible( m_iK98KBayonet[CLASSLOADOUT_FUNKER], false);
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->SetItemVisible( m_iHelmet[CLASSLOADOUT_FUNKER], false);
	
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->SetItemVisible( m_iK98KBayonet[CLASSLOADOUT_FUNKER], false);
	m_pAvailableEquipment[CLASSLOADOUT_FUNKER]->SetItemVisible( m_iHelmet[CLASSLOADOUT_FUNKER],	false);
	
	m_pAddEquipment[CLASSLOADOUT_FUNKER] = new Button(m_pClassPanel[CLASSLOADOUT_FUNKER], "add", "+", this, "add");
	m_pAddEquipment[CLASSLOADOUT_FUNKER]->SetContentAlignment( vgui::Label::a_center );

	m_pRemoveEquipment[CLASSLOADOUT_FUNKER] = new Button(m_pClassPanel[CLASSLOADOUT_FUNKER], "remove", "-", this, "remove");
	m_pRemoveEquipment[CLASSLOADOUT_FUNKER]->SetContentAlignment( vgui::Label::a_center );

	m_pSubmitCommand[CLASSLOADOUT_FUNKER] = new Button(m_pClassPanel[CLASSLOADOUT_FUNKER], "cmdsubmit", "Submit", this, "submit");
	m_pSubmitCommand[CLASSLOADOUT_FUNKER]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_FUNKER][0] = new Button(m_pClassPanel[CLASSLOADOUT_FUNKER], "rotateright", ">", this, "rotateright");
	m_pRotateCommand[CLASSLOADOUT_FUNKER][0]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_FUNKER][1] = new Button(m_pClassPanel[CLASSLOADOUT_FUNKER], "rotateleft", "<", this, "rotateleft");
	m_pRotateCommand[CLASSLOADOUT_FUNKER][1]->SetContentAlignment( vgui::Label::a_center );


	m_pCancelCommand[CLASSLOADOUT_FUNKER]= new Button(m_pClassPanel[CLASSLOADOUT_FUNKER], "cmdcancel", "Cancel", this, "vguicancel");
	m_pCancelCommand[CLASSLOADOUT_FUNKER]->SetContentAlignment( vgui::Label::a_center );
	//m_pCancelCommand[CLASSLOADOUT_FUNKER]->SetVisible( false );
	
	char szWeight[64];
	sprintf(szWeight, "%i / %i", m_iCurrentWeight[CLASSLOADOUT_FUNKER], iMaxAxisClassSlots[CLASSLOADOUT_FUNKER]);
	m_pWeight[CLASSLOADOUT_FUNKER] = new Label(m_pClassPanel[CLASSLOADOUT_FUNKER], "weight", szWeight);
	m_pWeight[CLASSLOADOUT_FUNKER]->MoveToFront();
	m_pWeight[CLASSLOADOUT_FUNKER]->SetContentAlignment( vgui::Label::a_center );

	m_pWeightProgress[CLASSLOADOUT_FUNKER] = new ProgressBar( m_pClassPanel[CLASSLOADOUT_FUNKER], "weightprogress");
	//aswell as the label, need to set the progress bar
	m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAxisClassSlots[m_iPlayerLoadoutPanel] );

	m_pItemImage[CLASSLOADOUT_FUNKER] = new ImagePanel( m_pClassPanel[CLASSLOADOUT_FUNKER], "itemimage");
	m_pItemImage[CLASSLOADOUT_FUNKER]->SetImage( "images/weapons/m1carbine" );
	m_pItemImage[CLASSLOADOUT_FUNKER]->SetShouldScaleImage( true );

	m_pItemDescription[CLASSLOADOUT_FUNKER] = new RichText( m_pClassPanel[CLASSLOADOUT_FUNKER], "itemdesc");
	m_pItemDescription[CLASSLOADOUT_FUNKER]->SetText( LoadItemDescription("resource/ui/descriptions/description_default.txt") );
}

void CRnLAxisClassLoadoutMenu::InitializeSanitaeterPanel()
{
	m_iCurrentWeight[CLASSLOADOUT_SANITAETER] = 0;
	m_iPrimaryWeaponWeight[CLASSLOADOUT_SANITAETER] = 0;

	m_pClassPanel[CLASSLOADOUT_SANITAETER] = new Panel(this, "PanelSanitaeter");
	m_pClassPanel[CLASSLOADOUT_SANITAETER]->SetVisible( false );
	m_pClassPanel[CLASSLOADOUT_SANITAETER]->SetPaintBorderEnabled( true );
	
	m_pPrimaryWeaponLabel[CLASSLOADOUT_SANITAETER] = new Label(m_pClassPanel[CLASSLOADOUT_SANITAETER], "primweaponlbl", "Primary weapon:");

	m_pWeightLabel[CLASSLOADOUT_SANITAETER] = new Label(m_pClassPanel[CLASSLOADOUT_SANITAETER], "weightlbl", "Weight:");

	m_pEquipmentLabel[CLASSLOADOUT_SANITAETER] = new Label(m_pClassPanel[CLASSLOADOUT_SANITAETER], "equipmentlbl", "Equipment:");
	
	m_pErrorLabel[CLASSLOADOUT_SANITAETER] = new Label(m_pClassPanel[CLASSLOADOUT_SANITAETER], "error", "");
	m_pErrorLabel[CLASSLOADOUT_SANITAETER]->SetContentAlignment( vgui::Label::a_east );

	m_pPrimaryWeaponComboBox[CLASSLOADOUT_SANITAETER] = new ComboBox(m_pClassPanel[CLASSLOADOUT_SANITAETER], "primarycombo", 3, false);
	
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_SANITAETER]->AddActionSignalTarget( this );

	KeyValues *primaryweapon = new KeyValues("primaryweap");
	primaryweapon->SetInt("primaryweapon", WEAPON_NONE);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_NONE]);
	//m_pPrimaryWeaponComboBox[CLASSLOADOUT_SANITAETER]->AddItem("unarmed", primaryweapon);
	//default option is first weapon in list, we set playerloadout here so the TextChange
	// function adds the correct weight for it
	m_iPlayerLoadoutPanel = CLASSLOADOUT_SANITAETER;
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_SANITAETER]->ActivateItemByRow(0);

	m_pAvailableEquipment[CLASSLOADOUT_SANITAETER] = new ListPanel(m_pClassPanel[CLASSLOADOUT_SANITAETER], "availequip", false);
	m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->AddColumnHeader(0, "name", "Equipment name", XRES(100), COLUMN_FIXEDSIZE);
	//m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->AddColumnHeader(1, "weight", "Slots", XRES(30), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->AddColumnHeader(1, "carry", "Carrying", XRES(50), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->AddColumnHeader(2, "max", "Max Carry", XRES(57), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->SetColumnSortable(0, false);
	m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->SetColumnSortable(1, false);
	m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->SetColumnSortable(2, false);
	// m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->SetColumnSortable(3, false);
	m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->AddActionSignalTarget( this );

	// Equipment
	KeyValues *equipment = new KeyValues("equipment");
	equipment->SetString("name", "Trench Knife");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_MELEE]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_MELEE]);
	m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->AddItem(equipment, 0, false, false);

	equipment->SetString("name", "Helmet");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_HELMET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_HELMET]);
	m_iHelmet[CLASSLOADOUT_SANITAETER] = m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->AddItem(equipment, 6, false, false);
	
	m_pAvailableEquipment[CLASSLOADOUT_SANITAETER]->SetItemVisible( m_iHelmet[CLASSLOADOUT_SANITAETER], false);

	m_pAddEquipment[CLASSLOADOUT_SANITAETER] = new Button(m_pClassPanel[CLASSLOADOUT_SANITAETER], "add", "+", this, "add");
	m_pAddEquipment[CLASSLOADOUT_SANITAETER]->SetContentAlignment( vgui::Label::a_center );
	
	m_pRemoveEquipment[CLASSLOADOUT_SANITAETER] = new Button(m_pClassPanel[CLASSLOADOUT_SANITAETER], "remove", "-", this, "remove");
	m_pRemoveEquipment[CLASSLOADOUT_SANITAETER]->SetContentAlignment( vgui::Label::a_center );
	
	m_pSubmitCommand[CLASSLOADOUT_SANITAETER] = new Button(m_pClassPanel[CLASSLOADOUT_SANITAETER], "cmdsubmit", "Submit", this, "submit");
	m_pSubmitCommand[CLASSLOADOUT_SANITAETER]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_SANITAETER][0] = new Button(m_pClassPanel[CLASSLOADOUT_SANITAETER], "rotateright", ">", this, "rotateright");
	m_pRotateCommand[CLASSLOADOUT_SANITAETER][0]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_SANITAETER][1] = new Button(m_pClassPanel[CLASSLOADOUT_SANITAETER], "rotateleft", "<", this, "rotateleft");
	m_pRotateCommand[CLASSLOADOUT_SANITAETER][1]->SetContentAlignment( vgui::Label::a_center );

	m_pCancelCommand[CLASSLOADOUT_SANITAETER]= new Button(m_pClassPanel[CLASSLOADOUT_SANITAETER], "cmdcancel", "Cancel", this, "vguicancel");
	m_pCancelCommand[CLASSLOADOUT_SANITAETER]->SetContentAlignment( vgui::Label::a_center );
	//m_pCancelCommand[CLASSLOADOUT_SANITAETER]->SetVisible( false );
	
	char szWeight[64];
	sprintf(szWeight, "%i / %i", m_iCurrentWeight[CLASSLOADOUT_SANITAETER], iMaxAxisClassSlots[CLASSLOADOUT_SANITAETER]);
	m_pWeight[CLASSLOADOUT_SANITAETER] = new Label(m_pClassPanel[CLASSLOADOUT_SANITAETER], "weight", szWeight);
	m_pWeight[CLASSLOADOUT_SANITAETER]->MoveToFront();
	m_pWeight[CLASSLOADOUT_SANITAETER]->SetContentAlignment( vgui::Label::a_center );

	m_pWeightProgress[CLASSLOADOUT_SANITAETER] = new ProgressBar( m_pClassPanel[CLASSLOADOUT_SANITAETER], "weightprogress");
	//aswell as the label, need to set the progress bar
	m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAxisClassSlots[m_iPlayerLoadoutPanel] );
	
	m_pItemImage[CLASSLOADOUT_SANITAETER] = new ImagePanel( m_pClassPanel[CLASSLOADOUT_SANITAETER], "itemimage");
	m_pItemImage[CLASSLOADOUT_SANITAETER]->SetImage( "image_descriptions/default" );
	m_pItemImage[CLASSLOADOUT_SANITAETER]->SetShouldScaleImage( true );

	m_pItemDescription[CLASSLOADOUT_SANITAETER] = new RichText( m_pClassPanel[CLASSLOADOUT_SANITAETER], "itemdesc");
	m_pItemDescription[CLASSLOADOUT_SANITAETER]->SetText( LoadItemDescription("resource/ui/descriptions/description_default.txt") );
}

void CRnLAxisClassLoadoutMenu::InitializeOffizierPanel()
{	
	m_iCurrentWeight[CLASSLOADOUT_OFFIZIER] = 3;
	m_iPrimaryWeaponWeight[CLASSLOADOUT_OFFIZIER] = 12;

	m_pClassPanel[CLASSLOADOUT_OFFIZIER] = new Panel(this, "PanelOffizier");
	m_pClassPanel[CLASSLOADOUT_OFFIZIER]->SetVisible( false );
	m_pClassPanel[CLASSLOADOUT_OFFIZIER]->SetPaintBorderEnabled( true );
	
	m_pPrimaryWeaponLabel[CLASSLOADOUT_OFFIZIER] = new Label(m_pClassPanel[CLASSLOADOUT_OFFIZIER], "primweaponlbl", "Primary weapon:");
	
	m_pWeightLabel[CLASSLOADOUT_OFFIZIER] = new Label(m_pClassPanel[CLASSLOADOUT_OFFIZIER], "weightlbl", "Weight:");
	
	m_pEquipmentLabel[CLASSLOADOUT_OFFIZIER] = new Label(m_pClassPanel[CLASSLOADOUT_OFFIZIER], "equipmentlbl", "Equipment:");
	
	m_pErrorLabel[CLASSLOADOUT_OFFIZIER] = new Label(m_pClassPanel[CLASSLOADOUT_OFFIZIER], "error", "");
	m_pErrorLabel[CLASSLOADOUT_OFFIZIER]->SetContentAlignment( vgui::Label::a_east );

	m_pPrimaryWeaponComboBox[CLASSLOADOUT_OFFIZIER] = new ComboBox(m_pClassPanel[CLASSLOADOUT_OFFIZIER], "primarycombo", 3, false);
	
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_OFFIZIER]->AddActionSignalTarget( this );

	KeyValues *primaryweapon = new KeyValues("primaryweap");
	primaryweapon->SetInt("primaryweapon", WEAPON_MP40);
	primaryweapon->SetInt("weight", iPrimaryWeaponWeights[WEAPON_MP40]);
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_OFFIZIER]->AddItem("MP40", primaryweapon);
	//default option is first weapon in list, we set playerloadout here so the TextChange
	// function adds the correct weight for it
	m_iPlayerLoadoutPanel = CLASSLOADOUT_OFFIZIER;
	m_pPrimaryWeaponComboBox[CLASSLOADOUT_OFFIZIER]->ActivateItemByRow(0);

	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER] = new ListPanel(m_pClassPanel[CLASSLOADOUT_OFFIZIER], "availequip", false);
	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->AddColumnHeader(0, "name", "Equipment name", XRES(100), COLUMN_FIXEDSIZE);
	//m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->AddColumnHeader(1, "weight", "Slots", XRES(30), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->AddColumnHeader(1, "carry", "Carrying", XRES(50), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->AddColumnHeader(2, "max", "Max Carry", XRES(57), COLUMN_FIXEDSIZE);
	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->SetColumnSortable(0, false);
	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->SetColumnSortable(1, false);
	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->SetColumnSortable(2, false);
	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->AddActionSignalTarget( this );

	KeyValues *equipment = new KeyValues("equipment");
	
	equipment->SetString("name", "Walther P38");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_WALTHER]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_WALTHER]);
	m_iWalther[CLASSLOADOUT_OFFIZIER] = m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->AddItem(equipment, 0, false, false);
	

	equipment->SetString("name", "Frag grenade");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_STIELHANDGRANATE]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 2);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_STIELHANDGRANATE]);
	m_iFragGrenades[CLASSLOADOUT_OFFIZIER] = m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->AddItem(equipment, 1, false, false);

	equipment->SetString("name", "Rifle grenade");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_RIFLEGRENADES]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_RIFLEGRENADES]);
	m_iRifleGrenades[CLASSLOADOUT_OFFIZIER] = m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->AddItem(equipment, 2, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->SetItemVisible(m_iRifleGrenades[CLASSLOADOUT_OFFIZIER], false);

	equipment->SetString("name", "Trench Knife");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_MELEE]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_MELEE]);
	m_iTrenchknife[CLASSLOADOUT_OFFIZIER] = m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->AddItem(equipment, 3, false, false);
	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->SetItemVisible(m_iTrenchknife[CLASSLOADOUT_OFFIZIER], false);


	equipment->SetString("name", "Extra ammo");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_EXTRAAMMO]);
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_EXTRAAMMO]);
	m_iExtraAmmo[CLASSLOADOUT_OFFIZIER] = m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->AddItem(equipment, 4, false, false);

	equipment->SetString("name", "Helmet");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_HELMET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_HELMET]);
	m_iHelmet[CLASSLOADOUT_OFFIZIER] = m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->AddItem(equipment, 5, false, false);
	
	// The bayonet is shown as carried equipment prop when the garand is selected, but you can neither add nor remove it
	// nor is it being send to the loadout but added by default - Stefan
	equipment->SetString("name", "Bayonet (K98K)");
	equipment->SetInt("weight", iAxisEquipmentWeights[EQUIPMENT_AXIS_K98KBAYONET]);
	equipment->SetInt("carry", 1);
	equipment->SetInt("max", iMaxAxisEquipmentCarry[EQUIPMENT_AXIS_K98KBAYONET]);
	m_iK98KBayonet[CLASSLOADOUT_OFFIZIER] = m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->AddItem(equipment, 6, false, false);
	
	equipment->SetString("name", "Smoke grenade");
	equipment->SetInt("weight", iAlliesEquipmentSlots[EQUIPMENT_AXIS_SMOKEGRENADE]);
	//start with zero, it just makes sense 8) - nuke
	equipment->SetInt("carry", 0);
	equipment->SetInt("max", iMaxAlliesEquipmentCarry[EQUIPMENT_AXIS_SMOKEGRENADE]);
	m_iSmokeGrenades[CLASSLOADOUT_OFFIZIER] = m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->AddItem(equipment, 7, false, false);

	
	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->SetItemVisible( m_iK98KBayonet[CLASSLOADOUT_OFFIZIER], false);
	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->SetItemVisible( m_iHelmet[CLASSLOADOUT_OFFIZIER], false);

	m_pAddEquipment[CLASSLOADOUT_OFFIZIER] = new Button(m_pClassPanel[CLASSLOADOUT_OFFIZIER], "add", "+", this, "add");
	m_pAddEquipment[CLASSLOADOUT_OFFIZIER]->SetContentAlignment( vgui::Label::a_center );

	m_pRemoveEquipment[CLASSLOADOUT_OFFIZIER] = new Button(m_pClassPanel[CLASSLOADOUT_OFFIZIER], "remove", "-", this, "remove");
	m_pRemoveEquipment[CLASSLOADOUT_OFFIZIER]->SetContentAlignment( vgui::Label::a_center );

	m_pSubmitCommand[CLASSLOADOUT_OFFIZIER] = new Button(m_pClassPanel[CLASSLOADOUT_OFFIZIER], "cmdsubmit", "Submit", this, "submit");
	m_pSubmitCommand[CLASSLOADOUT_OFFIZIER]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_OFFIZIER][0] = new Button(m_pClassPanel[CLASSLOADOUT_OFFIZIER], "rotateright", ">", this, "rotateright");
	m_pRotateCommand[CLASSLOADOUT_OFFIZIER][0]->SetContentAlignment( vgui::Label::a_center );

	m_pRotateCommand[CLASSLOADOUT_OFFIZIER][1] = new Button(m_pClassPanel[CLASSLOADOUT_OFFIZIER], "rotateleft", "<", this, "rotateleft");
	m_pRotateCommand[CLASSLOADOUT_OFFIZIER][1]->SetContentAlignment( vgui::Label::a_center );

	m_pCancelCommand[CLASSLOADOUT_OFFIZIER]= new Button(m_pClassPanel[CLASSLOADOUT_OFFIZIER], "cmdcancel", "Cancel", this, "vguicancel");
	m_pCancelCommand[CLASSLOADOUT_OFFIZIER]->SetContentAlignment( vgui::Label::a_center );
	m_pCancelCommand[CLASSLOADOUT_OFFIZIER]->SetVisible( false );
	
	// Hide the riflegrenades and the k98bayonet by default - Stefan
	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->SetItemVisible(m_iK98KBayonet[CLASSLOADOUT_OFFIZIER], false);
	m_pAvailableEquipment[CLASSLOADOUT_OFFIZIER]->SetItemVisible(m_iRifleGrenades[CLASSLOADOUT_OFFIZIER], false);

	char szWeight[64];
	sprintf(szWeight, "%i / %i", m_iCurrentWeight[CLASSLOADOUT_OFFIZIER], iMaxAxisClassSlots[CLASSLOADOUT_OFFIZIER]);
	m_pWeight[CLASSLOADOUT_OFFIZIER] = new Label(m_pClassPanel[CLASSLOADOUT_OFFIZIER], "weight", szWeight);
	m_pWeight[CLASSLOADOUT_OFFIZIER]->MoveToFront();
	m_pWeight[CLASSLOADOUT_OFFIZIER]->SetContentAlignment( vgui::Label::a_center );

	m_pWeightProgress[CLASSLOADOUT_OFFIZIER] = new ProgressBar( m_pClassPanel[CLASSLOADOUT_OFFIZIER], "weightprogress");
	//aswell as the label, need to set the progress bar
	m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAxisClassSlots[m_iPlayerLoadoutPanel] );
	
	m_pItemImage[CLASSLOADOUT_OFFIZIER] = new ImagePanel( m_pClassPanel[CLASSLOADOUT_OFFIZIER], "itemimage");
	m_pItemImage[CLASSLOADOUT_OFFIZIER]->SetImage( "images/weapons/mp40" );
	m_pItemImage[CLASSLOADOUT_OFFIZIER]->SetShouldScaleImage( true );

	m_pItemDescription[CLASSLOADOUT_OFFIZIER] = new RichText( m_pClassPanel[CLASSLOADOUT_OFFIZIER], "itemdesc");
	m_pItemDescription[CLASSLOADOUT_OFFIZIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_default.txt") );
}

//this means the combobox has been changed - johny (a weapon has been selected - stefan)
void CRnLAxisClassLoadoutMenu::OnTextChanged( KeyValues *data )
{
	if(m_iPlayerLoadoutPanel < 0 || m_iPlayerLoadoutPanel >= CLASSLOADOUT_AXIS_MAX)
		return;

	KeyValues *primaryWeapon = m_pPrimaryWeaponComboBox[m_iPlayerLoadoutPanel]->GetActiveItemUserData();
	
	//save off the current weapon
	m_iCurrentPrimaryWeapon[m_iPlayerLoadoutPanel] = m_pPrimaryWeaponComboBox[m_iPlayerLoadoutPanel]->GetActiveItem();
		
	// Set some visibility default values
	m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iK98KBayonet[m_iPlayerLoadoutPanel], false);
	m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iWalther[m_iPlayerLoadoutPanel], true);
	m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iRifleGrenades[m_iPlayerLoadoutPanel], true);

	//if the new weapon is a garand, add the bayonet to the equipment list, else make sure its invisible
	if( primaryWeapon->GetInt("primaryweapon") == WEAPON_K98K )
	{
		// Show the bayonet
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iK98KBayonet[m_iPlayerLoadoutPanel], true);
	}
	else if( primaryWeapon->GetInt("primaryweapon") == WEAPON_G43 )
	{
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iRifleGrenades[m_iPlayerLoadoutPanel], false);
		
		// Remove the riflegrenades and hide them
		KeyValues *RifleGrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iRifleGrenades[m_iPlayerLoadoutPanel] );
		KeyValues *BayonetItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iK98KBayonet[m_iPlayerLoadoutPanel] );

		

		if( RifleGrenadeItem->GetInt("carry") > 0 )
		{
			RifleGrenadeItem->SetInt("carry", 0);

			if( RifleGrenadeItem->GetInt("carry") == 1 )
				m_iCurrentWeight[m_iPlayerLoadoutPanel] -= RifleGrenadeItem->GetInt("weight");
			else if( RifleGrenadeItem->GetInt("carry") == 2 )
				m_iCurrentWeight[m_iPlayerLoadoutPanel] -= 2 * RifleGrenadeItem->GetInt("weight");
		}
		
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible( m_iK98KBayonet[m_iPlayerLoadoutPanel], false);
		// if the player has a bayonet remove the bayonet from his list
		if( BayonetItem->GetInt("carry") > 0 )
		{
			BayonetItem->SetInt("carry", 0);
		}
	}
	else if( primaryWeapon->GetInt("primaryweapon") == WEAPON_MP40 )
	{
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iRifleGrenades[m_iPlayerLoadoutPanel], false);
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iWalther[m_iPlayerLoadoutPanel], true);

		// Remove the riflegrenades and hide them
		KeyValues *RifleGrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iRifleGrenades[m_iPlayerLoadoutPanel] );
		KeyValues *WaltherItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iWalther[m_iPlayerLoadoutPanel] );
		KeyValues *BayonetItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iK98KBayonet[m_iPlayerLoadoutPanel] );

		if( RifleGrenadeItem->GetInt("carry") > 0 )
		{
			RifleGrenadeItem->SetInt("carry", 0);
			if( RifleGrenadeItem->GetInt("carry") == 1 )
				m_iCurrentWeight[m_iPlayerLoadoutPanel] -= RifleGrenadeItem->GetInt("weight");
			else if( RifleGrenadeItem->GetInt("carry") == 2 )
				m_iCurrentWeight[m_iPlayerLoadoutPanel] -= 2 * RifleGrenadeItem->GetInt("weight");
		}
		// Remove the walther as well
		if( WaltherItem->GetInt("carry") > 0 )
		{
			WaltherItem->SetInt("carry", 0);
			m_iCurrentWeight[m_iPlayerLoadoutPanel] -= WaltherItem->GetInt("weight");
		}
		// if the player has a bayonet remove the bayonet from his list
		if( BayonetItem->GetInt("carry") > 0 )
		{
			BayonetItem->SetInt("carry", 0);
		}
	}
	else if( primaryWeapon->GetInt("primaryweapon") == WEAPON_MP40 )
	{
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iRifleGrenades[m_iPlayerLoadoutPanel], false);

		// Remove the riflegrenades and hide them
		KeyValues *RifleGrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iRifleGrenades[m_iPlayerLoadoutPanel] );
		KeyValues *BayonetItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iK98KBayonet[m_iPlayerLoadoutPanel] );

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
		KeyValues *BayonetItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iK98KBayonet[m_iPlayerLoadoutPanel] );
		
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible( 
		m_iK98KBayonet[m_iPlayerLoadoutPanel],
		false);
		// if the player has a bayonet remove the bayonet from his list
		if( BayonetItem->GetInt("carry") > 0 )
		{
			BayonetItem->SetInt("carry", 0);
		}
	}
	char szWeight[64];
	sprintf(szWeight, "%i / %i", m_iCurrentWeight[m_iPlayerLoadoutPanel], iMaxAxisClassSlots[m_iPlayerLoadoutPanel]);
	m_pWeight[m_iPlayerLoadoutPanel]->SetText(szWeight);

	//aswell as the label, need to set the progress bar
	m_pWeightProgress[m_iPlayerLoadoutPanel]->SetProgress( (float)m_iCurrentWeight[m_iPlayerLoadoutPanel] / (float)iMaxAxisClassSlots[m_iPlayerLoadoutPanel] );	
}

void CRnLAxisClassLoadoutMenu::SetLoadoutModel(Vector vecOrigin, QAngle angAngles)
{
	// Set the coordinates for the loadout models
	vecModelOrigin = vecOrigin;
	angModelAngles = angAngles;
	
	// Rotate everything 90 degree
	// angModelAngles.y += 90; 

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

void CRnLAxisClassLoadoutMenu::SetLoadoutCamera(Vector vecOrigin, QAngle angAngles)
{
	vecCameraOrigin = vecOrigin;
	angCameraAngles = angAngles;
}

void CRnLAxisClassLoadoutMenu::CreateViewportEntity()
{
	//if the model has already been created, exit this func - nuke
	if( m_bViewportModelCreated )
		return;

	// create the models
	// the player loadout model
	m_pLoadoutModel = new C_BaseAnimatingOverlay;

	// The currently selected weapon model
	m_pLoadoutWeaponModel = new C_BaseAnimating;
	m_pLoadoutModelTemp = new C_BaseAnimating;

	// if it fails to create, destroy it
	if ( m_pLoadoutModel->InitializeAsClientEntity( AXIS_PLAYER_HEER_RIFLEMANLOADOUT, RENDER_GROUP_OPAQUE_ENTITY ) == false )
	{
		DevMsg("Loadout Model Axis has been destroyed!\n");
		m_pLoadoutModel->Release();
	}
	if ( m_pLoadoutWeaponModel->InitializeAsClientEntity( EQ_HEER_K98, RENDER_GROUP_OPAQUE_ENTITY ) == false )
	{
		DevMsg("Model was destroyed!\n");
		m_pLoadoutWeaponModel->Release();
	}
	if ( m_pLoadoutModelTemp->InitializeAsClientEntity( EQ_HEER_K98, RENDER_GROUP_OPAQUE_ENTITY ) == false )
	{
		DevMsg("Equipment Model was destroyed!\n");
		m_pLoadoutModelTemp->Release();
	}
	else
	{
		//start animating here
		//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_RIFLEMANLOADOUT );
		m_pLoadoutModel->ForceClientSideAnimationOn();
		//Andrew : Huzzah
		m_pLoadoutModel->SetNumAnimOverlays( 2 );
		m_pLoadoutModel->ResetSequence( m_pLoadoutModel->LookupSequence( "idle_lower" ));
		
		// The weapon model
		m_pLoadoutWeaponModel->FollowEntity(m_pLoadoutModel, true, m_pLoadoutModel->LookupAttachment( "M1_Root" ));
		
		SetModel(m_pLoadoutModelTemp, NULL );

		//reset this at map recreate, as gpGlobals->curtime is also reset
		m_flLastModelUpdateTime = 0.0;
	}
}

void CRnLAxisClassLoadoutMenu::Update()
{
	//model hasn't been created yet, let's exit
	if(m_bViewportModelCreated && gpGlobals->curtime > m_flLastModelUpdateTime)
	{
		angModelAngles.y += 0.3;
		m_pLoadoutModel->SetAbsAngles(angModelAngles);
		m_flLastModelUpdateTime = gpGlobals->curtime + 0.01;
	}
}

void CRnLAxisClassLoadoutMenu::OnComboMenuItemHighlight( KeyValues *data )
{
	int iItemID = -1;

	// if the player highlighted a combo box item
	// then the menu list is out of focus.
	// So there's no more need to display the temporary equipment model - Stefan
	m_pLoadoutModelTemp->SetModel ( NULL );

	switch( m_iPlayerLoadoutPanel )
	{
		case CLASSLOADOUT_GRENADIER:
		{
			KeyValues *currentItemKeyValues = m_pPrimaryWeaponComboBox[CLASSLOADOUT_GRENADIER]->GetItemUserData( data->GetInt("itemID") );
			
			if( currentItemKeyValues != NULL )
			{
				iItemID = currentItemKeyValues->GetInt("primaryweapon");
				//this mess of keyvalues gibberish gets the WEAPON_* value from the combobox selection
				ChangeGrenadierDescription( true, iItemID );
			}
			break;
		}
		case CLASSLOADOUT_MGSCHUTZE:
		{
			KeyValues *currentItemKeyValues = m_pPrimaryWeaponComboBox[CLASSLOADOUT_MGSCHUTZE]->GetItemUserData( data->GetInt("itemID") );
			
			if( currentItemKeyValues != NULL )
			{
				iItemID = currentItemKeyValues->GetInt("primaryweapon");
				//this mess of keyvalues gibberish gets the WEAPON_* value from the combobox selection
				ChangeMGSchutzeDescription( true, iItemID );
			}
			break;
		}
		case CLASSLOADOUT_FUNKER:
		{
			KeyValues *currentItemKeyValues = currentItemKeyValues = m_pPrimaryWeaponComboBox[CLASSLOADOUT_FUNKER]->GetItemUserData( data->GetInt("itemID") );
			
			if( currentItemKeyValues != NULL )
			{
				iItemID = currentItemKeyValues->GetInt("primaryweapon");
				//this mess of keyvalues gibberish gets the WEAPON_* value from the combobox selection
				ChangeFunkerDescription( true, iItemID );
			}
			break;
		}
		case CLASSLOADOUT_SANITAETER:
		{		
			KeyValues *currentItemKeyValues = m_pPrimaryWeaponComboBox[CLASSLOADOUT_SANITAETER]->GetItemUserData( data->GetInt("itemID") );
			
			if( currentItemKeyValues != NULL )
			{
				iItemID = currentItemKeyValues->GetInt("primaryweapon");
				//this mess of keyvalues gibberish gets the WEAPON_* value from the combobox selection
				ChangeSanitaeterDescription( true, iItemID );
			}
			break;
		}
		case CLASSLOADOUT_OFFIZIER:
		{		
			KeyValues *currentItemKeyValues = m_pPrimaryWeaponComboBox[CLASSLOADOUT_OFFIZIER]->GetItemUserData( data->GetInt("itemID") );
			
			if( currentItemKeyValues != NULL )
			{
				iItemID = currentItemKeyValues->GetInt("primaryweapon");
				//this mess of keyvalues gibberish gets the WEAPON_* value from the combobox selection
				ChangeOffizierDescription( true, iItemID );
			}
			break;
		}
	}
}

void CRnLAxisClassLoadoutMenu::ChangeGrenadierDescription( bool bWeapon, int iItemNumber )
{
	// Set the default model to reset the g43 model when a new weapon is being choosen
	//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_RIFLEMANLOADOUT );

	// Remove all grenades to clear them up if the class has been changed. They will be added afterwards with their proper count
	UpdateGrenadesInViewPort();

	if( bWeapon )
	{
		//Andrew : Used to get a layer of animation
		C_AnimationLayer *pLayer = NULL;

		switch( iItemNumber )
		{
		case WEAPON_K98K:
			m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_k98k.txt") );
			m_pItemImage[CLASSLOADOUT_GRENADIER]->SetImage( "images/weapons/k98k" );
			// Set the garand as model now
			SetModel(m_pLoadoutWeaponModel, EQ_HEER_K98 );
			//m_pLoadoutModel->SetSequence( m_pLoadoutModel->LookupSequence("idle_lower"));
			
			//Andrew : Update the upper body
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_k98");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}
			break;

		case WEAPON_G43:

			//strcpy(m_szCurrentClassModel, "models/player/heer/heer_rifleman02.mdl");
			//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_RIFLEMAN2LOADOUT );

			m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_g43.txt") );
			m_pItemImage[CLASSLOADOUT_GRENADIER]->SetImage( "images/weapons/g43" );
			// Set the carbine as model now
			SetModel(m_pLoadoutWeaponModel, EQ_HEER_G43 );
			//m_pLoadoutModel->SetSequence( m_pLoadoutModel->LookupSequence("idle_lower"));

			//Andrew : Update the upper body
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_k98");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}

			break;

		case WEAPON_MP40:
			m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_mp40.txt") );
			m_pItemImage[CLASSLOADOUT_GRENADIER]->SetImage( "images/weapons/mp40" );
			// Set the bar as model now
			SetModel(m_pLoadoutWeaponModel, EQ_HEER_MP40 );
			//m_pLoadoutModel->SetSequence( m_pLoadoutModel->LookupSequence("idle_lower"));

			//Andrew : Update the upper body
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_mp40");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}

			break;

		default:
			Warning("Loadout Rifleman: Something is broken here! - %i \n", iItemNumber);
			break;
		}

		UpdatePouchesInViewPort();
	}
	// If the player changed his equipment, display a text and image for the specified equipment.
	else
	{
		//m_pAddEquipment[CLASSLOADOUT_GRENADIER]->SetEnabled( true );
		//m_pRemoveEquipment[CLASSLOADOUT_GRENADIER]->SetEnabled ( true );
		switch( iItemNumber )
		{
		// Walther Pouch
		case 0:
			m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_walther.txt") );
			m_pItemImage[CLASSLOADOUT_GRENADIER]->SetImage( "images/weapons/walther" );
			//SetModel(m_pLoadoutModelTemp, NULL );
			//SetModel(m_pLoadoutWeaponModel, EQ_HEER_WALTHER );
			break;
		//EQUIPMENT_AXIS_STIELHANDGRANATE
		case 1:
			m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_stielhandgrenade.txt") );
			m_pItemImage[CLASSLOADOUT_GRENADIER]->SetImage( "images/weapons/stielhandgranate" );
			break;
		// rifle grenades
		case 2:
			m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_riflegrenade.txt") );
			m_pItemImage[CLASSLOADOUT_GRENADIER]->SetImage( "images/weapons/riflegrenade" );
			break;
		// Combat knife
		case 3:
			m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_trenchknife.txt") );
			m_pItemImage[CLASSLOADOUT_GRENADIER]->SetImage( "images/weapons/trenchknife" );
			//SetModel(m_pLoadoutModelTemp, EQ_HEER_TRENCHKNIFE_MODEL );
			m_pLoadoutModelTemp->SetParent( m_pLoadoutModel, m_pLoadoutModel->LookupAttachment("Knife"));
			break;
		// extra ammo
		case 4:
			m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_ammo.txt") );
			m_pItemImage[CLASSLOADOUT_GRENADIER]->SetImage( "images/weapons/ammo" );
			break;
		// helmet
		case 5:
			m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_helmet.txt") );
			m_pItemImage[CLASSLOADOUT_GRENADIER]->SetImage( "images/weapons/helmet" );
			break;
		// bayonet
		case 6:
			// Disable the add and remove button, so the player cannot remove the bayonet
			//m_pAddEquipment[CLASSLOADOUT_GRENADIER]->SetEnabled( false );
			//m_pRemoveEquipment[CLASSLOADOUT_GRENADIER]->SetEnabled ( false );
			m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_k98kbayonet.txt") );
			m_pItemImage[CLASSLOADOUT_GRENADIER]->SetImage( "images/weapons/k98kbayonet" );
			break;

		default:
			Warning("Loadout Rifleman: Something is broken here!\n");
			break;
		}
	}
}

void CRnLAxisClassLoadoutMenu::ChangeMGSchutzeDescription( bool bWeapon, int iItemNumber )
{
	// Set the default model to reset the g43 model when a new weapon is being choosen
	//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_RIFLEMANLOADOUT );

	// Remove all grenades to clear them up if the class has been changed. They will be added afterwards with their proper count
	UpdateGrenadesInViewPort();

	if( bWeapon )
	{
		//Andrew : Used to get a layer of animation
		C_AnimationLayer *pLayer = NULL;

		switch( iItemNumber )
		{
		case WEAPON_MG42:
			m_pItemDescription[CLASSLOADOUT_MGSCHUTZE]->SetText( LoadItemDescription("resource/ui/descriptions/description_mg42.txt") );
			m_pItemImage[CLASSLOADOUT_MGSCHUTZE]->SetImage( "images/weapons/mg42" );
			// Set the garand as model now
			SetModel(m_pLoadoutWeaponModel, EQ_HEER_K98 );
			//m_pLoadoutModel->SetSequence( m_pLoadoutModel->LookupSequence("idle_lower"));
			
			//Andrew : Update the upper body
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_k98");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}
			break;

		default:
			Warning("Loadout Rifleman: Something is broken here! - %i \n", iItemNumber);
			break;
		}

		UpdatePouchesInViewPort();
	}
	// If the player changed his equipment, display a text and image for the specified equipment.
	else
	{
		//m_pAddEquipment[CLASSLOADOUT_GRENADIER]->SetEnabled( true );
		//m_pRemoveEquipment[CLASSLOADOUT_GRENADIER]->SetEnabled ( true );
		switch( iItemNumber )
		{
		// Walther Pouch
		case 0:
			m_pItemDescription[CLASSLOADOUT_MGSCHUTZE]->SetText( LoadItemDescription("resource/ui/descriptions/description_walther.txt") );
			m_pItemImage[CLASSLOADOUT_MGSCHUTZE]->SetImage( "images/weapons/walther" );
			//SetModel(m_pLoadoutModelTemp, NULL );
			//SetModel(m_pLoadoutWeaponModel, EQ_HEER_WALTHER );
			break;
		//EQUIPMENT_AXIS_STIELHANDGRANATE
		case 1:
			m_pItemDescription[CLASSLOADOUT_MGSCHUTZE]->SetText( LoadItemDescription("resource/ui/descriptions/description_stielhandgrenade.txt") );
			m_pItemImage[CLASSLOADOUT_MGSCHUTZE]->SetImage( "images/weapons/stielhandgranate" );
			break;
		// rifle grenades
		case 2:
			m_pItemDescription[CLASSLOADOUT_MGSCHUTZE]->SetText( LoadItemDescription("resource/ui/descriptions/description_riflegrenade.txt") );
			m_pItemImage[CLASSLOADOUT_MGSCHUTZE]->SetImage( "images/weapons/riflegrenade" );
			break;
		// Combat knife
		case 3:
			m_pItemDescription[CLASSLOADOUT_MGSCHUTZE]->SetText( LoadItemDescription("resource/ui/descriptions/description_trenchknife.txt") );
			m_pItemImage[CLASSLOADOUT_MGSCHUTZE]->SetImage( "images/weapons/trenchknife" );
			//SetModel(m_pLoadoutModelTemp, EQ_HEER_TRENCHKNIFE_MODEL );
			m_pLoadoutModelTemp->SetParent( m_pLoadoutModel, m_pLoadoutModel->LookupAttachment("Knife"));
			break;
		// extra ammo
		case 4:
			m_pItemDescription[CLASSLOADOUT_MGSCHUTZE]->SetText( LoadItemDescription("resource/ui/descriptions/description_ammo.txt") );
			m_pItemImage[CLASSLOADOUT_MGSCHUTZE]->SetImage( "images/weapons/ammo" );
			break;
		// helmet
		case 5:
			m_pItemDescription[CLASSLOADOUT_MGSCHUTZE]->SetText( LoadItemDescription("resource/ui/descriptions/description_helmet.txt") );
			m_pItemImage[CLASSLOADOUT_MGSCHUTZE]->SetImage( "images/weapons/helmet" );
			break;
		// bayonet
		case 6:
			// Disable the add and remove button, so the player cannot remove the bayonet
			//m_pAddEquipment[CLASSLOADOUT_GRENADIER]->SetEnabled( false );
			//m_pRemoveEquipment[CLASSLOADOUT_GRENADIER]->SetEnabled ( false );
			m_pItemDescription[CLASSLOADOUT_MGSCHUTZE]->SetText( LoadItemDescription("resource/ui/descriptions/description_k98kbayonet.txt") );
			m_pItemImage[CLASSLOADOUT_MGSCHUTZE]->SetImage( "images/weapons/k98kbayonet" );
			break;

		default:
			Warning("Loadout Rifleman: Something is broken here!\n");
			break;
		}
	}
}

void CRnLAxisClassLoadoutMenu::ChangeFunkerDescription( bool bWeapon, int iItemNumber )
{
	// Remove all grenades to clear them up if the class has been changed. They will be added afterwards with their proper count
	UpdateGrenadesInViewPort();

	if( bWeapon )
	{
		ResetAttachments();

		//Andrew : Used to get a layer of animation
		C_AnimationLayer *pLayer = NULL;

		switch( iItemNumber )
		{
		
		case WEAPON_K98K:
			m_pItemDescription[CLASSLOADOUT_FUNKER]->SetText( LoadItemDescription("resource/ui/descriptions/description_k98k.txt") );
			m_pItemImage[CLASSLOADOUT_FUNKER]->SetImage( "images/weapons/k98k" );
			// Set the garand as model now
			SetModel(m_pLoadoutWeaponModel, EQ_HEER_K98 );
			//m_pLoadoutModel->SetSequence( m_pLoadoutModel->LookupSequence("idle_lower"));

			//Andrew : Update the upper body
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_k98");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}

			// Show the k98k pouch
			// waiting on Juggz
			// //SetModel(m_pLoadoutModels[EQUIPMENT_AXIS_K98POUCHES][0], EQ_HEER_K98_POUCH_MODEL );
			break;

		case WEAPON_G43:
			m_pItemDescription[CLASSLOADOUT_FUNKER]->SetText( LoadItemDescription("resource/ui/descriptions/description_g43.txt") );
			m_pItemImage[CLASSLOADOUT_FUNKER]->SetImage( "images/weapons/m1carbine" );

			//Andrew : Update the upper body
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_k98");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}

			// Set the carbine as model now
			//SetModel(m_pLoadoutWeaponModel, EQ_HEER_G43 );
			// Show the g43 pouch
			// //SetModel(m_pLoadoutModels[EQUIPMENT_AXIS_G43POUCHES][0], EQ_HEER_G43_POUCH_MODEL );
		
			break;

		case WEAPON_MP40:
			m_pItemDescription[CLASSLOADOUT_FUNKER]->SetText( LoadItemDescription("resource/ui/descriptions/description_mp40.txt") );
			m_pItemImage[CLASSLOADOUT_FUNKER]->SetImage( "images/weapons/mp40" );
			// show the mp40 weapon
			//SetModel(m_pLoadoutWeaponModel, EQ_HEER_MP40 );

			//Andrew : Update the upper body
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_mp40");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}

			break;

		default:
			Warning("Loadout Radioman: Something is broken here!\n");
			break;
		}

		UpdatePouchesInViewPort();
	}
	// If the player changed his equipment, display a text and image for the specified equipment.
	else
	{
		m_pAddEquipment[CLASSLOADOUT_FUNKER]->SetEnabled( true );
		m_pRemoveEquipment[CLASSLOADOUT_FUNKER]->SetEnabled ( true );

		switch( iItemNumber )
		{
		case 0:
			m_pItemDescription[CLASSLOADOUT_FUNKER]->SetText( LoadItemDescription("resource/ui/descriptions/description_walther.txt") );
			m_pItemImage[CLASSLOADOUT_FUNKER]->SetImage( "images/weapons/walther" );
			//SetModel(m_pLoadoutWeaponModel, EQ_HEER_WALTHER );
			//m_pLoadoutModel->SetSequence( m_pLoadoutModel->LookupSequence( "idle_lower" ) );
			break;
		//EQUIPMENT_AXIS_STIELHANDGRANATE
		case 1:
			m_pItemDescription[CLASSLOADOUT_FUNKER]->SetText( LoadItemDescription("resource/ui/descriptions/description_stielhandgrenade.txt") );
			m_pItemImage[CLASSLOADOUT_FUNKER]->SetImage( "images/weapons/stielhandgrenade" );
			break;
		// rifle grenades
		case 2:
			m_pItemDescription[CLASSLOADOUT_FUNKER]->SetText( LoadItemDescription("resource/ui/descriptions/description_riflegrenade.txt") );
			m_pItemImage[CLASSLOADOUT_FUNKER]->SetImage( "images/weapons/riflegrenade" );
			break;
		// Combat knife
		case 3:
			m_pItemDescription[CLASSLOADOUT_FUNKER]->SetText( LoadItemDescription("resource/ui/descriptions/description_trenchknife.txt") );
			m_pItemImage[CLASSLOADOUT_FUNKER]->SetImage( "images/weapons/trenchknife" );
			//SetModel(m_pLoadoutModelTemp, EQ_HEER_TRENCHKNIFE_MODEL );
			m_pLoadoutModelTemp->SetParent( m_pLoadoutModel, m_pLoadoutModel->LookupAttachment("Knife"));
			break;
		// extra ammo
		case 4:
			m_pItemDescription[CLASSLOADOUT_FUNKER]->SetText( LoadItemDescription("resource/ui/descriptions/description_ammo.txt") );
			m_pItemImage[CLASSLOADOUT_FUNKER]->SetImage( "images/weapons/ammo" );
			break;
		// helmet
		case 5:
			m_pItemDescription[CLASSLOADOUT_FUNKER]->SetText( LoadItemDescription("resource/ui/descriptions/description_german_helmet.txt") );
			m_pItemImage[CLASSLOADOUT_FUNKER]->SetImage( "images/weapons/helmet" );
			break;
		// bayonet
		case 6:
			m_pAddEquipment[CLASSLOADOUT_FUNKER]->SetEnabled( false );
			m_pRemoveEquipment[CLASSLOADOUT_FUNKER]->SetEnabled ( false );
			m_pItemDescription[CLASSLOADOUT_FUNKER]->SetText( LoadItemDescription("resource/ui/descriptions/description_k98kbayonet.txt") );
			m_pItemImage[CLASSLOADOUT_FUNKER]->SetImage( "images/weapons/k98kbayonet" );
			break;

		default:
			Warning("Loadout Radioman: Wrong Equipment Index\n");
			break;
		}
	}
}
void CRnLAxisClassLoadoutMenu::ChangeSanitaeterDescription( bool bWeapon, int iItemNumber )
{
	if( bWeapon )
	{
		m_pItemDescription[CLASSLOADOUT_SANITAETER]->SetText( LoadItemDescription("resource/ui/descriptions/description_unarmed.txt") );
	}
	else
	{
		//Here we need to add a switch for the medics special loadout equipment.
		switch( iItemNumber )
		{
		//EQUIPMENT_AXIS_STIELHANDGRANATE
		case 0:
			m_pItemDescription[CLASSLOADOUT_SANITAETER]->SetText( LoadItemDescription("resource/ui/descriptions/description_stielhandgrenade.txt") );
			break;
		// helmet
		case 5:
			m_pItemDescription[CLASSLOADOUT_SANITAETER]->SetText( LoadItemDescription("resource/ui/descriptions/description_helmet.txt") );
			m_pItemImage[CLASSLOADOUT_SANITAETER]->SetImage( "images/weapons/helmet" );
			break;
		}
	}
}
void CRnLAxisClassLoadoutMenu::ChangeOffizierDescription( bool bWeapon, int iItemNumber )
{
	//SetModel(m_pLoadoutModel, AXIS_PLAYER_HEER_NCOLOADOUT );

	// Hide the helmet for now, the offizier has a fixed one at the moment - Stefan
	//  NO LONGER! - nuke
	// m_pLoadoutModels[EQUIPMENT_AXIS_HELMET][0]->SetEffects( EF_NODRAW );

	// Remove all grenades to clear them up if the class has been changed. They will be added afterwards with their proper count
	UpdateGrenadesInViewPort();

	if( bWeapon )
	{
		ResetAttachments();

		//Andrew : Used to get a layer of animation
		C_AnimationLayer *pLayer = NULL;

		switch( iItemNumber )
		{
		case WEAPON_MP40:
			m_pItemDescription[CLASSLOADOUT_OFFIZIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_mp40.txt") );
			m_pItemImage[CLASSLOADOUT_OFFIZIER]->SetImage( "images/weapons/mp40" );
			SetModel(m_pLoadoutWeaponModel, EQ_HEER_MP40 );

			//Andrew : Update the upper body
			pLayer = m_pLoadoutModel->GetAnimOverlay( 1 );
			if( pLayer )
			{
				pLayer->m_flCycle = 0;
				pLayer->m_nSequence = m_pLoadoutModel->LookupSequence("idle_upper_mp40");

				pLayer->m_flPlaybackRate = 1.0;
				pLayer->m_flWeight = 1.0f;
				pLayer->m_nOrder = 1;
			}
			break;

		default:
			Warning("Loadout OFFIZIER: Wrong weapon index!\n");
			break;
		}

		UpdatePouchesInViewPort();
	}
// If the player changed his equipment, display a text and image for the specified equipment.
	else
	{
		m_pAddEquipment[CLASSLOADOUT_OFFIZIER]->SetEnabled( true );
		m_pRemoveEquipment[CLASSLOADOUT_OFFIZIER]->SetEnabled ( true );

		switch( iItemNumber )
		{
		// pistol
		case 0:
			m_pItemDescription[CLASSLOADOUT_OFFIZIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_walther.txt") );
			m_pItemImage[CLASSLOADOUT_OFFIZIER]->SetImage( "images/weapons/walther" );
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( P38HOLSTER_BODYGROUP ), 1 );
			
			break;
		//EQUIPMENT_AXIS_STIELHANDGRANATE
		case 1:
			m_pItemDescription[CLASSLOADOUT_OFFIZIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_stielhandgrenade.txt") );
			m_pItemImage[CLASSLOADOUT_OFFIZIER]->SetImage( "images/weapons/stielhandgrenade" );
			break;
		// rifle grenades
		case 2:
			m_pItemDescription[CLASSLOADOUT_OFFIZIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_riflegrenade.txt") );
			m_pItemImage[CLASSLOADOUT_OFFIZIER]->SetImage( "images/weapons/riflegrenade" );
			break;
		// Combat knife
		case 3:
			m_pItemDescription[CLASSLOADOUT_OFFIZIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_trenchknife.txt") );
			m_pItemImage[CLASSLOADOUT_OFFIZIER]->SetImage( "images/weapons/trenchknife" );
			// awaiting juggz to add knife
			break;
		// extra ammo
		case 4:
			m_pItemDescription[CLASSLOADOUT_OFFIZIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_ammo.txt") );
			m_pItemImage[CLASSLOADOUT_OFFIZIER]->SetImage( "images/weapons/ammo" );
			break;
		// helmet
		case 5:
			m_pItemDescription[CLASSLOADOUT_OFFIZIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_helmet.txt") );
			m_pItemImage[CLASSLOADOUT_OFFIZIER]->SetImage( "images/weapons/helmet" );
			break;
		// bayonet
		case 6:
			m_pAddEquipment[CLASSLOADOUT_OFFIZIER]->SetEnabled( false );
			m_pRemoveEquipment[CLASSLOADOUT_OFFIZIER]->SetEnabled ( false );
			m_pItemDescription[CLASSLOADOUT_OFFIZIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_k98kbayonet.txt") );
			m_pItemImage[CLASSLOADOUT_OFFIZIER]->SetImage( "images/weapons/k98kbayonet" );
			break;
		// Smoke grenades
		case 7:
			m_pItemDescription[CLASSLOADOUT_GRENADIER]->SetText( LoadItemDescription("resource/ui/descriptions/description_smokegrenade.txt") );
			m_pItemImage[CLASSLOADOUT_GRENADIER]->SetImage( "images/weapons/smokegranate" );
			break;
			break;
		default:
			Warning("Loadout OFFIZIER, Wrong item index\n");
			break;
		}
	}
}

//pretty much ripped from the team menu, why reinvent the wheel ;) - nuke
const char *CRnLAxisClassLoadoutMenu::LoadItemDescription( const char *szItemDescription )
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

void CRnLAxisClassLoadoutMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );
	m_pClassPanel[CLASSLOADOUT_GRENADIER]->SetBorder( null );//( pScheme->GetBorder("BaseBorder") );
	m_pClassPanel[CLASSLOADOUT_FUNKER]->SetBorder( null );//( pScheme->GetBorder("BaseBorder") );
	m_pClassPanel[CLASSLOADOUT_SANITAETER]->SetBorder( null );//( pScheme->GetBorder("BaseBorder") );
	m_pClassPanel[CLASSLOADOUT_OFFIZIER]->SetBorder( null );//( pScheme->GetBorder("BaseBorder") );
	m_pInsetBorder->SetBorder( pScheme->GetBorder("BaseBorder") );
	m_pDefaultPanelLabel->SetFont( pScheme->GetFont("LoadoutMenu") );
	m_pDefaultPanel->SetBorder( null );//pScheme->GetBorder("BaseBorder") );

	SetBgColor( Color( 0,0,0,0) );
}

void CRnLAxisClassLoadoutMenu::OnEquipmentItemSelected( KeyValues *data )
{
	//we can only supply description for one item at a time, so only the first
	int iUserDataID = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItemUserData( m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetSelectedItem(0) );
	
	//-1 is failed
	if(iUserDataID != -1)
	{
		// Reset the helmets drawing state here, so it shows up again
		// This is only temporary, until the hat is made an attachment - Stefan
		// m_pLoadoutModels[EQUIPMENT_AXIS_HELMET][0]->RemoveEffects( EF_NODRAW );

		switch(m_iPlayerLoadoutPanel)
		{
		case CLASSLOADOUT_GRENADIER:
				ChangeGrenadierDescription( false, iUserDataID );
				break;
		case CLASSLOADOUT_FUNKER:
				ChangeFunkerDescription( false, iUserDataID );
				break;
		case CLASSLOADOUT_SANITAETER:
				ChangeSanitaeterDescription( false, iUserDataID );
				break;
		case CLASSLOADOUT_OFFIZIER:
				ChangeOffizierDescription( false, iUserDataID );
				break;
		}

	}
}

// Attach the equipment to the loadout player model
// the +1 is necessary to fit the enum which starts with 0 as weapon_none
// while here: weapon_walther == 0 in this class...sort of cheesy - Stefan
// Could be fixed by adding a EQUIPMENT_AXIS_NONE
void CRnLAxisClassLoadoutMenu::AttachEquipmentToViewPort( int iItemNumber)
{
	switch( iItemNumber + 1 )
	{
		case EQUIPMENT_AXIS_STIELHANDGRANATE:
		case EQUIPMENT_AXIS_SMOKEGRENADE:
			UpdateGrenadesInViewPort();
			break;
		case EQUIPMENT_AXIS_WALTHER: // Should be walther...but hey, the enums wants the colt
			AddP38();
			break;
		case EQUIPMENT_AXIS_MELEE:
			////SetModel(m_pLoadoutModels[EQUIPMENT_AXIS_MELEE][0], EQ_HEER_TRENCHKNIFE_MODEL );
			break;
		case EQUIPMENT_AXIS_EXTRAAMMO:
			// IF the nco panel is open, add it, don't if not
			if( m_pClassPanel[CLASSLOADOUT_OFFIZIER]->IsVisible() )
			{

				UpdatePouchesInViewPort();

				// Remove the p38
				RemoveP38();

				// Disable the walther for now - Stefan
				m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iWalther[m_iPlayerLoadoutPanel], false);
			}
			break;
		default:
			DevMsg("Unanticipated equipment type in Loadout VGUI!\n");
		break;
	}
}
// Remove the equipment
void CRnLAxisClassLoadoutMenu::RemoveEquipmentFromViewPort( int iItemNumber )
{
	switch(iItemNumber + 1 )
	{
		case EQUIPMENT_AXIS_STIELHANDGRANATE:
		case EQUIPMENT_AXIS_SMOKEGRENADE:
			UpdateGrenadesInViewPort();
			break;
		case EQUIPMENT_AXIS_WALTHER:
			RemoveP38();
			break;
		case EQUIPMENT_AXIS_EXTRAAMMO:
			if( HasExtraAmmo() )
			{
				// Remove the p38
				RemoveP38();

				// Disable the walther for now - Stefan
				m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iWalther[m_iPlayerLoadoutPanel], false);
			}
			else
				// reshow the p38
				AddP38();

			UpdatePouchesInViewPort();
			break;
		default:
			DevMsg("Unanticipated equipment type in Loadout VGUI!\n");
			break;
	}

	//SetModel(m_pLoadoutModelTemp,NULL);

}

void CRnLAxisClassLoadoutMenu::UpdateGrenadesInViewPort()
{
	KeyValues *GrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iFragGrenades[m_iPlayerLoadoutPanel]);
	KeyValues *SmokeGrenadeItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iSmokeGrenades[m_iPlayerLoadoutPanel]);

	// set relevant submodels
	if( GrenadeItem )
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( GRENADES_BODYGROUP ), GrenadeItem->GetInt("carry") );
	if( SmokeGrenadeItem )
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( SMOKES_BODYGROUP ), SmokeGrenadeItem->GetInt("carry") );
}

void CRnLAxisClassLoadoutMenu::UpdatePouchesInViewPort()
{
	char szModelName[128];
	strcpy( szModelName, modelinfo->GetModelName( modelinfo->GetModel( m_pLoadoutWeaponModel->GetModelIndex() ) ) );

	if( !Q_strcmp( szModelName, EQ_HEER_MP40 ) )
	{
		if( HasExtraAmmo() )
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( AMMO_POUCHES_BODYGROUP ), MP40_POUCHES_EXTRA );
		else
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( AMMO_POUCHES_BODYGROUP ), MP40_POUCHES );
	}
	else if( !Q_strcmp( szModelName, EQ_HEER_K98 ) )
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( AMMO_POUCHES_BODYGROUP ), K98K_POUCHES );

}

bool CRnLAxisClassLoadoutMenu::HasExtraAmmo( void )
{
	KeyValues *ExtraAmmo = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iExtraAmmo[m_iPlayerLoadoutPanel]);
	if( ExtraAmmo->GetInt("carry") > 0 )
		return true;
	else
		return false;
}

void CRnLAxisClassLoadoutMenu::ResetAttachments()
{
	// Hide the weapon pouches
	m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( AMMO_POUCHES_BODYGROUP ), 0 );

	if( m_pClassPanel[CLASSLOADOUT_OFFIZIER]->IsVisible() )
	{
		// show gear and helmet
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( GEAR_BODYGROUP ), 2 );
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( HELMET_BODYGROUP ), 5 );

		// show the pistol if carrying it
		KeyValues *WaltherItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iWalther[m_iPlayerLoadoutPanel] );

		if( WaltherItem->GetInt("carry") == 1 )
			m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( P38HOLSTER_BODYGROUP ), 1 );
	}
	else
	{
		// show gear and helmet
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( GEAR_BODYGROUP ), 1 );
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( HELMET_BODYGROUP ), RandomInt(1, 4) );
		m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( P38HOLSTER_BODYGROUP ), 0 );
	}
}

void CRnLAxisClassLoadoutMenu::AddP38()
{
	if( m_pClassPanel[CLASSLOADOUT_OFFIZIER]->IsVisible() )
	{
		// Enable it again
		m_pAvailableEquipment[m_iPlayerLoadoutPanel]->SetItemVisible(m_iWalther[m_iPlayerLoadoutPanel], true);
	}

	// show the bodygroup
	m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( P38HOLSTER_BODYGROUP ), 1 );
	
	KeyValues *WaltherItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iWalther[m_iPlayerLoadoutPanel] );

	if( WaltherItem->GetInt("carry") < 1 )
	{
		WaltherItem->SetInt("carry", 1);
		m_iCurrentWeight[m_iPlayerLoadoutPanel] += WaltherItem->GetInt("weight");
	}
}

void CRnLAxisClassLoadoutMenu::RemoveP38()
{
	KeyValues *WaltherItem = m_pAvailableEquipment[m_iPlayerLoadoutPanel]->GetItem( m_iWalther[m_iPlayerLoadoutPanel] );
	
	// hide the bodygroup
	m_pLoadoutModel->SetBodygroup( m_pLoadoutModel->FindBodygroupByName( P38HOLSTER_BODYGROUP ), 0 );

	if( WaltherItem->GetInt("carry") > 0 )
	{
		WaltherItem->SetInt("carry", 0);
		m_iCurrentWeight[m_iPlayerLoadoutPanel] -= WaltherItem->GetInt("weight");
	}
}
// Do some checks before we actually set a sequence or a model
// to make sure the model instances we created really exist - Stefan
void CRnLAxisClassLoadoutMenu::SetModel(C_BaseAnimating *pModel, char cModelName[] )
{
	if( pModel )
		pModel->SetModel(cModelName );
}

void CRnLAxisClassLoadoutMenu::DrawLoadoutModels( void )
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

	if( IsVisible() && m_bIsClassPanelVisible && !vgui::ipanel()->IsVisible( vgui::ipanel()->GetParent( console ) ) )
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
