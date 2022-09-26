#ifndef RNL_CAMPAIGN_MANAGER_H
#define RNL_CAMPAIGN_MANAGER_H


class CRnLCampaignManager
{
public:
	CRnLCampaignManager();
	~CRnLCampaignManager();

	bool UsingCampaigns();
	bool UsingCampaignManager();
	void SetUsingCampaignManager( bool state );

	bool LoadNewCampaign( const char* pName );

	bool Update( void );
	bool RequiresLevelChange( void );
	
	bool OnLevelInit( void );
	bool OnLevelEnd( void );

	bool OnTeamWin( const char* pTeamName );
	bool OnForceNextSection( const char* pTeamChoice = NULL );

	KeyValues*	GetCampaignFile( void );
	KeyValues*	GetCampaignSection( void );
	const char* GetCampaignSectionName( void );
	const char*	GetCampaignName( void );
	const char*	GetCampaignFileName( void );
	const char* GetNextMapName( void );

private:

	KeyValues*	m_pCampaignFile;
	KeyValues*	m_pCampaignCurrent;
	KeyValues*	m_pPreviousCampaign;
	char		m_szCampaignFileName[256];
	bool		m_bUseCampaignManager;
};

extern CRnLCampaignManager* GetRnLCampaignManager( void );

#endif