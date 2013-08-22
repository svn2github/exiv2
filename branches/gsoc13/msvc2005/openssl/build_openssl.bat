@echo off
echo  +++
echo  +++ building %* +++
echo  +++

setlocal ENABLEEXTENSIONS

set PLATFORM=%1
set CONFIG=%2
set OutDir=%3
set VCInstallDir=%4
set ProjectDir=%5
set SolutionDir=%6
set Action=%7

echo  +++ PLATFORM     = %PLATFORM%
echo  +++ CONFIG       = %CONFIG%
echo  +++ OutDir       = %OutDir%
echo  +++ VCInstallDir = %VCInstallDir%
echo  +++ ProjectDir   = %ProjectDir%
echo  +++ SolutionDir  = %SolutionDir%
echo  +++ Action       = %Action%
echo  +++
echo.

perl --version > nul 2> nul
if ERRORLEVEL 1 (
  echo perl is not available
  goto ERROR
)

if NOT EXIST %PROJECTDIR%\..\..\..\openssl (
  echo %PROJECTDIR%\..\..\..\openssl does not exist
  goto ERROR
)

@echo on
cd %PROJECTDIR%\..\..\..\openssl

set VCVARS=vcvars32.bat
set VCCONFIG=VC-WIN32
set DO=ms\do_ms.bat
if %PLATFORM% == x64 (
    set VCCONFIG=VC-WIN64A
    set VCVARS=x86_amd64\vcvarsx86_amd64.bat
    SET DO=ms\do_win64a.bat
)
echo call %VCInstallDir%\bin\%VCVARS%

set SHARED=shared
if %CONFIG% == Debug   set SHARED=no-shared
if %CONFIG% == Release set SHARED=no-shared

set MAKEFILE=ms\ntdll.mak
set BUILD=0
if %ACTION% == rebuild set BUILD=1
if NOT EXIST %SolutionDir%bin\%PLATFORM%\%CONFIG%\ssleay32.lib set BUILD=1
if NOT EXIST %SolutionDir%bin\%PLATFORM%\%CONFIG%\libeay32.lib set BUILD=1
if %BUILD% == 0 GOTO POSTPROCESS

nmake -f %MAKEFILE% clean
nmake -f %MAKEFILE% 
nmake -f %MAKEFILE% install
nmake -f %MAKEFILE% clean
copy/y %ProjectDir%build\%PLATFORM%\%CONFIG%\lib\*.lib %SolutionDir%bin\%PLATFORM%\%CONFIG%\
if %CONFIG% == ReleaseDLL copy/y %ProjectDir%build\%PLATFORM%\%CONFIG%\bin\*.dll %SolutionDir%bin\%PLATFORM%\%CONFIG%\
if %CONFIG% == DebugDLL   copy/y %ProjectDir%build\%PLATFORM%\%CONFIG%\bin\*.dll %SolutionDir%bin\%PLATFORM%\%CONFIG%\

:POSTPROCESS

rem Force Release and Debug to adopt DLL libraries
rem if %CONFIG% == Release    copy/y %ProjectDir%build\%PLATFORM%\%CONFIG%DLL\bin\*.dll %SolutionDir%bin\%PLATFORM%\%CONFIG%\
rem if %CONFIG% == Debug      copy/y %ProjectDir%build\%PLATFORM%\%CONFIG%DLL\bin\*.dll %SolutionDir%bin\%PLATFORM%\%CONFIG%\
rem if %CONFIG% == Release    copy/y %ProjectDir%build\%PLATFORM%\%CONFIG%DLL\lib\*.lib %SolutionDir%bin\%PLATFORM%\%CONFIG%\
rem if %CONFIG% == Debug      copy/y %ProjectDir%build\%PLATFORM%\%CONFIG%DLL\lib\*.lib %SolutionDir%bin\%PLATFORM%\%CONFIG%\
rem if %CONFIG% == Release    copy/y %SolutionDir%bin\%PLATFORM%\%CONFIG%\libssh.*      %SolutionDir%bin\%PLATFORM%\%CONFIG%\
rem if %CONFIG% == Debug      copy/y %SolutionDir%bin\%PLATFORM%\%CONFIG%\libssh.*      %SolutionDir%bin\%PLATFORM%\%CONFIG%\

set ERRORLEVEL=0
goto EOF

:ERROR
set ERRORLEVEL=911

:EOF
rem That's all Folks
rem ----------------
