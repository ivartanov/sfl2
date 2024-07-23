// MidSvc.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include "SflBase.h"
#include "DeviceService.h"
#include "resource.h"

SFL_BEGIN_SERVICE_MAP(CSimpleServiceApp)
	SFL_SERVICE_ENTRY(CDeviceService, IDS_DEVICE_SERVICE)
SFL_END_SERVICE_MAP()