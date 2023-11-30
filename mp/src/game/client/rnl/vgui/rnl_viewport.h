//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_VIEWPORT_H
#define RNL_VIEWPORT_H


#include "rnl_shareddefs.h"
#include "baseviewport.h"


using namespace vgui;

namespace vgui 
{
	class Panel;
}

class CRnLViewport : public CBaseViewport
{

private:
	DECLARE_CLASS_SIMPLE( CRnLViewport, CBaseViewport );

public:

	IViewPortPanel* CreatePanelByName(const char *szPanelName) OVERRIDE;
	void CreateDefaultPanels( void ) OVERRIDE;

	void ApplySchemeSettings( vgui::IScheme *pScheme ) OVERRIDE;
		
	int GetDeathMessageStartHeight( void ) OVERRIDE;
};


#endif // RNL_VIEWPORT_H
