//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef RNL_CLIENTMODE_H
#define RNL_CLIENTMODE_H
#ifdef _WIN32
#pragma once
#endif

#include "clientmode_shared.h"
#include "rnl_viewport.h"
#include "c_rnl_player.h"
#include "GameUI/igameui.h"
#include "rnl_loadingpanel.h"


#define USERID2RNLPLAYER(i) ToRnLPlayer( ClientEntityList().GetEnt( engine->GetPlayerForUserID( i ) ) )	

class ClientModeRnLNormal : public ClientModeShared 
{
DECLARE_CLASS( ClientModeRnLNormal, ClientModeShared );

private:

// IClientMode overrides.
public:

					ClientModeRnLNormal();
	virtual			~ClientModeRnLNormal();

	void	Init() OVERRIDE;
	void	InitViewport() OVERRIDE;
	void	VGui_Shutdown() OVERRIDE;

	bool	ShouldDrawViewModel( void ) OVERRIDE;
	float	GetViewModelFOV( void ) OVERRIDE;

	int GetDeathMessageStartHeight( void );

	void PostRenderVGui() OVERRIDE;

	void FireGameEvent( IGameEvent *event ) OVERRIDE;

	bool CreateMove( float flInputSampleTime, CUserCmd *cmd ) OVERRIDE;

private:
	
	//	void	UpdateSpectatorMode( void );

	IGameUI*			m_pGameUI;
	CRnLLoadingPanel*	m_pLoadingPanel;
};


extern IClientMode *GetClientModeNormal();
extern ClientModeRnLNormal* GetClientModeRnLNormal();


#endif // RNL_CLIENTMODE_H
