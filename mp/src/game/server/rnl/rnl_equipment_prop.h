/********************************************************************
 Created for the Resistance and Liberation Mod for the Source-Engine
 Purpose: The equipment prop, that is attached to the player to show
 his current equipment state

 Created 29/4/05 by NuclearFriend AKA Jonathan Murphy
********************************************************************/

#ifndef RNL_EQUIPPROP_H
#define RNL_EQUIPPROP_H

#include "baseanimating.h"

class CRnLEquipmentProp : public CBaseAnimating
{
public:
	DECLARE_CLASS( CRnLEquipmentProp, CBaseAnimating );
	DECLARE_SERVERCLASS();

	CRnLEquipmentProp();
	~CRnLEquipmentProp();
};



class CRnLEquipmentParachute : public CRnLEquipmentProp
{
public:
	DECLARE_CLASS( CRnLEquipmentParachute, CRnLEquipmentProp );

	CRnLEquipmentParachute();
	~CRnLEquipmentParachute();

	void Precache() OVERRIDE;
	void Spawn( void ) OVERRIDE;

	void Deploy( void );

private:
	bool	m_bIsDeployed;
};

class CRnLEquipmentRadio : public CRnLEquipmentProp
{
public:
	DECLARE_CLASS( CRnLEquipmentRadio, CRnLEquipmentProp );
	
	CRnLEquipmentRadio();
	~CRnLEquipmentRadio();

	void Precache();
	void Spawn( void );

};

#endif //RNL_EQUIPPROP_H