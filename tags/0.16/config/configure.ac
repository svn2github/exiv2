#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(exiv2, 0.16, ahuggel@gmx.net)
# See http://www.gnu.org/software/libtool/manual.html#Updating-version-info
EXIV2_LTVERSION=3:0:1
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_SUBST(EXIV2_LTVERSION)
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)
AC_CONFIG_SRCDIR([src/exif.cpp])
AC_CONFIG_HEADER([./config/config.h])
AC_CONFIG_AUX_DIR([./config])

# ---------------------------------------------------------------------------
# Checks for programs.
# ---------------------------------------------------------------------------
# Don't use -g to compile C++ code
ac_cv_prog_cxx_g=no
AC_PROG_CXX
AC_PROG_CXXCPP
# Don't use -g to compile C code
ac_cv_prog_cc_g=no
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# ---------------------------------------------------------------------------
# i18n support
# ---------------------------------------------------------------------------
AM_GNU_GETTEXT([external])

# ---------------------------------------------------------------------------
# Libtool support
# ---------------------------------------------------------------------------
AC_PROG_LIBTOOL

# ---------------------------------------------------------------------------
# Checks for libraries.
# ---------------------------------------------------------------------------
AC_CHECK_LIBM()
LIBS="$LIBS $LIBM"
CHECK_ZLIB()
AC_SUBST(HAVE_LIBZ,$HAVE_LIBZ)
AM_ICONV

# ---------------------------------------------------------------------------
# Checks for header files.
# ---------------------------------------------------------------------------
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h locale.h malloc.h stdint.h stdlib.h string.h unistd.h sys/mman.h utime.h])

# ---------------------------------------------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
# ---------------------------------------------------------------------------
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM
#AC_TYPE_UINT8_T
#AC_TYPE_UINT16_T
#AC_TYPE_UINT32_T
#AC_TYPE_INT16_T
#AC_TYPE_INT32_T
AC_SUBST(GXX,$GXX)
AC_SUBST(GCC,$GCC)

# ---------------------------------------------------------------------------
# Checks for library functions.
# ---------------------------------------------------------------------------
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([gmtime_r memset strchr strerror strtol mmap munmap])
AC_CHECK_FUNCS([timegm], HAVE_TIMEGM=1)
AC_SUBST(HAVE_TIMEGM,$HAVE_TIMEGM)

# ---------------------------------------------------------------------------
# Miscellaneous
# ---------------------------------------------------------------------------
AC_MSG_CHECKING([whether to enable dependency tracking])
AC_ARG_ENABLE(dependency-tracking,
    [  --disable-dependency-tracking  speeds up one-time build
  --enable-dependency-tracking   do not reject slow dependency extractors],
    USE_TRACKING=$enableval, USE_TRACKING=yes)
AC_MSG_RESULT($USE_TRACKING)
DEP_TRACKING=
if test "$USE_TRACKING" = "yes"; then
    DEP_TRACKING=1
fi
AC_SUBST(DEP_TRACKING,$DEP_TRACKING)

AC_MSG_CHECKING([whether to use the lens database for Exif.Nikon3.LensData])
AC_ARG_ENABLE(lensdata,
    [  --disable-lensdata      do not use the lens database for Exif.Nikon3.LensData ],
    USE_LENSDATA=$enableval, USE_LENSDATA=yes)
AC_MSG_RESULT($USE_LENSDATA)
if test "$USE_LENSDATA" = "yes"; then
    AC_DEFINE(HAVE_LENSDATA,1)
fi

AC_MSG_CHECKING([whether to convert UCS2 encoded Windows tags with iconv ])
AC_ARG_ENABLE(printucs2,
    [  --disable-printucs2     do not convert UCS2 encoded Windows tag to UTF-8 ],
    USE_PRINTUCS2=$enableval, USE_PRINTUCS2=yes)
AC_MSG_RESULT($USE_PRINTUCS2)
if test "$USE_PRINTUCS2" = "yes"; then
    AC_DEFINE(HAVE_PRINTUCS2,1)
else
# if NLS is also disabled, don't link with iconv
# (since in some cases, AM_GNU_GETTEXT and/or AM_ICONV configure for this anyway)
    if test "$USE_NLS" = "no"; then
        unset LIBICONV
        unset LTLIBICONV
    fi
fi

AC_MSG_CHECKING([whether to compile a commercial version of the Exiv2 library])
AC_ARG_ENABLE(commercial,
    [  --enable-commercial     compile with the EXV_COMMERCIAL_VERSION symbol set ],
    COMMERCIAL_VERSION=$enableval, COMMERCIAL_VERSION=no)
AC_MSG_RESULT($COMMERCIAL_VERSION)
AC_SUBST(COMMERCIAL_VERSION,$COMMERCIAL_VERSION)

AC_MSG_CHECKING([whether to enable XMP support (requires expat)])
AC_ARG_ENABLE(xmp,
    [  --disable-xmp           do not compile XMP support and the XMP Toolkit ],
    USE_XMP_TOOLKIT=$enableval, USE_XMP_TOOLKIT=yes)
AC_MSG_RESULT($USE_XMP_TOOLKIT)

# expat option might change flags, so we save them initially
_cppflags="${CPPFLAGS}"
_ldflags="${LDFLAGS}"

AC_ARG_WITH(expat,
[  --with-expat[[=DIR]]    use libexpat in DIR],[
  if test "$withval" != "no" -a "$withval" != "yes"; then
    EXPAT_DIR=$withval
    CPPFLAGS="${CPPFLAGS} -I$withval/include"
    LDFLAGS="${LDFLAGS} -L$withval/lib"
  fi
])

ENABLE_XMP=
if test "$USE_XMP_TOOLKIT" = "yes"; then
    # Check for expat library
    found_expat=yes
    EXPAT_CPPFLAGS=
    EXPAT_LDFLAGS=
    EXPAT_LIBS=
    if test "$with_expat" != "no"; then
        AC_CHECK_HEADERS(expat.h,
            AC_CHECK_LIB(expat, XML_ParserCreate,[
                AC_DEFINE([HAVE_EXPAT], [], [Have Expat XML parser library])
                EXPAT_LIBS="-lexpat"
                if test "x${EXPAT_DIR}" != "x"; then
                    EXPAT_CPPFLAGS="-I${EXPAT_DIR}/include"
                    EXPAT_LDFLAGS="-L${EXPAT_DIR}/lib"
                fi],[found_expat=no]),[found_expat=no])
    fi
    if test "$with_expat" = "no" -o "$found_expat" = "no"; then
        AC_MSG_ERROR(either specify a valid expat installation with --with-expat=DIR or disable XMP support with --disable-xmp)
    fi
    AC_DEFINE(HAVE_XMP_TOOLKIT,1)
    ENABLE_XMP=1
    AC_SUBST(EXPAT_CPPFLAGS)
    AC_SUBST(EXPAT_LDFLAGS)
    AC_SUBST(EXPAT_LIBS)
fi
AC_SUBST(ENABLE_XMP)

CPPFLAGS=${_cppflags}
LDFLAGS=${_ldflags}

# Udi's hack to add libiconv to the Libs section in exiv2.pc
if test "$enable_shared" = "no"; then
    EXV_LIB_STATIC=$LTLIBICONV
else
    EXV_LIB_STATIC=
fi
AC_SUBST(EXV_LIB_STATIC)

# ---------------------------------------------------------------------------
# Create output files.
# ---------------------------------------------------------------------------
AC_CONFIG_FILES([./Makefile:./config/Makefile.in])
AC_CONFIG_FILES([./config/config.mk])
AC_CONFIG_FILES([./config/exiv2.pc])
AC_CONFIG_FILES([./po/Makefile.in])
AC_CONFIG_FILES([./src/doxygen.hpp])
AC_OUTPUT

# ---------------------------------------------------------------------------
# Configuration summary.
# ---------------------------------------------------------------------------

echo ""
echo "------------------------------------------------------------------"
echo "-- Exiv2 $VERSION feature configuration summary"
echo "--"

case "$enable_shared" in
yes) echo "-- Build a shared library......... YES" ;;
*)   echo "-- Build a shared library......... NO" ;;
esac

case "x$HAVE_LIBZ" in
x1)  echo "-- PNG image support.............. YES" ;;
*)   echo "-- PNG image support.............. NO"
     echo ""
     echo "zlib is required for PNG support. Make sure the zlib header files"
     echo "are installed and use --with-zlib=DIR if it is in a non-standard"
     echo "location. You can get zlib from http://www.zlib.net/"
     echo "" ;;
esac

case "$USE_NLS" in
yes) echo "-- Native language support........ YES" ;;
*)   echo "-- Native language support........ NO"
     echo ""
     echo "gettext is required for native language support. Make sure the"
     echo "gettext header files are installed. You can get gettext from"
     echo "http://www.gnu.org/software/gettext/"
     echo "" ;;
esac

case "$USE_PRINTUCS2" in
yes) echo "-- Conversion of Windows XP tags.. YES" ;;
*)   echo "-- Conversion of Windows XP tags.. NO"
     echo ""
     echo "libiconv is required for the conversion of Windows XP tags."
     echo "Make sure that the libiconv header files are installed and use"
     echo "--with-libiconv-prefix=DIR if it is in a non-standard location."
     echo "You can get libiconv from http://www.gnu.org/software/libiconv/"
     echo "" ;;
esac

case "$USE_LENSDATA" in
yes) echo "-- Nikon lens database............ YES" ;;
*)   echo "-- Nikon lens database............ NO" ;;
esac

case "$USE_XMP_TOOLKIT" in
yes) echo "-- XMP metadata support........... YES" ;;
*)   echo "-- XMP metadata support........... NO"
     echo ""
     echo "Expat is required for XMP support. Make sure the Expat header"
     echo "files are installed and use --with-expat=DIR if it is in a"
     echo "non-standard location. You can get Expat from"
     echo "http://expat.sourceforge.net/"
     echo "" ;;
esac

echo "------------------------------------------------------------------"
echo ""
