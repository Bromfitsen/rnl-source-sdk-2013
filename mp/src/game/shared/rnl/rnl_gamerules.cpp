//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: The TF Game rules 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "rnl_gamerules.h"
#include "ammodef.h"
#include "KeyValues.h"
#include "weapon_rnl_base.h"
#include "rnl_ammodef.h"
#include "rnl_weapon_parse.h"
#include "rnl_shareddefs.h"


#ifdef CLIENT_DLL
	#include "filesystem.h"
	#include "c_rnl_team.h"
#else
	#include "voice_gamemgr.h"
	#include "rnl_team.h"
	#include "filesystem.h"
	#include "rnl_campaign_manager.h"
	#include "rnl_player.h"	// cjd @add
	#include "rnl_player_resource.h"
	#include "gameinterface.h"
	#include "eventqueue.h"
	#include "viewport_panel_names.h"
	#include "player_resource.h"
	#include "rnl_mapentities.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifndef CLIENT_DLL

void VoiceCarry_ChangeCallback( IConVar *pConVar, char const *pOldString, float flOldValue );

ConVar	mp_defaulttickets( "mp_defaulttickets", "200", FCVAR_NOTIFY, "The number of starting tickets a team has." );
ConVar	mp_defaultspawntime( "mp_defaultspawntime", "35", FCVAR_NOTIFY, "The default spawn wave time in seconds." );
ConVar	mp_roundlimit( "mp_roundlimit", "0", FCVAR_NOTIFY|FCVAR_REPLICATED, "The number of rounds to be played per map. 0 to disable." );
ConVar	mp_limitteams( "mp_limitteams", "3", FCVAR_NOTIFY, "The maximum number of players any team can have more than the other team." );
ConVar	mp_voicecarry( "mp_voicecarry", "1024", FCVAR_NOTIFY, "The maximum distance away any player can hear any other player from unless alltalk is on.", VoiceCarry_ChangeCallback );
ConVar	mp_ticketdrain( "mp_ticketdrain", "1", FCVAR_NOTIFY, "Whether the server should automatically subtract tickets from each team over time based on how many objectives are held." );
ConVar	mp_ticketdraininterval( "mp_ticketdraininterval", "60.0", FCVAR_NOTIFY, "The amount of time between each ticket draining.", true, 1.0f, false, 0.0f );
ConVar	mp_ticketdrainmax( "mp_ticketdrainmax", "5", FCVAR_NOTIFY, "The maximum amount of tickets that can be drained." );
ConVar	sv_deadlisten( "sv_deadlisten", "0", FCVAR_NOTIFY, "Whether or not dead players can listen to alive players' voice chat." );

void VoiceCarry_ChangeCallback( IConVar *pConVar, char const *pOldString, float flOldValue )
{
	ConVarRef var( pConVar );
	if( GetVoiceGameMgr() )
		GetVoiceGameMgr()->SetProximityDistance( var.GetFloat() );
}
#endif

//spawn points defined here - nuke
#ifndef CLIENT_DLL
LINK_ENTITY_TO_CLASS(info_player_spectator, CPointEntity);
#endif

REGISTER_GAMERULES_CLASS( CRnLGameRules );


BEGIN_NETWORK_TABLE_NOBASE( CRnLGameRules, DT_RnLGameRules )
#ifndef CLIENT_DLL
	SendPropString( SENDINFO( m_szCurrentCampaignFile ) ),
	SendPropString( SENDINFO( m_szCurrentCampaignSection ) ),
	SendPropEHandle( SENDINFO( m_hGameManager ) ),
	SendPropInt( SENDINFO( m_iRoundState ) ),
	SendPropInt( SENDINFO( m_iWinningTeam ) ),
#else
	RecvPropString( RECVINFO( m_szCurrentCampaignFile ) ),
	RecvPropString( RECVINFO( m_szCurrentCampaignSection ) ),
	RecvPropEHandle( RECVINFO( m_hGameManager ) ),
	RecvPropInt( RECVINFO( m_iRoundState ) ),
	RecvPropInt( RECVINFO( m_iWinningTeam ) ),
#endif
END_NETWORK_TABLE()


LINK_ENTITY_TO_CLASS( rnl_gamerules, CRnLGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( RnLGameRulesProxy, DT_RnLGameRulesProxy )


#ifdef CLIENT_DLL
	void RecvProxy_RnLGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CRnLGameRules *pRules = RnLGameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CRnLGameRulesProxy, DT_RnLGameRulesProxy )
		RecvPropDataTable( "rnl_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_RnLGameRules ), RecvProxy_RnLGameRules )
	END_RECV_TABLE()

	void CRnLGameRulesProxy::OnDataChanged( DataUpdateType_t type )
	{
		BaseClass::OnDataChanged( type );

		if ( type == DATA_UPDATE_CREATED  /*|| type == DATA_UPDATE_DATATABLE_CHANGED*/ )
		{
			RnLGameRules()->InitialiseSharedActivities();
			RnLGameRules()->InitialiseWeapons();
			RnLGameRules()->InitialiseCampaign();
			RnLGameRules()->InitialiseSquads();
		}
	}
#else
	void *SendProxy_RnLGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CRnLGameRules *pRules = RnLGameRules();
		Assert( pRules );
		pRecipients->SetAllRecipients();
		return pRules;
	}

	BEGIN_SEND_TABLE( CRnLGameRulesProxy, DT_RnLGameRulesProxy )
		SendPropDataTable( "rnl_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_RnLGameRules ), SendProxy_RnLGameRules )
	END_SEND_TABLE()
#endif

#ifndef CLIENT_DLL
	ConVar sk_plr_dmg_grenade( "sk_plr_dmg_grenade","0");	
#endif

	// --------------------------------------------------------------------------------------------------- //
	// CRnLGameRules implementation.
	// --------------------------------------------------------------------------------------------------- //

	CRnLGameRules::CRnLGameRules()
	{
		m_pTeamData = NULL;
		m_hGameManager.Set( NULL );

#ifdef CLIENT_DLL	
		m_pCampaignFile = NULL;
		m_pCampaignSection = NULL;
#else
		m_pCurStateInfo = NULL;
		m_flRoundEndTime = -1;
		m_iRoundsPlayed = 0;
		m_flRestartRoundTime = -1;
		m_flRestartGameTime = -1;
#endif
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	CRnLGameRules::~CRnLGameRules()
	{
		if( m_pTeamData )
			m_pTeamData->deleteThis();
#ifndef CLIENT_DLL
		// Note, don't delete each team since they are in the gEntList and will 
		// automatically be deleted from there, instead.
		g_Teams.Purge();
#else
		if( m_pCampaignSection )
			m_pCampaignSection = NULL;
		if( m_pCampaignFile )
			m_pCampaignFile->deleteThis();
#endif
	}

	KeyValues* CRnLGameRules::GetTeamData( int iTeam )
	{
		if( m_pTeamData )
		{
			if( iTeam == TEAM_ALLIES )
				return m_pTeamData->FindKey( "Allies" );
			else if( iTeam == TEAM_AXIS )
				return m_pTeamData->FindKey( "Axis" );
		}

		return NULL;
	}

	int CRnLGameRules::GetTeamCount( int iTeam )
	{
		if( iTeam == TEAM_ALLIES && g_Teams.Element( TEAM_ALLIES ) )
			return g_Teams.Element( TEAM_ALLIES )->GetNumPlayers();
		else if( iTeam == TEAM_AXIS && g_Teams.Element( TEAM_AXIS ) )
			return g_Teams.Element( TEAM_AXIS )->GetNumPlayers();

		return 0;
	}

	void CRnLGameRules::InitialiseSharedActivities(void)
	{
	}

	void CRnLGameRules::InitialiseWeapons( void )
	{
		CScriptedWeaponLinker::Precache();
		PrecacheFileScriptedWeaponInfoDatabase( filesystem, GetEncryptionKey() );
	}

	void CRnLGameRules::InitialiseCampaign( void )
	{
#ifdef CLIENT_DLL
		if( m_pCampaignFile != NULL )
		{
			m_pCampaignFile->deleteThis();
		}

		m_pCampaignFile = new KeyValues( "campaign" );
		if( m_pCampaignFile->LoadFromFile( filesystem, m_szCurrentCampaignFile.Get() ) )
		{
			m_pCampaignSection = m_pCampaignFile->FindKey( m_szCurrentCampaignSection.Get() );
		}
		else
		{
			m_pCampaignFile->deleteThis();
			m_pCampaignFile = NULL;
			m_pCampaignSection = NULL;
		}

		DevMsg( "Client Loaded Campaign: %s\n", m_szCurrentCampaignFile.Get() );
		DevMsg( "Client Loaded Section: %s\n", m_szCurrentCampaignSection.Get() );
#else
		if( GetRnLCampaignManager() && GetRnLCampaignManager()->UsingCampaignManager() )
		{
			GetRnLCampaignManager()->OnLevelInit();

			Q_strncpy( m_szCurrentCampaignFile.GetForModify(), GetRnLCampaignManager()->GetCampaignFileName(), MAX_CAMPAIGN_FILE );
			Q_strncpy( m_szCurrentCampaignSection.GetForModify(), GetRnLCampaignManager()->GetCampaignSectionName(), MAX_CAMPAIGN_SECTION );

			if( m_pTeamData != NULL )
			{
				m_pTeamData->deleteThis();
			}

			// Create the team managers
			char szScripts[MAX_CAMPAIGN_SCRIPT+1];
			Q_snprintf( szScripts, MAX_CAMPAIGN_SCRIPT, "scripts/%s.res", GetRnLCampaignManager()->GetCampaignSection()->GetString( "script", "rnl_default_squads" ) );
				
			m_pTeamData = new KeyValues( "SquadInfo" );
			if( !m_pTeamData->LoadFromFile( filesystem, szScripts ) )
				m_pTeamData->LoadFromFile( filesystem, "scripts/rnl_default_squads.res" );
		}
		else
		{
			DevMsg( "Removing Old Campaign Data\n" );
			//Q_strncpy( m_szCurrentCampaignFile.GetForModify(), "", MAX_CAMPAIGN_FILE );
			//Q_strncpy( m_szCurrentCampaignSection.GetForModify(), "", MAX_CAMPAIGN_SECTION );

			DevMsg( "Preparing Team Data\n" );
			if( m_pTeamData != NULL )
			{
				m_pTeamData->deleteThis();
			}

			// Create the team managers
			char szScripts[MAX_CAMPAIGN_SCRIPT+1];
			Q_snprintf( szScripts, MAX_CAMPAIGN_SCRIPT, "scripts/%s_squads.res", gpGlobals->mapname );

			DevMsg( "Server loading squad script: %s\n", szScripts );

			m_pTeamData = new KeyValues( "SquadInfo" );
			if( !m_pTeamData->LoadFromFile( filesystem, szScripts ) )
				m_pTeamData->LoadFromFile( filesystem, "scripts/rnl_default_squads.res" );
		}
#endif
	}

	void CRnLGameRules::InitialiseSquads( void )
	{
#ifdef CLIENT_DLL
		// Create the team managers
		char szScripts[MAX_CAMPAIGN_SCRIPT+1];
		if( m_pCampaignSection )
			Q_snprintf( szScripts, MAX_CAMPAIGN_SCRIPT, "scripts/%s.res", m_pCampaignSection->GetString( "script", "rnl_default_squads" ) );
		else
		{
			char mapName[64];
			Q_FileBase( engine->GetLevelName(), mapName, sizeof(mapName) );
			Q_snprintf( szScripts, MAX_CAMPAIGN_SCRIPT, "scripts/%s_squads.res", mapName );
		}

		DevMsg( "Client Loading Script file: %s\n", szScripts );
		
		if( m_pTeamData != NULL )
		{
			m_pTeamData->deleteThis();
		}

		m_pTeamData = new KeyValues( "SquadInfo" );
		if( !m_pTeamData->LoadFromFile( g_pFullFileSystem, szScripts ) )
			m_pTeamData->LoadFromFile( g_pFullFileSystem, "scripts/rnl_default_squads.res" );
			
		KeyValues* pVal = GetTeamData( TEAM_AXIS );
		if( pVal )
			GetGlobalRnLTeam( TEAM_AXIS )->LoadClassDescriptions( pVal->FindKey( "kits" )  );

		pVal  = GetTeamData( TEAM_ALLIES );
		if( pVal )
			GetGlobalRnLTeam( TEAM_ALLIES )->LoadClassDescriptions( pVal->FindKey( "kits" )  );
#endif
	}

#ifdef CLIENT_DLL


#else
	// --------------------------------------------------------------------------------------------------- //
	// Voice helper
	// --------------------------------------------------------------------------------------------------- //

	class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
	{
	public:
		virtual bool CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity )
		{
			// Dead players can only be heard by other dead team mates
			if ( pTalker->IsAlive() == false )
			{
				if ( pListener->IsAlive() == false )
				{
					bProximity = false;
					return ( pListener->InSameTeam( pTalker ) );
				}

				return false;
			}

			// Dead players can only hear alive players if sv_deadlisten is set to 1
			if ( pListener->IsAlive() == false )
			{
				if ( pTalker->IsAlive() && !sv_deadlisten.GetBool() )
				{
					return false;
				}
			}

			// The listener can only hear the talker within mp_voicecarry units
			if ( pListener->GetAbsOrigin().DistTo(pTalker->GetAbsOrigin()) <= mp_voicecarry.GetFloat() )
			{
				bProximity = true;
				return true;
			}

			return false;
		}
	};
	CVoiceGameMgrHelper g_VoiceGameMgrHelper;
	IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;

	// --------------------------------------------------------------------------------------------------- //
	// Global helper functions.
	// --------------------------------------------------------------------------------------------------- //
	
	// World.cpp calls this but we don't use it in SDK.
	void InitBodyQue()
	{
	}

	bool CRnLGameRules::Init()
	{
		DevMsg( "Initialising Scripted Weapons\n" );
		InitialiseWeapons();
		
		DevMsg( "Initialising Campaign\n" );
		InitialiseCampaign();

		CBaseEntity::Create( "rnl_gamerules", vec3_origin, vec3_angle );

		DevMsg( "Creating Teams\n" );

		CRnLTeam *pTeam = static_cast<CRnLTeam*>(CreateEntityByName( "rnl_game_team" ));
		DevMsg( "Creating team with squad info: '%s'\n", TeamNumberToName(TEAM_UNASSIGNED) );
		pTeam->Init( TeamNumberToName(TEAM_UNASSIGNED), TEAM_UNASSIGNED, NULL );
		g_Teams.AddToTail( pTeam );

		pTeam = static_cast<CRnLTeam*>(CreateEntityByName( "rnl_game_team" ));
		DevMsg( "Creating team with squad info: '%s'\n", TeamNumberToName(TEAM_SPECTATOR) );
		pTeam->Init( TeamNumberToName(TEAM_SPECTATOR), TEAM_SPECTATOR, NULL );
		g_Teams.AddToTail( pTeam );

		pTeam = static_cast<CRnLTeam*>(CreateEntityByName( "rnl_game_team" ));
		DevMsg( "Creating team with squad info: '%s'\n", TeamNumberToName(TEAM_ALLIES) );
		pTeam->Init( TeamNumberToName(TEAM_ALLIES), TEAM_ALLIES, m_pTeamData->FindKey( TeamNumberToName(TEAM_ALLIES) ) );
		g_Teams.AddToTail( pTeam );

		pTeam = static_cast<CRnLTeam*>(CreateEntityByName( "rnl_game_team" ));
		DevMsg( "Creating team with squad info: '%s'\n", TeamNumberToName(TEAM_AXIS) );
		pTeam->Init( TeamNumberToName(TEAM_AXIS), TEAM_AXIS, m_pTeamData->FindKey( TeamNumberToName(TEAM_AXIS) ) );
		g_Teams.AddToTail( pTeam );

		State_Transition( RNL_GR_STATE_INIT );

		return BaseClass::Init();
	}

	void CRnLGameRules::InitDefaultAIRelationships(void)
	{
		BaseClass::InitDefaultAIRelationships();
	}

	void CRnLGameRules::GetNextLevelName( char *szNextMap, int bufsize )
	{
		if( GetRnLCampaignManager() && GetRnLCampaignManager()->UsingCampaigns() )
		{
			Q_strncpy( szNextMap, GetRnLCampaignManager()->GetNextMapName(), bufsize );
			return;
		}

		BaseClass::GetNextLevelName( szNextMap, bufsize );
	}

	//-----------------------------------------------------------------------------
	// Purpose: TF2 Specific Client Commands
	// Input  :
	// Output :
	//-----------------------------------------------------------------------------
	bool CRnLGameRules::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
	{
		if( pEdict && pEdict->IsPlayer () )
		{
			CBasePlayer *pPlayer = ToBasePlayer( pEdict );
			if( pPlayer && pPlayer->ClientCommand( args ) )
				return true;
		}
		return BaseClass::ClientCommand( pEdict, args );
	}

	void CRnLGameRules::ClientSettingsChanged( CBasePlayer *pPlayer )
	{
		CRnLPlayer* pRnLPlayer = ToRnLPlayer( pPlayer );
		if( pRnLPlayer )
		{
			pRnLPlayer->m_RnLLocal.m_bUseAutobolt = Q_atoi( engine->GetClientConVarValue( pRnLPlayer->entindex(), "cl_autobolt" ) ) != 0;
		}

		BaseClass::ClientSettingsChanged( pPlayer );
	}

	//-----------------------------------------------------------------------------
	// Purpose: Player has just spawned. Equip them.
	//-----------------------------------------------------------------------------

	void CRnLGameRules::RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore )
	{
		RadiusDamage( info, vecSrcIn, flRadius, iClassIgnore, false );
	}

	// Add the ability to ignore the world trace
	void CRnLGameRules::RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, bool bIgnoreWorld )
	{
		CBaseEntity *pEntity = NULL;
		trace_t		tr;
		float		flAdjustedDamage, falloff;
		Vector		vecSpot;
		Vector		vecToTarget;
		Vector		vecEndPos;

		Vector vecSrc = vecSrcIn;

		if ( flRadius )
			falloff = info.GetDamage() / flRadius;
		else
			falloff = 1.0;

		int bInWater = (UTIL_PointContents ( vecSrc ) & MASK_WATER) ? true : false;
		
		vecSrc.z += 1;// in case grenade is lying on the ground

		// iterate on all entities in the vicinity.
		for ( CEntitySphereQuery sphere( vecSrc, flRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if ( pEntity->m_takedamage != DAMAGE_NO )
			{
				// UNDONE: this should check a damage mask, not an ignore
				if ( iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore )
				{// houndeyes don't hurt other houndeyes with their attack
					continue;
				}

				// blast's don't tavel into or out of water
				if (bInWater && pEntity->GetWaterLevel() == 0)
					continue;
				if (!bInWater && pEntity->GetWaterLevel() == 3)
					continue;

				// radius damage can only be blocked by the world
				vecSpot = pEntity->BodyTarget( vecSrc );



				bool bHit = false;

				if( bIgnoreWorld )
				{
					vecEndPos = vecSpot;
					bHit = true;
				}
				else
				{
					UTIL_TraceLine( vecSrc, vecSpot, MASK_SOLID_BRUSHONLY, info.GetInflictor(), COLLISION_GROUP_NONE, &tr );

					if (tr.startsolid)
					{
						// if we're stuck inside them, fixup the position and distance
						tr.endpos = vecSrc;
						tr.fraction = 0.0;
					}

					vecEndPos = tr.endpos;

					if( tr.fraction == 1.0 || tr.m_pEnt == pEntity )
					{
						bHit = true;
					}
				}

				if ( bHit )
				{
					// the explosion can 'see' this entity, so hurt them!
					//vecToTarget = ( vecSrc - vecEndPos );
					vecToTarget = ( vecEndPos - vecSrc );

					// decrease damage for an ent that's farther from the bomb.
					flAdjustedDamage = vecToTarget.Length() * falloff;
					flAdjustedDamage = info.GetDamage() - flAdjustedDamage;
				
					if ( flAdjustedDamage > 0 )
					{
						CTakeDamageInfo adjustedInfo = info;
						adjustedInfo.SetDamage( flAdjustedDamage );

						Vector dir = vecToTarget;
						VectorNormalize( dir );

						// If we don't have a damage force, manufacture one
						if ( adjustedInfo.GetDamagePosition() == vec3_origin || adjustedInfo.GetDamageForce() == vec3_origin )
						{
							CalculateExplosiveDamageForce( &adjustedInfo, dir, vecSrc, 1.5	/* explosion scale! */ );
						}
						else
						{
							// Assume the force passed in is the maximum force. Decay it based on falloff.
							float flForce = adjustedInfo.GetDamageForce().Length() * falloff;
							adjustedInfo.SetDamageForce( dir * flForce );
							adjustedInfo.SetDamagePosition( vecSrc );
						}

						pEntity->TakeDamage( adjustedInfo );
			
						// Now hit all triggers along the way that respond to damage... 
						pEntity->TraceAttackToTriggers( adjustedInfo, vecSrc, vecEndPos, dir );
					}
				}
			}
		}
	}

	bool CRnLGameRules::FPlayerCanRespawn( CBasePlayer *pPlayer )
	{
		if( pPlayer->GetTeamNumber() == TEAM_AXIS || pPlayer->GetTeamNumber() == TEAM_ALLIES )
			return true;

		return BaseClass::FPlayerCanRespawn( pPlayer );
	}

	CBaseEntity *CRnLGameRules::GetPlayerSpawnSpot( CBasePlayer *pPlayer )
	{
		if( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
			return BaseClass::GetPlayerSpawnSpot( pPlayer );

		return NULL;
	}

	void CRnLGameRules::Think()
	{
		if( GetRnLCampaignManager() && GetRnLCampaignManager()->UsingCampaigns() )
		{
			GetRnLCampaignManager()->Update();
		}

		if ( !g_fGameOver )
		{
			State_Think();
		}

		BaseClass::Think();
	}

#endif

static CRnLViewVectors g_RnLDefaultViewVectors(
	Vector(0, 0, 64),		//VEC_VIEW (m_vView)
	Vector(-16, -16, 0),	//VEC_HULL_MIN (m_vHullMin)
	Vector(16, 16, 72),		//VEC_HULL_MAX (m_vHullMax)

	Vector(0, 0, 28),		//VEC_DUCK_VIEW		(m_vDuckView)
	Vector(0, 0, 36),		//VEC_DUCK_WALK_VIEW(m_vDuckWalkView)
	Vector(-16, -16, 0),	//VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector(16, 16, 36),		//VEC_DUCK_HULL_MAX	(m_vDuckHullMax)

	Vector(0, 0, 18),		//VEC_PRONE_VIEW	 (m_vProneView)
	Vector(-32, -32, 0),	//VEC_PRONE_HULL_MIN (m_vProneHullMin)
	Vector(32, 32, 24),		//VEC_PRONE_HULL_MAX (m_vProneHullMax)

	Vector(0, 0, 14),		//VEC_DEAD_VIEWHEIGHT (m_vObsViewHeight)
	Vector(-10, -10, -10),	//VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector(10, 10, 10)		//VEC_OBS_HULL_MAX	(m_vObsHullMax)
);

const CViewVectors* CRnLGameRules::GetViewVectors() const
{
	return &g_RnLDefaultViewVectors;
}

bool CRnLGameRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		::V_swap(collisionGroup0,collisionGroup1);
	}
	
	//Don't stand on COLLISION_GROUP_WEAPON
	if( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		return false;
	}
	
	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 
}


//-----------------------------------------------------------------------------
// Purpose: Init CS ammo definitions
//-----------------------------------------------------------------------------

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			1	

// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)

CAmmoDef* GetAmmoDef()
{
	static CRnLAmmoDef def;
	static bool bInitted = false;

	if ( !bInitted )
	{
		bInitted = true;

		// Ammo def:		name			dmg type		tracer type				plrdmg  npcdmg	carry	physimp							flags						clipsize
		// grenades
		def.AddAmmoType(	AMMO_MK2GREN,	DMG_BLAST,		TRACER_WHIZ_ONLY, 		0, 		0,		2,		1, 								AMMO_RNL_GRENADE_AMMO,		1 );
		def.AddAmmoType(	AMMO_STIEL24,	DMG_BLAST,		TRACER_WHIZ_ONLY, 		0, 		0, 		2,		1, 								AMMO_RNL_GRENADE_AMMO,		1 );
		// smoke grenades
		def.AddAmmoType(	AMMO_M18GREN,	DMG_GENERIC,	TRACER_WHIZ_ONLY, 		0, 		0, 		2,		1, 								AMMO_RNL_GRENADE_AMMO,		1 );
		def.AddAmmoType(	AMMO_STIEL39,	DMG_GENERIC,	TRACER_WHIZ_ONLY, 		0, 		0, 		2,		1, 								AMMO_RNL_GRENADE_AMMO,		1 );
		// garand
		def.AddAmmoType(	AMMO_GARAND,	DMG_BULLET,		TRACER_WHIZ_ONLY, 		0, 		0, 		56,		BULLET_IMPULSE(400, 1200),		AMMO_RNL_PRIMARY_AMMO,		8 );
		// k98k
		def.AddAmmoType(	AMMO_K98K,		DMG_BULLET,		TRACER_WHIZ_ONLY, 		0, 		0, 		45,		BULLET_IMPULSE(400, 1200), 		AMMO_RNL_PRIMARY_AMMO,		5 );
		// thompson	
		def.AddAmmoType(	AMMO_THOMPSON,	DMG_BULLET,		TRACER_WHIZ_ONLY, 		0, 		0, 		270,	BULLET_IMPULSE(200, 1225), 		AMMO_RNL_PRIMARY_AMMO,		30 );
		// mp40
		def.AddAmmoType(	AMMO_MP40,		DMG_BULLET,		TRACER_WHIZ_ONLY, 		0, 		0, 		288,	BULLET_IMPULSE(200, 1225), 		AMMO_RNL_PRIMARY_AMMO,		32 );
		// MG42
		def.AddAmmoType(	AMMO_MG42,		DMG_BULLET,		TRACER_LINE_AND_WHIZ, 	0, 		0, 		250,	BULLET_IMPULSE(200, 1225), 		AMMO_RNL_PRIMARY_AMMO,		250 );
		// Browning
		def.AddAmmoType(	AMMO_BROWNING,	DMG_BULLET,		TRACER_LINE_AND_WHIZ, 	0, 		0, 		150,	BULLET_IMPULSE(200, 1225), 		AMMO_RNL_PRIMARY_AMMO,		150 );
		// BAR
		def.AddAmmoType(	AMMO_BAR,		DMG_BULLET,		TRACER_WHIZ_ONLY,	 	0, 		0, 		180,	BULLET_IMPULSE(200, 1225), 		AMMO_RNL_PRIMARY_AMMO,		20 );
		// carbine
		def.AddAmmoType(	AMMO_CARBINE,	DMG_BULLET,		TRACER_WHIZ_ONLY, 		0, 		0, 		90,		BULLET_IMPULSE(400, 1200),		AMMO_RNL_PRIMARY_AMMO,		15 );
		// g43
		def.AddAmmoType(	AMMO_G43,		DMG_BULLET,		TRACER_WHIZ_ONLY, 		0, 		0, 		80,		BULLET_IMPULSE(400, 1200), 		AMMO_RNL_PRIMARY_AMMO,		10 );
		// colt
		def.AddAmmoType(	AMMO_COLT,		DMG_BULLET,		TRACER_WHIZ_ONLY, 		0, 		0, 		14, 	BULLET_IMPULSE(200, 1225), 		AMMO_RNL_SECONDARY_AMMO,	7 );
		// walther
		def.AddAmmoType(	AMMO_WALTHER,	DMG_BULLET,		TRACER_WHIZ_ONLY, 		0, 		0, 		16, 	BULLET_IMPULSE(200, 1225), 		AMMO_RNL_SECONDARY_AMMO,	8 );
		// Tony; added for the sdk_jeep
		def.AddAmmoType(	"JeepAmmo",		DMG_SHOCK,		TRACER_NONE,			"sdk_jeep_weapon_damage",		"sdk_jeep_weapon_damage", "sdk_jeep_max_rounds", BULLET_IMPULSE(650, 8000), 0 );
		
		def.SetMagazineSize(AMMO_MK2GREN, 1);
		def.SetMagazineSize(AMMO_STIEL24, 1);
		def.SetMagazineSize(AMMO_M18GREN, 1);
		def.SetMagazineSize(AMMO_STIEL39, 1);
		def.SetMagazineSize(AMMO_GARAND, 8);
		def.SetMagazineSize(AMMO_K98K, 5);
		def.SetMagazineSize(AMMO_THOMPSON, 30);
		def.SetMagazineSize(AMMO_MP40, 32);
		def.SetMagazineSize(AMMO_MG42, 250);
		def.SetMagazineSize(AMMO_BROWNING, 150);
		def.SetMagazineSize(AMMO_BAR, 20);
		def.SetMagazineSize(AMMO_CARBINE, 15);
		def.SetMagazineSize(AMMO_G43, 10);
		def.SetMagazineSize(AMMO_COLT, 7);
		def.SetMagazineSize(AMMO_WALTHER, 8);

		def.AddBallisticInfo( AMMO_GARAND,		33600.0,	18000.0	);
		def.AddBallisticInfo( AMMO_K98K,		31500.0,	18000.0	);
		def.AddBallisticInfo( AMMO_THOMPSON,	10200.0,	4320.0	);
		def.AddBallisticInfo( AMMO_MP40,		14964.0,	4320.0	);
		def.AddBallisticInfo( AMMO_MG42,		29700.0,	36000.0	);
		def.AddBallisticInfo( AMMO_BROWNING,	30840.0,	36000.0	);
		def.AddBallisticInfo( AMMO_BAR,			30840.0,	36000.0	);
		def.AddBallisticInfo( AMMO_CARBINE,		24000.0,	7200.0	);
		def.AddBallisticInfo( AMMO_G43,			31500.0,	18000.0	);
		def.AddBallisticInfo( AMMO_COLT,		9600.0,		2700.0	);
		def.AddBallisticInfo( AMMO_WALTHER,		14376.0,	2700.0	);

		// Bullet Penetration:	AmmoID			Wood	Metal	Concrete	Flesh	Dirt	Glass	Foliage
		def.AddPenetrationType( AMMO_GARAND,	10,		2,		15,			73,		55,		55,		240 );
		def.AddPenetrationType( AMMO_K98K,		10,		2,		15,			80,		80,		80,		240 );
		def.AddPenetrationType( AMMO_THOMPSON,	6,		0,		6,			29,		22,		22,		240 );
		def.AddPenetrationType( AMMO_MP40,		6,		0,		6,			32,		32,		32,		240 );
		def.AddPenetrationType( AMMO_MG42,		10,		2,		15,			80,		71,		71,		240 );
		def.AddPenetrationType( AMMO_BROWNING,	10,		2,		15,			80,		71,		71,		240 );
		def.AddPenetrationType( AMMO_BAR,		10,		2,		15,			67,		46,		46,		240 );
		def.AddPenetrationType( AMMO_CARBINE,	6,		0,		6,			29,		21,		21,		240	);
		def.AddPenetrationType( AMMO_G43,		10,		2,		15,			73,		55,		55,		240	);
		def.AddPenetrationType( AMMO_COLT,		3,		0,		2,			6,		6,		6,		240 );
		def.AddPenetrationType( AMMO_WALTHER,	3,		0,		2,			6,		6,		6,		240 );
		/*
		def.AddPenetrationType( WEAPON_MG34,		10,		2,		15,			80,		71,		71		);
		def.AddPenetrationType( WEAPON_SPRINGFIELD,	10,		2,		15,			73,		55,		55		);
		def.AddPenetrationType( WEAPON_STG44,		8,		2,		8,			40,		27,		27		);
		def.AddPenetrationType( WEAPON_GREASEGUN,	6,		0,		6,			29,		22,		22		);
		def.AddPenetrationType( WEAPON_FG42,		10,		2,		15,			67,		46,		46		);
		*/

		def.AddDeflectionType( AMMO_GARAND,			true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0,		false, 0.0 	);
		def.AddDeflectionType( AMMO_K98K,			true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0,		false, 0.0 	);
		def.AddDeflectionType( AMMO_THOMPSON,		true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0,		false, 0.0 	);
		def.AddDeflectionType( AMMO_MP40,			true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0,		false, 0.0 	);
		def.AddDeflectionType( AMMO_MG42,			true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0,		false, 0.0 	);
		def.AddDeflectionType( WEAPON_BROWNING,		true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0,		false, 0.0 	);
		def.AddDeflectionType( AMMO_BAR,			true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0,		false, 0.0 	);
		def.AddDeflectionType( AMMO_CARBINE,		true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0,		false, 0.0 	);
		def.AddDeflectionType( AMMO_G43,			true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0,		false, 0.0 	);
		def.AddDeflectionType( AMMO_COLT,			true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0,		false, 0.0 	);
		def.AddDeflectionType( AMMO_WALTHER,		true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0,		false, 0.0 	);
		
		/*
		def.AddDeflectionType( WEAPON_MG34,			true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0	);
		def.AddDeflectionType( WEAPON_SPRINGFIELD,	true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0	);
		def.AddDeflectionType( WEAPON_STG44,		true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0	);
		def.AddDeflectionType( WEAPON_GREASEGUN,	true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0	);
		def.AddDeflectionType( WEAPON_FG42,			true, 0.20,		true, 0.35,		true, 0.35,		false, 0.0,		false, 0.0,		false, 0.0	);
		*/
	}

	return &def;
}

CRnLAmmoDef* GetRnLAmmoDef()
{
	return (CRnLAmmoDef*)(GetAmmoDef());
}

#ifndef CLIENT_DLL

const char *CRnLGameRules::GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer )
{
	if( !pPlayer )
		return "";

	if( bTeamOnly )
	{
		if( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
			return "(Spectator)";
		else if( pPlayer->IsAlive() )
			return "(TEAM)";
		else
			return "*DEAD*(TEAM)";
	}
	else
	{
		if( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
			return "*SPEC*";
		else if( pPlayer->IsAlive() )
			return "";
		else
			return "*DEAD*";
	}
}

#endif

//-----------------------------------------------------------------------------
// Purpose: Find the relationship between players (teamplay vs. deathmatch)
//-----------------------------------------------------------------------------
int CRnLGameRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
#ifndef CLIENT_DLL
	// half life multiplay has a simple concept of Player Relationships.
	// you are either on another player's team, or you are not.
	if ( !pPlayer || !pTarget || !pTarget->IsPlayer() || IsTeamplay() == false )
		return GR_NOTTEAMMATE;

	if ( (*GetTeamID(pPlayer) != '\0') && (*GetTeamID(pTarget) != '\0') && !stricmp( GetTeamID(pPlayer), GetTeamID(pTarget) ) )
		return GR_TEAMMATE;

#endif

	return GR_NOTTEAMMATE;
}

bool CRnLGameRules::IsConnectedUserInfoChangeAllowed(CBasePlayer* pPlayer)
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Implementation of Objective Listener interface
//-----------------------------------------------------------------------------
void CRnLGameRules::RegisterObjective( IRnLObjective* pObjective )
{
	if( m_vObjectives.Find( pObjective ) < 0 )
	{
		DevMsg( "Objective added\n" );
		m_vObjectives.AddToTail( pObjective );
	}
}

void CRnLGameRules::RemoveObjective( IRnLObjective* pObjective )
{
	DevMsg( "Objective removed\n" );
	m_vObjectives.FindAndRemove( pObjective );
}

IRnLObjective* CRnLGameRules::GetObjective( IRnLObjective* pCurrent )
{
	if( m_vObjectives.Count() <= 0 )
		return NULL;

	if( pCurrent == NULL )
	{
		return m_vObjectives[0];
	}
	else
	{
		int pos = m_vObjectives.Find( pCurrent );

		if( m_vObjectives.Count() <= pos )
		{
			return NULL;
		}
		else
		{
			return m_vObjectives[pos+1];
		}
	}
}

IRnLObjective* CRnLGameRules::GetObjective( const char* pszName )
{
	for( int i = 0; i < m_vObjectives.Count(); i++ )
	{
		if( Q_stricmp( m_vObjectives[i]->GetObjectiveName(), pszName ) == 0 )
			return m_vObjectives[i];
	}

	return NULL;
}

void CRnLGameRules::ObjectiveStateChange( IRnLObjective* pObjective )
{
	DevMsg( "Objective state changed to %s\n", ObjectiveStateToName(pObjective->GetObjectiveState()) );
}

CRnLGameManager* CRnLGameRules::GetGameManager( void )
{
	return m_hGameManager.Get();
}

//Game manager bits
#ifndef CLIENT_DLL
	extern ConVar mp_alliedtickets;
	extern ConVar mp_axistickets;

	void CRnLGameRules::CreateStandardEntities()
	{
		g_pPlayerResource = (CRnLPlayerResource*)CBaseEntity::Create("rnl_player_manager", vec3_origin, vec3_angle);
		g_pPlayerResource->AddEFlags(EFL_KEEP_ON_RECREATE_ENTITIES);
	}

	void CRnLGameRules::InitialiseGameManager( void )
	{
		CBaseEntity* pEnt = gEntList.FindEntityByClassname( NULL, "rnl_game_manager" );
		if( pEnt )
		{
			CRnLGameManager* pManager = (CRnLGameManager*)pEnt;
			if( pManager )
			{
				if( mp_alliedtickets.GetInt() >= 0 )
				{
					if( mp_alliedtickets.GetInt() > 0 )
						pManager->SetAlliedTicketsRemaining( mp_alliedtickets.GetInt() );
					else
						pManager->SetAlliedTicketsRemaining( 1 );
				}
				if( mp_axistickets.GetInt() >= 0 )
				{
					if( mp_axistickets.GetInt() > 0 )
						pManager->SetAxisTicketsRemaining( mp_axistickets.GetInt() );
					else
						pManager->SetAxisTicketsRemaining( 1 );
				}

				m_hGameManager = pManager;
				return;
			}
		}

		pEnt = CreateEntityByName( "rnl_game_manager" );
		if( pEnt )
		{
			CRnLGameManager* pManager = (CRnLGameManager*)pEnt;
			if( pManager )
			{
				DispatchSpawn( pManager );
				pManager->SetSpawnTimer( TEAM_AXIS, mp_defaultspawntime.GetFloat() );
				pManager->SetSpawnTimer( TEAM_ALLIES, mp_defaultspawntime.GetFloat() );
				pManager->SetTicketsRemaining( TEAM_AXIS, mp_defaulttickets.GetInt() );
				pManager->SetTicketsRemaining( TEAM_ALLIES, mp_defaulttickets.GetInt() );
				m_hGameManager = pManager;
				return;
			}
		}
	}

	void CRnLGameRules::CheckRespawnWaves( void )
	{
		if( GetGameManager() )
			GetGameManager()->Update();

		for( int i = 0; i < TEAMS_COUNT; i++ )
		{
			if( GetGlobalRnLTeam( i ) )
				GetGlobalRnLTeam( i )->Update();
		}
	}

	static float flGameWinTime = -1.0f;
	static int iWinningTeam = TEAM_INVALID;

	void CRnLGameRules::CheckObjectives( void )
	{
		int iAxisObjs = 0;
		int iAlliesObjs = 0;

		int objectiveCount = 0;

		for( int i = 0; i < m_vObjectives.Count(); i++ )
		{
			if( m_vObjectives[i]->IsPrimary() )
			{
				objectiveCount++;

				switch( m_vObjectives[i]->GetObjectiveState() )
				{
				case RNL_OBJECTIVE_ALLIED_CONTROLLED:
					iAlliesObjs++;
					break;
				case RNL_OBJECTIVE_AXIS_CONTROLLED:
					iAxisObjs++;
					break;
				};
			}
		}

		if( objectiveCount > 0 )
		{
			if( GetRnLCampaignManager() && GetRnLCampaignManager()->UsingCampaigns() )
			{
				if( iAxisObjs == objectiveCount )
				{
					if( GetRnLCampaignManager()->OnTeamWin( "axis" ) )
					{
						RoundEnd( TEAM_AXIS, true );
						return;
					}
				}
				else if( iAlliesObjs == objectiveCount )
				{
					if( GetRnLCampaignManager()->OnTeamWin( "allies" ) )
					{
						RoundEnd( TEAM_ALLIES, true );
						return;
					}
				}
			}
			else
			{
				if( iAxisObjs == objectiveCount )
				{
					if( flGameWinTime < 0.0f && GetGameManager() && GetGameManager()->GetHoldToWinTime() > 0.0 )
					{
						flGameWinTime = gpGlobals->curtime + GetGameManager()->GetHoldToWinTime();
						iWinningTeam = TEAM_AXIS;
					}

					if( flGameWinTime < gpGlobals->curtime )
					{
						flGameWinTime = -1.0f;
						iWinningTeam = TEAM_INVALID;
						RoundEnd( TEAM_AXIS );
					}
					else
					{
						char szTime[16];
						Q_snprintf( szTime, sizeof( szTime), "%.1f", flGameWinTime - gpGlobals->curtime );
						UTIL_CenterPrintAll( "#RnL_HoldToWin", TeamNumberToName( iWinningTeam ), szTime );
					}
				}
				else if( iAlliesObjs == objectiveCount )
				{
					if( flGameWinTime < 0.0f && GetGameManager() && GetGameManager()->GetHoldToWinTime() > 0.0 )
					{
						flGameWinTime = gpGlobals->curtime + GetGameManager()->GetHoldToWinTime();
						iWinningTeam = TEAM_ALLIES;
					}

					if( flGameWinTime < gpGlobals->curtime )
					{
						flGameWinTime = -1.0f;
						iWinningTeam = TEAM_INVALID;
						RoundEnd( TEAM_ALLIES );
					}
					else
					{
						char szTime[16];
						Q_snprintf( szTime, sizeof( szTime), "%.1f", flGameWinTime - gpGlobals->curtime );
						UTIL_CenterPrintAll( "#RnL_HoldToWin", TeamNumberToName( iWinningTeam ), szTime );
					}
				}
				else if( flGameWinTime >= 0.0f )
				{
					UTIL_CenterPrintAll( "#RnL_WinBlock", TeamNumberToName( iWinningTeam ) );
					flGameWinTime = -1.0f;
					iWinningTeam = TEAM_INVALID;
				}
			}
		}
	}

	static float flNextDrainTime = -1.0f;

	void CRnLGameRules::CheckTickets( void )
	{
		CRnLGameManager *pGameManager = GetGameManager();
		if( !pGameManager )
			return;

		int iAlliedTickets = pGameManager->GetAlliedTicketsRemaining();
		int iAxisTickets = pGameManager->GetAxisTicketsRemaining();

		if( iAlliedTickets <= 0 || iAxisTickets <= 0 )
		{
			int totalPlayers[2];
			totalPlayers[0] = g_Teams.Element( TEAM_ALLIES )->GetNumPlayers();
			totalPlayers[1] = g_Teams.Element( TEAM_AXIS )->GetNumPlayers();

			if( totalPlayers[0] > 0 && totalPlayers[1] > 0 )
			{
				int playersAlive[2] = { 0, 0 };

				for( int i = 0; i < 2; i++ )
				{
					for( int j = 0; j < totalPlayers[i]; j++ )
					{
						CBasePlayer* pPlayer = g_Teams.Element( (TEAM_ALLIES + i) )->GetPlayer( j );
						if( pPlayer && pPlayer->IsAlive() )
							playersAlive[i]++;
					}
				}

				if( m_flRoundEndTime == -1 )
				{
					char time[32];

					m_flRoundEndTime = gpGlobals->curtime + (min(playersAlive[0], playersAlive[1]) * 15.0f);

					int min = (m_flRoundEndTime - gpGlobals->curtime) / 60;
					int seconds = (int)(m_flRoundEndTime - gpGlobals->curtime) % 60;

					Q_snprintf( time, sizeof( time ), "%d:%.2d", min, seconds );

					if( iAlliedTickets <= 0 )
						UTIL_CenterPrintAll( "#RnL_Allied_NoTickets", time );
					else
						UTIL_CenterPrintAll( "#RnL_Axis_NoTickets", time );
				}
				else if( m_flRoundEndTime < gpGlobals->curtime )
				{
					if( iAlliedTickets <= 0 && iAxisTickets > 0 )
						RoundEnd( TEAM_AXIS );
					else if( iAlliedTickets > 0 && iAxisTickets <= 0 ) 
						RoundEnd( TEAM_ALLIES );
					else if( playersAlive[1] > playersAlive[0] ) 
						RoundEnd( TEAM_ALLIES );
					else if( playersAlive[1] < playersAlive[0] ) 
						RoundEnd( TEAM_AXIS );
					else
						RoundEnd( TEAM_INVALID );

					return;
				}

				//Check if the Allies have no tickets and no one alive.
				if( iAlliedTickets <= 0 && playersAlive[0] <= 0 )
				{
					DevMsg( "Allies have no one alive and no tickets.\n" );
					//See if the Axis are all dead with no tickets either
					if( iAxisTickets <= 0 && playersAlive[1] <= 0 )
						RoundEnd( TEAM_INVALID );
					//Axis are still kicking so they win!
					else
						RoundEnd( TEAM_AXIS );

					return;
				}
				//Check if the Axis have no tickets and no one alive.
				else if( iAxisTickets <= 0 && playersAlive[1] <= 0 )
				{
					DevMsg( "Axis have no one alive and no tickets.\n" );
					//See if the Allies are all dead with no tickets either
					if( iAlliedTickets <= 0 && playersAlive[0] <= 0 )
						RoundEnd( TEAM_INVALID );
					//Axis are still kicking so they win!
					else
						RoundEnd( TEAM_ALLIES );

					return;
				}
			}
			else
			{
				if( iAlliedTickets <= 0 )
				{
					if( totalPlayers[0] <= 0 )
						UTIL_CenterPrintAll( "#RnL_Axis_WinAlone" );
					else
						UTIL_CenterPrintAll( "#RnL_Allied_LoseAlone" );
				}
				else if( iAxisTickets < 0 )
				{
					if( totalPlayers[1] <= 0 )
						UTIL_CenterPrintAll( "#RnL_Allied_WinAlone" );
					else
						UTIL_CenterPrintAll( "#RnL_Axis_LoseAlone" );
				}
				else
					UTIL_CenterPrintAll( "#RnL_Tie" );

				RoundEnd();
			}
		}
		else
		{
			if( mp_ticketdrain.GetBool() && flNextDrainTime < gpGlobals->curtime )
			{
				int iAlliesObjs = 0, iAxisObjs = 0, iTotalObjs = 0;

				for( int i = 0; i < m_vObjectives.Count(); i++ )
				{
					if( m_vObjectives[i]->IsPrimary() )
					{
						iTotalObjs++;

						switch( m_vObjectives[i]->GetObjectiveState() )
						{
						case RNL_OBJECTIVE_ALLIED_CONTROLLED:
							iAlliesObjs++;
							break;
						case RNL_OBJECTIVE_AXIS_CONTROLLED:
							iAxisObjs++;
							break;
						};
					}
				}

				iAlliedTickets -= floor(((1.0 - ((float)iAlliesObjs / (float)iTotalObjs)) * (float)mp_ticketdrainmax.GetInt()) + 0.5);
				iAxisTickets -= floor(((1.0 - ((float)iAxisObjs / (float)iTotalObjs)) * (float)mp_ticketdrainmax.GetInt()) + 0.5);

				pGameManager->SetAlliedTicketsRemaining( iAlliedTickets );
				pGameManager->SetAxisTicketsRemaining( iAxisTickets );

				flNextDrainTime = gpGlobals->curtime + mp_ticketdraininterval.GetFloat();
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::RoundRespawn( void )
	{
		CleanUpMap();
		if (RnLGameRules() && !RnLGameRules()->GetGameManager())
			RnLGameRules()->InitialiseGameManager();

		UTIL_LogPrintf("World triggered \"Round_Start\"\n");

		// reset per-round scores for each player
		for (int i = 1; i <= MAX_PLAYERS; i++)
		{
			CBasePlayer* pPlayer = ToBasePlayer(UTIL_PlayerByIndex(i));
			if (pPlayer)
			{
				pPlayer->ResetPerRoundStats();
				pPlayer->Spawn();
			}
		}
	}

	ConVar mp_rnl_showcleanedupents( "mp_rnl_showcleanedupents", "0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY, "Show entities that are removed on round respawn." );

	// Classnames of entities that are preserved across round restarts
	static const char* s_RnLRespawnPreserveEnts[] =
	{
		"player",
		"player_manager",
		"viewmodel",
		"worldspawn",
		"soundent",
		"ai_network",
		"ai_hint",
		"env_soundscape",
		"env_soundscape_proxy",
		"env_soundscape_triggerable",
		"env_sprite",
		"env_sun",
		"env_wind",
		"env_fog_controller",
		"func_wall",
		"func_illusionary",
		"info_node",
		"info_target",
		"info_node_hint",
		"point_commentary_node",
		"point_viewcontrol",
		"func_precipitation",
		"func_team_wall",
		"shadow_control",
		"sky_camera",
		"scene_manager",
		"trigger_soundscape",
		"commentary_auto",
		"point_commentary_node",
		"point_commentary_viewpoint",

		//RnL Stuff
		"rnl_gamerules",
		"rnl_team",
		"rnl_game_team",
		"rnl_player_manager",

		"", // END Marker
	};

	//-----------------------------------------------------------------------------
	// Purpose: Recreate all the map entities from the map data (preserving their indices),
	//			then remove everything else except the players.
	//-----------------------------------------------------------------------------
	void CRnLGameRules::CleanUpMap( void )
	{
		int printRespawnStages = mp_rnl_showcleanedupents.GetInt();
		
		if (printRespawnStages > 0)
		{
			Msg("CleanUpMap\n===============\n");
			Msg("  Entities: %d (%d edicts)\n", gEntList.NumberOfEntities(), gEntList.NumberOfEdicts());
		}

		// Delete entities that we want to recreate.
		{
			CRnLEntityFilter cleanupFilter(s_RnLRespawnPreserveEnts);
			
			// Get rid of all entities except players.
			CBaseEntity* pCur = gEntList.FirstEnt();
			while (pCur)
			{
				if (cleanupFilter.ShouldCreateEntity(pCur->GetClassname()))
				{
					if (printRespawnStages > 0)
					{
						Msg("Removed Entity: %s[%s]\n", pCur->GetDebugName(), pCur->GetClassname());
					}
					UTIL_Remove(pCur);
				}

				pCur = gEntList.NextEnt(pCur);
			}
		}

		// Do some book keeping to clean up the world state.
		{
			// Clear out the event queue
			g_EventQueue.Clear();

			// Really remove the entities so we can have access to their slots below.
			gEntList.CleanupDeleteList();

			engine->AllowImmediateEdictReuse();
		}

		if (printRespawnStages > 1)
		{
			Msg("  Entities Left:\n");
			CBaseEntity* pCur = gEntList.FirstEnt();
			while (pCur)
			{
				Msg("  %s (%d)\n", pCur->GetClassname(), pCur->entindex());
				pCur = gEntList.NextEnt(pCur);
			}
		}

		// Respawn the deleted entities.
		{
			CRnLRespawnEntityFilter filter(s_RnLRespawnPreserveEnts);
			MapEntity_ParseAllEntities(engine->GetMapEntitiesString(), &filter, true);
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::PlayStartRoundVoice( void )
	{
		for ( int i = LAST_SHARED_TEAM+1; i < GetNumberOfTeams(); i++ )
		{
			BroadcastSound( i, UTIL_VarArgs("Game.TeamRoundStart%d", i ) );
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::PlayWinSong( int team )
	{
		if ( team == TEAM_UNASSIGNED )
		{
			PlayStalemateSong();
		}
		else
		{
			for ( int i = 0; i < GetNumberOfTeams(); i++ )
			{
				if ( i < FIRST_GAME_TEAM || i == team )
				{
					BroadcastSound( i, UTIL_VarArgs("Game.Win_%s", TeamNumberToName(team) ) );
				}
				else
				{
					BroadcastSound( i, UTIL_VarArgs("Game.Lose_%s", TeamNumberToName(team) ) );
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::PlaySuddenDeathSong( void )
	{
		for ( int i = 0; i < GetNumberOfTeams(); i++ )
		{
			BroadcastSound( i, "Game.SuddenDeath" );
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::PlayStalemateSong( void )
	{
		for ( int i = 0; i < GetNumberOfTeams(); i++ )
		{
			BroadcastSound( i, "Game.Stalemate" );
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::BroadcastSound( int iTeam, const char *sound )
	{
		//send it to everyone
		IGameEvent *event = gameeventmanager->CreateEvent( "teamplay_broadcast_audio" );
		if ( event )
		{
			event->SetInt( "team", iTeam );
			event->SetString( "sound", sound );
			gameeventmanager->FireEvent( event );
		}
	}

	ConVar mp_restartround( "mp_restartround", "0", FCVAR_GAMEDLL, "If non-zero, the current round will restart in the specified number of seconds" );	
	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::CheckRoundRestart( void )
	{
		if( (GetGameManager() == NULL || mp_restartround.GetInt() > 0) && m_flRestartRoundTime < 0 )
		{
			m_flRestartRoundTime = gpGlobals->curtime + mp_restartround.GetInt();
			mp_restartround.SetValue( 0 );
		}

		if( (mp_restartgame.GetInt() > 0) && m_flRestartGameTime < 0 )
		{
			m_flRestartGameTime = gpGlobals->curtime + mp_restartgame.GetInt();
			mp_restartgame.SetValue( 0 );
		}

		// check round restart
		if( m_flRestartRoundTime > 0 && m_flRestartRoundTime < gpGlobals->curtime )
		{
			// time to restart!
			State_Transition( RNL_GR_STATE_RESTART );
			m_flRestartRoundTime = -1;
		}

		// check game restart
		if( m_flRestartGameTime > 0 && m_flRestartGameTime < gpGlobals->curtime )
		{
			// time to restart!
			State_Transition( RNL_GR_STATE_RESTART );
			m_flRestartGameTime = -1;
			m_iRoundsPlayed = 0;
		}
	}

	void CRnLGameRules::RoundStart( void )
	{
		m_iWinningTeam = TEAM_INVALID;
	}

	void CRnLGameRules::RoundEnd( int iWinningTeam, bool bForceMapChange )
	{
		m_iWinningTeam = iWinningTeam;
		m_bChangeLevelAfterWin = bForceMapChange;

		if( m_iWinningTeam > -1 )
		{
			if( m_iWinningTeam == TEAM_AXIS )
				UTIL_CenterPrintAll( "#RnL_Axis_Win" );
			else if( m_iWinningTeam == TEAM_ALLIES )
				UTIL_CenterPrintAll( "#RnL_Allied_Win" );
			else
				UTIL_CenterPrintAll( "#RnL_No_Win" );
		}

		for(int i = 0; i <= gpGlobals->maxClients; i++)
		{
			CRnLPlayer *pPlayer = ToRnLPlayer(UTIL_PlayerByIndex(i));

			if( !pPlayer )
				continue;

			if( pPlayer->IsInAVehicle() )
				pPlayer->LeaveVehicle( vec3_origin, vec3_angle ); 

			pPlayer->RemoveEquipment(RNL_EQUIPMENT_ANY);
			pPlayer->RemoveAllAmmo();
		}

		m_flRoundEndTime = -1;
		m_iRoundsPlayed++;

		if( mp_roundlimit.GetInt() > 0 && m_iRoundsPlayed >= mp_roundlimit.GetInt() )
			m_bChangeLevelAfterWin = true;

		State_Transition( RNL_GR_STATE_TEAM_WIN );
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Transition( rnl_gamerules_roundstate_t newState )
	{
		State_Leave();
		State_Enter( newState );
	}	

	ConVar mp_rnl_showroundtransitions( "mp_rnl_showroundtransitions", "0", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY, "Show gamestate round transitions." );
	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Enter( rnl_gamerules_roundstate_t newState )
	{
		m_iRoundState = newState;
		m_pCurStateInfo = State_LookupInfo( newState );

		if ( mp_rnl_showroundtransitions.GetInt() > 0 )
		{
			if ( m_pCurStateInfo )
				Msg( "Gamerules: entering state '%s'\n", m_pCurStateInfo->m_pStateName );
			else
				Msg( "Gamerules: entering state #%d\n", newState );
		}

		// Initialize the new state.
		if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
		{
			(this->*m_pCurStateInfo->pfnEnterState)();
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Leave()
	{
		if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
		{
			(this->*m_pCurStateInfo->pfnLeaveState)();
		}
	}


	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Think()
	{
		if ( m_pCurStateInfo && m_pCurStateInfo->pfnThink )
		{
			(this->*m_pCurStateInfo->pfnThink)();
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	CRnLGameRulesRoundStateInfo* CRnLGameRules::State_LookupInfo( rnl_gamerules_roundstate_t state )
	{
		static CRnLGameRulesRoundStateInfo gamerulesStateInfo[] =
		{
			{ RNL_GR_STATE_INIT,		"RNL_GR_STATE_INIT",		&CRnLGameRules::State_Enter_INIT,		NULL, &CRnLGameRules::State_Think_INIT },
			{ RNL_GR_STATE_PREGAME,		"RNL_GR_STATE_PREGAME",		&CRnLGameRules::State_Enter_PREGAME,	NULL, &CRnLGameRules::State_Think_PREGAME },
			{ RNL_GR_STATE_STARTGAME,	"RNL_GR_STATE_STARTGAME",	&CRnLGameRules::State_Enter_STARTGAME,	NULL, &CRnLGameRules::State_Think_STARTGAME },
			{ RNL_GR_STATE_PREROUND,	"RNL_GR_STATE_PREROUND",	&CRnLGameRules::State_Enter_PREROUND,	NULL, &CRnLGameRules::State_Think_PREROUND },
			{ RNL_GR_STATE_RND_RUNNING,	"RNL_GR_STATE_RND_RUNNING",	&CRnLGameRules::State_Enter_RND_RUNNING, NULL, &CRnLGameRules::State_Think_RND_RUNNING },
			{ RNL_GR_STATE_TEAM_WIN,	"RNL_GR_STATE_TEAM_WIN",	&CRnLGameRules::State_Enter_TEAM_WIN,	NULL, &CRnLGameRules::State_Think_TEAM_WIN },
			{ RNL_GR_STATE_RESTART,		"RNL_GR_STATE_RESTART",		&CRnLGameRules::State_Enter_RESTART,	NULL, &CRnLGameRules::State_Think_RESTART },
			{ RNL_GR_STATE_STALEMATE,	"RNL_GR_STATE_STALEMATE",	&CRnLGameRules::State_Enter_STALEMATE,	&CRnLGameRules::State_Leave_STALEMATE, &CRnLGameRules::State_Think_STALEMATE },
			{ RNL_GR_STATE_GAME_OVER,	"RNL_GR_STATE_GAME_OVER",	NULL, NULL, NULL },
		};

		for ( int i=0; i < ARRAYSIZE( gamerulesStateInfo ); i++ )
		{
			if ( gamerulesStateInfo[i].m_iRoundState == state )
				return &gamerulesStateInfo[i];
		}

		return NULL;
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Enter_INIT( void )
	{
		m_flStateTransitionTime = gpGlobals->curtime + 2.5f;
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Think_INIT( void )
	{
		if( gpGlobals->curtime > m_flStateTransitionTime )
		{
			State_Transition( RNL_GR_STATE_PREGAME );
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: The server is idle and waiting for enough players to start up again. 
	//			When we find an active player go to GR_STATE_STARTGAME.
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Enter_PREGAME( void )
	{
	}

	extern bool IsInCommentaryMode( void );
	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Think_PREGAME( void )
	{
		CheckRespawnWaves();

		// Commentary stays in this mode too
		if ( IsInCommentaryMode() )
			return;
		
		State_Transition( RNL_GR_STATE_STARTGAME );	
	}

	//-----------------------------------------------------------------------------
	// Purpose: Wait a bit and then spawn everyone into the preround
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Enter_STARTGAME( void )
	{
		m_flStateTransitionTime = gpGlobals->curtime;

		InitialiseGameManager();
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Think_STARTGAME()
	{
		if( gpGlobals->curtime > m_flStateTransitionTime )
		{
			State_Transition( RNL_GR_STATE_PREROUND );
		}
	}
		
	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Enter_PREROUND( void )
	{
		IGameEvent *event = gameeventmanager->CreateEvent( "teamplay_round_start" );
		if ( event )
		{
			gameeventmanager->FireEvent( event );
		}

		m_flStateTransitionTime = gpGlobals->curtime + 2.5f;
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Think_PREROUND( void )
	{
		if( gpGlobals->curtime > m_flStateTransitionTime )
		{
			State_Transition( RNL_GR_STATE_RND_RUNNING );
		}

		CheckRespawnWaves();
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Enter_RND_RUNNING( void )
	{
		IGameEvent *event = gameeventmanager->CreateEvent( "teamplay_round_active" );
		if ( event )
		{
			gameeventmanager->FireEvent( event );
		}

		PlayStartRoundVoice();
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Think_RND_RUNNING( void )
	{
		CheckRespawnWaves();
		CheckObjectives();
		CheckTickets();

		// check round restart
		CheckRoundRestart();
	}

	extern ConVar mp_chattime;
	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Enter_TEAM_WIN( void )
	{
		float flTime = 5.0f;

		if( m_bChangeLevelAfterWin )
		{
			flTime = mp_chattime.GetInt();

			for ( int i = 1; i <= MAX_PLAYERS; i++ )
			{
				CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

				if ( !pPlayer )
					continue;

				pPlayer->ShowViewPortPanel( PANEL_SCOREBOARD );
				pPlayer->RemoveAllAmmo();
			}
		}

		m_flStateTransitionTime = gpGlobals->curtime + flTime;
		PlayWinSong( m_iWinningTeam );
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Think_TEAM_WIN( void )
	{
		if( gpGlobals->curtime > m_flStateTransitionTime )
		{
			if ( m_bChangeLevelAfterWin )
				ChangeLevel();
			else
				State_Transition( RNL_GR_STATE_RESTART );
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Enter_STALEMATE( void )
	{
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Think_STALEMATE( void )
	{
		//if we don't find any active players, return to RNL_GR_STATE_PREGAME
		State_Transition( RNL_GR_STATE_PREGAME );
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Leave_STALEMATE( void )
	{
	}

	//-----------------------------------------------------------------------------
	// Purpose: manual restart
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Enter_RESTART( void )
	{
		// send restart event
		IGameEvent *event = gameeventmanager->CreateEvent( "teamplay_restart_round" );
		if ( event )
		{
			gameeventmanager->FireEvent( event );
		}

		// Reset everything in the level
		RoundRespawn();

		State_Transition( RNL_GR_STATE_PREROUND );
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CRnLGameRules::State_Think_RESTART( void )
	{
		// should never get here, State_Enter_RESTART sets us into a different state
		Assert( 0 ); 
	}
#endif