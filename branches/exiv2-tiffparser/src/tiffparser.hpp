// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006 Andreas Huggel <ahuggel@gmx.net>
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
  @file    tiffparser.hpp
  @brief   Class TiffParser to parse TIFF data.
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    15-Mar-06, ahu: created
 */
#ifndef TIFFPARSER_HPP_
#define TIFFPARSER_HPP_

// *****************************************************************************
// included header files
#include "tifffwd.hpp"
#include "image.hpp"
#include "types.hpp"

// + standard includes
#include <iosfwd>
#include <cassert>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    /*!
      @brief TIFF component factory for standard TIFF components.
     */
    class TiffCreator {
    public:
        /*!
          @brief Create the TiffComponent for TIFF entry \em extendedTag and
                 \em group based on the embedded lookup table. If the pointer 
                 that is returned is 0, then the TIFF entry should be ignored.
        */
        static std::auto_ptr<TiffComponent> create(uint32_t extendedTag,
                                                   uint16_t group);
        /*!  
          @brief Get the path, i.e., a list of TiffStructure pointers, from
                 the root TIFF element to the TIFF entry \em extendedTag and 
                 \em group.
        */
        static void getPath(TiffPath& tiffPath, uint32_t extendedTag, uint16_t group);

    private:
        static const TiffStructure tiffStructure_[]; //<! TIFF structure
    }; // class TiffCreator

    /*!
      @brief Stateless parser class for data in TIFF format. Images use this
             class to decode and encode TIFF-based data. Uses class
             CreationPolicy for the creation of TIFF components.
     */
    class TiffParser {
    public:
        /*!
          @brief Decode TIFF metadata from a data buffer \em pData of length
                 \em size into \em pImage.

          This is the entry point to access image data in TIFF format. The
          parser uses classes TiffHeade2 and the TiffComponent and TiffVisitor
          hierarchies.

          @param pImage    Pointer to the image to hold the metadata
          @param pData     Pointer to the data buffer. Must point to data
                           in TIFF format; no checks are performed.
          @param size      Length of the data buffer.
          @param createFct Factory function to create new TIFF components.
          @param findDecoderFct Function to access special decoding info.
        */
        static void decode(      Image*             pImage,
                           const byte*              pData,
                                 uint32_t           size,
                                 TiffCompFactoryFct createFct,
                                 FindDecoderFct     findDecoderFct);

        /*!
          @brief Encode TIFF metadata from \em pImage into a memory block
                 \em blob.
        */
        static void encode(      Blob&              blob,
                           const byte*              pData,
                                 uint32_t           size,
                           const Image*             pImage,
                                 TiffCompFactoryFct createFct,
                                 FindEncoderFct     findEncoderFct);

    private:
        /*!
          @brief Parse TIFF metadata from a data buffer \em pData of length
                 \em size into a TIFF composite structure. 

          @param pData     Pointer to the data buffer. Must point to data
                           in TIFF format; no checks are performed.
          @param size      Length of the data buffer.
          @return          An auto pointer with the root element of the TIFF
                           composite structure.
         */
        static std::auto_ptr<TiffComponent> 
        parse(const byte*              pData, 
                    uint32_t           size,
                    TiffCompFactoryFct createFct);

    }; // class TiffParser

    /*!
      @brief Table of TIFF decoding and encoding functions and find functions.
             This class is separated from the metadata decoder and encoder
             visitors so that the parser can be parametrized with a different 
             table if needed. This is used, eg., for CR2 format, which uses a 
             different decoder table.
     */
    class TiffMapping {
    public:
        /*!
          @brief Find the decoder function for a key. 

          If the returned pointer is 0, the tag should not be decoded, 
          else the decoder function should be used.

          @param make Camera make
          @param extendedTag Extended tag
          @param group %Group

          @return Pointer to the decoder function
         */
        static const DecoderFct findDecoder(const std::string& make, 
                                                  uint32_t     extendedTag,
                                                  uint16_t     group);
        /*!
          @brief Find the encoder function for a key. 

          If the returned pointer is 0, the tag should not be encoded, 
          else the encoder function should be used.

          @param make Camera make
          @param extendedTag Extended tag
          @param group %Group

          @return Pointer to the encoder function
         */
        static const EncoderFct findEncoder(const std::string& make, 
                                                  uint32_t     extendedTag,
                                                  uint16_t     group);

    private:
        static const TiffMappingInfo tiffMappingInfo_[]; //<! TIFF mapping table

    }; // class TiffMapping

}                                       // namespace Exiv2

#endif                                  // #ifndef TIFFPARSER_HPP_
