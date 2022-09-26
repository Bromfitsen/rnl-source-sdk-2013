#ifndef RNL_HITGROUPDATA_H
#define RNL_HITGROUPATA_H

#include <utlmap.h>

class CRnLHitGroupDetails
{
public:
	CRnLHitGroupDetails()
	{
		m_flDamageScale = 1.0;
		m_bInstantKill = false;
	}

	void Set( float scale, bool kill )
	{
		m_flDamageScale = scale;
		m_bInstantKill = kill;
	}

	float GetScale( void )		{ return m_flDamageScale; }
	bool IsInstantKill( void )	{ return m_bInstantKill; }

private:
	float	m_flDamageScale;
	bool	m_bInstantKill;
};

extern CUtlMap<int, CRnLHitGroupDetails>& GetRnLHitGroupDetails( void );

#endif	//RNL_HITGROUPDATA_H