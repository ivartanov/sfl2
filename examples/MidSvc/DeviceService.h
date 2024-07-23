#pragma once

class CDeviceService: public CServiceBaseT< CDeviceService, SERVICE_ACCEPT_STOP |
                                                            SERVICE_ACCEPT_HARDWAREPROFILECHANGE >
{
	SFL_DECLARE_SERVICECLASS_FRIENDS(CDeviceService)

	SFL_BEGIN_CONTROL_MAP_EX(CDeviceService)
		SFL_HANDLE_CONTROL_STOP()
		SFL_HANDLE_CONTROL_EX( SERVICE_CONTROL_DEVICEEVENT, OnDeviceChange )
	SFL_END_CONTROL_MAP_EX()
	
	DWORD OnStop(DWORD& dwWin32Err, DWORD& dwSpecificErr, BOOL& bHandled);
	DWORD OnDeviceChange(DWORD& dwState, DWORD& dwWin32Err, DWORD& dwSpecificErr, BOOL& bHandled, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
	void LogEvent(DWORD dwEvent, LPVOID lpParam);

#if defined(_MSC_VER) && (_MSC_VER < 1300)
public:
#endif
	LPVOID GetServiceContext() { return this; }
	BOOL InitInstance(DWORD dwArgc, LPTSTR* lpszArgv, DWORD& dwSpecificErr);

	CDeviceService();
	virtual ~CDeviceService();

private:
	HDEVNOTIFY m_hDevNotify;
	LPTSTR m_logfile;
};
