@echo off
rem --
rem build exiv2 library from Jenkins (or the command-line)

rem if NOT %label%==MSVC exit/b 0

if NOT DEFINED ACTION        set ACTION=/build
if NOT DEFINED COMPILER      set COMPILER=G++
if NOT DEFINED BuildEnv      set BuildEnv=native
if NOT DEFINED Builder       set Builder=2005

if NOT DEFINED release       set release=false
if NOT DEFINED debug         set debug=false
if NOT DEFINED static        set static=false
if NOT DEFINED dll           set dll=false
if NOT DEFINED x64           set x64=false
if NOT DEFINED Win32         set Win32=false

if NOT DEFINED tests         set tests=false
if NOT DEFINED teste         set teste=false
if NOT DEFINED testv         set testv=false
if NOT DEFINED testr         set testr=false

if NOT DEFINED expat         set expat=true
if NOT DEFINED zlib          set zlib=true
if NOT DEFINED curl          set curl=false
if NOT DEFINED libssh        set libssh=false
if NOT DEFINED openssl       set openssl=false
if %openssl%==true           set curl=true

if %ACTION%==/clean          set tests=false
if %ACTION%==/upgrade        set tests=false

if NOT DEFINED INCLUDE       set "INCLUDE=C:\Program Files (x86)\Microsoft Visual Studio 8\VC\ATLMFC\INCLUDE;C:\Program Files (x86)\Microsoft Visual Studio 8\VC\INCLUDE;C:\Program Files (x86)\Microsoft Visual Studio 8\VC\PlatformSDK\include;c:\Program Files (x86)\Microsoft Visual Studio .NET 2003\SDK\v1.1\include\;c:\home\rmills\dev\win32\boost\include\boost-1_42
if NOT DEFINED LIB           set "LIB=C:\Program Files (x86)\Microsoft Visual Studio 8\VC\ATLMFC\LIB;C:\Program Files (x86)\Microsoft Visual Studio 8\VC\LIB;C:\Program Files (x86)\Microsoft Visual Studio 8\VC\PlatformSDK\lib;C:\Program Files (x86)\Microsoft Visual Studio 8\SDK\v2.0\lib;c:\Program Files (x86)\Microsoft Visual Studio .NET 2003\SDK\v1.1\Lib\"
if NOT DEFINED LIBPATH       set "LIBPATH=C:\Windows\Microsoft.NET\Framework\v2.0.50727;C:\Program Files (x86)\Microsoft Visual Studio 8\VC\ATLMFC\LIB"
if NOT DEFINED VS80COMNTOOLS set "VS80COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\Tools\"

rem ----------------------------------------------
rem  set the build environment
call "%VS80COMNTOOLS%\..\..\Vc\bin\vcvars32.bat"

rem ----------------------------------------------
rem copy the support libraries
xcopy/yesihq  c:\exiv2libs\expat-2.1.0     ..\expat
xcopy/yesihq  c:\exiv2libs\zlib-1.2.7      ..\zlib
xcopy/yesihq  c:\exiv2libs\openssl-1.0.1j  ..\openssl
xcopy/yesihq  c:\exiv2libs\libssh-0.5.5    ..\libssh
xcopy/yesihq  c:\exiv2libs\curl-7.39.0     ..\curl

rem ----------------------------------------------
rem modify include/exiv2/exv_msvc.h to respect library requests
pushd include\exiv2

copy/y  exv_msvc.h t.h
if %curl%==false (
   type t.h | ..\..\msvc2005\tools\bin\sed.exe -e "s/EXV_USE_CURL 1/EXV_USE_CURL 0/g" > exv_msvc.h
)
del t.h

copy/y  exv_msvc.h t.h
if %curl%==false (
   type t.h | ..\..\msvc2005\tools\bin\sed.exe -e "s/EXV_USE_SSH 1/EXV_USE_SSH 0/g" > exv_msvc.h
)
del t.h
popd

rem ----------------------------------------------
rem modify msvc2005/exiv2.sln to respect library requests
pushd msvc2005

rem --
rem FOO is the current directory in cygwin (/cygdrive/c/users/shared/workspace/exiv2-trunk/label/msvc)
rem we need this to set the correct directory when we run the test suite from Cygwin
for /f "tokens=*" %%a in ('cygpath -au ..') do set FOO=%%a

copy exiv2.sln e.sln
copy exiv2.sln t.sln
rem find the lines which specify openssl, eay, ssh and curl
for /f "tokens=*" %%a in ('findstr /c:openssl e.sln') do set L_OPENSSL=%%a
for /f "tokens=*" %%a in ('findstr /c:libeay  e.sln') do set L_LIBEAY=%%a
for /f "tokens=*" %%a in ('findstr /c:ssleay  e.sln') do set L_SSLEAY=%%a
for /f "tokens=*" %%a in ('findstr /c:libssh  e.sln') do set L_LIBSSH=%%a
for /f "tokens=*" %%a in ('findstr /c:curl    e.sln') do set L_CURL=%%a

if %openssl%==false (
    call BatchSubstitute.bat "%%L_OPENSSL%%" "" e.sln > t.sln && copy/y t.sln e.sln
    call BatchSubstitute.bat "%%L_LIBEAY%%"  "" e.sln > t.sln && copy/y t.sln e.sln
    call BatchSubstitute.bat "%%L_SSLEAY%%"  "" e.sln > t.sln && copy/y t.sln e.sln
)
if %libssh%==false (
    call BatchSubstitute.bat "%%L_OPENSSL%%" "" e.sln > t.sln && copy t.sln/y e.sln
)
if %curl%==false (
    call BatchSubstitute.bat "%%L_CURL%%"    "" e.sln > t.sln && copy/y t.sln e.sln
)
del t.sln

rem --
rem Now build and test

if %Win32%==true (
  if %debug%==true (
    if %static%==true (
      devenv e.sln %ACTION% "Debug|Win32"     
      if %tests%==true (
        call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc2005/bin/Win32/Debug'
) ) ) )

if %Win32%==true (
  if %release%==true (
    if %static%==true  (
      devenv e.sln %ACTION% "Release|Win32"    
      if %tests%==true   (
        call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc2005/bin/Win32/Release'
) ) ) )

if %Win32%==true (
  if %debug%==true (
    if %dll%==true   (
      devenv e.sln %ACTION% "DebugDLL|Win32"   
      if %tests%==true (
        call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc2005/bin/Win32/DebugDLL'
) ) ) )

if %Win32%==true (
  if %release%==true (
    if %dll%==true     (
      devenv e.sln %ACTION% "ReleaseDLL|Win32" 
      if %tests%==true   (
        call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc2005/bin/Win32/ReleaseDLL'
) ) ) )

if %x64%==true (
  if %debug%==true (
    if %static%==true (
      devenv e.sln %ACTION% "Debug|x64"        
      if %tests%==true (
        call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc2005/bin/x64/Debug'
) ) ) )

if %x64%==true (
  if %release%==true (
    if %static%==true  (
      devenv e.sln %ACTION% "Release|x64"      
      if %tests%==true   (
        call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc2005/bin/x64/Release'
) ) ) )

if %x64%==true (
  if %debug%==true (
    if %dll%==true   (
      devenv e.sln %ACTION% "DebugDLL|x64"     
      if %tests%==true (
        call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc2005/bin/x64/DebugDLL'
) ) ) )

if %x64%==true   (
  if %release%==true (
    if %dll%==true     (
      devenv e.sln %ACTION% "ReleaseDLL|x64"   
      if %tests%==true   (
        call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc2005/bin/x64/ReleaseDLL'
) ) ) )

del e.sln
popd

rem delete the support libraries (with mozilla's native rm utility)
msvc2005\tools\bin\rm.exe -rf ..\expat ..\zlib ..\openssl ..\libssh ..\curl

rem That's all Folks!
rem -----------------
