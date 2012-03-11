// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2011 Andreas Huggel <ahuggel@gmx.net>
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
  @file    iptc.hpp
  @brief   Encoding and decoding of IPTC data
  @version $Rev$
  @author  Brad Schick (brad)
           <a href="mailto:brad@robotbattle.com">brad@robotbattle.com</a>
  @date    31-Jul-04, brad: created
 */
#ifndef IPTC_HPP_
#define IPTC_HPP_

// *****************************************************************************
// included header files
#include "metadatum.hpp"
#include "types.hpp"
#include "error.hpp"
#include "value.hpp"
#include "key.hpp"
#include "datasets.hpp"

// + standard includes
#include <string>
#include <vector>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class ExifData;

// *****************************************************************************
// class definitions

    //! Container type to hold all metadata
    typedef std::vector<Tag1> IptcMetadata;

    /*!
      @brief A container for IPTC data. This is a top-level class of
             the %Exiv2 library.

      Provide high-level access to the IPTC data of an image:
      - read IPTC information from JPEG files
      - access metadata through keys and standard C++ iterators
      - add, modify and delete metadata
      - write IPTC data to JPEG files
      - extract IPTC metadata to files, insert from these files
    */
    class EXIV2API IptcData {
    public:
        //! IptcMetadata iterator type
        typedef IptcMetadata::iterator iterator;
        //! IptcMetadata const iterator type
        typedef IptcMetadata::const_iterator const_iterator;

        // Use the compiler generated constructors and assignment operator

        //! @name Manipulators
        //@{
        /*!
          @brief Returns a reference to the %Tag1 that is associated with a
                 particular \em key. If %IptcData does not already contain such
                 a %Tag1, operator[] adds object \em Tag1(key).

          @note  Since operator[] might insert a new element, it can't be a const
                 member function.
         */
        Tag1& operator[](const std::string& key);
        /*!
          @brief Add an %Tag1 from the supplied key and value pair. This
                 method copies (clones) the value. A check for non-repeatable
                 datasets is performed.
          @return 0 if successful;<BR>
                  6 if the dataset already exists and is not repeatable
         */
        int add(const Key1& key, Value* value);
        /*!
          @brief Add a copy of the Tag1 to the IPTC metadata. A check
                 for non-repeatable datasets is performed.
          @return 0 if successful;<BR>
                 6 if the dataset already exists and is not repeatable;<BR>
         */
        int add(const Tag1& iptcdatum);
        /*!
          @brief Delete the Tag1 at iterator position pos, return the
                 position of the next Tag1. Note that iterators into
                 the metadata, including pos, are potentially invalidated
                 by this call.
         */
        iterator erase(iterator pos);
        /*!
          @brief Delete all Tag1 instances resulting in an empty container.
         */
        void clear() { iptcMetadata_.clear(); }
        //! Sort metadata by key
        void sortByKey();
        //! Sort metadata by tag (aka dataset)
        void sortByTag();
        //! Begin of the metadata
        iterator begin() { return iptcMetadata_.begin(); }
        //! End of the metadata
        iterator end() { return iptcMetadata_.end(); }
        /*!
          @brief Find the first Tag1 with the given key, return an iterator
                 to it.
         */
        iterator findKey(const Key1& key);
        /*!
          @brief Find the first Tag1 with the given record and dataset it,
                return a const iterator to it.
         */
        iterator findId(uint16_t dataset,
                        uint16_t record = IptcDataSets::application2);
        //@}

        //! @name Accessors
        //@{
        //! Begin of the metadata
        const_iterator begin() const { return iptcMetadata_.begin(); }
        //! End of the metadata
        const_iterator end() const { return iptcMetadata_.end(); }
        /*!
          @brief Find the first Tag1 with the given key, return a const
                 iterator to it.
         */
        const_iterator findKey(const Key1& key) const;
        /*!
          @brief Find the first Tag1 with the given record and dataset
                 number, return a const iterator to it.
         */
        const_iterator findId(uint16_t dataset,
                              uint16_t record = IptcDataSets::application2) const;
        //! Return true if there is no IPTC metadata
        bool empty() const { return count() == 0; }
        //! Get the number of metadata entries
        long count() const { return static_cast<long>(iptcMetadata_.size()); }
        /*!
          @brief Return the exact size of all contained IPTC metadata
         */
        long size() const;
        /*!
          @brief Return the metadata charset name or 0
         */
        const char *detectCharset() const;
        //@}

    private:
        // DATA
        IptcMetadata iptcMetadata_;
    }; // class IptcData

    /*!
      @brief Stateless parser class for IPTC data. Images use this class to
             decode and encode binary IPTC data.
     */
    class EXIV2API IptcParser {
    public:
        /*!
          @brief Decode binary IPTC data in IPTC IIM4 format from a buffer \em pData
                 of length \em size to the provided metadata container.

          @param iptcData Metadata container to add the decoded IPTC datasets to.
          @param pData    Pointer to the data buffer to read from.
          @param size     Number of bytes in the data buffer.

          @return 0 if successful;<BR>
                  5 if the binary IPTC data is invalid or corrupt
         */
        static int decode(
                  IptcData& iptcData,
            const byte*     pData,
                  uint32_t  size
        );
        /*!
          @brief Encode the IPTC datasets from \em iptcData to a binary
                 representation in IPTC IIM4 format.

          Convert the IPTC datasets to binary format and return it.  Caller owns
          the returned buffer. The copied data follows the IPTC IIM4 standard.

          @return Data buffer containing the binary IPTC data in IPTC IIM4 format.
         */
        static DataBuf encode(
            const IptcData& iptcData
        );

    private:
        // Constant data
        static const byte marker_;      // Dataset marker

    }; // class IptcParser

}                                       // namespace Exiv2

#endif                                  // #ifndef IPTC_HPP_
