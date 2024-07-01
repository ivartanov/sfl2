@echo off

set SFLDIR=..\..\include

if "%1" == "clean" (
  call :cleanup
  exit
)

call :build

:cleanup
del /Q *.obj *.exe
exit

:build
echo "Building ..."
cl MinSvc.cpp /MD /EHsc /I%SFLDIR% /link /subsystem:console
exit

