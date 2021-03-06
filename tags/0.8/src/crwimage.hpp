// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2005 Andreas Huggel <ahuggel@gmx.net>
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
  @file    crwimage.hpp
  @brief   Class CrwImage to access Canon Crw images.<BR>
           References:<BR>
           <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/canon_raw.html">The Canon RAW (CRW) File Format</a> by Phil Harvey
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    28-Aug-05, ahu: created
 */
#ifndef CRWIMAGE_HPP_
#define CRWIMAGE_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "image.hpp"
#include "basicio.hpp"
#include "exif.hpp"
#include "iptc.hpp"

// + standard includes
#include <iostream>
#include <string>
#include <vector>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class CiffComponent;
    struct CrwMapInfo;

// *****************************************************************************
// type definitions

    //! Function pointer for functions to extract Exif tags from a Crw entry
    typedef void (*CrwExtractFct)(const CiffComponent&,
                                  const CrwMapInfo*,
                                  Image&,
                                  ByteOrder);

    //! Function pointer for functions to insert Crw entries from an Exif tag
    typedef void (*CrwInsertFct)();

// *****************************************************************************
// class definitions

    // Add Crw to the supported image formats
    namespace ImageType {
        const int crw = 3;          //!< Crw image type (see class CrwImage)
    }

    /*!
      @brief Class to access raw Canon Crw images. Only Exif metadata and a
             comment are supported. Crw format does not contain Iptc metadata.
     */
    class CrwImage : public Image {
        friend bool isCrwType(BasicIo& iIo, bool advance);

        //! @name NOT Implemented
        //@{
        //! Copy constructor
        CrwImage(const CrwImage& rhs);
        //! Assignment operator
        CrwImage& operator=(const CrwImage& rhs);
        //@}

    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor that can either open an existing Crw image or create
              a new image from scratch. If a new image is to be created, any
              existing data is overwritten. Since the constructor can not return
              a result, callers should check the good() method after object
              construction to determine success or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method.  Use the Image::io()
              method to get a temporary reference.
          @param create Specifies if an existing image should be read (false)
              or if a new file should be created (true).
         */
        CrwImage(BasicIo::AutoPtr io, bool create);
        //! Destructor
        ~CrwImage() {}
        //@}

        //! @name Manipulators
        //@{
        void            readMetadata();
        /*!
          @brief Todo: Write metadata back to the image. This method is not
                 yet implemented.
         */
        void            writeMetadata();
        void            setExifData(const ExifData& exifData);
        void            clearExifData();
        /*!
          @brief Not supported. Crw format does not contain Iptc metadata.
                 Calling this function will raise an exception (Error).
         */
        void            setIptcData(const IptcData& iptcData);
        /*!
          @brief Not supported. Crw format does not contain Iptc metadata.
                 Calling this function will raise an exception (Error).
         */
        void            clearIptcData();
        void            setComment(const std::string& comment);
        void            clearComment();
        void            setMetadata(const Image& image);
        void            clearMetadata();
        ExifData&       exifData()       { return exifData_; }
        IptcData&       iptcData()       { return iptcData_; }
        //@}

        //! @name Accessors
        //@{
        bool            good()     const;
        const ExifData& exifData() const { return exifData_; }
        const IptcData& iptcData() const { return iptcData_; }
        std::string     comment()  const { return comment_; }
        BasicIo&        io()       const { return *io_; }
        //@}

    private:

        //! @name Manipulators
        //@{
        int initImage(const byte initData[], long dataSize);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Determine if the content of the BasicIo instance is a Crw image.

          The advance flag determines if the read position in the stream is
          moved (see below). This applies only if the type matches and the
          function returns true. If the type does not match, the stream
          position is not changed. However, if reading from the stream fails,
          the stream position is undefined. Consult the stream state to obtain
          more information in this case.

          @param iIo BasicIo instance to read from.
          @param advance Flag indicating whether the position of the io
              should be advanced by the number of characters read to
              analyse the data (true) or left at its original
              position (false). This applies only if the type matches.
          @return  true  if the data matches the type of this class;<BR>
                   false if the data does not match
         */
        bool isThisType(BasicIo& iIo, bool advance) const;
        /*!
          @brief Todo: Write Crw header. Not implemented yet.
         */
        int writeHeader(BasicIo& oIo) const;
        //@}

        // DATA
        static const byte blank_[];             //!< Minimal Crw image

        BasicIo::AutoPtr  io_;                  //!< Image data io pointer
        ExifData          exifData_;            //!< Exif data container
        IptcData          iptcData_;            //!< Iptc data container
        std::string       comment_;             //!< User comment

    }; // class CrwImage

    /*!
      Base class for all objects in a raw metadata parse tree.
      Defines the interface for raw metadata composites.
     */
    class RawMetadata {
    public:
        //! RawMetadata auto_ptr type
        typedef std::auto_ptr<RawMetadata> AutoPtr;
        //! Container type to hold all metadata
        typedef std::vector<RawMetadata*> Components;

        //! @name Creators
        //@{
        //! Virtual Destructor
        virtual ~RawMetadata() {}
        //@}

        //! @name Manipulators
        //@{
        //! Add a component to the composition
        virtual void add(AutoPtr component) =0;
        /*!
          @brief Read a component from a data buffer

          @param buf Pointer to the data buffer.
          @param len Number of bytes in the data buffer.
          @param start Component starts at \em buf + \em start.
          @param byteOrder Applicable byte order (little or big endian).
          @param shift offsets in component are relative to \em buf + \em shift.

          @throw Error If the component cannot be parsed.
         */
        virtual void read(const byte* buf,
                          uint32_t len,
                          uint32_t start,
                          ByteOrder byteOrder,
                          int32_t shift =0) =0;
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Extract metadata from the component and add it to
                 \em image.

          @param image Image to add metadata to
          @param byteOrder Byte order
         */
        virtual void extract(Image& image,
                             ByteOrder byteOrder) const =0;

        /*!
          @brief Print debug info about a component to \em os.

          @param os Output stream to write to
          @param byteOrder Byte order
          @param prefix Prefix to be written before each line of output
         */
        virtual void print(std::ostream& os,
                           ByteOrder byteOrder,
                           const std::string& prefix ="") const =0;
        //@}

    }; // class RawMetadata

    /*!
      @brief Interface class for components of the CIFF directory hierarchy of
             a Crw (Canon Raw data) image. Both CIFF directories as well as
             entries implement this interface.
     */
    class CiffComponent : public RawMetadata {
    public:
        //! @name Creators
        //@{
        // Default and copy constructors are fine

        //! Virtual destructor.
        virtual ~CiffComponent() {}
        //@}

        //! @name Manipulators
        //@{
        // Default assignment operator is fine

        // See base class comment
        virtual void add(RawMetadata::AutoPtr component) =0;

        // See base class comment
        virtual void read(const byte* buf,
                          uint32_t len,
                          uint32_t start,
                          ByteOrder byteOrder,
                          int32_t shift =0);

        //! Set the directory tag for this component.
        void setDir(uint16_t dir)      { dir_ = dir; }
        //@}

        //! Return the type id for a tag
        static TypeId typeId(uint16_t tag);
        //! Return the data location id for a tag
        static DataLocId dataLocation(uint16_t tag);

        //! @name Accessors
        //@{
        // See base class comment
        virtual void extract(Image& image,
                             ByteOrder byteOrder) const =0;

        // See base class comment
        virtual void print(std::ostream& os,
                           ByteOrder byteOrder,
                           const std::string& prefix ="") const;

        //! Return the tag of the directory containing this component
        uint16_t dir()           const { return dir_; }

        //! Return the tag of this component
        uint16_t tag()           const { return tag_; }

        /*!
          @brief Return the data size of this component

          @note If the data is contained in the directory entry itself,
                this method returns 8, which is the maximum number
                of data bytes this component can have. The actual size,
                i.e., used data bytes, may be less than 8.
         */
        uint32_t size()          const { return size_; }

        //! Return the offset to the data from the start of the directory
        uint32_t offset()        const { return offset_; }

        //! Return a pointer to the data area of this component
        const byte* pData()      const { return pData_; }

        //! Return the tag id of this component
        uint16_t tagId()         const { return tag_ & 0x3fff; }

        //! Return the type id of thi component
        TypeId typeId()          const { return typeId(tag_); }

        //! Return the data location for this component
        DataLocId dataLocation() const { return dataLocation(tag_); }
        //@}

    private:
        // DATA
        uint16_t    dir_;    //!< Tag of the directory containing this component
        uint16_t    tag_;    //!< Tag of the entry
        uint32_t    size_;   //!< Size of the data area
        uint32_t    offset_; //!< Offset to the data area from the start of the dir
        const byte* pData_;  //!< Pointer to the data area

    }; // class CiffComponent

    /*!
      @brief This class models one directory entry of a CIFF directory of
             a Crw (Canon Raw data) image.
     */
    class CiffEntry : public CiffComponent {
    public:
        //! @name Creators
        //@{
        // Default and copy constructors are fine

        //! Virtual destructor.
        virtual ~CiffEntry() {}
        //@}

        //! @name Manipulators
        //@{
        // Default assignment operator is fine

        // See base class comment
        virtual void add(RawMetadata::AutoPtr component);
        //@}

        //! @name Accessors
        //@{
        // See base class comment
        virtual void extract(Image& image, ByteOrder byteOrder) const;
        //@}

    }; // class CiffEntry

    //! This class models a CIFF directory of a Crw (Canon Raw data) image.
    class CiffDirectory : public CiffComponent {
    public:
        //! @name Creators
        //@{
        // Default and copy constructors are fine

        //! Virtual destructor
        virtual ~CiffDirectory();
        //@}

        //! @name Manipulators
        //@{
        // Default assignment operator is fine

        // See base class comment
        virtual void add(RawMetadata::AutoPtr component);

        // See base class comment
        virtual void read(const byte* buf,
                          uint32_t len,
                          uint32_t start,
                          ByteOrder byteOrder,
                          int32_t shift =0);

        /*!
          @brief Parse a CIFF directory from a memory buffer

          @param buf       Pointer to the memory buffer containing the directory
          @param len       Size of the memory buffer
          @param start     Directory starts at buf + start
          @param byteOrder Applicable byte order (little or big endian)
          @param shift     Not used
         */
        void readDirectory(const byte* buf,
                           uint32_t len,
                           uint32_t start,
                           ByteOrder byteOrder,
                           int32_t shift =0);
        //@}

        //! @name Accessors
        //@{
        // See base class comment
        virtual void extract(Image& image,
                             ByteOrder byteOrder) const;

        // See base class comment
        virtual void print(std::ostream& os,
                           ByteOrder byteOrder,
                           const std::string& prefix ="") const;
        //@}

    private:
        // DATA
        RawMetadata::Components components_; //!< List of components in this dir

    }; // class CiffDirectory

    //! This class models the header of a Crw (Canon Raw data) image.
    class CiffHeader : public RawMetadata {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        CiffHeader()
            : rootDirectory_ (0),
              byteOrder_     (littleEndian),
              offset_        (0x0000001a)
            {}
        //! Virtual destructor
        virtual ~CiffHeader();
        //@}

        //! @name Manipulators
        //@{
        // See base class comment
        virtual void add(RawMetadata::AutoPtr component);

        // See base class comment
        virtual void read(const byte* buf,
                          uint32_t len,
                          uint32_t start,
                          ByteOrder byteOrder,
                          int32_t shift =0);
        //@}

        //! @name Accessors
        //@{
        // See base class comment
        virtual void extract(Image& image,
                             ByteOrder byteOrder) const;

        // See base class comment
        virtual void print(std::ostream& os,
                           ByteOrder byteOrder,
                           const std::string& prefix ="") const;
        //@}

        // DATA
        static const char signature_[];   //!< Canon Crw signature "HEAPCCDR"

    private:
        // DATA
        CiffDirectory*    rootDirectory_; //!< Pointer to the root directory
        ByteOrder         byteOrder_;     //!< Applicable byte order
        uint32_t          offset_;        //!< Offset to the start of the root dir

    }; // class CiffHeader

    //! Structure for conversion info for CIFF entries
    struct CrwMapInfo {
        //! @name Creators
        //@{
        //! Default constructor
        CrwMapInfo(
            uint16_t      crwTagId,
            uint16_t      crwDir,
            uint32_t      size,
            uint16_t      tag,
            IfdId         ifdId,
            CrwExtractFct toExif,
            CrwInsertFct  fromExif)
            : crwTagId_ (crwTagId),
              crwDir_   (crwDir),
              size_     (size),
              tag_      (tag),
              ifdId_    (ifdId),
              toExif_   (toExif),
              fromExif_ (fromExif)
            {}
        //@}

        // DATA
        uint16_t      crwTagId_;  //!< Crw tag id
        uint16_t      crwDir_;    //!< Crw directory tag
        uint32_t      size_;      //!< Data size (overwrites the size from the entry)
        uint16_t      tag_;       //!< Exif tag to map to
        IfdId         ifdId_;     //!< Exif Ifd id to map to
        CrwExtractFct toExif_;    //!< Conversion function
        CrwInsertFct  fromExif_;  //!< Reverse conversion function

    }; // struct CrwMapInfo

    /*!
      @brief Static class providing mapping functionality from Crw entries
             to image metadata and vice versa
     */
    class CrwMap {
        //! @name Not implemented
        //@{
        //! Default constructor
        CrwMap();
        //@}

    public:
        /*!
          @brief Extract image metadata from a Crw entry convert and add it
                 to the image metadata.

          @param ciffComponent Source CIFF entry
          @param image         Destination image for the metadata
          @param byteOrder     Byte order in which the data of the entry
                               is encoded
         */
        static void extract(const CiffComponent& ciffComponent,
                            Image& image,
                            ByteOrder byteOrder);

    private:
        //! Return conversion information for one Crw \em dir and \em tagId
        static const CrwMapInfo* crwMapInfo(uint16_t dir, uint16_t tagId);

        /*!
          @brief Standard extraction function to convert Crw entries to
                 Exif metadata.

          Uses the mapping defined in the conversion structure \em crwMapInfo
          to convert the data. If the \em size field in the conversion structure
          is not 0, then it is used instead of the \em size provided by the
          entry itself.
         */
        static void extractBasic(const CiffComponent& ciffComponent,
                                 const CrwMapInfo* crwMapInfo,
                                 Image& image,
                                 ByteOrder byteOrder);

        //! Extract the user comment
        static void extract0x0805(const CiffComponent& ciffComponent,
                                  const CrwMapInfo* crwMapInfo,
                                  Image& image,
                                  ByteOrder byteOrder);

        //! Extract camera Make and Model information
        static void extract0x080a(const CiffComponent& ciffComponent,
                                  const CrwMapInfo* crwMapInfo,
                                  Image& image,
                                  ByteOrder byteOrder);

        //! Extract Canon Camera Settings 2
        static void extract0x102a(const CiffComponent& ciffComponent,
                                  const CrwMapInfo* crwMapInfo,
                                  Image& image,
                                  ByteOrder byteOrder);

        //! Extract Canon Camera Settings 1
        static void extract0x102d(const CiffComponent& ciffComponent,
                                  const CrwMapInfo* crwMapInfo,
                                  Image& image,
                                  ByteOrder byteOrder);

        //! Extract the date when the picture was taken
        static void extract0x180e(const CiffComponent& ciffComponent,
                                  const CrwMapInfo* crwMapInfo,
                                  Image& image,
                                  ByteOrder byteOrder);

        //! Extract image width and height
        static void extract0x1810(const CiffComponent& ciffComponent,
                                  const CrwMapInfo* crwMapInfo,
                                  Image& image,
                                  ByteOrder byteOrder);

        //! Extract the thumbnail image
        static void extract0x2008(const CiffComponent& ciffComponent,
                                  const CrwMapInfo* crwMapInfo,
                                  Image& image,
                                  ByteOrder byteOrder);

        // DATA
        static const CrwMapInfo crwMapInfos_[]; //!< Metadata conversion table

    }; // class CrwMap

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new CrwImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    Image::AutoPtr newCrwInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a Crw image.
    bool isCrwType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // #ifndef CRWIMAGE_HPP_
