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

	virtual void	Init();
	virtual void	InitViewport();
	virtual void	VGui_Shutdown();

	virtual bool	ShouldDrawViewModel( void );
	virtual float	GetViewModelFOV( void );

	int				GetDeathMessageStartHeight( void );

	virtual void	PostRenderVGui();

	virtual void FireGameEvent( IGameEvent *event );

	virtual bool CreateMove( float flInputSampleTime, CUserCmd *cmd );

private:
	
	//	void	UpdateSpectatorMode( void );

	IGameUI*			m_pGameUI;
	CRnLLoadingPanel*	m_pLoadingPanel;
};


extern IClientMode *GetClientModeNormal();
extern ClientModeRnLNormal* GetClientModeRnLNormal();


#endif // RNL_CLIENTMODE_H
