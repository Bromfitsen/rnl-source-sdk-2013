//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: RnL resupply crate
//
//=============================================================================//

#include "cbase.h"
#include "c_rnl_player.h"
#include "rnl_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


class C_RnLResupplyCrate : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_RnLResupplyCrate, C_BaseAnimating );

	DECLARE_CLIENTCLASS();
	
	C_RnLResupplyCrate();

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	//virtual bool	ShouldDraw( void ) { return false; }

	virtual void	ClientThink( );

private:
};


IMPLEMENT_CLIENTCLASS_DT( C_RnLResupplyCrate, DT_RnLResupplyCrate, CRnLResupplyCrate )
END_RECV_TABLE()


C_RnLResupplyCrate::C_RnLResupplyCrate()
{
}

//-----------------------------------------------------------------------------
// Post data update!
//-----------------------------------------------------------------------------
void C_RnLResupplyCrate::OnDataChanged( DataUpdateType_t updateType )
{
	if( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink(0.0f);
	}
	BaseClass::OnDataChanged( updateType );
}

void C_RnLResupplyCrate::ClientThink( )
{
	int open = LookupSequence( "Open" );
	int close = LookupSequence( "Close" );
	int idle = LookupSequence( "idle" );

	bool found = false;
	for ( int i = 1; i <= MAX_PLAYERS; i++ )
	{
		if ( UTIL_PlayerByIndex( i ) )
		{
			C_RnLPlayer* pPlayer = ToRnLPlayer( UTIL_PlayerByIndex( i ) );
			if( GetTeamNumber() == pPlayer->GetTeamNumber() && !pPlayer->IsDeployed() )
			{
				if( GetAbsOrigin().DistToSqr( pPlayer->GetAbsOrigin() ) < (128*128) )
					found = true;
			}
		}
	}

	if( !found )
	{
		if( GetSequence() == open )
			ResetSequence( close );
		else if( GetSequence() == close && IsSequenceFinished() )
			ResetSequence( idle );
	}
	else if( GetSequence() != open )
		ResetSequence( open );

	SetNextClientThink( gpGlobals->curtime + 1.0f );
}