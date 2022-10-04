//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef RNL_MAPENTITIES_H
#define RNL_MAPENTITIES_H
#ifdef _WIN32
#pragma once
#endif

#include "mapentities.h"
#include "gameinterface.h"

class CRnLEntityFilter : public CMapLoadEntityFilter
{
public:
	CRnLEntityFilter(const char** filterEntityList);
	virtual bool ShouldCreateEntity( const char *pClassname );
	virtual CBaseEntity* CreateNextEntity( const char *pClassname );

protected:
	const char** m_pszPreserveEntityList;
};

class CRnLRespawnEntityFilter : public CRnLEntityFilter
{
public:
	CRnLRespawnEntityFilter(const char** filterEntityList);
	virtual bool ShouldCreateEntity( const char *pClassname );
	virtual CBaseEntity* CreateNextEntity( const char *pClassname );

public:
	int m_iIterator; // Iterator into g_MapEntityRefs.
};

bool UTIL_FindInList(const char** pStrings, const char* pToFind);

#endif //RNL_MAPENTITIES_H