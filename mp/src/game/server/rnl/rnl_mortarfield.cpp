#include "cbase.h"
//Access to BIG BOOMs.
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#include "explode.h"

#include "tier0/memdbgon.h"

enum
{
	MORTAR_FIRE = 0,
	MORTAR_INCOMING,
	MORTAR_EXPLODE,
	SINGLE_MORTAR_FIRE,
	SINGLE_MORTAR_INCOMING,
	SINGLE_MORTAR_EXPLODE,
};

//This is the mortar class.  It's a big box that has mortars "landing" at
// random locations inside of it
class CRnLMortarField : public CBaseEntity
{
public:
	//Holy macros Batman.
	DECLARE_CLASS( CRnLMortarField, CBaseEntity );
	//Important stuff to grab from hammer/.bsp's here
	DECLARE_DATADESC();

	//Bog standard stuff
	void Precache( void );
	void Spawn( void );
	void Think();

	//Start and stop Mr. Field
	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );

	//This is sexy, change the bombing rates, could be cool to see 
	//what can be done.
	void InputSetMaxDelay( inputdata_t &inputdata );
	void InputSetMinDelay( inputdata_t &inputdata );

	// fire one round into the mortar field
	void InputFireRound( inputdata_t &inputdata );

	//This is the VIP section of the club.. only cool people get in here
private:

	//Prepares our next strike
	void PrepNextStrike( void );

	//Play the sound of incoming mortars
	void PlayIncomingSound( void );

	//Explosivo!
	void FireMortar( void );

	//Used to decide if we play the
	// incoming mortar sound or just make
	//  a really big explosion
	int		m_iMortarState;

	//This will be our next lateral strike locations
	float		m_flNextXLocation;
	float		m_flNextYLocation;
	float		m_flExplosionLocation;

	//Are we currently active?
	bool		m_bIsActive;
	
	//These are the delays between strikes
	float		m_flMinStrikeDelay;
	float		m_flMaxStrikeDelay;
	
	//Damage info, how much and how far
	float		m_flDamage;
	float		m_flDamageRadius;

	//information about screen shakes
	float		m_flShakeAmplitude;
	float		m_flShakeFrequency;
	float		m_flShakeRadius;

	string_t m_MortarSource;

	CBaseEntity* m_pSourceEnt;
};

BEGIN_DATADESC( CRnLMortarField )

	//Done so we can start active if we want to
	DEFINE_KEYFIELD( m_bIsActive, FIELD_BOOLEAN, "Enabled" ),

	DEFINE_KEYFIELD( m_MortarSource, FIELD_STRING,	"MortarSource" ),

	//Timing
	DEFINE_KEYFIELD( m_flMinStrikeDelay, FIELD_FLOAT, "MinDelay" ),
	DEFINE_KEYFIELD( m_flMaxStrikeDelay, FIELD_FLOAT, "MaxDelay" ),

	//Damage
	DEFINE_KEYFIELD( m_flDamage, FIELD_FLOAT, "Damage" ),
	DEFINE_KEYFIELD( m_flDamageRadius, FIELD_FLOAT, "DamageRadius" ),

	//Screen Shaking
	DEFINE_KEYFIELD( m_flShakeAmplitude,	FIELD_FLOAT, "ShakeAmplitude" ),
	DEFINE_KEYFIELD( m_flShakeFrequency,	FIELD_FLOAT, "ShakeFrequency" ),
	DEFINE_KEYFIELD( m_flShakeRadius,		FIELD_FLOAT, "ShakeRadius" ),

	///////////////////
	//Inputs
	//////////////////
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),

	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetMaxDelay", InputSetMaxDelay ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetMinDelay", InputSetMinDelay ),

	DEFINE_INPUTFUNC( FIELD_VOID, "FireRound", InputFireRound ),

	//Importante: we gots ta fink
	DEFINE_THINKFUNC( Think ),

END_DATADESC()

//Valve are marcoholics serious!
LINK_ENTITY_TO_CLASS( rnl_mortarfield, CRnLMortarField );

//Precache stuff
void CRnLMortarField::Precache()
{
	PrecacheScriptSound( "Mortar.Fire" );
	PrecacheScriptSound( "Mortar.Incoming" );
	PrecacheScriptSound( "Mortar.Hit" );
}

//handle our spawning.
void CRnLMortarField::Spawn( void )
{
	BaseClass::Spawn();

	Precache();

	//Not moving, no way no how... could be changed later
	SetMoveType( MOVETYPE_NONE );
	SetModel( STRING( GetModelName() ) );    // set size and link into world
	//Invisible... atleast make it look like this is a REAL mortar zone
	AddEffects( EF_NODRAW );

	//********************************
	//
	// These are just to make sure we dont
	// get too many bogus values, if any at all
	//
	//********************************************

	//Start out playing the incoming sound
	m_iMortarState = MORTAR_FIRE;

	if( m_flMinStrikeDelay <= 0 )
	{
		m_flMinStrikeDelay = 0.01;
	}

	if( m_flMaxStrikeDelay <= 0 )
	{
		m_flMaxStrikeDelay = 0.01;
	}

	if( m_flDamage < 0 )
	{
		m_flDamage = 0;
	}

	if( m_flDamageRadius < 0 )
	{
		m_flDamageRadius = 1.0f;
	}

	if( m_flShakeAmplitude < 1 )
	{
		m_flShakeAmplitude = 1.0f;
	}

	if( m_flShakeFrequency < 1 )
	{
		m_flShakeFrequency = 1.0f;
	}

	if( m_flShakeRadius < 1 )
	{
		m_flShakeRadius = 1.0f;
	}

	//Starting active?
	if( m_bIsActive )
	{
		SetThink( &CRnLMortarField::Think );
		SetNextThink( gpGlobals->curtime );
	}
	else
	{
		SetThink( NULL );
	}

	m_pSourceEnt = gEntList.FindEntityByName( NULL, m_MortarSource );
}

void CRnLMortarField::PrepNextStrike( void )
{
	m_flNextXLocation = random->RandomFloat( 0.0f, 1.0f );
	m_flNextYLocation = random->RandomFloat( 0.0f, 1.0f );
}

void CRnLMortarField::PlayIncomingSound( void )
{
	CCollisionProperty *pProp = CollisionProp();
	if( pProp )
	{
		Vector vecNormalized( m_flNextXLocation, m_flNextYLocation, 1.0f );
		Vector vecStrikeLocation;

		pProp->NormalizedToWorldSpace( vecNormalized, &vecStrikeLocation );

		trace_t tr;
		Vector endPos = vecStrikeLocation;
		endPos.z -= 2048;
		UTIL_TraceLine( vecStrikeLocation, endPos, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

		if( tr.DidHit() )
		{
			m_flExplosionLocation = tr.endpos.z + 2;
		}
		else
		{
			m_flExplosionLocation = endPos.z;
		}

		//Play a sound
		CPASAttenuationFilter filter( vecStrikeLocation );
		filter.MakeReliable();
		EmitSound( filter, entindex(), "Mortar.Incoming", &vecStrikeLocation );
	}
}

//Create the acctual explosion for the mortar
void CRnLMortarField::FireMortar( void )
{
	CCollisionProperty *pProp = CollisionProp();
	if( pProp )
	{
		Vector vecNormalized( m_flNextXLocation, m_flNextYLocation, 0 );
		Vector vecStrikeLocation;

		pProp->NormalizedToWorldSpace( vecNormalized, &vecStrikeLocation );

		Vector vecStrikeEnd = vecStrikeLocation;
		vecStrikeEnd.z = m_flExplosionLocation;

		CEffectData data;
		data.m_vOrigin = vecStrikeEnd;

		if( UTIL_PointContents( data.m_vOrigin ) & (CONTENTS_WATER ) )
		{
			DispatchEffect( "MortarWaterSurfaceExplosion", data );
		}
		else
		{
			DispatchEffect( "MortarExplosion", data );
			EmitSound( "BaseExplosionEffect.Sound" );
		}

		////Explode
		//ExplosionCreate( vecStrikeEnd, GetAbsAngles(), this, m_flDamage, m_flDamageRadius, SF_ENVEXPLOSION_NODECAL, 200.0f, this );
		ExplosionCreate( vecStrikeEnd, GetAbsAngles(), this, m_flDamage, m_flDamageRadius, SF_ENVEXPLOSION_NOSPARKS|SF_ENVEXPLOSION_NOSMOKE, m_flDamage, this );
		UTIL_ScreenShake( vecStrikeEnd, m_flShakeAmplitude, m_flShakeFrequency, 1.0, m_flShakeRadius, SHAKE_START );
	}
}

//**********************************
//**
//** This is a logic function that does this:
//   it decides if we are going to play an incoming
//   mortar sound or create an exploision, then does so.
//   then sets itself to be called again at a random 
//   interval that falls between the min and max delays.
// 
//   K? You got that, I'm not gonna comment much more..
//
//**********************************************************
void CRnLMortarField::Think( void )
{
	if( m_iMortarState == SINGLE_MORTAR_FIRE )
	{
		DevMsg( "Firing Single Mortar Round\n" );
		if ( m_pSourceEnt )
			m_pSourceEnt->EmitSound( "Mortar.Fire" );

		SetNextThink( gpGlobals->curtime + 2.5f );
		m_iMortarState = SINGLE_MORTAR_INCOMING;
		return;
	}
	else if( m_iMortarState == SINGLE_MORTAR_INCOMING )
	{
		DevMsg( "Incoming Single Mortar Round\n" );
		PrepNextStrike();
		PlayIncomingSound();
		// explode 1 seconds after playing incoming sound
		SetNextThink( gpGlobals->curtime + 1.0f );
		m_iMortarState = SINGLE_MORTAR_EXPLODE;
		return;
	}
	else if( m_iMortarState == SINGLE_MORTAR_EXPLODE )
	{
		DevMsg( "Exploding Single Mortar Round\n" );
		FireMortar();
		SetNextThink( gpGlobals->curtime + random->RandomFloat( m_flMinStrikeDelay, m_flMaxStrikeDelay ) );
		m_iMortarState = MORTAR_FIRE;
		return;
	}

	if( m_bIsActive == false )
	{
		SetThink( NULL );
		return;
	}

	//Starts as our sound duration.
	float nextThink = 1.0f;
	
	if( m_iMortarState == MORTAR_FIRE )
	{
		if ( m_pSourceEnt )
		{
			CPASAttenuationFilter filter( this, "Mortar.Fire" );
			m_pSourceEnt->EmitSound( filter, m_pSourceEnt->entindex(), "Mortar.Fire" ); 
		}

		nextThink = 2.5f;
		m_iMortarState = MORTAR_INCOMING;
	}
	else if( m_iMortarState == MORTAR_INCOMING )
	{
		PrepNextStrike();
		PlayIncomingSound();
		m_iMortarState = MORTAR_EXPLODE;
	}
	else
	{
		FireMortar();
		m_iMortarState = MORTAR_FIRE;
		nextThink = random->RandomFloat( m_flMinStrikeDelay, m_flMaxStrikeDelay );
	}

	SetNextThink( gpGlobals->curtime + nextThink );
}

//***********************************************
//**********************************************
// INPUT BODIES!
//*******************************************
//*****************************************
// COMPUTER EAT BODIES!!
//*************************************
//**********************************
// ONLY MESSING, they're function bodies
//***************************************

void CRnLMortarField::InputEnable( inputdata_t &inputdata )
{
	if( m_bIsActive )
		return;

	m_bIsActive = true;
	m_iMortarState = MORTAR_FIRE;
	SetThink( &CRnLMortarField::Think );
	SetNextThink( gpGlobals->curtime );
}

void CRnLMortarField::InputDisable( inputdata_t &inputdata )
{
	m_bIsActive = false;
}

void CRnLMortarField::InputSetMaxDelay( inputdata_t &inputdata )
{
	m_flMaxStrikeDelay = inputdata.value.Float();

	if( m_flMaxStrikeDelay <= 0 )
	{
		m_flMaxStrikeDelay = 0.1;
	}
}

void CRnLMortarField::InputSetMinDelay( inputdata_t &inputdata )
{
	m_flMinStrikeDelay = inputdata.value.Float();

	if( m_flMinStrikeDelay <= 0 )
	{
		m_flMinStrikeDelay = 0.1;
	}
}

void CRnLMortarField::InputFireRound( inputdata_t &inputdata )
{
	DevMsg( "Firing Single Mortar Round\n" );
	m_iMortarState = SINGLE_MORTAR_FIRE;
	// start firing round immediately
	SetThink( &CRnLMortarField::Think );
	SetNextThink( gpGlobals->curtime );
}