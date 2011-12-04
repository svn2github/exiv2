// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2010 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*!
  @file    exif.hpp
  @brief   Encoding and decoding of Exif data
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    09-Jan-04, ahu: created
 */
#ifndef EXIF_HPP_
#define EXIF_HPP_

// *****************************************************************************
// included header files
#include "metadatum.hpp"
#include "tags.hpp"
#include "key.hpp"
#include "value.hpp"
#include "types.hpp"

// + standard includes
#include <string>
#include <list>
#include <memory>

// *****************************************************************************
// namespace extensions
/*!
  @brief Provides classes and functions to encode and decode Exif and Iptc data.
         The <b>libexiv2</b> API consists of the objects of this namespace.
 */
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class ExifData;

// *****************************************************************************
// class definitions

    /*!
      @brief Access to a Exif %thumbnail image. This class provides higher level
             accessors to the thumbnail image that is optionally embedded in IFD1
             of the Exif data. These methods do not write to the Exif metadata.
             Manipulators are provided in subclass ExifThumb.

      @note Various other preview and thumbnail images may be contained in an
            image, depending on its format and the camera make and model. This
            class only provides access to the Exif thumbnail as specified in the
            Exif standard.
     */
    class EXIV2API ExifThumbC {
    public:
        //! @name Creators
        //@{
        //! Constructor.
        ExifThumbC(const ExifData& exifData);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Return the thumbnail image in a %DataBuf. The caller owns the
                 data buffer and %DataBuf ensures that it will be deleted.
         */
        DataBuf copy() const;
        /*!
          @brief Write the thumbnail image to a file.

          A filename extension is appended to \em path according to the image
          type of the thumbnail, so \em path should not include an extension.
          The function will overwrite an existing file of the same name.

          @param path File name of the thumbnail without extension.
          @return The number of bytes written.
        */
        long writeFile(const std::string& path) const;
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like writeFile() but accepts a unicode path in an std::wstring.
          @note This function is only available on Windows.
         */
        long writeFile(const std::wstring& wpath) const;
#endif
        /*!
          @brief Return the MIME type of the thumbnail, either \c "image/tiff"
                 or \c "image/jpeg".
         */
        const char* mimeType() const;
        /*!
          @brief Return the file extension for the format of the thumbnail
                 (".tif" or ".jpg").
         */
        const char* extension() const;
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like extension() but returns the extension in a wchar_t.
          @note This function is only available on Windows.
         */
        const wchar_t* wextension() const;
#endif
        //@}

    private:
        // DATA
        const ExifData& exifData_; //!< Const reference to the Exif metadata.

    }; // class ExifThumb

    /*!
      @brief Access and modify an Exif %thumbnail image. This class implements
             manipulators to set and erase the thumbnail image that is optionally
             embedded in IFD1 of the Exif data. Accessors are provided by the
             base class, ExifThumbC.

      @note Various other preview and thumbnail images may be contained in an
            image, depending on its format and the camera make and model. This
            class only provides access to the Exif thumbnail as specified in the
            Exif standard.
     */
    class EXIV2API ExifThumb : public ExifThumbC {
    public:
        //! @name Creators
        //@{
        //! Constructor.
        ExifThumb(ExifData& exifData);
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Set the Exif thumbnail to the JPEG image \em path. Set
                 XResolution, YResolution and ResolutionUnit to \em xres,
                 \em yres and \em unit, respectively.

          This results in the minimal thumbnail tags being set for a JPEG
          thumbnail, as mandated by the Exif standard.

          @throw Error if reading the file fails.

          @note  No checks on the file format or size are performed.
          @note  Additional existing Exif thumbnail tags are not modified.
          @note  The JPEG image inserted as thumbnail image should not
                 itself contain Exif data (or other metadata), as existing
                 applications may have problems with that. (The preview
                 application that comes with OS X for one.) - David Harvey.
         */
        void setJpegThumbnail(
            const std::string& path,
                  URational    xres,
                  URational    yres,
                  uint16_t     unit
        );
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like setJpegThumbnail() but accepts a unicode path in an
                 std::wstring.
          @note This function is only available on Windows.
         */
        void setJpegThumbnail(
            const std::wstring& wpath,
                  URational     xres,
                  URational     yres,
                  uint16_t      unit
        );
#endif
        /*!
          @brief Set the Exif thumbnail to the JPEG image pointed to by \em buf,
                 and size \em size. Set XResolution, YResolution and
                 ResolutionUnit to \em xres, \em yres and \em unit, respectively.

          This results in the minimal thumbnail tags being set for a JPEG
          thumbnail, as mandated by the Exif standard.

          @throw Error if reading the file fails.

          @note  No checks on the image format or size are performed.
          @note  Additional existing Exif thumbnail tags are not modified.
          @note  The JPEG image inserted as thumbnail image should not
                 itself contain Exif data (or other metadata), as existing
                 applications may have problems with that. (The preview
                 application that comes with OS X for one.) - David Harvey.
         */
        void setJpegThumbnail(
            const byte*     buf,
                  long      size,
                  URational xres,
                  URational yres,
                  uint16_t  unit
        );
        /*!
          @brief Set the Exif thumbnail to the JPEG image \em path.

          This sets only the Compression, JPEGInterchangeFormat and
          JPEGInterchangeFormatLength tags, which is not all the thumbnail
          Exif information mandatory according to the Exif standard. (But it's
          enough to work with the thumbnail.)

          @throw Error if reading the file fails.

          @note  No checks on the file format or size are performed.
          @note  Additional existing Exif thumbnail tags are not modified.
         */
        void setJpegThumbnail(const std::string& path);
#ifdef EXV_UNICODE_PATH
        /*!
          @brief Like setJpegThumbnail(const std::string& path) but accepts a
                 unicode path in an std::wstring.
          @note This function is only available on Windows.
         */
        void setJpegThumbnail(const std::wstring& wpath);
#endif
        /*!
          @brief Set the Exif thumbnail to the JPEG image pointed to by \em buf,
                 and size \em size.

          This sets only the Compression, JPEGInterchangeFormat and
          JPEGInterchangeFormatLength tags, which is not all the thumbnail
          Exif information mandatory according to the Exif standard. (But it's
          enough to work with the thumbnail.)

          @note  No checks on the image format or size are performed.
          @note  Additional existing Exif thumbnail tags are not modified.
         */
        void setJpegThumbnail(const byte* buf, long size);
        /*!
          @brief Delete the thumbnail from the Exif data. Removes all
                 Exif.%Thumbnail.*, i.e., Exif IFD1 tags.
         */
        void erase();
        //@}

    private:
        // DATA
        ExifData& exifData_;    //!< Reference to the related Exif metadata.

    }; // class ExifThumb

    //! Container type to hold all metadata
    typedef std::list<Tag1> ExifMetadata;

    /*!
      @brief A container for Exif data.  This is a top-level class of the %Exiv2
             library. The container holds Tag1 objects.

      Provide high-level access to the Exif data of an image:
      - read Exif information from JPEG files
      - access metadata through keys and standard C++ iterators
      - add, modify and delete metadata
      - write Exif data to JPEG files
      - extract Exif metadata to files, insert from these files
      - extract and delete Exif thumbnail (JPEG and TIFF thumbnails)
    */
    class EXIV2API ExifData {
    public:
        //! ExifMetadata iterator type
        typedef ExifMetadata::iterator iterator;
        //! ExifMetadata const iterator type
        typedef ExifMetadata::const_iterator const_iterator;

        //! @name Manipulators
        //@{
        /*!
          @brief Returns a reference to the %Tag1 that is associated with a
                 particular \em key. If %ExifData does not already contain such
                 a %Tag1, operator[] adds object \em Tag1(key).

          @note  Since operator[] might insert a new element, it can't be a const
                 member function.
         */
        Tag1& operator[](const std::string& key);
        /*!
          @brief Add a Tag1 from the supplied key and value pair.  This
                 method copies (clones) key and value. No duplicate checks are
                 performed, i.e., it is possible to add multiple metadata with
                 the same key.
         */
        void add(const Key1& key, const Value* pValue);
        /*!
          @brief Add a copy of the \em tag to the Exif metadata.  No
                 duplicate checks are performed, i.e., it is possible to add
                 multiple metadata with the same key.

          @throw Error if the makernote cannot be created
         */
        void add(const Tag1& tag);
        /*!
          @brief Delete the Tag1 at iterator position \em pos, return the
                 position of the next tag. Note that iterators into the
                 metadata, including \em pos, are potentially invalidated by
                 this call.
         */
        iterator erase(iterator pos);
        /*!
          @brief Remove all elements of the range \em beg, \em end, return the
                 position of the next element. Note that iterators into
                 the metadata are potentially invalidated by this call.
         */
        iterator erase(iterator beg, iterator end);
        /*!
          @brief Delete all tags resulting in an empty container.
                 Note that this also removes thumbnails.
         */
        void clear();
        //! Sort metadata by key
        void sortByKey();
        //! Sort metadata by tag
        void sortByTag();
        //! Begin of the metadata
        iterator begin() { return exifMetadata_.begin(); }
        //! End of the metadata
        iterator end() { return exifMetadata_.end(); }
        /*!
          @brief Find the first Tag1 with the given \em key, return an
                 iterator to it.
         */
        iterator findKey(const Key1& key);
        //@}

        //! @name Accessors
        //@{
        //! Begin of the metadata
        const_iterator begin() const { return exifMetadata_.begin(); }
        //! End of the metadata
        const_iterator end() const { return exifMetadata_.end(); }
        /*!
          @brief Find the first Tag1 with the given \em key, return a const
                 iterator to it.
         */
        const_iterator findKey(const Key1& key) const;
        //! Return true if there is no Exif metadata
        bool empty() const { return count() == 0; }
        //! Get the number of metadata entries
        long count() const { return static_cast<long>(exifMetadata_.size()); }
        //@}

    private:
        // DATA
        ExifMetadata exifMetadata_;

    }; // class ExifData

    /*!
      @brief Stateless parser class for Exif data. Images use this class to
             decode and encode binary Exif data.

      @note  Encode is lossy and is not the inverse of decode.
     */
    class EXIV2API ExifParser {
    public:
        /*!
          @brief Decode metadata from a buffer \em pData of length \em size
                 with binary Exif data to the provided metadata container.

                 The buffer must start with a TIFF header. Return byte order
                 in which the data is encoded.

          @param exifData Exif metadata container.
          @param pData 	  Pointer to the data buffer. Must point to data in
                          binary Exif format; no checks are performed.
          @param size 	  Length of the data buffer
          @return Byte order in which the data is encoded.
        */
        static ByteOrder decode(
                  ExifData& exifData,
            const byte*     pData,
                  uint32_t  size
        );
        /*!
          @brief Encode Exif metadata from the provided metadata to binary Exif
                 format.

          The original binary Exif data in the memory block \em pData, \em size
          is parsed and updated in-place if possible ("non-intrusive"
          writing). If that is not possible (e.g., if new tags were added), the
          entire Exif structure is re-written to the \em blob ("intrusive"
          writing). The return value indicates which write method was used. If
          it is \c wmNonIntrusive, the original memory \em pData, \em size
          contains the result and \em blob is empty. If the return value is
          \c wmIntrusive, a new Exif structure was created and returned in
          \em blob. The memory block \em pData, \em size may be partly updated in
          this case and should not be used anymore.

          Encode is a lossy operation. It attempts to fit the Exif data into a
          binary block suitable as the payload of a JPEG APP1 Exif segment,
          which can be at most 65527 bytes large. Encode omits IFD0 tags that
          are "not recorded" in compressed images according to the Exif 2.2
          specification. It also doesn't write tags in groups which do not occur
          in JPEG images. If the resulting binary block is larger than allowed,
          it further deletes specific large preview tags and unknown tags. The
          operation succeeds even if the end result is still larger than the
          allowed size. Application should therefore always check the size of
          the \em blob.

          @param blob      Container for the binary Exif data if "intrusive"
                           writing is necessary. Empty otherwise.
          @param pData     Pointer to the binary Exif data buffer. Must
                           point to data in Exif format; no checks are
                           performed. Will be modified if "non-intrusive"
                           writing is possible.
          @param size      Length of the data buffer.
          @param byteOrder Byte order to use.
          @param exifData  Exif metadata container.

          @return Write method used.
        */
        static WriteMethod encode(
                  Blob&     blob,
            const byte*     pData,
                  uint32_t  size,
                  ByteOrder byteOrder,
            const ExifData& exifData
        );
        /*!
          @brief Encode metadata from the provided metadata to Exif format.

          Encode Exif metadata from the \em ExifData container to binary Exif
          format in the \em blob, encoded in \em byteOrder.

          This simpler encode method uses "intrusive" writing, i.e., it builds
          the binary representation of the metadata from scratch. It does not
          attempt "non-intrusive", i.e., in-place updating. It's better to use
          the other encode() method, if the metadata is already available in
          binary format, in order to allow for "non-intrusive" updating of the
          existing binary representation.

          This is just an inline wrapper for
          ExifParser::encode(blob, 0, 0, byteOrder, exifData).

          @param blob      Container for the binary Exif data.
          @param byteOrder Byte order to use.
          @param exifData  Exif metadata container.
        */
        static void encode(
                  Blob&     blob,
                  ByteOrder byteOrder,
            const ExifData& exifData
        )
        {
            encode(blob, 0, 0, byteOrder, exifData);
        }

    }; // class ExifParser

}                                       // namespace Exiv2

#endif                                  // #ifndef EXIF_HPP_
