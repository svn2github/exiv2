# CMake build system for exiv2 library and executables
# Copyright 2008 by Patrick Spendrin <ps_ml@gmx.de>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    1. Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#    2. Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#    3. The name of the author may not be used to endorse or promote
#       products derived from this software without specific prior
#       written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

project( exiv2 )

cmake_minimum_required( VERSION 2.6 )

set( PACKAGE_BUGREPORT  "ahuggel@gmx.net" )
set( PACKAGE            "exiv2" )
set( PACKAGE_NAME       "exiv2" )
set( PACKAGE_STRING     "exiv2 0.18" )
set( PACKAGE_TARNAME    "exiv2" )
set( PACKAGE_VERSION    "0.18" )

# options and there default values
option( EXV_HAVE_DLL "Build exiv2 as a shared library (dll)" ON )
option( BUILD_samples "Build the unit tests" OFF )

if( EXV_HAVE_DLL )
    add_definitions( -DEXV_HAVE_DLL )
    set( STATIC_FLAG SHARED )
else( EXV_HAVE_DLL )
    set( STATIC_FLAG STATIC )
endif( EXV_HAVE_DLL )

if( BUILD_samples )
    ENABLE_TESTING()
endif( BUILD_samples )


include( config/ConfigureChecks.cmake )

set( INSTALL_TARGET_STANDARD_ARGS RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib )

# add sources from XMPSDK directory - this needs to be done here
set( XMPSRC ${CMAKE_SOURCE_DIR}/xmpsdk/src/XMPMeta.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/XMPMeta-GetSet.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/XMPMeta-Parse.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/XMPMeta-Serialize.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/XMPIterator.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/XMPUtils.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/XMPUtils-FileInfo.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/XMPCore_Impl.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/ExpatAdapter.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/ParseRDF.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/UnicodeConversions.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/MD5.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/WXMPMeta.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/WXMPIterator.cpp
            ${CMAKE_SOURCE_DIR}/xmpsdk/src/WXMPUtils.cpp )

add_subdirectory( src )

if( BUILD_samples )
    add_subdirectory( samples )
endif( BUILD_samples )
