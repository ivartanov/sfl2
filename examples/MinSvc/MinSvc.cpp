#include <windows.h>
#include "SflBase.h"


class CMyService: public CServiceBaseT<CMyService, SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_PAUSE_CONTINUE>
{
	SFL_DECLARE_SERVICECLASS_FRIENDS(CMyService)

		SFL_BEGIN_CONTROL_MAP(CMyService)
			SFL_HANDLE_CONTROL_STOP()
			SFL_HANDLE_CONTROL_PAUSE()
			SFL_HANDLE_CONTROL_CONTINUE()
		SFL_END_CONTROL_MAP()

	DWORD OnStop(DWORD& /*dwWin32Err*/, DWORD& /*dwSpecificErr*/, BOOL& bHandled )
	{
		bHandled = TRUE;
		return SERVICE_STOPPED;
	}
	DWORD OnPause(DWORD& /*dwWin32Err*/, DWORD& /*dwSpecificErr*/, BOOL& bHandled )
	{
		bHandled = TRUE;
		return SERVICE_PAUSED;
	}
	DWORD OnContinue(DWORD& /*dwWin32Err*/, DWORD& /*dwSpecificErr*/, BOOL& bHandled )
	{
		bHandled = TRUE;
		return SERVICE_RUNNING;
	}

};

SFL_BEGIN_SERVICE_MAP(CSimpleServiceApp)
	SFL_SERVICE_ENTRY2(CMyService, 0, "My Service")
SFL_END_SERVICE_MAP()
