// ***************************************************************** -*- C++ -*-
/*
 */
/*!
  @file    doxygen.hpp
  @brief   Additional documentation, this file contains no source code
  @version $Name:  $ $Revision: 1.2 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    07-Feb-04, ahu: created
 */
/*!
  @mainpage Exiv2 v0.3

  Exiv2 is a command line program to 
  - print the %Exif metadata of JPEG images (as summary info, interpreted values, 
    or the plain data for each tag)
  - adjust the %Exif timestamp (that's how it all started...)
  - rename %Exif image files according to the %Exif timestamp
  - extract %Exif metadata from image files and insert it again
  - extract the thumbnail image embedded in the %Exif metadata
  - delete the thumbnail or the complete %Exif metadata from an image

  Exiv2 is also a C++ %Exif metadata library, providing high-level access to the
  %Exif data of an image. Supported functionality includes
  - read %Exif information from JPEG files
  - access metadata through unique keys and standard C++ iterators
  - add, modify and delete metadata 
  - write %Exif data to JPEG files
  - extract %Exif metadata to files, insert from these files
  - extract and delete %Exif thumbnail (JPEG and TIFF thumbnails)

  In addition, the library provides generic lower-level functionality to access
  IFD (Image File Directory) data structures.

  Here is a (sortable) <A HREF="../tags.xml">list of %Exif tags</A> as defined
  in Exiv2.


  <A HREF="../makernote.html">MakerNote Formats and Specifications</A>



  <P>Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net></P>
 
  <P>Exiv2 is free software; you can redistribute it and/or modify it under the
  terms of the <a href="../gpl-license.html">GNU General Public License</a> as
  published by the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.</P>
 
  <P>Exiv2 is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.</P>
 
  <P>You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA 02111-1307, USA.</P>

*/
/*!
  @example example1.cpp
  Sample usage of high-level metadata operations.
*/
