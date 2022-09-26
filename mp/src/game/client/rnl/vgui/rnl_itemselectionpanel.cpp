#include "cbase.h"
#include "rnl_itemselectionpanel.h"
#include <vgui/IScheme.h>

#include "tier0/memdbgon.h"

using namespace vgui;

DECLARE_BUILD_FACTORY( ItemSelectionPanel );

ItemSelectionPanel::ItemSelectionPanel( vgui::Panel *pParent, const char *pName ) : BaseClass( pParent, pName )
{
	m_pTitleLabel = new Label( this, "item_name", "none" );
	m_pValueLabel = new Label( this, "item_value", "none" );
	m_pNextButton = new Button( this, "item_next", ">", this, "Next" );
	m_pPrevButton = new Button( this, "item_prev", "<", this, "Prev" );

	m_iSelectedIndex = 0;

	m_pTitleLabel->SetVisible( true );
	m_pValueLabel->SetVisible( true );
	m_pPrevButton->SetVisible( true );
	m_pNextButton->SetVisible( true );
}

ItemSelectionPanel::ItemSelectionPanel( vgui::Panel *pParent, const char *pName, ItemSelectionDataCollection Data )
{
	ItemSelectionPanel::ItemSelectionPanel(pParent, pName);
	m_pData = Data;
}

ItemSelectionPanel::~ItemSelectionPanel()
{
	m_pTitleLabel->MarkForDeletion();
	m_pValueLabel->MarkForDeletion();
	m_pNextButton->MarkForDeletion();
	m_pPrevButton->MarkForDeletion();
	
	m_pData.PurgeAndDeleteElements();
}

void ItemSelectionPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	m_pNextButton->SetPaintBackgroundEnabled( false );
	m_pNextButton->SetPaintBorderEnabled( false );
	m_pPrevButton->SetPaintBackgroundEnabled( false );
	m_pPrevButton->SetPaintBorderEnabled( false );
	BaseClass::ApplySchemeSettings(pScheme);
}

void ItemSelectionPanel::ApplySettings(KeyValues *inResourceData)
{
	m_pTitleLabel->SetText( inResourceData->GetString( "item_title" ) );

	m_pTitleLabel->SetPos( 
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_title_x", 2 )), 
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_title_y", 2 )));
	m_pTitleLabel->SetSize( 
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_title_wide", 234 )), 
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_title_tall", 20 )));

	m_pNextButton->SetPos( 
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_next_x", 208 )), 
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_next_y", 24 )));
	m_pNextButton->SetSize(
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_next_wide", 20 )),
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_next_tall", 20 )));

	m_pPrevButton->SetPos( 
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_prev_x", 2 )), 
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_prev_y", 24 )));
	m_pPrevButton->SetSize(
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_prev_wide", 20 )),
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_prev_tall", 20 )));

	m_pValueLabel->SetPos( 
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_desc_x", 20 )), 
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_desc_y", 24 )));
	m_pValueLabel->SetSize( 
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_desc_wide", 206 )), 
		scheme()->GetProportionalScaledValue(inResourceData->GetInt( "item_desc_tall", 20 )));

	BaseClass::ApplySettings( inResourceData );
}

void ItemSelectionPanel::SetSelectedIndex( int iIndex )
{
	if( iIndex + 1 > m_pData.Count() )
		return;

	m_iSelectedIndex = iIndex;
	m_pValueLabel->SetText( m_pData[iIndex]->m_szName );
}

void ItemSelectionPanel::SetSelectionValue( int iValue )
{
	int index = 0;
	for( int i = 0; i < m_pData.Count(); i++ )
	{
		if( m_pData[i] == NULL )
			break;
		if( m_pData[i]->m_iValue == iValue )
		{
			index = i;
			break;
		}
	}
	SetSelectedIndex( index );
}

int ItemSelectionPanel::GetSelectionValue( void ) const
{
	if( m_pData.Count() == 0 ) // no data to select, return a bad number
		return -1;
	
	return m_pData[m_iSelectedIndex]->m_iValue;
}

void ItemSelectionPanel::AddValue( const char *pszName, int iValue )
{
	m_pData.AddToTail( pszName, iValue );
}

void ItemSelectionPanel::OnCommand( const char *command )
{
	DevMsg( "%s\n", command );

	// all commands are invalid unless the data array is populated or one is reference
	if( m_pData.Count() == 0 )
		return;

	if( FStrEq( command, "Next" ) )
	{
		if( m_iSelectedIndex + 1 >= m_pData.Count() )
			return;
		else
		{
			// if the selection value is NULL, don't continue
			if( m_pData[m_iSelectedIndex + 1] != NULL )
				m_iSelectedIndex++;
			else
				return;
		}
	}
	else if( FStrEq( command, "Prev" ) )
	{
		if( m_iSelectedIndex - 1 < 0 )
			return;
		else
		{
			if( m_pData[m_iSelectedIndex + 1] != NULL )
				m_iSelectedIndex--;
			else
				return;
		}
	}
	else
		return;	// critical that this stays here, or it will fall through and update things that don't need it

	m_pValueLabel->SetText( m_pData[m_iSelectedIndex]->m_szName );

	KeyValues *data = new KeyValues( "SelectionPanelChanged" );
	data->SetString("Command", command );
	data->SetInt( "CurrentValue", m_pData[m_iSelectedIndex]->m_iValue );
	data->SetString( "Sender", GetName() );
	PostActionSignal( data );
}