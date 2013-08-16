echo +++ building %* +++

setlocal ENABLEEXTENSIONS

set PLATFORM=%1
set CONFIG=%2
set OutDir=%3
set VCInstallDir=%4
set ProjectDir=%5

echo PLATFORM     = %PLATFORM%
echo CONFIG       = %CONFIG%
echo OutDir       = %OutDir%
echo VCInstallDir = %VCInstallDir%
echo ProjectDir   = %ProjectDir%

perl --version > nul 2> nul
if ERRORLEVEL 1 (
  echo perl is not available
  goto ERROR
)

if NOT EXIST %PROJECTDIR%\..\..\..\openssl (
  echo %PROJECTDIR%\..\..\..\openssl does not exist
  goto ERROR
)
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
if "%SHARED%" == "no-shared" set MAKEFILE=ms\nt.mak

echo cd %CD%
perl Configure %VCCONFIG% no-asm %SHARED% --prefix=%ProjectDir%\%PLATFORM%\%CONFIG%
call     %DO%
nmake -f %MAKEFILE% clean
nmake -f %MAKEFILE% 
nmake -f %MAKEFILE% install
nmake -f %MAKEFILE% clean
set ERRORLEVEL=0
goto EOF

:ERROR
set ERRORLEVEL=911

:EOF
rem That's all Folks
rem ----------------
