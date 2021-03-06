// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*!
  @file    exif.hpp
  @brief   Encoding and decoding of Exif data
  @version $Name:  $ $Revision: 1.50 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    09-Jan-04, ahu: created
 */
#ifndef EXIF_HPP_
#define EXIF_HPP_

// *****************************************************************************
// included header files
#include "metadatum.hpp"
#include "types.hpp"
#include "error.hpp"
#include "image.hpp"
#include "value.hpp"
#include "ifd.hpp"
#include "tags.hpp"

// + standard includes
#include <string>
#include <vector>

// *****************************************************************************
// namespace extensions
/*!
  @brief Provides classes and functions to encode and decode Exif and Iptc data.
         This namespace corresponds to the <b>libexiv2</b> library. 

 */
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class ExifData;
    class MakerNote;

// *****************************************************************************
// class definitions

    //! Concrete keys for Exif metadata.
    class ExifKey : public Key {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor to create an Exif key from a key string.

          @param key The key string.
          @throw Error ("Invalid key") if the key cannot be parsed into three
                 parts or the first part of the key is not '<b>Exif</b>'.
        */
        explicit ExifKey(const std::string& key);
        //! Constructor to build an ExifKey from an IFD entry.
        explicit ExifKey(const Entry& e);
        //! Copy constructor
        ExifKey(const ExifKey& rhs);
        // A destructor is not needed, as we do *not* delete the makernote
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Assignment operator.
         */
        ExifKey& operator=(const ExifKey& rhs);
        //! Set the makernote pointer. 
        void setMakerNote(MakerNote* pMakerNote);
        //@}

        //! @name Accessors
        //@{
        virtual std::string key() const { return key_; }
        virtual const char* familyName() const { return ExifTags::familyName(); }
        /*!
          @brief Return the name of the group (the second part of the key).
                 For Exif keys, the group name is the IFD name.
        */ 
        virtual std::string groupName() const { return ifdName(); }
        virtual std::string tagName() const;
        /*!
          @brief Return the tag.
          @throw Error ("Invalid key") if the tag is not set.
        */
        virtual uint16 tag() const;
        virtual ExifKey* clone() const;

        //! Interpret and print the value of an Exif tag
        std::ostream& printTag(std::ostream& os, const Value& value) const;
        /*!
          @brief Return the IFD id
          @throw Error ("Invalid key") if the IFD id is not set.
        */
        IfdId ifdId() const;
        //! Return the name of the IFD
        const char* ifdName() const { return ExifTags::ifdName(ifdId()); }
        //! Return the related image item
        const char* ifdItem() const { return ExifTags::ifdItem(ifdId()); }
        //! Return the name of the Exif section (deprecated) 
        std::string sectionName() const;
        //! Return the index (unique id of this key within the original IFD)
        int idx() const { return idx_; }
        //! Return the pointer to the associated MakerNote
        MakerNote* makerNote() const { return pMakerNote_; }
        //@}

    protected:
        //! @name Manipulators
        //@{
        /*!
          @brief Parse and convert the key string into tag, and IFD Id. 
                 Forwards the request to MakerNote::decomposeKey if necessary.
                 Updates key_ and ifdId_ if the string can be decomposed,
                 or throws Error ("Invalid key").

          @throw Error ("Invalid key") if pMakerNote_ is ot 0 and the key 
                 cannot be parsed into family name, group name and tag name 
                 parts.
         */
        void decomposeKey();
        //@}

    private:
        // DATA
        uint16 tag_;                    //!< Tag value
        IfdId ifdId_;                   //!< The IFD associated with this tag
        int idx_;                       //!< Unique id of an entry within one IFD
        MakerNote* pMakerNote_;         //!< Pointer to the associated MakerNote
        std::string key_;               //!< Key
    }; // class ExifKey

    /*!
      @brief Information related to one Exif tag.
     */
    class Exifdatum : public Metadatum {
        friend std::ostream& operator<<(std::ostream&, const Exifdatum&);
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor for new tags created by an application. The
                 %Exifdatum is created from a key / value pair. %Exifdatum copies
                 (clones) the key and value if one is provided. Alternatively, 
                 a program can create an 'empty' %Exifdatum with only a key
                 and set the value using setValue().

          @param key ExifKey.
          @param pValue Pointer to a Exifdatum value.
          @throw Error ("Invalid key") if the key cannot be parsed and converted.
         */
        explicit Exifdatum(const ExifKey& key, const Value* pValue =0);
        //! Constructor to build a Exifdatum from an IFD entry.
        Exifdatum(const Entry& e, ByteOrder byteOrder);
        //! Copy constructor
        Exifdatum(const Exifdatum& rhs);
        //! Destructor
        virtual ~Exifdatum();
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator
        Exifdatum& operator=(const Exifdatum& rhs);
        /*!
          @brief Set the value. This method copies (clones) the value pointed
                 to by pValue.
         */
        void setValue(const Value* pValue);
        /*!
          @brief Set the value from an IFD entry.
         */
        void setValue(const Entry& e, ByteOrder byteOrder);
        /*!
          @brief Set the value to the string buf. 
                 Uses Value::read(const std::string& buf). If the Exifdatum does
                 not have a value yet, then an AsciiValue is created.
         */
        void setValue(const std::string& buf);
        //@}

        //! @name Accessors
        //@{
        //! Return the key of the %Exifdatum. 
        std::string key() const 
            { return pKey_ == 0 ? "" : pKey_->key(); }
        //! Return the name of the group (the second part of the key)
        std::string groupName() const
            { return pKey_ == 0 ? "" : pKey_->groupName(); }
        //! Return the name of the tag (which is also the third part of the key)
        std::string tagName() const
            { return pKey_ == 0 ? "" : pKey_->tagName(); }
        //! Return the tag
        uint16 tag() const
            { return pKey_ == 0 ? 0xffff : pKey_->tag(); }
        //! Return the IFD id
        IfdId ifdId() const 
            { return pKey_ == 0 ? ifdIdNotSet : pKey_->ifdId(); }
        //! Return the name of the IFD
        const char* ifdName() const
            { return pKey_ == 0 ? "" : pKey_->ifdName(); }
        //! Return the related image item (deprecated)
        const char* ifdItem() const 
            { return pKey_ == 0 ? "" : pKey_->ifdItem(); }
        //! Return the index (unique id of this key within the original IFD)
        int idx() const
            { return pKey_ == 0 ? 0 : pKey_->idx(); }
        /*!
          @brief Write value to a data buffer and return the number
                 of bytes written.

          The user must ensure that the buffer has enough memory. Otherwise
          the call results in undefined behaviour.

          @param buf Data buffer to write to.
          @param byteOrder Applicable byte order (little or big endian).
          @return Number of characters written.
        */
        long copy(byte* buf, ByteOrder byteOrder) const 
            { return pValue_ == 0 ? 0 : pValue_->copy(buf, byteOrder); }
        //! Return the type id of the value
        TypeId typeId() const 
            { return pValue_ == 0 ? invalidTypeId : pValue_->typeId(); }
        //! Return the name of the type
        const char* typeName() const 
            { return TypeInfo::typeName(typeId()); }
        //! Return the size in bytes of one component of this type
        long typeSize() const 
            { return TypeInfo::typeSize(typeId()); }
        //! Return the number of components in the value
        long count() const 
            { return pValue_ == 0 ? 0 : pValue_->count(); }
        //! Return the size of the value in bytes
        long size() const 
            { return pValue_ == 0 ? 0 : pValue_->size(); }
        //! Return the value as a string.
        std::string toString() const 
            { return pValue_ == 0 ? "" : pValue_->toString(); }
        /*!
          @brief Return the n-th component of the value converted to long. The
                 return value is -1 if the value of the Exifdatum is not set and
                 the behaviour of the method is undefined if there is no n-th
                 component.
         */
        long toLong(long n =0) const 
            { return pValue_ == 0 ? -1 : pValue_->toLong(n); }
        /*!
          @brief Return the n-th component of the value converted to float.  The
                 return value is -1 if the value of the Exifdatum is not set and
                 the behaviour of the method is undefined if there is no n-th
                 component.
         */
        float toFloat(long n =0) const 
            { return pValue_ == 0 ? -1 : pValue_->toFloat(n); }
        /*!
          @brief Return the n-th component of the value converted to
                 Rational. The return value is -1/1 if the value of the
                 Exifdatum is not set and the behaviour of the method is
                 undefined if there is no n-th component.
         */
        Rational toRational(long n =0) const 
            { return pValue_ == 0 ? Rational(-1, 1) : pValue_->toRational(n); }
        /*!
          @brief Return a pointer to a copy (clone) of the value. The caller
                 is responsible to delete this copy when it's no longer needed.

          This method is provided for users who need full control over the 
          value. A caller may, e.g., downcast the pointer to the appropriate
          subclass of Value to make use of the interface of the subclass to set
          or modify its contents.
          
          @return A pointer to a copy (clone) of the value, 0 if the value is 
                  not set.
         */
        Value* getValue() const 
            { return pValue_ == 0 ? 0 : pValue_->clone(); }
        /*!
          @brief Return a constant reference to the value. 

          This method is provided mostly for convenient and versatile output of
          the value which can (to some extent) be formatted through standard
          stream manipulators.  Do not attempt to write to the value through
          this reference. 

          <b>Example:</b> <br>
          @code
          ExifData::const_iterator i = exifData.findKey(key);
          if (i != exifData.end()) {
              std::cout << i->key() << " " << std::hex << i->value() << "\n";
          }
          @endcode

          @return A constant reference to the value.
          @throw Error ("Value not set") if the value is not set.
         */
        const Value& value() const 
            { if (pValue_) return *pValue_; throw Error("Value not set"); }
        //@}

    private:
        // DATA
        ExifKey* pKey_;                //!< Key 
        Value* pValue_;                //!< Pointer to the value

    }; // class Exifdatum

    /*!
      @brief Output operator for Exifdatum types, prints the interpreted
             tag value.
     */
    std::ostream& operator<<(std::ostream& os, const Exifdatum& md);

    /*!
      @brief Exif %Thumbnail image. This abstract base class provides the
             interface for the thumbnail image that is optionally embedded in
             the Exif data.
     */
    class Thumbnail {
    public:
        //! @name Creators
        //@{
        //! Virtual destructor
        virtual ~Thumbnail() {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the thumbnail from the data buffer buf, using Exif
                 metadata exifData. Return 0 if successful. 

          @param buf Data buffer containing the thumbnail data. The buffer must
                 start with the TIFF header.
          @param len Number of bytes in the data buffer.
          @param exifData Exif data corresponding to the data buffer.
          @param byteOrder The byte order used for the encoding of TIFF
                 thumbnails. It determines the byte order of the resulting
                 thumbnail image, if it is in TIFF format. For JPEG thumbnails
                 the byte order is not used.
          @return 0 if successful<br>
                 -1 if there is no thumbnail image in the Exif data<BR>
                  1 in case of inconsistent JPEG thumbnail Exif data<BR>
                  2 in case of inconsistent TIFF thumbnail Exif data
         */
        virtual int read(const byte* buf,
                         long len,
                         const ExifData& exifData,
                         ByteOrder byteOrder =littleEndian) =0;
        /*!
          @brief Update the internal offset and the thumbnail data offsets 
                 in IFD1 assuming the thumbnail data follows immediately after
                 IFD1.  

          If the type of the thumbnail image is JPEG, JPEGInterchangeFormat is
          set to point directly behind the data area of IFD1. If the type is
          TIFF, StripOffsets from the thumbnail image are adjusted to point to
          the strips, which have to follow immediately after IFD1. Use copy() to
          write the thumbnail image data. The offset of IFD1 must be set
          correctly. Changing the size or data size of IFD1 invalidates the
          thumbnail data offsets set by this method.
         */
        virtual void setOffsets(Ifd& ifd1, ByteOrder byteOrder) =0;
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief  Write thumbnail to file path.
          @return  0 if successful;<BR>
                  -1 if the file couldn't be open;<BR>
                   4 if writing to the output stream failed.
         */
        virtual int write(const std::string& path) const =0;
        /*!
          @brief Return a short string for the format of the thumbnail 
                 ("TIFF", "JPEG").
         */
        virtual const char* format() const =0;
        /*!
          @brief Return the file extension for the format of the thumbnail 
                 (".tif", ".jpg").
         */
        virtual const char* extension() const =0;
        /*!
          @brief Copy the thumbnail image data (without the IFD, if any) to the
                 data buffer buf. The user must ensure that the buffer has
                 enough memory. Otherwise the call results in undefined
                 behaviour. Return the number of characters written.
         */
        virtual long copy(byte* buf) const =0;
        /*!
          @brief Update the Exif data according to the actual thumbnail image.
          
          If the type of the thumbnail image is JPEG, JPEGInterchangeFormat is
          set to 0. If the type is TIFF, StripOffsets are set to the offsets of
          the IFD of the thumbnail image itself.
         */
        virtual void update(ExifData& exifData) const =0;
        /*!
          @brief Return the position of the thumbnail image data from the 
                 start of the TIFF header in the original Exif data.
         */
        virtual long offset() const =0;
        /*!
          @brief Return the size of the thumbnail image (the size it
                 would occupy when extracted from the Exif data)
         */
        virtual long size() const =0;
        /*!
          @brief Return the size of the thumbnail data (data only, without the 
                 IFD, in case of a TIFF thumbnail).
         */
        virtual long dataSize() const =0;
        //@}

    protected:
        //! @name Manipulators
        //@{
        /*!
          @brief Assignment operator. Protected so that it can only be used
                 by subclasses but not directly.
         */
        Thumbnail& operator=(const Thumbnail& rhs);
        //@}

    }; // class Thumbnail

    //! Exif thumbnail image in TIFF format
    class TiffThumbnail : public Thumbnail {
    public:
        //! @name Creators
        //@{
        //! Virtual destructor
        TiffThumbnail();
        //! Virtual destructor
        virtual ~TiffThumbnail();
        //! Copy constructor
        TiffThumbnail(const TiffThumbnail& rhs);
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator.
        TiffThumbnail& operator=(const TiffThumbnail& rhs);
        int read(const byte* buf,
                 long len,
                 const ExifData& exifData,
                 ByteOrder byteOrder =littleEndian);
        void setOffsets(Ifd& ifd1, ByteOrder byteOrder);
        //@}

        //! @name Accessors
        //@{
        int write(const std::string& path) const;
        const char* format() const;
        const char* extension() const;
        long copy(byte* buf) const;
        void update(ExifData& exifData) const;
        long offset() const;
        long size() const;
        long dataSize() const;
        //@}

    private:
        // DATA
        long offset_;            // Original offset of the thumbnail data
                                 // from the start of the TIFF header 
        long size_;              //!< Size of the image data
        byte* pImage_;           //!< Thumbnail image data
        TiffHeader tiffHeader_;  //!< Thumbnail TIFF Header
        Ifd ifd_;                //!< Thumbnail IFD (IFD1 of the Exif data)

    }; // class TiffThumbnail

    //! Exif thumbnail image in JPEG format
    class JpegThumbnail : public Thumbnail {
    public:
        //! @name Creators
        //@{
        //! Virtual destructor
        JpegThumbnail();
        //! Virtual destructor
        virtual ~JpegThumbnail();
        //! Copy constructor
        JpegThumbnail(const JpegThumbnail& rhs);
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator.
        JpegThumbnail& operator=(const JpegThumbnail& rhs);
        int read(const byte* buf,
                 long len,
                 const ExifData& exifData,
                 ByteOrder byteOrder =littleEndian);
        void setOffsets(Ifd& ifd1, ByteOrder byteOrder);
        //@}

        //! @name Accessors
        //@{
        int write(const std::string& path) const;
        const char* format() const;
        const char* extension() const;
        long copy(byte* buf) const;
        void update(ExifData& exifData) const;
        long offset() const;
        long size() const;
        long dataSize() const;
        //@}

    private:
        // DATA
        long offset_;            // Original offset of the thumbnail data 
                                 // from the start of the TIFF header 
        long size_;              // Size of the image data
        byte* pImage_;           // Thumbnail image data

    }; // class JpegThumbnail

    //! Container type to hold all metadata
    typedef std::vector<Exifdatum> ExifMetadata;

    //! Unary predicate that matches a Exifdatum with a given ifd id and idx
    class FindMetadatumByIfdIdIdx {
    public:
        //! Constructor, initializes the object with the ifd id and idx to look for
        FindMetadatumByIfdIdIdx(IfdId ifdId, int idx)
            : ifdId_(ifdId), idx_(idx) {}
        /*!
          @brief Returns true if the ifd id and idx of the argument Exifdatum 
                 is equal to that of the object.
        */
        bool operator()(const Exifdatum& exifdatum) const
            { return ifdId_ == exifdatum.ifdId() && idx_ == exifdatum.idx(); }

    private:
        IfdId ifdId_;
        int idx_;
        
    }; // class FindMetadatumByIfdIdIdx

    /*!
      @brief A container for Exif data. This is a top-level class of 
             the %Exiv2 library.

      Provide high-level access to the Exif data of an image:
      - read Exif information from JPEG files
      - access metadata through keys and standard C++ iterators
      - add, modify and delete metadata 
      - write Exif data to JPEG files
      - extract Exif metadata to files, insert from these files
      - extract and delete Exif thumbnail (JPEG and TIFF thumbnails)
    */
    class ExifData {
        //! @name Not implemented
        //@{
        //! Copying not allowed (Todo: implement me!)
        ExifData(const ExifData& rhs);
        //! Assignment not allowed (IFD does not have an assignment operator)
        ExifData& operator=(const ExifData& rhs);
        //@}
    public:
        //! ExifMetadata iterator type
        typedef ExifMetadata::iterator iterator;
        //! ExifMetadata const iterator type
        typedef ExifMetadata::const_iterator const_iterator;

        //! @name Creators
        //@{
        //! Default constructor
        ExifData();
        //! Destructor
        ~ExifData();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the Exif data from file path.
          @param path Path to the file
          @return  0 if successful;<BR>
                   3 if the file contains no Exif data;<BR>
                  the return code of Image::readMetadata()
                    if the call to this function fails<BR>
                  the return code of read(const char* buf, long len)
                    if the call to this function fails
         */
        int read(const std::string& path);
        /*!
          @brief Read the Exif data from a byte buffer. The data buffer
                 must start with the TIFF header.
          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer 
          @return 0 if successful.
         */
        int read(const byte* buf, long len);
        /*!
          @brief Write the Exif data to file path. If an Exif data section
                 already exists in the file, it is replaced.  If there is no
                 metadata and no thumbnail to write, the Exif data section is
                 deleted from the file.  Otherwise, an Exif data section is
                 created. See copy(byte* buf) for further details.

          @return 0 if successful.
         */
        int write(const std::string& path);
        /*!
          @brief Write the Exif data to a binary file. By convention, the
                 filename extension should be ".exv". This file format contains
                 the Exif data as it is found in a JPEG file, starting with the
                 APP1 marker 0xffe1, the size of the data and the string
                 "Exif\0\0". Exv files can be read with 
                 int read(const std::string& path) just like image Exif data.
         */
        int writeExifData(const std::string& path);
        /*!
          @brief Write the Exif data to a data buffer, return number of bytes 
                 written. The copied data starts with the TIFF header.

          Tries to update the original data buffer and write it back with
          minimal changes, in a 'non-intrusive' fashion, if possible. In this
          case, tag data that ExifData does not understand stand a good chance
          to remain valid. (In particular, if the Exif data contains a
          Makernote in IFD format, the offsets in its IFD will remain valid.)
          <BR>
          If 'non-intrusive' writing is not possible, the Exif data will be
          re-built from scratch, in which case the absolute position of the
          metadata entries within the data buffer may (and in most cases will)
          be different from their original position. Furthermore, in this case,
          the Exif data is updated with the metadata from the actual thumbnail
          image (overriding existing metadata).

          @param buf The data buffer to write to.  The user must ensure that the
                 buffer has enough memory. Otherwise the call results in
                 undefined behaviour.
          @return Number of characters written to the buffer.
         */
        long copy(byte* buf);
        /*!
          @brief Add all (IFD) entries in the range from iterator position begin
                 to iterator position end to the Exif metadata. No duplicate
                 checks are performed, i.e., it is possible to add multiple
                 metadata with the same key.
         */
        void add(Entries::const_iterator begin, 
                 Entries::const_iterator end,
                 ByteOrder byteOrder);
        /*!
          @brief Add a Exifdatum from the supplied key and value pair.  This
                 method copies (clones) key and value and adds a pointer to
                 the MakerNote to the cloned key. No duplicate checks are
                 performed, i.e., it is possible to add multiple metadata with
                 the same key.
         */
        void add(const ExifKey& key, Value* pValue);
        /*! 
          @brief Add a copy of the Exifdatum to the Exif metadata.  No
                 duplicate checks are performed, i.e., it is possible to add
                 multiple metadata with the same key.
         */
        void add(const Exifdatum& Exifdatum);
        /*!
          @brief Delete the Exifdatum at iterator position pos, return the 
                 position of the next Exifdatum. Note that iterators into
                 the metadata, including pos, are potentially invalidated 
                 by this call.
         */
        iterator erase(iterator pos);
        //! Sort metadata by key
        void sortByKey();
        //! Sort metadata by tag
        void sortByTag();
        //! Begin of the metadata
        iterator begin() { return exifMetadata_.begin(); }
        //! End of the metadata
        iterator end() { return exifMetadata_.end(); }
        /*!
          @brief Find a Exifdatum with the given key, return an iterator to it.
                 If multiple metadata with the same key exist, it is undefined 
                 which of the matching metadata is found.
         */
        iterator findKey(const ExifKey& key);
        /*!
          @brief Find the Exifdatum with the given ifd id and idx, return an 
                 iterator to it. 

          This method can be used to uniquely identify a Exifdatum that was
          created from an IFD or from the makernote (with idx greater than
          0). Metadata created by an application (not read from an IFD or a
          makernote) all have their idx field set to 0, i.e., they cannot be
          uniquely identified with this method.  If multiple metadata with the
          same key exist, it is undefined which of the matching metadata is
          found.
         */
        iterator findIfdIdIdx(IfdId ifdId, int idx);
        /*!
          @brief Delete the thumbnail from the Exif data. Removes all related
                 (%Thumbnail.*.*, i.e., IFD1) metadata as well.

          @return The number of bytes of thumbnail data erased from the original
                  Exif data. Note that the original image size may differ from
                  the size of the image after deleting the thumbnail by more
                  than this number. This is the case if the Exif data contains
                  extra bytes (often at the end of the Exif block) or gaps and
                  the thumbnail is not located at the end of the Exif block so
                  that non-intrusive writing of a truncated Exif block is not
                  possible. Instead it is in this case necessary to write the
                  Exif data, without the thumbnail, from the metadata and all
                  extra bytes and gaps are lost, resulting in a smaller image.
         */
        long eraseThumbnail();
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Erase the Exif data section from file path. 
          @param path Path to the file.
          @return 0 if successful.
         */
        int erase(const std::string& path) const;
        //! Begin of the metadata
        const_iterator begin() const { return exifMetadata_.begin(); }
        //! End of the metadata
        const_iterator end() const { return exifMetadata_.end(); }
        /*!
          @brief Find a Exifdatum with the given key, return a const iterator to
                 it.  If multiple metadata with the same key exist, it is
                 undefined which of the matching metadata is found.
         */
        const_iterator findKey(const ExifKey& key) const;
        /*!
          @brief Find the Exifdatum with the given ifd id and idx, return an 
                 iterator to it. 

          This method can be used to uniquely identify a Exifdatum that was
          created from an IFD or from the makernote (with idx greater than
          0). Metadata created by an application (not read from an IFD or a
          makernote) all have their idx field set to 0, i.e., they cannot be
          uniquely identified with this method.  If multiple metadata with the
          same key exist, it is undefined which of the matching metadata is
          found.
         */
        const_iterator findIfdIdIdx(IfdId ifdId, int idx) const;
        //! Get the number of metadata entries
        long count() const { return static_cast<long>(exifMetadata_.size()); }
        /*!
          @brief Return the approximate size of all Exif data (TIFF header plus 
                 metadata). The number returned may be bigger than the actual 
                 size of the Exif data, but it is never smaller. Only copy()
                 returns the exact size.
         */
        long size() const;
        //! Returns the byte order as specified in the TIFF header
        ByteOrder byteOrder() const { return tiffHeader_.byteOrder(); }
        /*!
          @brief Write the thumbnail image to a file. A filename extension
                 is appended to path according to the image type of the
                 thumbnail, so the path should not include an extension.
                 This will overwrite an existing file of the same name.
         */
        int writeThumbnail(const std::string& path) const 
            { return pThumbnail_ ? pThumbnail_->write(path) : 0; }
        /*!
          @brief Return a short string describing the format of the Exif 
                 thumbnail ("TIFF", "JPEG").
         */
        const char* thumbnailFormat() const
            { return pThumbnail_ ? pThumbnail_->format() : ""; }
        /*!
          @brief Return the file extension for the Exif thumbnail depending
                 on the format (".tif", ".jpg").
         */
        const char* thumbnailExtension() const 
            { return pThumbnail_ ? pThumbnail_->extension() : ""; }
        /*!
          @brief Return the size of the thumbnail image. This is the size that
                 the thumbnail would occupy when extracted from the Exif data.
         */
        long thumbnailSize() const 
            { return pThumbnail_ ? pThumbnail_->size() : 0; }
        //@}

        /*!
          @brief Convert the return code from 
                 int read(const std::string& path),
                 int write(const std::string& path),
                 int writeExifData(const std::string& path), 
                 int writeThumbnail(const std::string& path) const and 
                 int erase(const std::string& path) const 
                 into an error message.

                 Todo: Implement global handling of error messages
         */
        static std::string strError(int rc, const std::string& path);

    private:
        //! @name Manipulators
        //@{
        /*!
          @brief Read the thumbnail from the data buffer. Return 0 if successful,
                 otherwise the thumbnail is deleted.
         */
        int readThumbnail();
        /*!
          @brief Check if the metadata changed and update the internal IFDs and
                 the MakerNote if the changes are compatible with the existing
                 data (non-intrusive write support). 

          @return True if only compatible changes were detected in the metadata
                  and the internal IFDs and MakerNote (and thus the data buffer)
                  were updated successfully. Return false, if non-intrusive
                  writing is not possible. The internal IFDs and the MakerNote
                  (and thus the data buffer) may or may not be modified in this
                  case.
         */
        bool updateEntries();
        /*!
          @brief Update the metadata for a range of entries. Called by
                 updateEntries() for each of the internal IFDs and the MakerNote
                 (if any).
         */
        bool updateRange(const Entries::iterator& begin,
                         const Entries::iterator& end,
                         ByteOrder byteOrder);
        /*!
          @brief Write Exif data to a data buffer the hard way, return number of
                 bytes written.

          Rebuilds the Exif data from scratch, using the TIFF header, metadata
          container and thumbnail. In particular, the internal IFDs and the
          original data buffer are not used. Furthermore, this method updates
          the Exif data with the metadata from the actual thumbnail image
          (overriding existing metadata).
         */
        long copyFromMetadata(byte* buf);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Check if the metadata is compatible with the internal IFDs for
                 non-intrusive writing. Return true if compatible, false if not.

          @note This function does not detect deleted metadata as incompatible,
                although the deletion of metadata is not (yet) a supported
                non-intrusive write operation.
         */
        bool compatible() const;
        /*!
          @brief Find the IFD or makernote entry corresponding to ifd id and idx.

          @return A pair of which the first part determines if a match was found
                  and, if true, the second contains an iterator to the entry.
         */
        std::pair<bool, Entries::const_iterator> 
        findEntry(IfdId ifdId, int idx) const;
        //! Return a pointer to the internal IFD identified by its IFD id
        const Ifd* getIfd(IfdId ifdId) const;
        /*! 
          @brief Check if IFD1, the IFD1 data and thumbnail data are located at 
                 the end of the Exif data. Return true, if they are or if there
                 is no thumbnail at all, else return false.
         */
        bool stdThumbPosition() const;
        //@}

        // DATA
        TiffHeader tiffHeader_;
        ExifMetadata exifMetadata_;
        Thumbnail* pThumbnail_;  //!< Pointer to the Exif thumbnail image
        MakerNote* pMakerNote_;  //!< Pointer to the MakerNote
                                 //   Todo: implement reference counting instead
                                 //   of making ExifData own this pointer

        Ifd ifd0_;
        Ifd exifIfd_;
        Ifd iopIfd_;
        Ifd gpsIfd_;
        Ifd ifd1_;

        long size_;              //!< Size of the Exif raw data in bytes
        byte* pData_;            //!< Exif raw data buffer

        /*!
          Can be set to false to indicate that non-intrusive writing is not
          possible. If it is true (the default), then the compatibility checks
          will be performed to determine which writing method to use.
         */
        bool compatible_;

    }; // class ExifData

// *****************************************************************************
// free functions

    /*!
      @brief Add all metadata in the range from iterator position begin to
             iterator position end, which have an IFD id matching that of the
             IFD to the list of directory entries of ifd.  No duplicate checks
             are performed, i.e., it is possible to add multiple metadata with
             the same key to an IFD.
     */
    void addToIfd(Ifd& ifd,
                  ExifMetadata::const_iterator begin, 
                  ExifMetadata::const_iterator end, 
                  ByteOrder byteOrder);
    /*!
      @brief Add the Exifdatum to the IFD.  No duplicate checks are performed,
             i.e., it is possible to add multiple metadata with the same key to
             an IFD.
     */
    void addToIfd(Ifd& ifd, const Exifdatum& exifdatum, ByteOrder byteOrder);
    /*!
      @brief Add all metadata in the range from iterator position begin to
             iterator position end with IFD id 'makerIfd' to the list of
             makernote entries of the object pointed to be makerNote.  No
             duplicate checks are performed, i.e., it is possible to add
             multiple metadata with the same key to a makernote.
     */
    void addToMakerNote(MakerNote* makerNote,
                        ExifMetadata::const_iterator begin,
                        ExifMetadata::const_iterator end, 
                        ByteOrder byteOrder);
    /*!
      @brief Add the Exifdatum to makerNote, encoded in byte order byteOrder.
             No duplicate checks are performed, i.e., it is possible to add
             multiple metadata with the same key to a makernote.
     */
    void addToMakerNote(MakerNote* makerNote,
                        const Exifdatum& exifdatum,
                        ByteOrder byteOrder);
    /*!
      @brief Return a key for the entry.  The key is of the form
             '<b>Exif</b>.ifdItem.tagName'. This function knows about
             MakerNotes, i.e., it will invoke MakerNote::makeKey if necessary.
    */
    std::string makeKey(const Entry& entry);
    /*!
      @brief Return the tag and IFD id pair for the key. This function knows
             about MakerNotes, i.e., it will forward the request to
             MakerNote::decomposeKey if necessary.
      @return A pair consisting of the tag and IFD id.
      @throw Error ("Invalid key") if the key cannot be parsed into
      item item, section name and tag name parts.
    */
    std::pair<uint16, IfdId> decomposeKey(const std::string& key,
                                          const MakerNote* makerNote);

}                                       // namespace Exiv2

#endif                                  // #ifndef EXIF_HPP_

