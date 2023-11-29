#ifndef RNL_TASK_DESTRUCTABLE_H
#define RNL_TASK_DESTRUCTABLE_H

#ifdef _WIN32
#pragma once
#endif

#include "rnl_task_base.h"

#ifdef CLIENT_DLL
	#define CRnLTaskDestructable C_RnLTaskDestructable
#else
	#include "rnl_player.h"
	class CRnLBaseExplosive;
#endif
class CRnLTaskDestructable : public CRnLTaskBase
{
public:
	//obligatory declare class
	DECLARE_CLASS( CRnLTaskDestructable, CRnLTaskBase );
	DECLARE_NETWORKCLASS() ;
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CRnLTaskDestructable();
	~CRnLTaskDestructable() override = default;

#ifdef CLIENT_DLL
	bool ShouldDraw( void ) override;
	void OnDataChanged( DataUpdateType_t type ) override;
#endif
	
	void Precache( void ) override;
	void Spawn( void ) override;
	void UpdateOnRemove() override;

	RnLTaskType	GetTaskType() override { return RNL_TASK_TYPE_DESTROY; }

#ifndef CLIENT_DLL
	virtual void Activate( void ) override;
	virtual int OnTakeDamage( const CTakeDamageInfo &info ) override;

	virtual bool CanAttachExplosive( CRnLPlayer* pPlayer );
	virtual bool AttachExplosive( CRnLBaseExplosive* pTnT  );
#endif

protected:
#ifndef CLIENT_DLL
	COutputEvent m_OnAxisDestroyed;
	COutputEvent m_OnAxisChargePlaced;

	COutputEvent m_OnAlliedDestroyed;
	COutputEvent m_OnAlliedChargePlaced;

	COutputEvent m_OnDestroyed;
	
	CRnLBaseExplosive* m_pAttachedTnT;
#endif
	CNetworkVar( bool, m_bDestroyed );
	CNetworkVar( bool, m_bEnabled );
	CNetworkVar( bool, m_bPlanted );
};

#endif //RNL_TASK_DESTRUCTABLE_H	