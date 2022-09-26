//=============================================================================//
//
// Purpose:		Holds definitions and functions for bullet penetration
// Author:		Toni Kreska
//
//=============================================================================//

#ifndef RNL_PENETRATION_H
#define RNL_PENETRATION_H

#ifdef _WIN32
#pragma once
#endif


struct Pen_t 
{
	int					wood;
	int					metal;
	int					concrete;
	int					flesh;
	int					dirt;
	int					glass;
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
};

#define	MAX_PENETRATION_TYPES	32

//=============================================================================
//	>> CPenetration
//=============================================================================
class CPenetration
{

public:
	Pen_t				m_PenetrationType[MAX_PENETRATION_TYPES];
	Deflect_t			m_DeflectionType[MAX_PENETRATION_TYPES];
	int					m_nPenIndex;
	int					m_nDefIndex;
	
	bool				GetDeflection(int wepID, unsigned short surfaceProp, float& flAng );
	bool				AddDeflectionType(int wepID, bool wood, float woodAng, bool metal, float mentAng, bool concrete, float concreteAng,
		bool flesh, float fleshAng, bool dirt, float dirtAng, bool glass, float glassAng );

	float				getDistance(int wepID, unsigned short surfaceProp);
	bool				AddPenetrationType(int wepID, int penWood, int penMetal, int penConcrete, int penFlesh, int penDirt, int penGlass );

	CPenetration( void );
	virtual ~CPenetration( void );

};

CPenetration* GetPenetrationDef();

#endif //RNL_PENETRATION_H