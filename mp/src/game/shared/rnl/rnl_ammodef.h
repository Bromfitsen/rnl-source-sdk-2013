//=============================================================================//
// Purpose:		Holds definitions and functions for bullet penetration
// Author:		Toni Kreska
//=============================================================================//
#ifndef RNL_AMMODEF_H
#define RNL_AMMODEF_H
#pragma once

#include "ammodef.h"

struct Pen_t 
{
	int					wood;
	int					metal;
	int					concrete;
	int					flesh;
	int					dirt;
	int					glass;
	int					foliage;
};

struct Deflect_t 
{
	bool				wood;
	float				woodAng;
	bool				metal;
	float				metalAng;
	bool				concrete;
	float				concreteAng;
	bool				flesh;
	float				fleshAng;
	bool				dirt;
	float				dirtAng;
	bool				glass;
	float				glassAng;
	bool				foliage;
	float				foliageAng;
};

struct BallisticInfo_t
{
	float effectiveRange;
	float velocity;
};

class CRnLAmmoDef : public CAmmoDef
{

public:
	CRnLAmmoDef(void);
	~CRnLAmmoDef( void );

	void	AddBallisticInfo( const char* pszAmmoType, float flRange, float flVelocity );
	void	AddBallisticInfo( int iAmmoID, float flRange, float flVelocity );

	float	GetEffectiveRange( const char* pszAmmoType );
	float	GetEffectiveRange( int iAmmoID );

	float	GetVelocity( const char* pszAmmoType );
	float	GetVelocity( int iAmmoID );

	bool	GetDeflection( const char* pszAmmoType, unsigned short surfaceProp, float& flAng );
	bool	GetDeflection(int iAmmoID, unsigned short surfaceProp, float& flAng );
	
	bool	AddDeflectionType(int iAmmoID, bool wood, float woodAng, bool metal, float mentAng, bool concrete, float concreteAng,
								bool flesh, float fleshAng, bool dirt, float dirtAng, bool glass, float glassAng, bool foliage, float foliageAng );
	bool	AddDeflectionType( const char* pszAmmoType, bool wood, float woodAng, bool metal, float mentAng, bool concrete, float concreteAng,
								bool flesh, float fleshAng, bool dirt, float dirtAng, bool glass, float glassAng, bool foliage, float foliageAng );

	float	GetPenetrationDistance(const char* pszAmmoType, unsigned short surfaceProp);
	float	GetPenetrationDistance(int iAmmoID, unsigned short surfaceProp);
	
	bool	AddPenetrationType(const char* pszAmmoType, int penWood, int penMetal, int penConcrete, int penFlesh, int penDirt, int penGlass, int penfoliage );
	bool	AddPenetrationType(int iAmmoID, int penWood, int penMetal, int penConcrete, int penFlesh, int penDirt, int penGlass, int penfoliage );

private:
	Pen_t				m_PenetrationType[MAX_AMMO_TYPES];
	Deflect_t			m_DeflectionType[MAX_AMMO_TYPES];
	BallisticInfo_t		m_BallisticInfo[MAX_AMMO_TYPES];
};

// Get the global ammodef object. This is usually implemented in each mod's game rules file somewhere,
// so the mod can setup custom ammo types.
CRnLAmmoDef* GetRnLAmmoDef();

#endif //RNL_AMMODEF_H