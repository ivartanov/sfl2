#include <windows.h>
#include <stdio.h>
#include <dbt.h>
#include <initguid.h>
#include <ioevent.h>
#include "SflBase.h"
#include "DeviceService.h"

CDeviceService::CDeviceService()
{
	m_hDevNotify = NULL;
	DWORD dwSize = MAX_PATH * sizeof(TCHAR);
	m_logfile = (LPTSTR)LocalAlloc(LPTR, dwSize );
	GetModuleFileName( GetModuleHandle(0), m_logfile, dwSize );
	LPTSTR p = _tcsrchr(m_logfile, '.');
	_tcscpy( p, TEXT("_device.log") );
}

CDeviceService::~CDeviceService()
{
	LocalFree(m_logfile);
}

DWORD CDeviceService::OnStop(DWORD& dwWin32Err, DWORD& dwSpecificErr, BOOL& bHandled)
{
	if(m_hDevNotify)
		UnregisterDeviceNotification(m_hDevNotify);
	LogEvent(-1, NULL);
	bHandled =  TRUE;
	return SERVICE_STOPPED;
}

BOOL CDeviceService::InitInstance(DWORD dwArgc, LPTSTR* lpszArgv, DWORD& dwSpecificErr)
{
	TCHAR cdDrive[] = _T("\\\\.\\A:\\");
	DWORD drives = GetLogicalDrives();

	for( int i = 0; i < 27; i++ )
	{
		if( i == 26 )
			return FALSE; // no cd-rom

		if( (drives % 2) && (DRIVE_CDROM == GetDriveType( cdDrive + 4 )) )
		{
			cdDrive[6] = _T('\0');
			break;
		}
		cdDrive[4]++;
		drives >>= 1;
	}

	DEV_BROADCAST_HANDLE nf = {0};
	nf.dbch_size = sizeof(nf);
	nf.dbch_devicetype = DBT_DEVTYP_HANDLE;
	nf.dbch_handle     = CreateFile( cdDrive, GENERIC_READ, FILE_SHARE_READ, 
                                     NULL, OPEN_EXISTING, 0, NULL );

	if( INVALID_HANDLE_VALUE == nf.dbch_handle )
	{
		dwSpecificErr = GetLastError();
		return FALSE;
	}

	m_hDevNotify = RegisterDeviceNotification( m_status.GetHandle(), 
                                               (LPVOID)&nf,
                                               DEVICE_NOTIFY_SERVICE_HANDLE );
	if(!m_hDevNotify)
	{
		dwSpecificErr = GetLastError();
		return FALSE;
	}

	LogEvent(0, cdDrive);
	return TRUE;
}

DWORD CDeviceService::OnDeviceChange(DWORD& dwState, DWORD& dwWin32Err, DWORD& dwSpecificErr, BOOL& bHandled, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
	LogEvent(dwEventType, lpEventData);
	bHandled = TRUE;
	return NO_ERROR;
}

void CDeviceService::LogEvent(DWORD dwEvent, LPVOID lpParam)
{
	char szLogFile[MAX_PATH] = {0};
#if UNICODE
	WideCharToMultiByte(CP_ACP, 0, m_logfile, -1, szLogFile, MAX_PATH, NULL, NULL);
#else
	strncpy(szLogFile, m_logfile, MAX_PATH);
#endif
	SYSTEMTIME st;
	GetLocalTime(&st);
	PDEV_BROADCAST_HDR phdr = (PDEV_BROADCAST_HDR)lpParam;
	FILE* fLog = fopen( szLogFile, "a+" );
	switch(dwEvent)
	{
	case 0:
        fprintf(fLog, "%02d.%02d.%04d-%02d:%02d:%02d * START * (%s)\n", 
                st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, (LPCTSTR)lpParam );
		break;
	case DWORD(-1):
        fprintf(fLog, "%02d.%02d.%04d-%02d:%02d:%02d * STOP *\n", 
                st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond );
		break;
	default:
		{
#define CASE_EVENT(x) case x: pszEvent = #x; break;
			LPCSTR pszEvent = NULL;
			switch(dwEvent)
			{
			CASE_EVENT(DBT_DEVICEARRIVAL)
			CASE_EVENT(DBT_DEVICEREMOVECOMPLETE)
			CASE_EVENT(DBT_CUSTOMEVENT)
			}

			if(DBT_CUSTOMEVENT == dwEvent && DBT_DEVTYP_HANDLE == phdr->dbch_devicetype)
			{
				PDEV_BROADCAST_HANDLE ph = (PDEV_BROADCAST_HANDLE)lpParam;
				LPCSTR szEventGuid = NULL;

#define GUID_IS(x) if(IsEqualGUID(ph->dbch_eventguid, x)) szEventGuid = #x;

				GUID_IS(GUID_IO_MEDIA_ARRIVAL)          else
				GUID_IS(GUID_IO_MEDIA_REMOVAL)          else
				GUID_IS(GUID_IO_VOLUME_CHANGE)          else
				GUID_IS(GUID_IO_VOLUME_DISMOUNT)        else
				GUID_IS(GUID_IO_VOLUME_DISMOUNT_FAILED) else
				GUID_IS(GUID_IO_VOLUME_LOCK)            else
				GUID_IS(GUID_IO_VOLUME_LOCK_FAILED)     else
				GUID_IS(GUID_IO_VOLUME_MOUNT)           else
				GUID_IS(GUID_IO_VOLUME_NAME_CHANGE)     else
				GUID_IS(GUID_IO_VOLUME_PHYSICAL_CONFIGURATION_CHANGE) else
				GUID_IS(GUID_IO_VOLUME_UNLOCK)

                fprintf(fLog, "%02d.%02d.%04d-%02d:%02d:%02d * EvType = %d (%s), EventGUID = %s \n", 
                        st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond,
                        dwEvent, pszEvent, szEventGuid);
			}
		}
	}
	fclose(fLog);
}
