//=============================================================================//
// Purpose:		Holds definitions and functions for bullet penetration
// Author:		Toni Kreska
//=============================================================================//

#include "cbase.h"
#include "decals.h"
#include "rnl_penetration.h"
#include "tier0/memdbgon.h"

bool CPenetration::GetDeflection(int wepID, unsigned short surfaceProp, float& flAng )
{
	switch (surfaceProp)
	{
	case CHAR_TEX_TILE:
	case CHAR_TEX_CONCRETE:
		flAng = m_DeflectionType[wepID].concreteAng;
		return m_DeflectionType[wepID].concrete;
	
	case CHAR_TEX_VENT:
	case CHAR_TEX_METAL:
		flAng = m_DeflectionType[wepID].metalAng;
		return m_DeflectionType[wepID].metal;
	
	case CHAR_TEX_FOLIAGE:
	case CHAR_TEX_SAND:
	case CHAR_TEX_SLOSH:
	case CHAR_TEX_DIRT:
		flAng = m_DeflectionType[wepID].dirtAng;
		return m_DeflectionType[wepID].dirt;
	
	case CHAR_TEX_WOOD:
	case CHAR_TEX_PLASTIC:
	case CHAR_TEX_GRATE:
	case CHAR_TEX_HEDGEROW:
		flAng = m_DeflectionType[wepID].woodAng;
		return m_DeflectionType[wepID].wood;
	
	case CHAR_TEX_GLASS:
		flAng = m_DeflectionType[wepID].glassAng;
		return m_DeflectionType[wepID].glass;
	
	case CHAR_TEX_BLOODYFLESH:
	case CHAR_TEX_FLESH:
		flAng = m_DeflectionType[wepID].fleshAng;
		return m_DeflectionType[wepID].flesh;
	}

	return false;
}

bool CPenetration::AddDeflectionType(int wepID, bool wood, float woodAng, bool metal, float metalAng, bool concrete, float concreteAng,
		bool flesh, float fleshAng, bool dirt, float dirtAng, bool glass, float glassAng )
{
	if (wepID >= MAX_PENETRATION_TYPES || wepID < 0 )
		return false;

	m_DeflectionType[wepID].wood = wood;
	m_DeflectionType[wepID].woodAng = woodAng;
	m_DeflectionType[wepID].metal = metal;
	m_DeflectionType[wepID].metalAng = metalAng;
	m_DeflectionType[wepID].concrete = concrete;
	m_DeflectionType[wepID].concreteAng = concreteAng;
	m_DeflectionType[wepID].flesh = flesh;
	m_DeflectionType[wepID].fleshAng = fleshAng;
	m_DeflectionType[wepID].dirt = dirt;
	m_DeflectionType[wepID].dirtAng = dirtAng;
	m_DeflectionType[wepID].glass = glass;
	m_DeflectionType[wepID].glassAng = glassAng;

	return true;
}

bool CPenetration::AddPenetrationType(int wepID, int penWood, int penMetal, int penConcrete, int penFlesh, int penDirt, int penGlass )
{
	if (wepID >= MAX_PENETRATION_TYPES || wepID < 0 )
		return false;

	m_PenetrationType[wepID].wood = penWood;
	m_PenetrationType[wepID].metal = penMetal;
	m_PenetrationType[wepID].concrete = penConcrete;
	m_PenetrationType[wepID].flesh = penFlesh;
	m_PenetrationType[wepID].dirt = penDirt;
	m_PenetrationType[wepID].glass = penGlass;

	return true;
}

float CPenetration::getDistance(int wepID, unsigned short surfaceProp)
{
	switch (surfaceProp)
	{
	case CHAR_TEX_CONCRETE:
		return (float)m_PenetrationType[wepID].concrete;
	case CHAR_TEX_METAL:
		return (float)m_PenetrationType[wepID].metal;
	case CHAR_TEX_DIRT:
		return (float)m_PenetrationType[wepID].dirt;
	case CHAR_TEX_WOOD:
		return (float)m_PenetrationType[wepID].wood;
	case CHAR_TEX_GLASS:
		return (float)m_PenetrationType[wepID].glass;
	case CHAR_TEX_FLESH:
		return (float)m_PenetrationType[wepID].flesh;
	case CHAR_TEX_VENT:
		return (float)m_PenetrationType[wepID].metal;
	case CHAR_TEX_GRATE:
		return (float)m_PenetrationType[wepID].wood;
	case CHAR_TEX_TILE:
		return (float)m_PenetrationType[wepID].concrete;
	case CHAR_TEX_SLOSH:
		return (float)m_PenetrationType[wepID].dirt;
	case CHAR_TEX_BLOODYFLESH:
		return (float)m_PenetrationType[wepID].flesh;
	case CHAR_TEX_FOLIAGE:
		return (float)m_PenetrationType[wepID].dirt;
	case CHAR_TEX_SAND:
		return (float)m_PenetrationType[wepID].dirt;
	case CHAR_TEX_PLASTIC:
		return (float)m_PenetrationType[wepID].wood;
	case CHAR_TEX_HEDGEROW:
		return (float)m_PenetrationType[wepID].wood;
	}
	
	return 0.0f;
}
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CPenetration::CPenetration(void)
{
	// Start with an index of 1.  Client assumes 0 is an invalid ammo type
	m_nPenIndex = 1;
	memset( m_PenetrationType, 0, sizeof( m_PenetrationType ) );
}

CPenetration::~CPenetration( void )
{
}


