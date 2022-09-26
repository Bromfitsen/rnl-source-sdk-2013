#ifndef RNL_APPEARANCEMENU
#define RNL_APPEARANCEMENU
#ifdef WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include "game_controls/basemodelpanel.h"
#include <game/client/iviewport.h>
#include "rnl_itemselectionpanel.h"

struct AppearanceMenuData
{
	AppearanceMenuData()
	{
		m_iHelmetNumber = 1;
		m_iSkinNumber = 0;
	}
	int m_iHelmetNumber;
	int m_iSkinNumber;
};

class CRnLAppearanceMenu : public vgui::Frame, public IViewPortPanel
{
public:
	DECLARE_CLASS_SIMPLE( CRnLAppearanceMenu, vgui::Frame );
	CRnLAppearanceMenu( IViewPort *pViewPort );
	~CRnLAppearanceMenu();

	const char	*GetName( void ) { return PANEL_APPEARANCE; }
	void	SetData( KeyValues *data );
	void	Reset( void );
	void	Update( void );
	bool	NeedsUpdate( void );
	bool	HasInputElements( void ) { return true; }
	void	ShowPanel( bool bShow );
	void	OnThink( void );
	
	vgui::VPANEL	GetVPanel( void ) { return BaseClass::GetVPanel(); }
	bool			IsVisible( void ) { return BaseClass::IsVisible(); }
	void			SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	MESSAGE_FUNC_PARAMS( OnSelectionPanelChanged, "SelectionPanelChanged", data );

protected:
	void		ApplySchemeSettings( vgui::IScheme *pScheme );
	void		OnCommand( const char *command );

	IViewPort	*m_pViewPort;

private:

	int		m_iDisplayTeam;
	char	m_szCurrentPlayerModel[CLASS_DESC_MODEL_LEN];

	CModelPanel	*m_pPreviewPanel;
	ItemSelectionPanel	*m_pHelmetSelection;
	ItemSelectionPanel	*m_pSkinSelection;

	AppearanceMenuData	AppearanceData;
};

#endif