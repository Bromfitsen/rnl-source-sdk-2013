//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "../EventLog.h"
#include "KeyValues.h"

class CRnLEventLog : public CEventLog
{
private:
	typedef CEventLog BaseClass;

public:
	virtual ~CRnLEventLog() {};

public:
	bool PrintEvent( IGameEvent * event ) OVERRIDE	// override virtual function
	{
		if ( BaseClass::PrintEvent( event ) )
		{
			return true;
		}
	
		if ( Q_strcmp(event->GetName(), "sdk_") == 0 )
		{
			return PrintRnLEvent( event );
		}

		return false;
	}

protected:

	bool PrintRnLEvent( IGameEvent * event )	// print Mod specific logs
	{
		//const char * name = event->GetName() + Q_strlen("sdk_"); // remove prefix
		return false;
	}

};

CRnLEventLog g_RnLEventLog;

//-----------------------------------------------------------------------------
// Singleton access
//-----------------------------------------------------------------------------
IGameSystem* GameLogSystem()
{
	return &g_RnLEventLog;
}

