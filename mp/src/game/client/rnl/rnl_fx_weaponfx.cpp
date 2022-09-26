//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Game-specific impact effect hooks
//
//=============================================================================//
#include "cbase.h"
#include "fx_impact.h"
#include "tempent.h"
#include "c_te_effect_dispatch.h"
#include "c_te_legacytempents.h"


//-----------------------------------------------------------------------------
// Purpose: Handle weapon effect callbacks
//-----------------------------------------------------------------------------
void RnL_EjectBrass( int shell, const CEffectData &data )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if( !pPlayer )
		return;

	tempents->CSEjectBrass( data.m_vOrigin, data.m_vAngles, data.m_fFlags, shell, pPlayer );
}

void RnL_FX_EjectBrass_Callback( const CEffectData &data )
{
	RnL_EjectBrass( RNL_SHELL, data );
}

void RnL_FX_EjectBrass_Pistol_Callback( const CEffectData &data )
{
	RnL_EjectBrass( RNL_SHELL_PISTOL, data );
}

void RnL_FX_EjectBrass_Rifle_Callback( const CEffectData &data )
{
	RnL_EjectBrass( RNL_SHELL_RIFLE, data );
}

void RnL_FX_EjectBrass_MG_Callback( const CEffectData &data )
{
	RnL_EjectBrass( RNL_SHELL_MG, data );
}



DECLARE_CLIENT_EFFECT( "EjectBrass", RnL_FX_EjectBrass_Callback );
DECLARE_CLIENT_EFFECT( "EjectBrass_Pistol",RnL_FX_EjectBrass_Pistol_Callback );
DECLARE_CLIENT_EFFECT( "EjectBrass_Rifle",RnL_FX_EjectBrass_Rifle_Callback );
DECLARE_CLIENT_EFFECT( "EjectBrass_MG",RnL_FX_EjectBrass_MG_Callback );

