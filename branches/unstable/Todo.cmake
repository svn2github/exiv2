DONE * Need soname versioning for shared library

* Need g++ specific compiler options (warnings) and settings (visibility)

* enable build types in cmake. In case required, add support for it

* The following options of the configure script should be supported
   DONE --without-zlib
   --disable-nls
   DONE --disable-printucs2
   DONE --disable-xmp
   --enable-commercial
   DONE --disable-lensdata
   DONE --disable-shared

DONE * Add further messages to display option settings

   ------------------------------------------------------------------
   -- Exiv2 0.18-pre1 feature configuration summary
   --
   -- Build a shared library......... NO
   -- PNG image support.............. YES
   -- Native language support........ NO

   gettext is required for native language support. Make sure the
   gettext header files are installed. You can get gettext from
   http://www.gnu.org/software/gettext/
   
   -- Conversion of Windows XP tags.. YES
   -- Nikon lens database............ YES
   -- XMP metadata support........... YES
   ------------------------------------------------------------------

* Need targets to build/install doc, DONE man page

DONE * add possibility to build xmp as a convenience library

DONE * Compilation fails if Expat (possibly also other dependencies?) is in a
  non-standard place

WONTFIX * This is probably not necessary in src/CMakeLists.txt / can be removed:
   set( LIBEXIV2_SRC   ${LIBEXIV2_SRC} getopt_win32.c )
   it is necessary as both windows compilers do not autoexport

DONE * Do we need to worry about MSVC warnings like this:
d:\home\ahuggel\msys\src\exiv2\src\exif.hpp(245) : warning C4251: 'Exiv2::Exifdatum::value_' : class 'std::auto_ptr<_Ty>' needs to have dll-interface to be used by clients of class 'Exiv2::Exifdatum'
        with
        [
            _Ty=Exiv2::Value
        ]

* add information on how to cross-compile on Linux for Windows (MinGW) (some applications and
  my own release-build process need this) to the README.cmake

INFO * check How does cmake handle RPATH? (I'll read up on this)

DONE * Is the pkgconfig file not installed in MinGW? - it is now

* Can the logic needed to build the doc be re-written using cmake?
