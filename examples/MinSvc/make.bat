@echo off

set SFLDIR=..\..\include
set DBGOPT=
set BLDCMD=0
set BLDTYPE=%1

if "%1" == "clean" (
 call :cleanup
 exit /b
) else if "%1" == "debug" (
  set DBGOPT=/Zi
  set BLDCMD=1
) else if "%1" == "release" (
  set BLDCMD=1
) else if ".%1" == "." (
  rem Debug implied
  set DBGOPT=/Zi
  set BLDCMD=1
  set BLDTYPE=debug
)

if %BLDCMD% == 1 (
  call :build %BLDTYPE%
  exit /b
) else (
  echo Syntax is wrong.
  exit /b
)

:cleanup
del /Q *.obj *.exe *.pdb *.ilk
exit /b

:build
echo Building %1...
cl MinSvc.cpp /MD /EHsc /I%SFLDIR% %DBGOPT% /link /subsystem:console
exit /b
