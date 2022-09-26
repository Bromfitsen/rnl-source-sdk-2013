#ifndef RNL_ITEMSELECTIONPANEL
#define RNL_ITEMSELECTIONPANEL
#ifdef WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>

#define MAX_ITEM_SELECTION_NAME_LEN	32

struct ItemSelectionDataElement
{
	ItemSelectionDataElement()
	{
		memset( m_szName, 0, MAX_ITEM_SELECTION_NAME_LEN );
		m_iValue = 0;
	}

	char m_szName[MAX_ITEM_SELECTION_NAME_LEN];
	int m_iValue;
};

class ItemSelectionDataCollection : public CUtlVector<ItemSelectionDataElement *>
{
public:
	ItemSelectionDataCollection()
	{ }
	~ItemSelectionDataCollection()
	{
		PurgeAndDeleteElements();
	}
	
	void AddToHead( const char *pszName, int iValue )
	{
		ItemSelectionDataElement *pElement = new ItemSelectionDataElement();
		Q_strncpy( pElement->m_szName, pszName, MAX_ITEM_SELECTION_NAME_LEN );
		pElement->m_iValue = iValue;

		CUtlVector<ItemSelectionDataElement *>::AddToHead( pElement );
	}

	void AddToTail( const char *pszName, int iValue )
	{
		ItemSelectionDataElement *pElement = new ItemSelectionDataElement();
		Q_strncpy( pElement->m_szName, pszName, MAX_ITEM_SELECTION_NAME_LEN );
		pElement->m_iValue = iValue;

		CUtlVector<ItemSelectionDataElement *>::AddToTail( pElement );
	}
};

class ItemSelectionPanel : public vgui::Panel
{
	typedef vgui::Panel BaseClass;
public:
	// constructor
	ItemSelectionPanel( vgui::Panel *pParent, const char *pName );
	ItemSelectionPanel( vgui::Panel *pParent, const char *pName, ItemSelectionDataCollection Data );
	~ItemSelectionPanel();

	virtual void ApplySettings(KeyValues *inResourceData);
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnCommand( const char *command );

	void	SetSelectedIndex( int iIndex );
	void	SetSelectionValue( int iValue );
	void	AddValue( const char *pszName, int iValue );
	int		GetSelectionValue( void ) const;
	ItemSelectionDataCollection	&GetData( void ) { return m_pData; }

private:
	vgui::Label*	m_pTitleLabel;
	vgui::Label*	m_pValueLabel;
	vgui::Button*	m_pNextButton;
	vgui::Button*	m_pPrevButton;

	int				m_iSelectedIndex;
	ItemSelectionDataCollection	m_pData;
};

#endif