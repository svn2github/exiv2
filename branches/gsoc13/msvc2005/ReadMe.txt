exiv2\msvc2005\ReadMe.txt
-------------------------

+-------------------------------------------------------+
| msvc2005 builds 32bit and 64bit binaries              |
|          with Visual Studio 2005/8/10/12              |
|                                                       |
| msvc2003 builds 32bit binaries                        |
|          with Visual Studio 2003/5/8                  |
| ***************************************************** |
| Exiv2 0.25 is the final release with msvc2003 support |
| ***************************************************** |
+-------------------------------------------------------+

Updated: 2013-08-20 

Robin Mills
http://clanmills.com
email: robin@clanmills.com

####
T A B L E  o f  C O N T E N T S

1    Build Instructions
1.1  Tools
1.2  Install zlib, expat, and curl sources.
1.3  Open exiv2\msvc2005\exiv2.sln
1.4  What is build
1.5  Building with zlib1.2.3 (or 1.2.5) and expat 2.0.1
1.6  Building without curl (or expat or zlib)
1.7  Building with build.bat

2    Design
2.1  Architecture
2.2  Relationship with msvc build environment

3    Acknowledgment of prior work
3.1  Differences between inherited project and the exiv2 projects

4    Running the test suite

## End Table of Contents End ##
####

1    Build Instructions

     +-------------------------------------------------+
     | Caution: Disable Visual Studio Parallel builds  |
     | Tools/Options/Projects & Solutions/Build        |
     | Max Parallel Builds: 1                          |
     | ** Parallel builds create linking issues     ** |
     +-------------------------------------------------+

     +-------------------------------------------------+
     | Caution: Don't build in a path with spaces      |
     | Example: c:\gnu           GOOD                  |
     |          c:\Open Source   BAD                   |
     +-------------------------------------------------+

     +-------------------------------------------------+
     | You need Perl on your path to build openssl     |
     | I use the following on Windows7/64              |
     | C:\Users\rmills\Downloads>perl --version        |
     | ... v5.10.1 built for MSWin32-x64-multi-thread  |
     +-------------------------------------------------+

1.1  Tools
     This has been tested with
     : "Pro"     edition of VS 2005/08/10
     : "Express" edition of VS 2012

1.2  Install zlib, expat, curl, openssl and libssh sources.
     I use the directory c:\gnu for this purpose, however the name isn't important.

     c:\gnu>dir
     Directory of c:\gnu
     2010-12-05  10:05    <DIR>             exiv2                <--- this tree
     2012-05-04  23:35    <DIR>             expat                <--- "vanilla" expat 2.1.0 source tree
     2012-05-04  23:35    <DIR>             zlib                 <--- "vanilla" zlib  1.2.7 source tree
     2012-05-04  23:35    <DIR>             curl                 <--- "vanilla" curl  7.30.0 source tree
     2012-05-04  23:35    <DIR>             openssl              <--- "vanilla" openssl-1.0.1e
     2012-05-04  23:35    <DIR>             libssh               <--- "vanilla" libssh-0.5.5
     c:\gnu>

     expat-2.1.0 is available from http://voxel.dl.sourceforge.net/sourceforge/expat/expat-2.1.0.tar.gz
     zlib-1.2.7  is available from http://zlib.net/zlib-1.2.7.tar.gz
     curl-7.30.0 is available from http://curl.haxx.se/download/curl-7.30.0.tar.gz
     openssl-1.0.1e available from http://www.openssl.org/source/openssl-1.0.1e.tar.gz
     libssh-0.5.5   available from https://red.libssh.org/attachments/download/51/libssh-0.5.5.tar.gz
     
1.3  Open exiv2\msvc2005\exiv2.sln
     Projects are zlib1, xmpsdk, libcurl, libexpat, libssh, openssl, libexiv2, exiv2.exe, addmoddel.exe etc...
     Build/Batch build...  Select All, Build
     - 37 projects       (zlib1, xmpsdk, libexpat, libcurl, libexiv2, exiv2, addmoddel etc)
      x 2 Platforms      (x64|Win32)
      x 4 Configurations (Debug|Release|DebugDLL|ReleaseDLL)
     =  2x4*37 = 292 builds.

     If you haven't installed the x64 compiler, remove the 64 bit Platform!

     Build time is about 30 minutes on a 2.2GHz Duo Core and consumes 3.0 gBytes of disk space.

     +-------------------------------------------------+
     | Caution: Visual Studio 2010+ Users              |
     | Don't build Platforms Win32 & x64 simulateously |
     | Or use build.bat to do this for you             |
     +-------------------------------------------------+

1.4  What is built
     The DLL builds use the DLL version of the C runtime libraries
     The Debug|Release builds use static C runtime libraries
     This is discussed in exiv2\msvc2003\ReadMe.txt 

1.5  Building with zlib1.2.5 (or 1.2.3) and/or expat 2.0.1 and/or curl-7.31.0
     msvc2005 is routinely tested with zlib-1.2.7 and expat 2.1.0

     Exiv2 has been successfully built and tested in the past with expat-2.0.1,
     zlib-1.2.3, zlib-1.2.5, zlib-1.2.6 and zlib-1.2.8
     
1.6  Building without curl (or without expat or any other dependant library)
     To build with a dependant library:
     a) Open msvc2005/exiv2.sln in Visual Studio
     b) Select and delete the libcurl (or libexpat or zlib1) from the Project Explorer
     c) Modify include/exiv2/exv_msvc.h to unset build flags (eg EXV_USE_CURL)

1.7  Building with build.bat
     The batch file build.bat is provided for batch building outside the Visual Studio IDE
     a) Setup the Visual Studio Environment with vcvars32.bat 
        vcvars32.bat is typically in C:\Program Files (x86)\Microsoft Visual Studio 8\VC\bin\vcvars32.bat
     b) build.bat /build will build all targets
     or build.bat /upgrade will upgrade the project files (if you're using Visual Studio 2008+
     or build.bat /rebuild
     or build.bat /build Release^^^|Win32 to build a specific target|platform

2    Design

     expat-2.10.0 and zlib1.2.5 (and earlier) do not provide 64 bit builds for DevStudio.
     
     The projects provided for zlib1.2.7 support 64 bit builds, however it didn't
     work cleanly for me.  They use different projects for VC9 and VC10.
     They don't provide support for VC8 or VC11
     
     I have created build environments for zlib1, libexpat and libcurl within exiv2/msvc2005.
     I don't include the source code for zlib1, libexpat or libcurl - only the build environment.
     
     You are expected to install the "vanilla" expat, zlib and curl source code
     in a directory at the same level as exiv2.
     I personally always build in the directory c:\gnu,
     however the name/location/spaces of the build directory are all irrelevant,
     it's only the relative position of zlib, expat and curl that matter.

2.1  Architecture
     There are directories for every component:
     The libraries: zlib1, libexpat, xmpsdk, libcurl, libexiv2
     Applications:  exiv2.exe
     Sample Apps:   exifprint.exe, addmoddel.exe and many more (used by test suite)

     For each component, there are three build directories:
     libexiv2\build                                         intermediate results
     libexiv2\Win32\{Debug|Release|DebugDLL|ReleaseDLL}     32 bit builds
     libexiv2\x64\{Debug|Release|DebugDLL|ReleaseDLL}       64 bit builds

     Final builds and include directories (for export to "foreign" projects)
     bin\{win32|x64}\Win32\{Debug|Release|DebugDLL|ReleaseDLL} 

2.2  Relationship with msvc2003 build environment
     msvc2005 is similar to msvc2003.
     However there are significant differences:
     1) msvc2005 supports 64 bit and 32 bit builds
     2) msvc2005 provides projects to build expat and zlib
     3) msvc2005 is designed to accomodate new versions of expat and zlib when they become available.
     4) msvc2005 supports DevStudio 2005, 2008, 2010 and 2012.  (no support for 2003)
     5) msvc2005 does not require you to build 'vanilla' expat and zlib projects in advance
     6) msvc2005 does not support the organize application
     7) msvc2005 supports building with zlib1.2.7 (default) or zlib1.2.3/5
     8) msvc2005 supports building with expat2.1.0 (default) or expa2.0.1
     9) msvc2005 supports curl, libssh and openssl.  msvc2003 only support http (no https/ftp/ssh)
     
     msvc2003 will continue to be supported for 32 bit builds using DevStudio 2003/05/08,
     however there is no plan to enhance or develop msvc2003 going forward.
     Exiv2 v0.25 will be the final release with msvc2003 support.

3    Acknowledgement of prior work
     This work is based on work by the following people:
     zlib 64 bit build

        Copyright (C) 2000-2004 Simon-Pierre Cadieux.
        Copyright (C) 2004 Cosmin Truta.
        For conditions of distribution and use, see copyright notice in zlib.h.

        And was obtained from:  http://www.achacha.org/vc2008_zlib/

     expat 64 bit build
     http://www.start64.com/index.php?option=com_content&task=view&id=3461&Itemid=114

     I recognise and respect the work performed by those individuals.

3.1  Differences between inherited projects and the exiv2 projects
     There is no compatiblity.

4    Running the test suite
     You will need to install cygwin to run the test suite.
     
     This is a two stage process:
     Step1:  Build exiv2 for cygwin and run the test suite
             Typical Unix type build:
             make config
             ./configure --disable-visibility
             export PKG_CONFIG_PATH=$PWD/config
             make clean ; make ; make samples ; make install ; cd test ; make test
             
     Step2:  set the environment variable EXIV2_BINDIR appropriately and rerun make test
             export EXIV2_BINDIR=<path-to-directory-with-exiv2.exe>

             I find the following little bash loop very useful.  You should test
             against all the directories in the msvc2005/bin directory:
             for d in $(find /c/gnu.2005/exiv2/msvc2005/bin -name exiv2.exe -exec dirname {} ";"); do
                export EXIV2_BINDIR=$d
                echo ---------------------------------
                echo ---- $d ----
                echo ---------------------------------
                make test
            done
            
    Free gift: (you get what you pay for)
            The script testMSVC.sh to runs this loop for you.
            The script verifyMSVC.sh validates the output of testMSVC.sh
            I've added those for my convenience and I hope you'll find them useful.

    And to pass the time (the test suite takes about an hour to run)
            I recommend running listdlls exiv2.exe occasionally during testing to be confident
            that the test suite is running the MSVC built versions of exiv2 and libraries.
            
            From cygwin:
            while sleep 1 do; listdlls exiv2.exe ; done
            or
            while sleep 10 do ; listdlls exiv2.exe | grep exiv2.exe ; done
            
    Note: Cygwin currently ships diff-utils 2.9.2 which treats binary files differently
    from 2.8.7 (on Mac) and 3.2 (on Ubuntu 12.04).  For this reason, the executable (and
    support dlls) for GNU diff.exe 2.8.7 is provided in msvc2003/diff.exe.
    The test suite has been "doctored" on cygwin to modify the path appropriately to
    use this preferred version of diff.exe.
             
# That's all Folks!
##
