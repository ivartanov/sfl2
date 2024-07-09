# MinSvc - minimal Windows service application

## Application composition

The application implements a very basic service behavior - starting, stopping, pausing and continuing (as we will see that later) - without doing anything useful. Due to this the service code is perfectly clean and easy to understand.

### Your service class

The code is free of any dependencies other than Win32 SDK and **SflBase.h** itself.

```C++
#include <windows.h>
#include "SflBase.h"
```

#### Class declaration

```C++
class CMyService: public CServiceBaseT<CMyService, SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_PAUSE_CONTINUE>
{
```

The second template parameter of `CServiceBaseT<>` defines the user control commands the service will be registered with and therefore able to react to. In this case the commands are _stop_, _pause_ and _continue_, and of cause _start_ command is implied by default.

> [!NOTE]
> For example, you may comment out the `SERVICE_ACCEPT_PAUSE_CONTINUE` constant and at runtime you see that your service is not eligible for pausing, and only option available is stopping. 

#### Control map

```C++
	SFL_BEGIN_CONTROL_MAP(CMyService)
		SFL_HANDLE_CONTROL_STOP()
		SFL_HANDLE_CONTROL_PAUSE()
		SFL_HANDLE_CONTROL_CONTINUE()
	SFL_END_CONTROL_MAP()
```

You can easily see the connection between service declaration and control command handling: the control map just confirms handling of all the commands listed in the above section.

In fact, each `SFL_HANDLE_CONTROL_XXX` entry declares corresponding `OnXxx` member function as a control handler.

#### Control handlers

```C++
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
```

A control handler has to explicitly confirm handling by setting `bHandled` parameter to TRUE. Return value here must be a service state code the service to switch to.

### Your service application class

```C++
SFL_BEGIN_SERVICE_MAP(CSimpleServiceApp)
	SFL_SERVICE_ENTRY2(CMyService, 0, "My Service")
SFL_END_SERVICE_MAP()
```

From the map you can see that your application includes the only service instance named "My Service" and implemented by `CMyService` class defined above.

## Making executable

To build the app binary you need to run Visual Studio Command Prompt window of the bitness x86 or x64 that you require, change working directory to MinSvc folder and run the command:

```
> make
```

As a result, `MinSvc.exe` application will be created in the same folder.

> [!NOTE] 
> Running bare `make` will build a debug version, same as `make debug`. If you want to build a release version, you call `make release`

## Bring your service to action

> [!TIP]
> Prior to registration please make sure there is no service in your system registered with the name "My Service"

SC.EXE application from Windows standard tools will be used for manipulations with the service.

> [!NOTE]
> SC.EXE must be run with Administrator privilege.

### Create service entry in the Registry

```
> sc create "My Service" binPath= <full path to MinSvc.exe>
[SC] CreateService SUCCESS
```

> [!WARNING]
> The space character after `binPath=` key does matter and therefore is mandatory!

Test the registration success by:

```
> sc query "My Service"
SERVICE_NAME: My Service
        TYPE               : 10  WIN32_OWN_PROCESS
        STATE              : 1  STOPPED
                                (NOT_STOPPABLE,NOT_PAUSABLE,IGNORES_SHUTDOWN)
        WIN32_EXIT_CODE    : 1077       (0x435)
        SERVICE_EXIT_CODE  : 0  (0x0)
        CHECKPOINT         : 0x0
        WAIT_HINT          : 0x0
```

The newly created service must be in `SERVICE_STOPPED` state.

### Start service

```
> sc start "My Service"
SERVICE_NAME: My Service
        TYPE               : 10  WIN32_OWN_PROCESS
        STATE              : 2  START_PENDING
                                (NOT_STOPPABLE,NOT_PAUSABLE,IGNORES_SHUTDOWN)
        WIN32_EXIT_CODE    : 0  (0x0)
        SERVICE_EXIT_CODE  : 0  (0x0)
        CHECKPOINT         : 0x0
        WAIT_HINT          : 0x7d0
        PID                : 2752
        FLAGS              :
```

Test the start success by:

```
> sc query "My Service"
SERVICE_NAME: My Service
        TYPE               : 10  WIN32_OWN_PROCESS
        STATE              : 4  RUNNING
                                (STOPPABLE,PAUSABLE,IGNORES_SHUTDOWN)
        WIN32_EXIT_CODE    : 0  (0x0)
        SERVICE_EXIT_CODE  : 0  (0x0)
        CHECKPOINT         : 0x0
        WAIT_HINT          : 0x0
```

The newly created service must be in `SERVICE_RUNNING` state.

### Pause service

```
> sc pause "My Service"
```

Test the pausing success by:

```
> sc query "My Service"
```

The service must be in `SERVICE_PAUSED` state.

### Continue service

```
> sc continue "My Service"
```

Test the continuation success by:

```
> sc query "My Service"
```

The service must be in `SERVICE_RUNNING` state.

### Stop service

```
> sc stop "My Service"
```

Test the stopping success by:

```
> sc query "My Service"
```

The service must be in `SERVICE_STOPPED` state.

### Delete service

```
> sc delete "My Service"
[SC] DeleteService SUCCESS
```

Test the deletion success by:

```
> sc query "My Service"
[SC] EnumQueryServicesStatus:OpenService FAILED 1060:

The specified service does not exist as an installed service.
```

The service must not exist to the moment.
