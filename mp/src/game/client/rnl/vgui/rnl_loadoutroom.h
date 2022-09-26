/*********************************
* Resistance and Liberation Loadout UI
*
* Copyright (c) 2008, Blackened Interactive
* Author: Cale "Mazor" Dunlap (cale@blackenedinteractive.com)
* Date: 7/27/2008
*
* Purpose: A giant pile of model panels and buttons to create the loadout "room"
**********************************/

#ifndef VGUI_LOADOUT_PANEL
#define VGUI_LOADOUT_PANEL

#include <vgui_controls/Frame.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Button.h>
#include <game_controls/basemodelpanel.h>
#include <game/client/iviewport.h>
#include <vgui/KeyCode.h>

// Sub menus
#include "rnl_loadoutmenu.h"
#include "rnl_squadselectmenu.h"
#include "rnl_appearancemenu.h"
	
struct LoadoutRoomData
{
	LoadoutRoomData()
	{
		m_iHelmetNumber = 1;
		m_iSkinNumber = 0;
		m_iPrimaryWeapon = WEAPON_NONE;
		m_iPrimaryAmmoExtra = 0;
		m_iSecondaryWeapon = WEAPON_NONE;
		m_iSecondaryAmmoExtra = 0;
		m_iSmokeGrenades = 0;
		m_iFragGrenades = 0;
		m_iSquad = 0;
		m_iClass = 0;
		memset( m_szModelName, 0, CLASS_DESC_MODEL_LEN );
	}
	int m_iHelmetNumber;
	int m_iSkinNumber;
	int m_iPrimaryWeapon;
	int m_iPrimaryAmmoExtra;
	int m_iSecondaryWeapon;
	int	m_iSecondaryAmmoExtra;
	int m_iSmokeGrenades;
	int m_iFragGrenades;
	int m_iSquad;
	int m_iClass;
	char m_szModelName[CLASS_DESC_MODEL_LEN];
};

class CRnLLoadoutRoomButton : public vgui::Button
{
public:
	DECLARE_CLASS_SIMPLE( CRnLLoadoutRoomButton, vgui::Button );

	CRnLLoadoutRoomButton( vgui::Panel *pParent, const char *pszName, const char *pszText, vgui::Panel *pActionSignalTarget = 0, const char *pCmd = 0  ) 
		: vgui::Button( pParent, pszName, pszText, pActionSignalTarget, pCmd )
	{}
	~CRnLLoadoutRoomButton()
	{}

	virtual void	OnCursorEntered( void )
	{
		KeyValues *data = new KeyValues("LoadoutButton_CursorEntered");
		data->SetString("Sender", GetName() );
		PostActionSignal(data);

		BaseClass::OnCursorEntered();
	}
	virtual void	OnCursorExited( void )
	{
		KeyValues *data = new KeyValues("LoadoutButton_CursorExited");
		data->SetString("Sender", GetName() );
		PostActionSignal(data);

		BaseClass::OnCursorExited();
	}
};

class CRnLLoadoutRoom : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CRnLLoadoutRoom, vgui::Frame );

public:
	CRnLLoadoutRoom( IViewPort *pViewPort );
	~CRnLLoadoutRoom();

	const char		*GetName( void ) { return PANEL_LOADOUT_ROOM; }
	void			SetData( KeyValues *data );
	void			Reset( void );
	void			Update( void );
	bool			NeedsUpdate( void );
	bool			HasInputElements( void ) { return true; }
	void			ShowPanel( bool bShow );
	void			OnTick( void );

	vgui::VPANEL	GetVPanel( void ) { return BaseClass::GetVPanel(); }
	bool			IsVisible( void ) { return BaseClass::IsVisible(); }
	void			SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	MESSAGE_FUNC_PARAMS( OnSquadSelected, "SquadSelected", data );
	MESSAGE_FUNC_PARAMS( OnEquipmentSelected, "EquipmentSelected", data );
	MESSAGE_FUNC_PARAMS( OnAppearanceSelected, "AppearanceSelected", data );
	MESSAGE_FUNC_PARAMS( OnLoadoutButtonEntered, "LoadoutButton_CursorEntered", data );
	MESSAGE_FUNC_PARAMS( OnLoadoutButtonExited, "LoadoutButton_CursorExited", data );
	
protected:
	void			ApplySchemeSettings( vgui::IScheme *pScheme );
	void			OnKeyCodePressed( vgui::KeyCode code );
	void			OnCommand( const char *command );


	IViewPort		*m_pViewPort;

private:
	void					ChangeDisplayTeam( int iTeam );		// Sets up the panel for a different team
	inline void				SetAllModelPanelsDirty( void );
	inline const char		*GetSongFromPlayerTeam( void );
	inline bool				RequirementsMetToJoin( void ) const;
	void					SendLoadoutString( void );

	CModelPanel		*m_pSavedLoadoutsModelPanel;
	CModelPanel		*m_pHelmetModelPanel;
	CModelPanel		*m_pEquipmentModelPanel;
	CModelPanel		*m_pTrashModelPanel;
	CModelPanel		*m_pBackgroundModelPanel;
	CModelPanel		*m_pSquadModelPanel;
	CModelPanel		*m_pMapInfoModelPanel;

	CRnLLoadoutRoomButton			*m_pSavedLoadoutsButton;
	CRnLLoadoutRoomButton			*m_pHelmetButton;
	CRnLLoadoutRoomButton			*m_pEquipmentButton;
	CRnLLoadoutRoomButton			*m_pTrashButton;
	CRnLLoadoutRoomButton			*m_pSquadButton;
	CRnLLoadoutRoomButton			*m_pMapInfoButton;

	vgui::Button			*m_pJoinGameButton;

	// Sub menus
	CRnLLoadoutMenu		*m_pSubLoadoutMenu;
	CRnLSquadSelectMenu	*m_pSquadSelectMenu;
	CRnLAppearanceMenu	*m_pAppearanceMenu;

	LoadoutRoomData	LoadoutData;
	int				m_iLoadoutSongGuid;
	float			m_flNextSoundThinkTime;
	float			m_flNextUpdateTime;
	int				m_iDisplayTeam;

	bool			m_bPlayerSelectedSquad, m_bPlayerSelectedAppearance, m_bPlayerSelectedEquipment; // for requirements checking
};

#endif
