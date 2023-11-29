#ifndef RNL_TASK_PERIMETER_H
#define RNL_TASK_PERIMETER_H

#ifdef _WIN32
#pragma once
#endif

#include "rnl_task_count_base.h"

#ifdef CLIENT_DLL
	#define CRnLTaskCapturePerimeter C_RnLTaskCapturePerimeter
#endif

class CRnLTaskCapturePerimeter : public CRnLTaskCountBase
{
public:
	//obligatory declare class
	DECLARE_CLASS( CRnLTaskCapturePerimeter, CRnLTaskCountBase);
	DECLARE_NETWORKCLASS() ;
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CRnLTaskCapturePerimeter();
	~CRnLTaskCapturePerimeter() override = default;

	RnLTaskType	GetTaskType() override { return RNL_TASK_TYPE_CAPTURE; }

#ifndef CLIENT_DLL
	void Spawn( void ) override;
	virtual void Think() override;
#endif

private:
#ifndef CLIENT_DLL
	float m_flCaptureDuration;
	float m_flCaptureTimeRemaining;
#endif
	CNetworkVar( int, m_iCaptureRadius );
};

#endif //RNL_TASK_PERIMETER_H