# CMake build system for exiv2 library and executables
# Copyright 2012+     Robin Mills      <robin@clanmills.com>
# Copyright 2010-2012 Gilles Caulier   <caulier dot gilles at gmail dot com>
# Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>

# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if ( NOT MSVC )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# Private headers which are only needed for the library itself
SET( LIBEXIV2_PRIVATE_HDR ../include/exiv2/canonmn_int.hpp
                          ../include/exiv2/cr2image_int.hpp
                          ../include/exiv2/crwimage_int.hpp
                          ../include/exiv2/fujimn_int.hpp
                          ../include/exiv2/makernote_int.hpp
                          ../include/exiv2/minoltamn_int.hpp
                          ../include/exiv2/nikonmn_int.hpp
                          ../include/exiv2/olympusmn_int.hpp
                          ../include/exiv2/orfimage_int.hpp
                          ../include/exiv2/panasonicmn_int.hpp
                          ../include/exiv2/pentaxmn_int.hpp
                          ../include/exiv2/pngchunk_int.hpp
                          ../include/exiv2/rcsid_int.hpp
                          ../include/exiv2/rw2image_int.hpp
                          ../include/exiv2/samsungmn_int.hpp
                          ../include/exiv2/sigmamn_int.hpp
                          ../include/exiv2/sonymn_int.hpp
                          ../include/exiv2/tags_int.hpp
                          ../include/exiv2/tiffcomposite_int.hpp
                          ../include/exiv2/tifffwd_int.hpp
                          ../include/exiv2/tiffimage_int.hpp
                          ../include/exiv2/tiffvisitor_int.hpp
   )

# Add standalone C++ header files to this list
SET( LIBEXIV2_HDR         ../include/exiv2/asfvideo.hpp
                          ../include/exiv2/basicio.hpp
                          ../include/exiv2/bmpimage.hpp
                          ../include/exiv2/convert.hpp
                          ../include/exiv2/cr2image.hpp
                          ../include/exiv2/crwimage.hpp
                          ../include/exiv2/datasets.hpp
                          ../include/exiv2/easyaccess.hpp
                          ../include/exiv2/epsimage.hpp
                          ../include/exiv2/error.hpp
                          ../include/exiv2/exif.hpp
                          ../include/exiv2/exiv2.hpp
                          ../include/exiv2/futils.hpp
                          ../include/exiv2/gifimage.hpp
                          ../include/exiv2/http.hpp
                          ../include/exiv2/ssh.hpp
                          ../include/exiv2/image.hpp
                          ../include/exiv2/iptc.hpp
                          ../include/exiv2/jp2image.hpp
                          ../include/exiv2/jpgimage.hpp
                          ../include/exiv2/Jzon.h
                          ../include/exiv2/matroskavideo.hpp
                          ../include/exiv2/metadatum.hpp
                          ../include/exiv2/mrwimage.hpp
                          ../include/exiv2/orfimage.hpp
                          ../include/exiv2/pgfimage.hpp
                          ../include/exiv2/preview.hpp
                          ../include/exiv2/properties.hpp
                          ../include/exiv2/psdimage.hpp
                          ../include/exiv2/quicktimevideo.hpp
                          ../include/exiv2/rafimage.hpp
                          ../include/exiv2/riffvideo.hpp
                          ../include/exiv2/rw2image.hpp
                          ../include/exiv2/tags.hpp
                          ../include/exiv2/tgaimage.hpp
                          ../include/exiv2/tiffimage.hpp
                          ../include/exiv2/types.hpp
                          ../include/exiv2/value.hpp
                          ../include/exiv2/version.hpp
                          ../include/exiv2/xmp.hpp
                          ../include/exiv2/xmpsidecar.hpp
   )

# Add library C++ source files to this list
SET( LIBEXIV2_SRC         asfvideo.cpp
                          basicio.cpp
                          bmpimage.cpp
                          canonmn.cpp
                          convert.cpp
                          cr2image.cpp
                          crwedit.cpp
                          crwimage.cpp
                          datasets.cpp
                          easyaccess.cpp
                          epsimage.cpp
                          error.cpp
                          exif.cpp
                          futils.cpp
                          fujimn.cpp
                          gifimage.cpp
                          http.cpp
                          ssh.cpp
                          image.cpp
                          iptc.cpp
                          jp2image.cpp
                          jpgimage.cpp
                          Jzon.cpp
                          makernote.cpp
                          matroskavideo.cpp
                          metadatum.cpp
                          minoltamn.cpp
                          mrwimage.cpp
                          nikonmn.cpp
                          olympusmn.cpp
                          orfimage.cpp
                          panasonicmn.cpp
                          pentaxmn.cpp
                          pgfimage.cpp
                          preview.cpp
                          properties.cpp
                          psdimage.cpp
                          quicktimevideo.cpp
                          rafimage.cpp
                          riffvideo.cpp
                          rw2image.cpp
                          samsungmn.cpp
                          sigmamn.cpp
                          sonymn.cpp
                          tags.cpp
                          tgaimage.cpp
                          tiffcomposite.cpp
                          tiffimage.cpp
                          tiffvisitor.cpp
                          types.cpp
                          value.cpp
                          version.cpp
                          xmp.cpp
                          xmpsidecar.cpp
   )

##
# add png*.cpp files if PNG support requested
IF( EXIV2_ENABLE_PNG )
    IF( ZLIB_FOUND )
        SET( LIBEXIV2_SRC ${LIBEXIV2_SRC}
                          pngchunk.cpp
                          pngimage.cpp
           )
        set( LIBEXIV2_HDR ${LIBEXIV2_HDR} ../include/exiv2/pngimage.hpp )
    ENDIF( ZLIB_FOUND )
ENDIF( EXIV2_ENABLE_PNG )

##
# add xmp source if xmp is requested and xmp library not built
IF( EXIV2_ENABLE_XMP )
    IF( NOT EXIV2_ENABLE_LIBXMP )
        SET( LIBEXIV2_SRC ${XMPSRC} ${LIBEXIV2_SRC} )
    ENDIF( NOT EXIV2_ENABLE_LIBXMP )
ENDIF( EXIV2_ENABLE_XMP )

##
# Create source file lists for applications
# exiv2 application
SET( EXIV2_SRC            exiv2.cpp
                          actions.cpp
                          utils.cpp
)
# connection test application
SET( CONNTEST             ../samples/conntest.cpp)
# exifprint application
SET( EXIFPRINT            ../samples/exifprint.cpp)
# cmdfiletest application
SET( REMOTETEST           ../samples/remotetest.cpp)

##
# modify source lists to suit environment
IF(NOT HAVE_TIMEGM )
    SET( LIBEXIV2_SRC     ${LIBEXIV2_SRC} localtime.c    )
    SET( EXIV2_SRC        ${EXIV2_SRC}    localtime.c    )
    SET( PATHTEST_SRC     ${PATHTEST_SRC} localtime.c    )
ENDIF( NOT HAVE_TIMEGM )

IF( MSVC )
    SET( MC_SRC           ${MC_SRC}       getopt_win32.c )
    SET( EXIV2_SRC        ${EXIV2_SRC}    getopt_win32.c )
    SET( LIBEXIV2_SRC     ${LIBEXIV2_SRC} getopt_win32.c )
    SET( PATHTEST_SRC     ${PATHTEST_SRC} getopt_win32.c )
ENDIF( MSVC )

##
# msvn tuning
include(../CMake_msvc.txt)
msvc_runtime_configure(${EXIV2_ENABLE_SHARED})

# ******************************************************************************
# exiv2lib library
ADD_LIBRARY( exiv2lib ${STATIC_FLAG} ${LIBEXIV2_SRC} ${LIBEXIV2_HDR} )

SET_TARGET_PROPERTIES( exiv2lib PROPERTIES
    VERSION       ${GENERIC_LIB_VERSION}
    SOVERSION     ${GENERIC_LIB_SOVERSION}
    DEFINE_SYMBOL EXV_BUILDING_LIB
    OUTPUT_NAME   exiv2
)

if ( MSVC ) 
    source_group("Header Files" FILES ${LIBEXIV2_HDR} )
    TARGET_LINK_LIBRARIES( exiv2lib ${EXPAT_LIBRARIES} )
    source_group("Header Files" FILES ${LIBCURL_HDR} )
    TARGET_LINK_LIBRARIES( exiv2lib ${CURL_LIBRARIES} )
    source_group("Header Files" FILES ${SSH_HDR} )
    TARGET_LINK_LIBRARIES( exiv2lib ${SSH_LIBRARIES} )
else()
    if ( NOT MINGW )
        TARGET_LINK_LIBRARIES( exiv2lib dl ${EXPAT_LIBRARIES} )
        TARGET_LINK_LIBRARIES( exiv2lib dl ${CURL_LIBRARIES} )
        TARGET_LINK_LIBRARIES( exiv2lib dl ${SSH_LIBRARIES} )
    else()
        TARGET_LINK_LIBRARIES( exiv2lib    ${EXPAT_LIBRARIES} )
        TARGET_LINK_LIBRARIES( exiv2lib    ${CURL_LIBRARIES} )
        TARGET_LINK_LIBRARIES( exiv2lib    ${SSH_LIBRARIES} )
    endif()
    
endif()

if( EXIV2_ENABLE_LIBXMP )
    ADD_DEPENDENCIES( exiv2lib xmp )
    if  ( MSVC ) 
        LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH}/$(ConfigurationName))
    endif(MSVC)
    TARGET_LINK_LIBRARIES( exiv2lib xmp )
ENDIF()

IF( EXIV2_ENABLE_PNG )
    IF( ZLIB_FOUND )
        IF( MSVC )
            if ( EXIV2_ENABLE_SHARED )      
                TARGET_LINK_LIBRARIES( exiv2lib optimized ${ZLIB_LIBRARIES} debug ${ZLIB_LIBRARIES}d )
            else()
                TARGET_LINK_LIBRARIES( exiv2lib optimized zlibstatic.lib debug zlibstaticd.lib )
            endif()
        ELSE()
            TARGET_LINK_LIBRARIES( exiv2lib ${ZLIB_LIBRARIES} )
        ENDIF()
    ENDIF()
ENDIF()

IF( EXIV2_ENABLE_NLS )
    TARGET_LINK_LIBRARIES( exiv2lib ${LIBINTL_LIBRARIES} )
ENDIF( EXIV2_ENABLE_NLS )

IF( ICONV_FOUND )
    TARGET_LINK_LIBRARIES( exiv2lib ${ICONV_LIBRARIES} )
ENDIF( ICONV_FOUND )

IF (CYGWIN OR MINGW)
    TARGET_LINK_LIBRARIES( exiv2lib psapi )
ENDIF(CYGWIN OR MINGW)

##
# copy curl dlls to output directory (TODO: rewrite for NMake)
IF ( MSVC AND EXIV2_ENABLE_CURL )
    get_filename_component(CURL_LIBRARY_DIR ${CURL_LIBRARY} PATH)
    string(REGEX REPLACE "/" "\\\\" CURL_LIBRARY_DIR ${CURL_LIBRARY_DIR})
    ADD_CUSTOM_COMMAND(
      TARGET exiv2lib
      POST_BUILD
      COMMAND copy /y ${CURL_LIBRARY_DIR}\\*.dll $(SolutionDir)\\bin\\${P_DIR}\\${T_DIR}\\$(ConfigurationName)
    )
ENDIF()
INSTALL( TARGETS exiv2lib ${INSTALL_TARGET_STANDARD_ARGS} )

##
# copy libssh dlls to output directory (TODO: rewrite for NMake)
IF ( MSVC AND EXIV2_ENABLE_SSH )
    get_filename_component(SSH_LIBRARY_DIR ${SSH_LIBRARY} PATH)
    string(REGEX REPLACE "/" "\\\\" SSH_LIBRARY_DIR ${SSH_LIBRARY_DIR})
    ADD_CUSTOM_COMMAND(
      TARGET exiv2lib
      POST_BUILD
      COMMAND copy /y ${SSH_LIBRARY_DIR}\\*.dll $(SolutionDir)\\bin\\${P_DIR}\\${T_DIR}\\$(ConfigurationName)
    )
ENDIF()
INSTALL( TARGETS exiv2lib ${INSTALL_TARGET_STANDARD_ARGS} )

##
# copy zlib and expat dlls to output directory (TODO: rewrite for NMake)
IF ( MSVC AND EXIV2_ENABLE_SHARED )
    get_filename_component(EXPAT_LIBRARY_DIR ${EXPAT_LIBRARY} PATH)
    string(REGEX REPLACE "/" "\\\\" EXPAT_LIBRARY_DIR ${EXPAT_LIBRARY_DIR})
    get_filename_component(ZLIB_LIBRARY_DIR ${ZLIB_LIBRARY} PATH)
    string(REGEX REPLACE "/" "\\\\" ZLIB_LIBRARY_DIR ${ZLIB_LIBRARY_DIR})
    ADD_CUSTOM_COMMAND(
      TARGET exiv2lib
      POST_BUILD
      COMMAND copy /y ${EXPAT_LIBRARY_DIR}\\*.dll $(SolutionDir)\\bin\\${P_DIR}\\${T_DIR}\\$(ConfigurationName)
    )
    ADD_CUSTOM_COMMAND(
      TARGET exiv2lib
      POST_BUILD
      COMMAND copy /y ${ZLIB_LIBRARY_DIR}\\*.dll $(SolutionDir)\\bin\\${P_DIR}\\${T_DIR}\\$(ConfigurationName)
    )
ENDIF()
INSTALL( TARGETS exiv2lib ${INSTALL_TARGET_STANDARD_ARGS} )

# ******************************************************************************
# exiv2 application
ADD_EXECUTABLE( exiv2 ${EXIV2_SRC} )
TARGET_LINK_LIBRARIES( exiv2 exiv2lib )
INSTALL( TARGETS exiv2 ${INSTALL_TARGET_STANDARD_ARGS} )

# ******************************************************************************
# connection test application
ADD_EXECUTABLE( conntest ${CONNTEST} )
TARGET_LINK_LIBRARIES( conntest exiv2lib )

# ******************************************************************************
# exifprint application
ADD_EXECUTABLE( exifprint ${EXIFPRINT} )
TARGET_LINK_LIBRARIES( exifprint exiv2lib )

# ******************************************************************************
# remotetest application
ADD_EXECUTABLE( remotetest ${REMOTETEST} )
TARGET_LINK_LIBRARIES( remotetest exiv2lib )

# ******************************************************************************
# Headers
INSTALL( FILES ${LIBEXIV2_HDR} DESTINATION include/exiv2 )

# ******************************************************************************
# Man page
INSTALL( FILES exiv2.1 DESTINATION man/man1 )

# That's all Folks!
##
