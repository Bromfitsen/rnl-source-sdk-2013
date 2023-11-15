//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: TF2 specific input handling
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "kbutton.h"
#include "input.h"

#include <vgui/IInput.h>
#include "vgui_controls/controls.h"

#include "c_rnl_player.h"

#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: TF Input interface
//-----------------------------------------------------------------------------
class CRnLInput : public CInput
{
public:
	typedef CInput BaseClass;
/*
	void CAM_Think( void );
	int CAM_IsThirdPerson();
	void CAM_GetCameraOffset( Vector& ofs );
*/
protected:
	void AdjustPitch( float speed, QAngle& viewangles );
	void AdjustYaw( float speed, QAngle& viewangles );
	void ApplyMouse( QAngle& viewangles, CUserCmd *cmd, float mouse_x, float mouse_y );
/*
	void CAM_ThirdpersonOverride( void );
*/
protected:
/*
	Vector m_vThirdPersonOverrideOffset;
	QAngle m_vThirdPersonOverrideAngle;
	Vector2D m_vThirdPersonOverrideLimits;
	int m_iThirdpersoOverridenClamp;
*/
};

static CRnLInput g_Input;

// Expose this interface
IInput *input = ( IInput * )&g_Input;
/*
void CRnLInput::CAM_Think( void )
{
	VPROF("CRnLInput::CAM_Think");
	//
	if( CBasePlayer::GetLocalPlayer() != NULL )
	{
		CRnLPlayer* pLocalPlayer = CRnLPlayer::GetLocalRnLPlayer();
		if( pLocalPlayer && pLocalPlayer->OverrideThirdPersonCamera() )
		{
			pLocalPlayer->GetThirdPersonOverride(m_vThirdPersonOverrideOffset, m_vThirdPersonOverrideAngle, m_vThirdPersonOverrideLimits, m_iThirdpersoOverridenClamp );
			return;
		}
	}

	BaseClass::CAM_Think();
}

int CRnLInput::CAM_IsThirdPerson()
{
	CRnLPlayer* pLocalPlayer = CRnLPlayer::GetLocalRnLPlayer();
	if( pLocalPlayer && pLocalPlayer->OverrideThirdPersonCamera() )
	{
		return true;
	}
	else
	{
		return BaseClass::CAM_IsThirdPerson();
	}
}

void CRnLInput::CAM_GetCameraOffset( Vector& ofs )
{
	CRnLPlayer* pLocalPlayer = CRnLPlayer::GetLocalRnLPlayer();
	if( pLocalPlayer && pLocalPlayer->OverrideThirdPersonCamera() )
	{
		ofs = m_vThirdPersonOverrideOffset;
	}
	else
	{
		BaseClass::CAM_GetCameraOffset( ofs );
	}
}
*/
void CRnLInput::AdjustPitch(float speed, QAngle& viewangles)
{
	CRnLPlayer* pLocalPlayer = CRnLPlayer::GetLocalRnLPlayer();
	if (pLocalPlayer)
	{
		pLocalPlayer->AdjustPitch(speed, viewangles);
		BaseClass::AdjustPitch(speed, pLocalPlayer->m_RnLLocal.w_angle);
	}
	else
	{
		BaseClass::AdjustPitch(speed, viewangles);
	}
}

void CRnLInput::AdjustYaw(float speed, QAngle& viewangles)
{
	CRnLPlayer* pLocalPlayer = CRnLPlayer::GetLocalRnLPlayer();
	if (pLocalPlayer)
	{
		pLocalPlayer->AdjustYaw(speed, viewangles);
		BaseClass::AdjustYaw(speed, pLocalPlayer->m_RnLLocal.w_angle);
	}
	else
	{
		BaseClass::AdjustYaw(speed, viewangles);
	}
}

//-----------------------------------------------------------------------------
// Purpose: ApplyMouse -- applies mouse deltas to CUserCmd
// Input  : viewangles - 
//			*cmd - 
//			mouse_x - 
//			mouse_y - 
//-----------------------------------------------------------------------------
void CRnLInput::ApplyMouse( QAngle& viewangles, CUserCmd *cmd, float mouse_x, float mouse_y )
{
	CRnLPlayer* pLocalPlayer = CRnLPlayer::GetLocalRnLPlayer();
	if( pLocalPlayer )
	{
		pLocalPlayer->PreMouseMove( viewangles );
	}

	BaseClass::ApplyMouse( viewangles, cmd, mouse_x, mouse_y );

	if( pLocalPlayer )
	{
		pLocalPlayer->PostMouseMove( viewangles );
	}
}