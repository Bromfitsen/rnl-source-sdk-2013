#ifndef RNL_LOADINGPANEL_H
#define RNL_LOADINGPANEL_H

#include "gameeventlistener.h"

class CRnLLoadingImage
{
public:
	CUtlString	m_WidescreenFile;
	CUtlString	m_NormalFile;
};

class CRnLLoadingPanel : public vgui::EditablePanel, public CGameEventListener
{
private:
	DECLARE_CLASS_SIMPLE( CRnLLoadingPanel, vgui::EditablePanel );

public:
	static CRnLLoadingPanel*	GetInstance( void );
	static void					DestroyInstance( void );

public:
	CRnLLoadingPanel();	
	~CRnLLoadingPanel();

	void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
	void FireGameEvent( IGameEvent *event ) OVERRIDE;

private:
	MESSAGE_FUNC( OnActivate, "activate" );
	MESSAGE_FUNC( OnDeactivate, "deactivate" );

	void Reset();
	void Update();
	void UpdateTip();
	void UpdateBackground();
	void ClearMapLabel();

	// Helper function to parse tip labels
	// to replace the names of functions (ie +duck)
	// with the name of the key it is bound to (ie CTRL)
	wchar_t *ParseTipForKeys( wchar_t* wzStr );

	vgui::Label		*m_pTipLabel;

	CUtlVector<CRnLLoadingImage>	m_ContentPanelFiles;
	CUtlVector<CUtlString>			m_TipKeys;
	float							m_fLastTipTime;
	float							m_fLastContentTime;
	bool							m_bMapSet;
};

#endif //RNL_LOADINGPANEL_H