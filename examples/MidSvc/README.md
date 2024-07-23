# MidSvc - Service application capable of receiving hardware events

## Overview

This example demonstrates a service application of NT5 type (Windows2000 or higher) unlike to NT4 type used in [MinSvc](../MinSvc/).

The hardware events will come from the first CD-ROM drive found in your system under test, indicating CD volume mounting/unmounting.

To avoid hassle with communication to session 0, the event arrival will be logged to a file.

## Making executable

To build the app binary you need to run Visual Studio Command Prompt window of the bitness x86 or x64 that you require, change working directory to MidSvc folder and run the command:

```
> make
```

As a result, `MinSvc.exe` application will be created in the same folder.

> [!NOTE] 
> Running bare `make` will build a debug version, same as `make debug`. If you want to build a release version, you call `make release`

> [!TIP]
> To get an idea how to start playing with the example please see [the other README doc](../MinSvc/README.md#bring-your-service-to-action)

## Application structure

Service map is placed to [MidSvc.cpp](MidSvc.cpp) and declares a single service entry implemented by `CDeviceService` class. This time the service name is defined as a string resource.

In its turn [DeviceService.h](DeviceService.h) declares `CDeviceService` class, where service control map is declared along with control handlers.

The class implementation is in [DeviceService.cpp](DeviceService.cpp) where special point of interest will be `CDeviceService::InitInstance` method that was not used in [MinSvc](../MinSvc/) example.

## Enabling hardware events

The enabling is done in several coherent steps:

- The service first of all declares `SERVICE_ACCEPT_HARDWAREPROFILECHANGE` code.

```C++
class CDeviceService: public CServiceBaseT< CDeviceService, SERVICE_ACCEPT_STOP |
                                                            SERVICE_ACCEPT_HARDWAREPROFILECHANGE >
```

- NT5 service type must be used to comply with the acceptance code above, otherwise no HW events will be coming. In SFL the NT5 type is declared by using `SFL_BEGIN_CONTROL_MAP_EX` macro; internally it results in [RegisterServiceCtrlHandlerEx](https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-registerservicectrlhandlerexa) Win32 API called. The events will come with `SERVICE_CONTROL_DEVICEEVENT` control code.

```C++
	SFL_BEGIN_CONTROL_MAP_EX(CDeviceService)
		SFL_HANDLE_CONTROL_STOP()
		SFL_HANDLE_CONTROL_EX( SERVICE_CONTROL_DEVICEEVENT, OnDeviceChange )
	SFL_END_CONTROL_MAP_EX()
```

- Finally, [RegisterDeviceNotification](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerdevicenotificationa) must be called along with `DEVICE_NOTIFY_SERVICE_HANDLE` flag to indicate that the notification recipient is a service process, but not a window. It is logical to register notifications on the service initialization step to be able to receive notifications since the very service start.

## InitInstance

The method does all the required preparations, like detecting CD-ROM drive and registering it as the source for hardware notifications.

## OnStop

`CDeviceService::OnStop` is responsible for resource cleanup, i.e. unregistering notifications.

> [!NOTE]
> There is nothing like `ExitInstance` in SFL framework, so `OnStop` is the best place for releasing anything you previously obtained from the system.

## Expected work trace

The log file `MidSvc_device.log` will be created on the first start, located next to the service executable.

On CD volume mounting the arrival event type is sent to the service, and on ejecting the CD the removal event is sent.

```
24.07.2024-01:17:19 * START * (\\.\R:)
24.07.2024-01:18:01 * EvType = 32774 (DBT_CUSTOMEVENT), EventGUID = GUID_IO_MEDIA_ARRIVAL 
24.07.2024-01:19:13 * EvType = 32774 (DBT_CUSTOMEVENT), EventGUID = GUID_IO_MEDIA_REMOVAL 
24.07.2024-01:19:14 * EvType = 32774 (DBT_CUSTOMEVENT), EventGUID = GUID_IO_MEDIA_ARRIVAL 
24.07.2024-01:19:14 * EvType = 32774 (DBT_CUSTOMEVENT), EventGUID = GUID_IO_MEDIA_REMOVAL 
24.07.2024-01:19:26 * STOP *

```