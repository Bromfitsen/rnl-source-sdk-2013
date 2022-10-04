#include "cbase.h"
#include "rnl_task_count_base.h"
#include "rnl_gamerules.h"
#include "rnl_shareddefs.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(RnLTaskCountBase, DT_RnLTaskCountBase)
BEGIN_NETWORK_TABLE( CRnLTaskCountBase, DT_RnLTaskCountBase )
#ifndef CLIENT_DLL
	SendPropInt(	SENDINFO( m_iAlliesCountRequired ) ),
	SendPropInt(	SENDINFO( m_iAlliesCount ) ),
	SendPropInt(	SENDINFO( m_iAxisCountRequired ) ),
	SendPropInt(	SENDINFO( m_iAxisCount ) ),
#else
	RecvPropInt(	RECVINFO( m_iAlliesCountRequired ) ),
	RecvPropInt(	RECVINFO( m_iAlliesCount ) ),
	RecvPropInt(	RECVINFO( m_iAxisCountRequired ) ),
	RecvPropInt(	RECVINFO( m_iAxisCount )),
#endif
END_NETWORK_TABLE()

#ifndef CLIENT_DLL
	BEGIN_DATADESC( CRnLTaskCountBase )
		DEFINE_KEYFIELD( m_iInitialAxisCountRequired, FIELD_INTEGER, "axis_count_required" ),
		DEFINE_KEYFIELD( m_iInitialAlliedCountRequired, FIELD_INTEGER, "allied_count_required" ),
	END_DATADESC()
#endif

CRnLTaskCountBase::CRnLTaskCountBase()
{
	m_iAlliesCount = 0;
	m_iAxisCount = 0;

#ifndef CLIENT_DLL
	m_iAxisCountRequired = m_iInitialAxisCountRequired = 1;
	m_iAlliesCountRequired = m_iInitialAlliedCountRequired = 1;
#endif
}
	
void CRnLTaskCountBase::Spawn( void )
{
	BaseClass::Spawn();

	m_iAlliesCount = 0;
	m_iAxisCount = 0;

#ifndef CLIENT_DLL
	m_iAxisCountRequired = m_iInitialAxisCountRequired;
	m_iAlliesCountRequired = m_iInitialAlliedCountRequired;
#endif
}

void CRnLTaskCountBase::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();
}

#ifndef CLIENT_DLL
	void CRnLTaskCountBase::Activate( void )
	{
		BaseClass::Activate();

		m_iAlliesCount = 0;
		m_iAxisCount = 0;

		m_iAxisCountRequired = m_iInitialAxisCountRequired;
		m_iAlliesCountRequired = m_iInitialAlliedCountRequired;
	}
#endif
	
int CRnLTaskCountBase::GetTeamCount( int iTeam )
{
	if( iTeam == TEAM_ALLIES )
	{
		return m_iAlliesCount;
	}
	else if( iTeam == TEAM_AXIS )
	{
		return m_iAxisCount;
	}

	return 0;
}

int CRnLTaskCountBase::GetRequiredCount( int iTeam )
{
	if( iTeam == TEAM_ALLIES )
	{
		return m_iAlliesCountRequired;
	}
	else if( iTeam == TEAM_AXIS )
	{
		return m_iAxisCountRequired;
	}

	return 0;
}

void CRnLTaskCountBase::SetCount( int iTeam, int iCount )
{
	OnCountChanged( iTeam, iCount );

	if( iTeam == TEAM_ALLIES )
	{
		m_iAlliesCount = iCount;
	}
	else if( iTeam == TEAM_AXIS )
	{
		m_iAxisCount = iCount;
	}
}

void CRnLTaskCountBase::SetRequiredCount( int iTeam, int iCount )
{
	OnCountChanged( iTeam, iCount );

	if( iTeam == TEAM_ALLIES )
	{
		m_iAlliesCountRequired = iCount;
	}
	else if( iTeam == TEAM_AXIS )
	{
		m_iAxisCountRequired = iCount;
	}
}

void CRnLTaskCountBase::OnCountChanged( int iTeam, int iNewCount )
{
#ifndef CLIENT_DLL
	if( iTeam == TEAM_ALLIES )
	{
		if( m_iAlliesCount < iNewCount )
		{
			m_OnAlliedCountIncrease.FireOutput( this, this );
		}
		else if( m_iAlliesCount > iNewCount )
		{
			m_OnAlliedCountDecrease.FireOutput( this, this );
		}

		if( m_iAlliesCount < m_iAlliesCountRequired && iNewCount >= m_iAlliesCountRequired )
		{
			OnRequiredCountAcheived( TEAM_ALLIES );
		}
		else if( m_iAlliesCount >= m_iAlliesCountRequired && iNewCount < m_iAlliesCountRequired )
		{
			OnRequiredCountLost( TEAM_ALLIES );
		}

		m_OnAlliedCountChanged.FireOutput( this, this );
	}
	else if( iTeam == TEAM_AXIS )
	{
		if( m_iAxisCount < iNewCount )
		{
			m_OnAxisCountIncrease.FireOutput( this, this );
		}
		else if( m_iAxisCount > iNewCount )
		{
			m_OnAxisCountDecrease.FireOutput( this, this );
		}

		if( m_iAxisCount < m_iAxisCountRequired && iNewCount >= m_iAxisCountRequired )
		{
			OnRequiredCountAcheived( TEAM_AXIS );
		}
		else if( m_iAxisCount >= m_iAxisCountRequired && iNewCount < m_iAxisCountRequired )
		{
			OnRequiredCountLost( TEAM_AXIS );
		}

		m_OnAxisCountChanged.FireOutput( this, this );
	}
#endif
}