#ifndef RNL_MINE_ENTITY_H
#define RNL_MINE_ENTITY_H

#include "cbase.h"

class CRnLMineEntity : public CBaseAnimating
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CRnLMineEntity, CBaseAnimating );

public:
	void CreateMine( const char* pszModelName );
	void Spawn( void ) OVERRIDE;
	void ExplodeThink( void );

	// Input function
	void InputToggle( inputdata_t &inputData );

	CNetworkVar( float, m_DmgRadius );				// How far do I do damage?

	virtual void SetDamageRadius(float flDamageRadius)
	{
		m_DmgRadius = flDamageRadius;
	}

	virtual void SetDamage(float flDamage)
	{
		m_flDamage = flDamage;
	}

	virtual Vector		GetBlastForce() { return vec3_origin; }

protected:
	CNetworkVar( float, m_flDamage );		// Damage to inflict.
};

#endif

