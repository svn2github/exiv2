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
  @file    xmp.hpp
  @brief   Encoding and decoding of XMP data
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    13-Jul-07, ahu: created
 */
#ifndef XMP_HPP_
#define XMP_HPP_

// *****************************************************************************
// included header files
#include "metadatum.hpp"
#include "properties.hpp"
#include "key.hpp"
#include "value.hpp"
#include "types.hpp"

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
    typedef std::vector<Tag1> XmpMetadata;

    /*!
      @brief A container for XMP data. This is a top-level class of
             the %Exiv2 library.

      Provide high-level access to the XMP data of an image:
      - read XMP information from an XML block
      - access metadata through keys and standard C++ iterators
      - add, modify and delete metadata
      - serialize XMP data to an XML block
    */
    class EXIV2API XmpData {
    public:
        //! XmpMetadata iterator type
        typedef XmpMetadata::iterator iterator;
        //! XmpMetadata const iterator type
        typedef XmpMetadata::const_iterator const_iterator;

        //! @name Manipulators
        //@{
        /*!
          @brief Returns a reference to the %Tag1 that is associated with a
                 particular \em key. If %XmpData does not already contain such
                 an %Tag1, operator[] adds object \em Tag1(key).

          @note  Since operator[] might insert a new element, it can't be a const
                 member function.
         */
        Tag1& operator[](const std::string& key);
        /*!
          @brief Add an %Tag1 from the supplied key and value pair. This
                 method copies (clones) the value.
          @return 0 if successful.
         */
        int add(const Key1& key, const Value* value);
        /*!
          @brief Add a copy of the Tag1 to the XMP metadata.
          @return 0 if successful.
         */
        int add(const Tag1& xmpdatum);
        /*!
          @brief Delete the Tag1 at iterator position pos, return the
                 position of the next Tag1.

          @note  Iterators into the metadata, including pos, are potentially
                 invalidated by this call.
         */
        iterator erase(iterator pos);
        //! Delete all Tag1 instances resulting in an empty container.
        void clear();
        //! Sort metadata by key
        void sortByKey();
        //! Begin of the metadata
        iterator begin();
        //! End of the metadata
        iterator end();
        /*!
          @brief Find the first Tag1 with the given key, return an iterator
                 to it.
         */
        iterator findKey(const Key1& key);
        //@}

        //! @name Accessors
        //@{
        //! Begin of the metadata
        const_iterator begin() const;
        //! End of the metadata
        const_iterator end() const;
        /*!
          @brief Find the first Tag1 with the given key, return a const
                 iterator to it.
         */
        const_iterator findKey(const Key1& key) const;
        //! Return true if there is no XMP metadata
        bool empty() const;
        //! Get the number of metadata entries
        long count() const;
        //@}

    private:
        // DATA
        XmpMetadata xmpMetadata_;
    }; // class XmpData

    /*!
      @brief Stateless parser class for XMP packets. Images use this
             class to parse and serialize XMP packets. The parser uses
             the XMP toolkit to do the job.
     */
    class EXIV2API XmpParser {
    public:
        //! Options to control the format of the serialized XMP packet.
        enum XmpFormatFlags {
            omitPacketWrapper   = 0x0010UL,  //!< Omit the XML packet wrapper.
            readOnlyPacket      = 0x0020UL,  //!< Default is a writeable packet.
            useCompactFormat    = 0x0040UL,  //!< Use a compact form of RDF.
            includeThumbnailPad = 0x0100UL,  //!< Include a padding allowance for a thumbnail image.
            exactPacketLength   = 0x0200UL,  //!< The padding parameter is the overall packet length.
            writeAliasComments  = 0x0400UL,  //!< Show aliases as XML comments.
            omitAllFormatting   = 0x0800UL   //!< Omit all formatting whitespace.
        };
        /*!
          @brief Decode XMP metadata from an XMP packet \em xmpPacket into
                 \em xmpData. The format of the XMP packet must follow the
                 XMP specification. This method clears any previous contents
                 of \em xmpData.

          @param xmpData   Container for the decoded XMP properties
          @param xmpPacket The raw XMP packet to decode
          @return 0 if successful;<BR>
                  1 if XMP support has not been compiled-in;<BR>
                  2 if the XMP toolkit failed to initialize;<BR>
                  3 if the XMP toolkit failed and raised an XMP_Error
        */
        static int decode(      XmpData&     xmpData,
                          const std::string& xmpPacket);
        /*!
          @brief Encode (serialize) XMP metadata from \em xmpData into a
                 string xmpPacket. The XMP packet returned in the string
                 follows the XMP specification. This method only modifies
                 \em xmpPacket if the operations succeeds (return code 0).

          @param xmpPacket   Reference to a string to hold the encoded XMP
                             packet.
          @param xmpData     XMP properties to encode.
          @param formatFlags Flags that control the format of the XMP packet,
                             see enum XmpFormatFlags.
          @param padding     Padding length.
          @return 0 if successful;<BR>
                  1 if XMP support has not been compiled-in;<BR>
                  2 if the XMP toolkit failed to initialize;<BR>
                  3 if the XMP toolkit failed and raised an XMP_Error
        */
        static int encode(      std::string& xmpPacket,
                          const XmpData&     xmpData,
                                uint16_t     formatFlags =useCompactFormat,
                                uint32_t     padding =0);
        /*!
          @brief Initialize the XMP Toolkit.

          Calling this method is usually not needed, as encode() and
          decode() will initialize the XMP Toolkit if necessary.

          @return True if the initialization was successful, else false.
         */
        static bool initialize();
        /*!
          @brief Terminate the XMP Toolkit and unregister custom namespaces.

          Call this method when the XmpParser is no longer needed to
          allow the XMP Toolkit to cleanly shutdown.
         */
        static void terminate();

    private:
        /*!
          @brief Register a namespace with the XMP Toolkit.
         */
        static void registerNs(const std::string& ns,
                               const std::string& prefix);
        /*!
          @brief Delete a namespace from the XMP Toolkit.

          XmpProperties::unregisterNs calls this to synchronize namespaces.
        */
        static void unregisterNs(const std::string& ns);

        // DATA
        static bool initialized_; //! Indicates if the XMP Toolkit has been initialized

    }; // class XmpParser

// *****************************************************************************
// free functions, template and inline definitions

}                                       // namespace Exiv2

#endif                                  // #ifndef XMP_HPP_
