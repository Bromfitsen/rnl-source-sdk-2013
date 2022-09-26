//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef RNL_EXPLOSIVE_H
#define RNL_EXPLOSIVE_H
#ifdef _WIN32
#pragma once
#endif


#include "basegrenade_shared.h"


#ifdef CLIENT_DLL
	#define CRnLBaseExplosive C_RnLBaseExplosive
#endif


class CRnLBaseExplosive : public CBaseGrenade
{
public:
	DECLARE_CLASS( CRnLBaseExplosive, CBaseGrenade );
	DECLARE_NETWORKCLASS(); 

	virtual void Spawn();

#ifdef CLIENT_DLL
	CRnLBaseExplosive() {}

	virtual int DrawModel( int flags );
	virtual void PostDataUpdate( DataUpdateType_t type );
#else
	DECLARE_DATADESC();

	//Think function to emit danger sounds for the AI
	void DangerSoundThink( void );
	virtual void		Detonate( void );

protected:

	//Set the time to detonate ( now + timer )
	void SetDetonateTimerLength( float timer );

private:	
	float	m_flDetonateTime;
#endif
};


#endif // RNL_EXPLOSIVE_H
