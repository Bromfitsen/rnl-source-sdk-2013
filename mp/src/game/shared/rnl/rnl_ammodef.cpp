//=============================================================================//
// Purpose:		Holds definitions and functions for bullet penetration
// Author:		Toni Kreska
//=============================================================================//

#include "cbase.h"
#include "decals.h"
#include "rnl_ammodef.h"
#include "rnl_shareddefs.h"

#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CRnLAmmoDef::CRnLAmmoDef(void)
{
	// Start with an index of 1.  Client assumes 0 is an invalid ammo type
	memset( m_PenetrationType, 0, sizeof( m_PenetrationType ) );
	memset( m_DeflectionType, 0, sizeof( m_DeflectionType ) );
	memset( m_BallisticInfo, 0, sizeof( m_BallisticInfo ) );
}

CRnLAmmoDef::~CRnLAmmoDef( void )
{
}

void CRnLAmmoDef::AddBallisticInfo( const char* pszAmmoType, float flRange, float flVelocity )
{
	AddBallisticInfo( Index( pszAmmoType ), flRange, flVelocity );
}

void CRnLAmmoDef::AddBallisticInfo( int iAmmoID, float flRange, float flVelocity )
{
	if (iAmmoID >= MAX_AMMO_TYPES || iAmmoID < 0 )
		return;

	m_BallisticInfo[iAmmoID].velocity = flVelocity;
	m_BallisticInfo[iAmmoID].effectiveRange = flRange;
}

float CRnLAmmoDef::GetEffectiveRange( const char* pszAmmoType )
{
	return GetEffectiveRange( Index( pszAmmoType ) );
}

float CRnLAmmoDef::GetEffectiveRange( int iAmmoID )
{
	if (iAmmoID >= MAX_AMMO_TYPES || iAmmoID < 0 )
		return 0.0;

	return m_BallisticInfo[iAmmoID].effectiveRange;
}

float CRnLAmmoDef::GetVelocity( const char* pszAmmoType )
{
	return GetVelocity( Index( pszAmmoType ) );
}

float CRnLAmmoDef::GetVelocity( int iAmmoID )
{
	if (iAmmoID >= MAX_AMMO_TYPES || iAmmoID < 0 )
		return 0.0;

	return m_BallisticInfo[iAmmoID].velocity;
}

void CRnLAmmoDef::SetMagazineSize(const char* pszAmmoType, int32 iMagSize)
{
	int32 iAmmoID = Index(pszAmmoType);
	if (iAmmoID >= MAX_AMMO_TYPES || iAmmoID < 0)
		return;

	m_MagazineInfo[iAmmoID] = iMagSize;
}

int32 CRnLAmmoDef::GetMagazineSize(const char* pszAmmoType)
{
	return GetMagazineSize(Index(pszAmmoType));
}

int32 CRnLAmmoDef::GetMagazineSize(int iAmmoID)
{
	if (iAmmoID >= MAX_AMMO_TYPES || iAmmoID < 0)
		return 0;

	return m_MagazineInfo[iAmmoID];
}

bool CRnLAmmoDef::GetDeflection(const char* pszAmmoType, unsigned short surfaceProp, float& flAng )
{
	return GetDeflection( Index( pszAmmoType ), surfaceProp, flAng );
}

bool CRnLAmmoDef::GetDeflection(int iAmmoID, unsigned short surfaceProp, float& flAng )
{
	switch (surfaceProp)
	{
	case CHAR_TEX_TILE:
	case CHAR_TEX_CONCRETE:
		flAng = m_DeflectionType[iAmmoID].concreteAng;
		return m_DeflectionType[iAmmoID].concrete;
	
	case CHAR_TEX_VENT:
	case CHAR_TEX_METAL:
		flAng = m_DeflectionType[iAmmoID].metalAng;
		return m_DeflectionType[iAmmoID].metal;
	
	case CHAR_TEX_FOLIAGE:
	case CHAR_TEX_HEDGEROW:
		flAng = m_DeflectionType[iAmmoID].foliageAng;
		return m_DeflectionType[iAmmoID].foliage;

	case CHAR_TEX_SAND:
	case CHAR_TEX_SLOSH:
	case CHAR_TEX_DIRT:
		flAng = m_DeflectionType[iAmmoID].dirtAng;
		return m_DeflectionType[iAmmoID].dirt;
	
	case CHAR_TEX_WOOD:
	case CHAR_TEX_PLASTIC:
	case CHAR_TEX_GRATE:
		flAng = m_DeflectionType[iAmmoID].woodAng;
		return m_DeflectionType[iAmmoID].wood;
	
	case CHAR_TEX_GLASS:
		flAng = m_DeflectionType[iAmmoID].glassAng;
		return m_DeflectionType[iAmmoID].glass;
	
	case CHAR_TEX_BLOODYFLESH:
	case CHAR_TEX_FLESH:
		flAng = m_DeflectionType[iAmmoID].fleshAng;
		return m_DeflectionType[iAmmoID].flesh;
	}

	return false;
}

bool CRnLAmmoDef::AddDeflectionType(const char* pszAmmoType, bool wood, float woodAng, bool metal, float metalAng, bool concrete, float concreteAng,
		bool flesh, float fleshAng, bool dirt, float dirtAng, bool glass, float glassAng, bool foliage, float foliageAng )
{
	return AddDeflectionType(Index( pszAmmoType), wood, woodAng, metal, metalAng, concrete, concreteAng,
		flesh, fleshAng, dirt, dirtAng, glass, glassAng, foliage, foliageAng );
}

bool CRnLAmmoDef::AddDeflectionType(int iAmmoID, bool wood, float woodAng, bool metal, float metalAng, bool concrete, float concreteAng,
		bool flesh, float fleshAng, bool dirt, float dirtAng, bool glass, float glassAng, bool foliage, float foliageAng )
{
	if (iAmmoID >= MAX_AMMO_TYPES || iAmmoID < 0 )
		return false;

	m_DeflectionType[iAmmoID].wood = wood;
	m_DeflectionType[iAmmoID].woodAng = woodAng;
	m_DeflectionType[iAmmoID].metal = metal;
	m_DeflectionType[iAmmoID].metalAng = metalAng;
	m_DeflectionType[iAmmoID].concrete = concrete;
	m_DeflectionType[iAmmoID].concreteAng = concreteAng;
	m_DeflectionType[iAmmoID].flesh = flesh;
	m_DeflectionType[iAmmoID].fleshAng = fleshAng;
	m_DeflectionType[iAmmoID].dirt = dirt;
	m_DeflectionType[iAmmoID].dirtAng = dirtAng;
	m_DeflectionType[iAmmoID].glass = glass;
	m_DeflectionType[iAmmoID].glassAng = glassAng;
	m_DeflectionType[iAmmoID].foliage = foliage;
	m_DeflectionType[iAmmoID].foliageAng = foliageAng;

	return true;
}

bool CRnLAmmoDef::AddPenetrationType(const char* pszAmmoType, int penWood, int penMetal, int penConcrete, int penFlesh, int penDirt, int penGlass, int penFoliage )
{
	return AddPenetrationType( Index( pszAmmoType ), penWood, penMetal, penConcrete, penFlesh, penDirt, penGlass, penFoliage );
}

bool CRnLAmmoDef::AddPenetrationType(int iAmmoID, int penWood, int penMetal, int penConcrete, int penFlesh, int penDirt, int penGlass, int penFoliage )
{
	if (iAmmoID >= MAX_AMMO_TYPES || iAmmoID < 0 )
		return false;

	m_PenetrationType[iAmmoID].wood = penWood;
	m_PenetrationType[iAmmoID].metal = penMetal;
	m_PenetrationType[iAmmoID].concrete = penConcrete;
	m_PenetrationType[iAmmoID].flesh = penFlesh;
	m_PenetrationType[iAmmoID].dirt = penDirt;
	m_PenetrationType[iAmmoID].glass = penGlass;
	m_PenetrationType[iAmmoID].foliage = penFoliage;

	return true;
}

float CRnLAmmoDef::GetPenetrationDistance(const char* pszAmmoType, unsigned short surfaceProp)
{
	return GetPenetrationDistance( Index( pszAmmoType ), surfaceProp);
}

float CRnLAmmoDef::GetPenetrationDistance(int iAmmoID, unsigned short surfaceProp)
{
	switch (surfaceProp)
	{
	case CHAR_TEX_CONCRETE:
		return (float)m_PenetrationType[iAmmoID].concrete;
	case CHAR_TEX_METAL:
		return (float)m_PenetrationType[iAmmoID].metal;
	case CHAR_TEX_DIRT:
		return (float)m_PenetrationType[iAmmoID].dirt;
	case CHAR_TEX_WOOD:
		return (float)m_PenetrationType[iAmmoID].wood;
	case CHAR_TEX_GLASS:
		return (float)m_PenetrationType[iAmmoID].glass;
	case CHAR_TEX_FLESH:
		return (float)m_PenetrationType[iAmmoID].flesh;
	case CHAR_TEX_VENT:
		return (float)m_PenetrationType[iAmmoID].metal;
	case CHAR_TEX_GRATE:
		return (float)m_PenetrationType[iAmmoID].wood;
	case CHAR_TEX_TILE:
		return (float)m_PenetrationType[iAmmoID].concrete;
	case CHAR_TEX_SLOSH:
		return (float)m_PenetrationType[iAmmoID].dirt;
	case CHAR_TEX_BLOODYFLESH:
		return (float)m_PenetrationType[iAmmoID].flesh;
	case CHAR_TEX_SAND:
		return (float)m_PenetrationType[iAmmoID].dirt;
	case CHAR_TEX_PLASTIC:
		return (float)m_PenetrationType[iAmmoID].wood;
	case CHAR_TEX_FOLIAGE:
		return (float)m_PenetrationType[iAmmoID].foliage;
	case CHAR_TEX_HEDGEROW:
		return (float)m_PenetrationType[iAmmoID].foliage;
	}
	
	return 0.0f;
}