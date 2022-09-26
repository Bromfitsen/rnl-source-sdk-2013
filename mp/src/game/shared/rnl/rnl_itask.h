//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef IRNLTASK_H
#define IRNLTASK_H
#ifdef _WIN32
#pragma once
#endif

typedef enum eRnLTaskType
{
	RNL_TASK_TYPE_UNKNOWN = 0,
	RNL_TASK_TYPE_CAPTURE,
	RNL_TASK_TYPE_DESTROY,
	//Must Come Up With More Tasks
	RNL_TASK_TYPE_DANCE,

	RNL_TASK_TYPE_MAX,

} RnLTaskType;

typedef enum eRnLTaskState
{
	RNL_TASK_INVALID = -1,
	//Controlled By
	RNL_TASK_NEUTRAL = 0,

	//Allied Controls and Captures
	RNL_TASK_ALLIED_STATES_MIN,
	RNL_TASK_ALLIED_CONTROLLED,
	RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL,
	RNL_TASK_ALLIED_CAPTURING_FROM_NEUTRAL_BLOCKED,
	RNL_TASK_ALLIED_CAPTURING_FROM_AXIS,
	RNL_TASK_ALLIED_CAPTURING_FROM_AXIS_BLOCKED,
	RNL_TASK_ALLIED_STATES_MAX,

	//Axis Controls and Captures
	RNL_TASK_AXIS_STATES_MIN,
	RNL_TASK_AXIS_CONTROLLED,
	RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL,
	RNL_TASK_AXIS_CAPTURING_FROM_NEUTRAL_BLOCKED,
	RNL_TASK_AXIS_CAPTURING_FROM_ALLIED,
	RNL_TASK_AXIS_CAPTURING_FROM_ALLIED_BLOCKED,
	RNL_TASK_AXIS_STATES_MAX,

	RNL_TASK_STATES_MAX,

} RnLTaskState;

extern const char *TaskStateToName( int state );

abstract_class IRnLTask
{
public:
	//Can this be captured
	virtual bool			AreRequirementsMet( int iTeam ) = 0;

	// This returns the current state of our Task
	virtual int				GetTaskState() = 0;
	// This returns the previous state of our Task
	virtual int				GetTaskPrevState() = 0;

	// This returns the type of Task
	virtual RnLTaskType		GetTaskType() = 0;

	// This returns the center of the Task and is used to show it on the map
	virtual Vector			GetTaskOrigin() = 0;

	// Is this Task visible on the map
	virtual bool			IsTaskVisibleOnMap() = 0;

	// Handle reseting a task to it's default state
	virtual void			Reset( void ) = 0;

	virtual void			OnStateChange( int iState, int iTeam ) = 0;
};


#endif // IRNLTASK_H