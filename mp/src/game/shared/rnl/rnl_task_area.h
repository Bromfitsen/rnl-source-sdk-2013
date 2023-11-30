#ifndef RNL_TASK_AREA_H
#define RNL_TASK_AREA_H

#ifdef _WIN32
#pragma once
#endif

#include "rnl_task_count_base.h"

#ifdef CLIENT_DLL
	#define CRnLTaskCaptureArea C_RnLTaskCaptureArea
#endif

class CRnLTaskCaptureArea : public CRnLTaskCountBase
{
public:
	//obligatory declare class
	DECLARE_CLASS( CRnLTaskCaptureArea, CRnLTaskCountBase);
	DECLARE_NETWORKCLASS() ;
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CRnLTaskCaptureArea();
	~CRnLTaskCaptureArea() = default;

	RnLTaskType	GetTaskType() OVERRIDE { return RNL_TASK_TYPE_CAPTURE; }

#ifndef CLIENT_DLL
	void Spawn( void ) OVERRIDE;
	void Think() OVERRIDE;

	void PostClientActive( void ) OVERRIDE;
	void UpdateOnRemove( void ) OVERRIDE;

	void StartTouch(CBaseEntity *pOther) OVERRIDE;
	void EndTouch(CBaseEntity *pOther) OVERRIDE;
#endif

private:
#ifndef CLIENT_DLL
	float m_flCaptureDuration;
	float m_flCaptureTimeRemaining;

	// Entities currently being touched by this trigger
	CUtlVector< EHANDLE >	m_hTouchingEntities;
#endif
};

#endif //RNL_TASK_AREA_H