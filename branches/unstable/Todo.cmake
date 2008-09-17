* Need soname versioning for shared library

* Need g++ specific compiler options (warnings) and settings (visibility)

* How do I (quickly) recompile a configured library with debugging symbols
  (-ggdb) for debugging?

* The following options of the configure script should be supported
   --without-zlib
   --disable-nls
   --disable-printucs2
   --disable-xmp
   --enable-commercial
   --disable-lensdata
   --disable-shared

* Add further messages to display option settings

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

* Need targets to build/install doc, man page

* More generally, are all the targets of the old framework supported / 
  how do I perform these:
   Makefile: xmpsdk, uninstall, doc, samples, (maintainer-)clean
   src/Makefile: uninstall, (maintainer-)clean
   samples/Makefile: relink

* How to build XMPSDK as a "convenience library" (as libtool calls it) 
  instead of just adding the source files individually?

* Is config/FindEXPAT needed? (a file with the same name is included in the
  cmake-2.6 distribution)

* Compilation fails if Expat (possibly also other dependencies?) is in a
  non-standard place

* This is probably not necessary in src/CMakeLists.txt / can be removed:
   set( LIBEXIV2_SRC   ${LIBEXIV2_SRC} getopt_win32.c )

* Do we need to worry about MSVC warnings like this:
d:\home\ahuggel\msys\src\exiv2\src\exif.hpp(245) : warning C4251: 'Exiv2::Exifdatum::value_' : class 'std::auto_ptr<_Ty>' needs to have dll-interface to be used by clients of class 'Exiv2::Exifdatum'
        with
        [
            _Ty=Exiv2::Value
        ]

* How to cross-compile on Linux for Windows (MinGW) (some applications and
  my own release-build process need this)

* How does cmake handle RPATH? (I'll read up on this)

* Is the pkgconfig file not installed in MinGW?

* Can cmake be used to improve platform check in XMPSDK
   if ( APPLE ); if ( UNIX ); if ( WIN32 )

* Can the logic needed to build the doc be re-written using cmake?
