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
  @file    jpgimage.hpp
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
#ifndef JPGIMAGE_HPP_
#define JPGIMAGE_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "image.hpp"
#include "basicio.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {


// *****************************************************************************
// class definitions

    /*! 
      @brief Abstract helper base class to access JPEG images
     */
    class JpegBase : public Image {
    public:
        //! @name Creators
        //@{
        //! Virtual destructor.
        virtual ~JpegBase();
        //@}
        //! @name Manipulators
        //@{
        /*!
          @brief Read all metadata from the file into the internal 
                 data buffers. This method returns success even when
                 no metadata is found in the image. Callers must therefore
                 check the size of indivdual metadata types before
                 accessing the data.
          @return 0 if successful;<BR>
                  1 if reading from the file failed 
                    (could be caused by invalid image);<BR>
                  2 if the file does not contain a valid image;<BR>
         */
        int readMetadata();
        /*!
          @brief Write all buffered metadata to associated file. All existing
                metadata sections in the file are either replaced or erased.
                If data for a given metadata type has not been assigned,
                then that metadata type will be erased from the file.
          @return 0 if successful;<br>
                  1 if reading from the file failed;<BR>
                  2 if the file does not contain a valid image;<BR>
                  4 if the temporary output file can not be written to;<BR>
                  -1 if the newly created file could not be reopened;<BR>
                  -3 if the temporary output file can not be opened;<BR>
                  -4 if renaming the temporary file fails;<br>
         */
        int writeMetadata();
        /*!
          @brief Set the Exif data. The data is copied into an internal data
                 buffer and is not written until writeMetadata is called.
          @param buf Pointer to the new Exif data.
          @param size Size in bytes of new Exif data.

          @throw Error ("Exif data too large") if the exif data is larger than
                 65535 bytes (the maximum size of JPEG APP segments)
         */
        void setExifData(const byte* buf, long size);
        void clearExifData();
        void setIptcData(const byte* buf, long size);
        void clearIptcData();
        void setComment(const std::string& comment);
        void clearComment();
        void setMetadata(const Image& image);
        void clearMetadata();
        //@}

        //! @name Accessors
        //@{
        bool good() const;
        long sizeExifData() const { return sizeExifData_; }
        const byte* exifData() const { return pExifData_; }
        long sizeIptcData() const { return sizeIptcData_; }
        const byte* iptcData() const { return pIptcData_; }
        std::string comment() const { return comment_; }
        //@}        
    protected:
        //! @name Creators
        //@{
        /*! 
          @brief Constructor that can either open an existing image or create
                 a new image from scratch. If a new image is to be created, any
                 existing file is overwritten
          @param path Full path to image file.
          @param create Specifies if an existing file should be opened (false)
                 or if a new file should be created (true).
          @param initData Data to initialize newly created files. Only used
                 when %create is true. Should contain the data for the smallest
                 valid image of the calling subclass.
          @param dataSize Size of initData in bytes.
         */
        JpegBase(const std::string& path, bool create,
                 const byte initData[], long dataSize);
        //@}
        //! @name Accessors
        //@{
        /*!
          @brief Writes the image header (aka signature) to the file stream.
          @param ofp File stream that the header is written to.
          @return 0 if successful;<BR>
                 4 if the output file can not be written to;<BR>
         */
        virtual int writeHeader(BasicIo& oIo) const =0;
        /*!
          @brief Determine if the content of the stream is of the type of this
                 class.

          The advance flag determines if the read position in the stream is
          moved (see below). This applies only if the type matches and the
          function returns true. If the type does not match, the stream
          position is not changed. However, if reading from the stream fails,
          the stream position is undefined. Consult the stream state to obtain 
          more information in this case.
          
          @param ifp Input file stream.
          @param advance Flag indicating whether the read position in the stream
                         should be advanced by the number of characters read to
                         analyse the stream (true) or left at its original
                         position (false). This applies only if the type matches.
          @return  true  if the stream data matches the type of this class;<BR>
                   false if the stream data does not match;<BR>
         */
        virtual bool isThisType(BasicIo& iIo, bool advance) const =0;
        //@}

        // Constant Data
        static const byte sos_;                 //!< JPEG SOS marker
        static const byte eoi_;                 //!< JPEG EOI marker
        static const byte app0_;                //!< JPEG APP0 marker
        static const byte app1_;                //!< JPEG APP1 marker
        static const byte app13_;               //!< JPEG APP13 marker
        static const byte com_;                 //!< JPEG Comment marker
        static const char exifId_[];            //!< Exif identifier
        static const char jfifId_[];            //!< JFIF identifier
        static const char ps3Id_[];             //!< Photoshop marker
        static const char bimId_[];             //!< Photoshop marker
        static const uint16_t iptc_;              //!< Photoshop Iptc marker

    private:
        // DATA
        const std::string path_;                //!< Image file name
        long sizeExifData_;                     //!< Size of the Exif data buffer
        byte* pExifData_;                       //!< Exif data buffer
        long sizeIptcData_;                     //!< Size of the Iptc data buffer
        byte* pIptcData_;                       //!< Iptc data buffer
        std::string comment_;                   //!< JPEG comment

        // METHODS
        /*!
          @brief Advances file stream to one byte past the next Jpeg marker
                 and returns the marker. This method should be called when the
                 file stream is positioned one byte past the end of a Jpeg segment.
          @param fp File stream to advance
          @return the next Jpeg segment marker if successful;<BR>
                 -1 if a maker was not found before EOF;<BR>
         */
        int advanceToMarker(BasicIo& io) const;
        /*!
          @brief Locates Photoshop formated Iptc data in a memory buffer.
                 Operates on raw data (rather than file streams) to simplify reuse.
          @param pPsData Pointer to buffer containing entire payload of 
                 Photoshop formated APP13 Jpeg segment.
          @param sizePsData Size in bytes of pPsData.
          @param record Output value that is set to the start of the Iptc
                 data block within pPsData (may not be null).
          @param sizeHdr Output value that is set to the size of the header
                 within the Iptc data block pointed to by record (may not
                 be null).
          @param sizeIptc Output value that is set to the size of the actual
                 Iptc data within the Iptc data block pointed to by record
                 (may not be null).
          @return 0 if successful;<BR>
                  3 if no Iptc data was found in pPsData;<BR>
                  -2 if the pPsData buffer does not contain valid data;<BR>
         */
        int locateIptcData(const byte *pPsData, 
                           long sizePsData,
                           const byte **record, 
                           uint16_t *const sizeHdr,
                           uint16_t *const sizeIptc) const;
        /*!
          @brief Write to the specified file stream with the provided data.
          @param fp File stream to be written to (should be "w+b" mode)
          @param initData Data to be written to the associated file
          @param dataSize Size in bytes of data to be written
          @return 0 if successful;<BR>
                  4 if the output file can not be written to;<BR>
         */
        int initFile(BasicIo& io, const byte initData[], long dataSize);
        /*!
          @brief Provides the main implementation of writeMetadata by 
                writing all buffered metadata to associated file. 
          @param ifp Input file stream. Non-metadata is copied to output file.
          @param ofp Output file stream to write to (e.g., a temporary file).
          @return 0 if successful;<br>
                  1 if reading from input file failed;<BR>
                  2 if the input file does not contain a valid image;<BR>
                  4 if the output file can not be written to;<BR>
         */
        int doWriteMetadata(BasicIo& iIo, BasicIo& oIo) const;

        // NOT Implemented
        //! Default constructor.
        JpegBase();
        //! Copy constructor
        JpegBase(const JpegBase& rhs);
        //! Assignment operator
        JpegBase& operator=(const JpegBase& rhs);
    }; // class JpegBase

    /*! 
      @brief Helper class to access JPEG images
     */
    class JpegImage : public JpegBase {
        friend bool isJpegType(BasicIo& iIo, bool advance);
    public:
        //! @name Creators
        //@{
        /*! 
          @brief Constructor that can either open an existing Jpeg image or create
                 a new image from scratch. If a new image is to be created, any
                 existing file is overwritten. Since the constructor can not return
                 a result, callers should check the %good method after object
                 construction to determine success or failure.
          @param path Full path to image file.
          @param create Specifies if an existing file should be opened (false)
                 or if a new file should be created (true).
         */
        JpegImage(const std::string& path, bool create);
        //! Destructor
        ~JpegImage() {}
        //@}
        
        //! Public so that we can create the static, but not mean for real public use.
        struct Register{
            //! Default constructor
            Register();
        };
    protected:
        //! @name Accessors
        //@{
        /*!
          @brief Writes a Jpeg header (aka signature) to the file stream.
          @param ofp File stream that the header is written to.
          @return 0 if successful;<BR>
                 4 if the output file can not be written to;<BR>
         */
        int writeHeader(BasicIo& oIo) const;
        /*!
          @brief Determine if the content of the file stream is a Jpeg image.
                 See base class for more details.
          @param ifp Input file stream.
          @param advance Flag indicating whether the read position in the stream
                         should be advanced by the number of characters read to
                         analyse the stream (true) or left at its original
                         position (false). This applies only if the type matches.
          @return  true  if the file stream data matches a Jpeg image;<BR>
                   false if the stream data does not match;<BR>
         */
        bool isThisType(BasicIo& iIo, bool advance) const;
        //@}
    private:
        // Constant data
        static const byte soi_;          // SOI marker
        static const byte blank_[];      // Minimal Jpeg image

        // NOT Implemented
        //! Default constructor
        JpegImage();
        //! Copy constructor
        JpegImage(const JpegImage& rhs);
        //! Assignment operator
        JpegImage& operator=(const JpegImage& rhs);
    }; // class JpegImage

    static JpegImage::Register jpgReg;

    //! Helper class to access %Exiv2 files
    class ExvImage : public JpegBase {
        friend bool isExvType(BasicIo& iIo, bool advance);
    public:
        //! @name Creators
        //@{
        /*! 
          @brief Constructor that can either open an existing Exv image or create
                 a new image from scratch. If a new image is to be created, any
                 existing file is overwritten. Since the constructor can not return
                 a result, callers should check the %good method after object
                 construction to determine success or failure.
          @param path Full path to image file.
          @param create Specifies if an existing file should be opened (false)
                 or if a new file should be created (true).
         */
        ExvImage(const std::string& path, bool create);
        //! Destructor
        ~ExvImage() {}
        //@}
        
        //! Public so that we can create the static, but not mean for real public use.
        struct Register{
            //! Default constructor
            Register();
        };
    protected:
        //! @name Accessors
        //@{
        /*!
          @brief Writes an Exv header (aka signature) to the file stream.
          @param ofp File stream that the header is written to.
          @return 0 if successful;<BR>
                  4 if the output file can not be written to;<BR>
         */
        int writeHeader(BasicIo& oIo) const;
        /*!
          @brief Determine if the content of the file stream is a Exv image.
                 See base class for more details.
          @param ifp Input file stream.
          @param advance Flag indicating whether the read position in the stream
                         should be advanced by the number of characters read to
                         analyse the stream (true) or left at its original
                         position (false). This applies only if the type matches.
          @return  true  if the file stream data matches a Exv image;<BR>
                   false if the stream data does not match;<BR>
         */
        virtual bool isThisType(BasicIo& iIo, bool advance) const;
        //@}
    private:
        // Constant data
        static const char exiv2Id_[];    // Exv identifier
        static const byte blank_[];      // Minimal exiv file

        // NOT Implemented
        //! Default constructor
        ExvImage();
        //! Copy constructor
        ExvImage(const ExvImage& rhs);
        //! Assignment operator
        ExvImage& operator=(const ExvImage& rhs);
    }; // class ExvImage

    static ExvImage::Register exvReg;
}                                       // namespace Exiv2


#endif                                  // #ifndef JPGIMAGE_HPP_
