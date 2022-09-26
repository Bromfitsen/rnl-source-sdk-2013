//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef IRNLOBJECTIVE_H
#define IRNLOBJECTIVE_H
#ifdef _WIN32
#pragma once
#endif

#include "rnl_itask.h"

typedef enum eRnLObjectiveState
{
	RNL_OBJECTIVE_INVALID = -1,
	//Controlled By
	RNL_OBJECTIVE_NEUTRAL = 0,
	RNL_OBJECTIVE_ALLIED_CONTROLLED,
	RNL_OBJECTIVE_AXIS_CONTROLLED,
	RNL_OBJECTIVE_STATES_MAX,

} RnLObjectiveState;

extern const char *ObjectiveStateToName( int state );

abstract_class IRnLObjective
{
public:
	virtual int				GetTaskCount( void ) = 0;
	virtual IRnLTask*		GetTask( int index ) = 0;
	virtual void			OnTaskStateChanged( IRnLTask* pTask ) = 0;

	virtual bool			IsPrimary( void ) = 0;
	virtual int				GetObjectiveState() = 0;
	virtual int				GetObjectivePrevState() = 0;
	virtual const char*		GetObjectiveName() = 0;
	virtual bool			IsObjectiveVisibleOnMap() = 0;

	virtual void			Reset( void ) = 0;
};


#endif // IRNLOBJECTIVE_H