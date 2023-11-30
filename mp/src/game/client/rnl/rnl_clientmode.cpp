//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#include "cbase.h"
#include "hud.h"
#include "rnl_clientmode.h"
#include "cdll_client_int.h"
#include "iinput.h"
#include "vgui/isurface.h"
#include "vgui/ipanel.h"
#include <vgui_controls/AnimationController.h>
#include "ivmodemanager.h"
#include "BuyMenu.h"
#include "filesystem.h"
#include "vgui/ivgui.h"
#include "hud_chat.h"
#include "view_shared.h"
#include "view.h"
#include "ivrenderview.h"
#include "model_types.h"
#include "iefx.h"
#include "dlight.h"
#include <imapoverview.h>
#include "c_playerresource.h"
#include <keyvalues.h>
#include "text_message.h"
#include "panelmetaclassmgr.h"
#include "c_weapon__stubs.h"		//Tony; add stubs
#include <vgui/ILocalize.h>
#include "c_rnl_game_team.h"
#include "rnl_squad.h"
#include "c_rnl_player.h"	// cjd @add
#include "hud_macros.h"	// cjd @add
#include "ScreenSpaceEffects.h"

#include "engine/ienginesound.h"
#include "rnl_iobjective.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CDllDemandLoader g_GameUI( "gameui" );

ConVar default_fov( "default_fov", "90", FCVAR_CHEAT );

IClientMode *g_pClientMode = NULL;

//Tony; add stubs for cycler weapon and cubemap.
STUB_WEAPON_CLASS( cycler_weapon,   WeaponCycler,   C_BaseCombatWeapon );
STUB_WEAPON_CLASS( weapon_cubemap,  WeaponCubemap,  C_BaseCombatWeapon );

static void __MsgFunc_BlurVision( bf_read &msg )
{
	// Create a keyvalue block to set these params
	KeyValues *pKeys = new KeyValues( "keys" );
	if ( pKeys == NULL )
		return;

	// Set our keys
	pKeys->SetFloat( "bluriness", msg.ReadFloat() );
	pKeys->SetFloat( "duration", msg.ReadFloat() );
	pKeys->SetFloat( "scale", msg.ReadFloat() );
	pKeys->SetFloat( "speed", msg.ReadFloat() );

	g_pScreenSpaceEffects->SetScreenSpaceEffectParams( "rnl_visionblur", pKeys );
	g_pScreenSpaceEffects->EnableScreenSpaceEffect( "rnl_visionblur" );
}

// --------------------------------------------------------------------------------- //
// CRnLModeManager.
// --------------------------------------------------------------------------------- //

class CRnLModeManager : public IVModeManager
{
public:
	void	Init() OVERRIDE;
	void	SwitchMode( bool commander, bool force ) OVERRIDE {}
	void	LevelInit( const char *newmap ) OVERRIDE;
	void	LevelShutdown( void ) OVERRIDE;
	virtual void	ActivateMouse( bool isactive ) {}
};

static CRnLModeManager g_ModeManager;
IVModeManager *modemanager = ( IVModeManager * )&g_ModeManager;

// --------------------------------------------------------------------------------- //
// CRnLModeManager implementation.
// --------------------------------------------------------------------------------- //

#define SCREEN_FILE		"scripts/vgui_screens.txt"

void CRnLModeManager::Init()
{
	g_pClientMode = GetClientModeNormal();
	
	PanelMetaClassMgr()->LoadMetaClassDefinitionFile( SCREEN_FILE );

	HOOK_MESSAGE( BlurVision );
}

void CRnLModeManager::LevelInit( const char *newmap )
{
	g_pClientMode->LevelInit( newmap );
}

void CRnLModeManager::LevelShutdown( void )
{
	g_pClientMode->LevelShutdown();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeRnLNormal::ClientModeRnLNormal()
{
	m_pLoadingPanel = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: If you don't know what a destructor is by now, you are probably going to get fired
//-----------------------------------------------------------------------------
ClientModeRnLNormal::~ClientModeRnLNormal()
{
}


void ClientModeRnLNormal::Init()
{
	BaseClass::Init();

	ListenForGameEvent( "player_squad" );
	ListenForGameEvent( "rnl_task_state_change" );
	ListenForGameEvent( "rnl_objective_state_change" );
}

void ClientModeRnLNormal::InitViewport()
{
	m_pViewport = new CRnLViewport();
	m_pViewport->Start( gameuifuncs, gameeventmanager );

	CreateInterfaceFn gameUIFactory = g_GameUI.GetFactory();
	if ( gameUIFactory )
	{
		m_pGameUI = (IGameUI *) gameUIFactory(GAMEUI_INTERFACE_VERSION, NULL );
		if ( NULL != m_pGameUI )
		{
			// insert stats summary panel as the loading background dialog
			m_pLoadingPanel = CRnLLoadingPanel::GetInstance();
			m_pGameUI->SetLoadingBackgroundDialog( m_pLoadingPanel->GetVPanel() );
		}		
	}
}

void ClientModeRnLNormal::VGui_Shutdown()
{
	if ( NULL != m_pGameUI )
		m_pGameUI->SetLoadingBackgroundDialog( NULL );
	
	CRnLLoadingPanel::DestroyInstance();
	m_pLoadingPanel = NULL;

	BaseClass::VGui_Shutdown();
}

ClientModeRnLNormal g_ClientModeNormal;

IClientMode *GetClientModeNormal()
{
	return &g_ClientModeNormal;
}


ClientModeRnLNormal* GetClientModeRnLNormal()
{
	Assert( dynamic_cast< ClientModeRnLNormal* >( GetClientModeNormal() ) );

	return static_cast< ClientModeRnLNormal* >( GetClientModeNormal() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ClientModeRnLNormal::ShouldDrawViewModel()
{
	CRnLPlayer *player = CRnLPlayer::GetLocalRnLPlayer();
	if ( player )
		return player->ShouldDrawViewModel();

	return BaseClass::ShouldDrawViewModel();
}

float ClientModeRnLNormal::GetViewModelFOV( void )
{
	return 74.0f;
}

int ClientModeRnLNormal::GetDeathMessageStartHeight( void )
{
	return m_pViewport->GetDeathMessageStartHeight();
}

void ClientModeRnLNormal::PostRenderVGui()
{
}

void ClientModeRnLNormal::FireGameEvent( IGameEvent* event )
{
	const char *eventname = event->GetName();
	/*
	CBaseHudChat *hudChat = (CBaseHudChat *)GET_HUDELEMENT( CHudChat );
	
	if ( Q_stricmp( "player_squad", eventname ) == 0 )
	{
		C_RnLPlayer *pPlayer = USERID2RNLPLAYER( event->GetInt("userid") );
		if ( !hudChat )
			return;
		if ( !pPlayer )
			return;

		bool bDisconnected = event->GetBool("disconnect");

		if ( bDisconnected )
			return;

		int iSquad = event->GetInt( "squad" );
		int iSlot = event->GetInt( "slot" );

		C_RnLGameTeam* pTeam = (C_RnLGameTeam*)(GetGlobalRnLTeam( pPlayer->GetTeamNumber() ));

		if( !pTeam )
			return;

		C_RnLBaseSquad* pSquad = pTeam->GetSquad( iSquad );

		if( !pSquad )
			return;

		const char *pszName = pPlayer->GetPlayerName();

		wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( pszName, wszPlayerName, sizeof(wszPlayerName) );

		//if ( !IsInCommentaryMode() )
		{
			wchar_t wszLocalized[256];
			g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#Game_Join_Squad" ), 3, wszPlayerName, g_pVGuiLocalize->Find(pSquad->m_szSquadTitle), 
				g_pVGuiLocalize->Find(pTeam->GetKitDescription( pSquad->GetKitDescription( iSlot )).title) );

			char szLocalized[256];
			g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

			hudChat->Printf( CHAT_FILTER_TEAMCHANGE, "%s", szLocalized );
		}
	}
	else*/
	if ( Q_stricmp( "rnl_objective_state_change", eventname ) == 0 )
	{
		CLocalPlayerFilter filter;

		int iNewState = event->GetInt("new_state");
		int iCurState = event->GetInt("current_state");
		int iPreState = event->GetInt("previous_state");
		switch( iNewState )
		{
		case RNL_OBJECTIVE_NEUTRAL:
			{
				switch( iCurState )
				{
				case RNL_OBJECTIVE_AXIS_CONTROLLED:
					C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Objective.Lost_GER" );
					break;
				case RNL_OBJECTIVE_ALLIED_CONTROLLED:
					C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Objective.Lost_US" );
					break;
				};
			}
			break;
		
		case RNL_OBJECTIVE_ALLIED_CONTROLLED:
			{
				switch( iCurState )
				{
				case RNL_OBJECTIVE_AXIS_CONTROLLED:
					C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Objective.Lost_GER" );
					break;
				};

				if( iPreState == RNL_OBJECTIVE_ALLIED_CONTROLLED || iCurState == RNL_OBJECTIVE_ALLIED_CONTROLLED )
					C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Objective.Regained_US" );
				else
					C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Objective.Captured_US" );
			}
			break;
		
		case RNL_OBJECTIVE_AXIS_CONTROLLED:
			{
				switch( iCurState )
				{
				case RNL_OBJECTIVE_ALLIED_CONTROLLED:
					C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Objective.Lost_US" );
					break;
				};

				if( iPreState == RNL_OBJECTIVE_AXIS_CONTROLLED || iCurState == RNL_OBJECTIVE_AXIS_CONTROLLED )
					C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Objective.Regained_GER" );
				else
					C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Objective.Captured_GER" );
			}
			break;
		};
	}
	else if ( Q_stricmp( "rnl_task_state_change", eventname ) == 0 )
	{
		CLocalPlayerFilter filter;

		int iNewState = event->GetInt("new_state");
		int iCurState = event->GetInt("current_state");
		//int iPreState = event->GetInt("previous_state");

		if( iNewState == RNL_TASK_NEUTRAL )
		{
			switch( iCurState )
			{
			case RNL_TASK_AXIS_CONTROLLED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Lost_GER" );
				break;
			case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL:
			case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL_BLOCKED:
			case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED:
			case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED_BLOCKED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Blocked_GER" );
				break;
			case RNL_TASK_ALLIED_CONTROLLED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Lost_US" );
				break;
			case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL:
			case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL_BLOCKED:
			case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS:
			case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS_BLOCKED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Blocked_US" );
				break;
			};
		}
		else if( iNewState > RNL_TASK_ALLIED_STATES_MIN && iNewState < RNL_TASK_ALLIED_STATES_MAX )
		{
			switch( iNewState )
			{
			case RNL_TASK_ALLIED_CONTROLLED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Captured_US" );
				break;
			case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL_BLOCKED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Blocked_US" );
				break;
			case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS_BLOCKED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Blocked_US" );
				break;
			};

			switch( iCurState )
			{
			case RNL_TASK_AXIS_CONTROLLED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Lost_GER" );
				break;
			case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL:
			case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL_BLOCKED:
			case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED:
			case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED_BLOCKED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Blocked_GER" );
				break;
			};			
		}
		else if( iNewState > RNL_TASK_AXIS_STATES_MIN && iNewState < RNL_TASK_AXIS_STATES_MAX )
		{
			switch( iNewState )
			{
			case RNL_TASK_AXIS_CONTROLLED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Captured_GER" );
				break;
			case RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL_BLOCKED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Blocked_GER" );
				break;
			case RNL_TASK_AXIS_CAPTURING_FROM_ALLIED_BLOCKED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Blocked_GER" );
				break;
			};

			switch( iCurState )
			{
			case RNL_TASK_ALLIED_CONTROLLED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Lost_US" );
				break;
			case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL:
			case RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL_BLOCKED:
			case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS:
			case RNL_TASK_ALLIED_CAPTURING_FROM_AXIS_BLOCKED:
				C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, "Task.Blocked_US" );
				break;
			};
		}
	}
	else
	{
		BaseClass::FireGameEvent( event );
	}
}

bool ClientModeRnLNormal::CreateMove( float flInputSampleTime, CUserCmd *cmd )
{
	CRnLPlayer *player = CRnLPlayer::GetLocalRnLPlayer();
	if (player)
	{
		cmd->mod_data.SetCount(1024);

		bf_write writeBuffer(cmd->mod_data.Base(), cmd->mod_data.Count());
		writeBuffer.WriteOneBit(1);
		writeBuffer.WriteBitAngles(player->GetWeaponAngle());

		cmd->mod_data.SetCountNonDestructively(writeBuffer.GetNumBytesWritten());
	}
	return BaseClass::CreateMove( flInputSampleTime, cmd );;
}


