#include "cbase.h"
#include "rnl_campaign_manager.h"
#include "rnl_gamerules.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CRnLCampaignManager g_CampaignManager;
CRnLCampaignManager* GetRnLCampaignManager( void )
{
	return &g_CampaignManager;
}

void CC_RnL_UseCampaign_Change( IConVar *var, const char *pOldValue, float flOldValue );
ConVar rnl_usecampaign( "rnl_usecampaign", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY, "If set RnL will use the campaign system for map cycles", CC_RnL_UseCampaign_Change );
void CC_RnL_UseCampaign_Change( IConVar *var, const char *pOldValue, float flOldValue )
{
	g_CampaignManager.SetUsingCampaignManager(rnl_usecampaign.GetBool());
}

void CC_RnL_Campaign_Change( IConVar *var, const char *pOldValue, float flOldValue );
static ConVar rnl_campaign_file( "rnl_campaign_file", 
			  "default", 
			  FCVAR_GAMEDLL | FCVAR_NOTIFY,
			  "If set to a valid map name, will change to this map during the next changelevel",
			  CC_RnL_Campaign_Change );

void CC_RnL_Campaign_Change( IConVar *var, const char *pOldValue, float flOldValue )
{
	if( Q_strcmp( pOldValue, rnl_campaign_file.GetString() ) == 0 )
		return;

	char campaignFileName[256];
	Q_snprintf( campaignFileName, sizeof( campaignFileName ), "campaigns/%s.res", rnl_campaign_file.GetString() );
	if ( filesystem->FileExists( campaignFileName, "MOD" ) )
	{
		if( !g_CampaignManager.LoadNewCampaign( campaignFileName ) )
		{
			var->SetValue( "default" );
		}
		else
		{
			rnl_usecampaign.SetValue( 1 );
		}
	}
	else
	{
		var->SetValue( pOldValue );
	}
}

CON_COMMAND_F( rnl_campaign_progress, "If set to a valid map name, will change to this map during the next changelevel",  FCVAR_GAMEDLL | FCVAR_NOTIFY )
{
	if( args.ArgC() > 1 )
	{
		g_CampaignManager.OnForceNextSection( args[1] );
		return;
	}

	g_CampaignManager.OnForceNextSection();
}

CRnLCampaignManager::CRnLCampaignManager()
{
	m_bUseCampaignManager = false;
	m_pCampaignFile = NULL;
	m_pCampaignCurrent = NULL;
	m_pPreviousCampaign = NULL;
}

CRnLCampaignManager::~CRnLCampaignManager()
{
	if( m_pCampaignFile )
		m_pCampaignFile->deleteThis();

	m_pCampaignFile = NULL;
	m_pCampaignCurrent = NULL;
	m_pPreviousCampaign = NULL;
}

bool CRnLCampaignManager::UsingCampaigns()
{
	return (m_pCampaignFile != NULL) && UsingCampaignManager();
}

bool CRnLCampaignManager::UsingCampaignManager()
{
	return m_bUseCampaignManager;
}

void CRnLCampaignManager::SetUsingCampaignManager( bool state )
{
	m_bUseCampaignManager = state;

	if( m_bUseCampaignManager && m_pCampaignFile == NULL )
	{
		m_bUseCampaignManager = false;

		char campaignFileName[256];
		Q_snprintf( campaignFileName, sizeof( campaignFileName ), "campaigns/%s.res", rnl_campaign_file.GetString() );
		if ( filesystem->FileExists( campaignFileName, "MOD" ) )
		{
			if( LoadNewCampaign( campaignFileName ) )
			{
				m_bUseCampaignManager = true;
			}
		}
	}
}

bool CRnLCampaignManager::LoadNewCampaign( const char* pName )
{
	if( m_pCampaignFile != NULL )
	{
		m_pCampaignFile->deleteThis();
	}

	Q_strncpy( m_szCampaignFileName, pName, sizeof( m_szCampaignFileName ) );
	
	m_pCampaignFile = new KeyValues( "campaign" );
	if( !m_pCampaignFile->LoadFromFile( filesystem, m_szCampaignFileName ) )
	{
		Q_strncpy( m_szCampaignFileName, "campaigns/default.res", sizeof( m_szCampaignFileName ) );
		m_pCampaignFile->LoadFromFile( filesystem, "campaigns/default.res" );
		m_pCampaignCurrent = m_pCampaignFile->GetFirstSubKey();
		return false;
	}

	m_pCampaignCurrent = m_pCampaignFile->GetFirstSubKey();
	m_pPreviousCampaign = NULL;
	return true;
}

bool CRnLCampaignManager::Update( void )
{
	if( g_fGameOver )
		return true;

	if( m_pCampaignCurrent != m_pPreviousCampaign )
	{
		if( Q_strcmp( m_pCampaignCurrent->GetString( "map" ), STRING(gpGlobals->mapname) ) != 0 && engine->IsMapValid( m_pCampaignCurrent->GetString( "map" ) ) )
		{
			RnLGameRules()->EndMultiplayerGame();
			return true;
		}
	}

	return false;
}

bool CRnLCampaignManager::OnLevelInit( void )
{
	if( m_pCampaignFile == NULL )
	{
		Q_snprintf( m_szCampaignFileName, sizeof( m_szCampaignFileName ), "campaigns/%s.res", rnl_campaign_file.GetString() );
		m_pCampaignFile = new KeyValues( "campaign" );
		
		if( !m_pCampaignFile->LoadFromFile( filesystem, m_szCampaignFileName ) )
		{
			Q_strncpy( m_szCampaignFileName, "campaigns/default.res", sizeof( m_szCampaignFileName ) );
			rnl_campaign_file.SetValue( "default" );
			m_pCampaignFile->LoadFromFile( filesystem, "campaigns/default.res" );
		}
	}
		
	if( m_pCampaignCurrent == NULL )
	{
		m_pCampaignCurrent = m_pCampaignFile->GetFirstSubKey();
		m_pPreviousCampaign = NULL;

	}

	if( Q_strcmp( m_pCampaignCurrent->GetString( "map" ), STRING(gpGlobals->mapname) ) == 0 )
	{
		m_pPreviousCampaign = m_pCampaignCurrent;
		mp_timelimit.SetValue( m_pCampaignCurrent->GetFloat( "timelimit", 30.0f ) );
	}

	return true;
}

bool CRnLCampaignManager::OnLevelEnd( void )
{
	return true;
}

bool CRnLCampaignManager::OnForceNextSection( const char* pTeamChoice )
{
	if( m_pCampaignCurrent && m_pCampaignFile )
	{
		KeyValues*	pWin = m_pCampaignCurrent->FindKey( "win" );
		if( pWin )
		{
			if( pTeamChoice )
			{
				KeyValues* pTeam = pWin->FindKey( pTeamChoice );
				if( pTeam )
				{
					KeyValues* pNew = m_pCampaignFile->FindKey( pTeam->GetString() );
					if( pNew )
					{
						m_pCampaignCurrent = pNew;
						return true;
					}
				}
			}
			else
			{
				KeyValues* pTeam = pWin->FindKey( "allies" );
				if( pTeam )
				{
					KeyValues* pNew = m_pCampaignFile->FindKey( pTeam->GetString() );
					if( pNew )
					{
						m_pCampaignCurrent = pNew;
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool CRnLCampaignManager::OnTeamWin( const char* pTeamName )
{
	if( m_pCampaignCurrent && m_pCampaignFile )
	{
		KeyValues*	pWin = m_pCampaignCurrent->FindKey( "win" );
		if( pWin )
		{
			KeyValues* pTeam = pWin->FindKey( pTeamName );
			if( pTeam )
			{
				KeyValues* pNew = m_pCampaignFile->FindKey( pTeam->GetString() );
				if( pNew )
				{
					m_pCampaignCurrent = pNew;
					return true;
				}
			}
		}
	}
	return false;
}

KeyValues* CRnLCampaignManager::GetCampaignFile( void )
{
	return m_pCampaignFile;
}

KeyValues* CRnLCampaignManager::GetCampaignSection( void )
{
	return m_pCampaignCurrent;
}

const char* CRnLCampaignManager::GetCampaignFileName( void )
{
	return m_szCampaignFileName;
}

const char* CRnLCampaignManager::GetCampaignSectionName( void )
{
	return m_pCampaignCurrent->GetName();
}

const char*	CRnLCampaignManager::GetCampaignName( void )
{
	return rnl_campaign_file.GetString();
}

const char* CRnLCampaignManager::GetNextMapName( void )
{
	return m_pCampaignCurrent->GetString( "map" );
}