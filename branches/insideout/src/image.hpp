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
  @file    image.hpp
  @brief   Class JpegImage to access JPEG images
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Brad Schick (brad) 
           <a href="mailto:brad@robotbattle.com">brad@robotbattle.com</a>
  @date    09-Jan-04, ahu: created<BR>
           11-Feb-04, ahu: isolated as a component<BR>
           19-Jul-04, brad: revamped to be more flexible and support Iptc
 */
#ifndef IMAGE_HPP_
#define IMAGE_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "basicio.hpp"

// + standard includes
#include <string>
#include <map>
//#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// enumerations

// *****************************************************************************
// class definitions

    /*!
      @brief Abstract base class defining the interface for an image.
     */
    class Image {
    public:
        //! Supported image formats
        enum Type { none, jpeg, exv };
     
        //! Image auto_ptr type
        typedef std::auto_ptr<Image> AutoPtr;

        //! @name Creators
        //@{
        //! Virtual Destructor
        virtual ~Image() {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read metadata from assigned image file into internal 
                 buffers.
          @return 0 if successful.
         */
        virtual int readMetadata() =0;
        /*!
          @brief Write metadata from internal buffers into to the image fle.
          @return 0 if successful.
         */
        virtual int writeMetadata() =0;
        /*!
          @brief Set the Exif data. The data is copied into an internal data
                 buffer and is not written until writeMetadata is called.
          @param buf Pointer to the new Exif data.
          @param size Size in bytes of new Exif data.
         */
        virtual void setExifData(const byte* buf, long size) =0;
//        virtual void setExifData( const ExifData& exifData ) =0;
        /*!
          @brief Erase any buffered Exif data. Exif data is not removed
                from the actual file until writeMetadata is called.
         */
        virtual void clearExifData() =0;
        /*!
          @brief Set the Iptc data. The data is copied into an internal data
                 buffer and is not written until writeMetadata is called.
          @param buf Pointer to the new Iptc data.
          @param size Size in bytes of new Iptc data.
         */
        virtual void setIptcData(const byte* buf, long size) =0;
//        virtual void setIptcData( const IptcData& iptcData ) =0;
        /*!
          @brief Erase any buffered Iptc data. Iptc data is not removed
                from the actual file until writeMetadata is called.
         */
        virtual void clearIptcData() =0;
        /*!
          @brief Set the image comment. The data is copied into an internal data
                 buffer and is not written until writeMetadata is called.
          @param comment String containing comment.
         */
        virtual void setComment(const std::string& comment) =0;
        /*!
          @brief Erase any buffered comment. Comment is not removed
                 from the actual file until writeMetadata is called.
         */
        virtual void clearComment() =0;
        /*!
          @brief Copy all existing metadata from source %Image. The data is
                 copied into internal buffers and is not written until
                 writeMetadata is called.
          @param image Metadata source. All metadata types are copied.
         */
        virtual void setMetadata(const Image& image) =0;
        /*!
          @brief Erase all buffered metadata. Metadata is not removed
                 from the actual file until writeMetadata is called.
         */
        virtual void clearMetadata() =0;
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Check if the %Image instance is valid. Use after object 
                 construction.
          @return true if the %Image is in a valid state.
         */
        virtual bool good() const =0;
        //! Return the size of the Exif data in bytes.
        virtual long sizeExifData() const =0;
        /*!
          @brief Return a read-only pointer to an Exif data buffer. Do not
                 attempt to write to this buffer.
         */
        virtual const byte* exifData() const =0;
        //! Return the size of the Iptc data in bytes.
        virtual long sizeIptcData() const =0;
//        virtual const ExifData& exifData() const =0;
//        virtual IptcData& exifData() =0;
        /*!
          @brief Return a read-only pointer to an Iptc data buffer. Do not
                 attempt to write to this buffer.
         */
        virtual const byte* iptcData() const =0;
//        virtual const IptcData& iptcData() const =0;
  //      virtual IptcData& iptcData() =0;
        /*!
          @brief Return a copy of the image comment. May be an empty string.
         */
        virtual std::string comment() const =0;
//        virtual const std::string& comment() const =0;
  //      virtual std::string& comment() =0;
        /*!
          @brief Return a reference to the BasicIo instance being used for Io.
               This refence is particularly useful to reading the results of
               operations on a MemIo instance. If the data is modified,
               the changes will not be noticed by this class until the
               next call to readMetadata.
         */
        virtual BasicIo& io() const = 0;
        //@}

    protected:
        //! @name Creators
        //@{
        //! Default Constructor
        Image() {}
        //@}

    private:
        // NOT Implemented
        //! Copy constructor
        Image(const Image& rhs);
        //! Assignment operator
        Image& operator=(const Image& rhs);

    }; // class Image

    //! Type for function pointer that creates new Image instances
    typedef Image::AutoPtr (*NewInstanceFct)(BasicIo::AutoPtr io, bool create);
    //! Type for function pointer that checks image types
    typedef bool (*IsThisTypeFct)(BasicIo& iIo, bool advance);

    /*!
      @brief Image factory.

      Creates an instance of the image of the requested type.  The factory is
      implemented as a singleton, which can be accessed only through the static
      member function instance().
    */
    class ImageFactory {
    public:
        //! @name Manipulators
        //@{
        /*!
          @brief Register image type together with its function pointers.

          The image factory creates new images calling their associated
          function pointer. Additional images can be added by registering
          new type and function pointers. If called for a type that already
          exists in the list, the corresponding prototype is replaced.

          @param type Image type.
          @param newInst Function pointer for creating image instances.
          @param isType Function pointer to test for matching image types.
        */
        static void registerImage(Image::Type type, 
                           NewInstanceFct newInst, 
                           IsThisTypeFct isType);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief  Create an %Image of the appropriate type by opening the
                  specified file. File type is derived from the contents of the
                  file.
          @param  path %Image file. The contents of the file are tested to
                  determine the image type to open. File extension is ignored.
          @return An auto-pointer that owns an %Image of the type derived from
                  the file. If no image type could be determined, the pointer is 0.
         */
        static Image::AutoPtr open(const std::string& path);
        static Image::AutoPtr open(const byte* data, long size);
        static Image::AutoPtr open(BasicIo::AutoPtr io);
        /*!
          @brief  Create an %Image of the requested type by creating a new
                  file. If the file already exists, it will be overwritten.
          @param  type Type of the image to be created.
          @param  path %Image file. The contents of the file are tested to
                  determine the image type to open. File extension is ignored.
          @return An auto-pointer that owns an %Image of the requested type. 
                  If the image type is not supported, the pointer is 0.
         */
        static Image::AutoPtr create(Image::Type type, const std::string& path);
        static Image::AutoPtr create(Image::Type type);
        static Image::AutoPtr create(Image::Type type, BasicIo::AutoPtr io);
        /*!
          @brief  Returns the image type of the provided file. 
          @param  path %Image file. The contents of the file are tested to
                  determine the image type. File extension is ignored.
          @return %Image type of Image::none if the type is not recognized.
         */
        static Image::Type getType(const std::string& path);
        static Image::Type getType(const byte* data, long size);
        static Image::Type getType(BasicIo& io);
        //@}


    private:
        //! @name Creators
        //@{
        //! Prevent construction other than through instance().
        ImageFactory();
        //! Prevent copy construction: not implemented.
        ImageFactory(const ImageFactory& rhs);
        //@}

        //! Struct for storing image function pointers.
        struct ImageFcts
        {
            NewInstanceFct newInstance;
            IsThisTypeFct isThisType;
            ImageFcts(NewInstanceFct newInst, IsThisTypeFct isType) 
                : newInstance(newInst), isThisType(isType) {}
            ImageFcts() : newInstance(0), isThisType(0) {}
        };

        //! Make sure statis data is created
        static void ImageFactory::init();

        // DATA
        //! Type used to store Image creation functions
        typedef std::map<Image::Type, ImageFcts> Registry;
        //! List of image types and corresponding creation functions.
        static Registry* registry_;
    }; // class ImageFactory


    //! Helper class modelling the TIFF header structure.
    class TiffHeader {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Default constructor. Optionally sets the byte order 
                 (default: little endian).
         */
        explicit TiffHeader(ByteOrder byteOrder =littleEndian);
        //@}

        //! @name Manipulators
        //@{
        //! Read the TIFF header from a data buffer. Returns 0 if successful.
        int read(const byte* buf);
        //@}

        //! @name Accessors
        //@{
        /*! 
          @brief Write a standard TIFF header into buf as a data string, return
                 number of bytes copied.

          Only the byte order of the TIFF header varies, the values written for
          offset and tag are constant, i.e., independent of the values possibly
          read before a call to this function. The value 0x00000008 is written
          for the offset, tag is set to 0x002a.

          @param buf The data buffer to write to.
          @return The number of bytes written.
         */
        long copy(byte* buf) const;
        //! Return the size of the TIFF header in bytes.
        long size() const { return 8; }
        //! Return the byte order (little or big endian).
        ByteOrder byteOrder() const { return byteOrder_; }
        //! Return the tag value.
        uint16_t tag() const { return tag_; }
        /*!
          @brief Return the offset to IFD0 from the start of the TIFF header.
                 The offset is 0x00000008 if IFD0 begins immediately after the 
                 TIFF header.
         */
        uint32_t offset() const { return offset_; }
        //@}

    private:
        ByteOrder byteOrder_;
        uint16_t tag_;
        uint32_t offset_;

    }; // class TiffHeader   

// *********************************************************************
// free functions

    /*!
      @brief Test if a file exists.
  
      @param  path Name of file to verify.
      @param  ct   Flag to check if <i>path</i> is a regular file.
      @return true if <i>path</i> exists and, if <i>ct</i> is set,
      is a regular file, else false.
  
      @note The function calls <b>stat()</b> test for <i>path</i>
      and its type, see stat(2). <b>errno</b> is left unchanged 
      in case of an error.
     */
    bool fileExists(const std::string& path, bool ct =false);

}                                       // namespace Exiv2

#endif                                  // #ifndef IMAGE_HPP_
