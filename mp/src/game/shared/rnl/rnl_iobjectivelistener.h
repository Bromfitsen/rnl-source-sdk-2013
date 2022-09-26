//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef RNL_IOBJECTIVELISTENER_H
#define RNL_IOBJECTIVELISTENER_H
#ifdef _WIN32
#pragma once
#endif


#include "rnl_iobjective.h"


abstract_class IRnLObjectiveListener
{
public:
	virtual void			RegisterObjective( IRnLObjective* pObjective ) = 0;
	virtual void			RemoveObjective( IRnLObjective* pObjective ) = 0;
	virtual IRnLObjective*	GetObjective( IRnLObjective* pCurrent ) = 0;
	//State is identified usually by one of the RnLObjectiveState_t enums
	virtual void			ObjectiveStateChange( IRnLObjective* pObjective ) = 0;
};


#endif // RNL_IOBJECTIVELISTENER_H