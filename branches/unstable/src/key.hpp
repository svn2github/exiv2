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
  @file    key.hpp
  @brief   Unified Exiv2 keys, used for all types of metadata
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    31-Aug-10, ahu: created
 */
#ifndef KEY_HPP_
#define KEY_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <iostream>
#include <string>
#include <memory>
#include <cstring>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

    class Key1Impl;

// *****************************************************************************
// class definitions

    /*!
      @brief Key class for all types of metadata.

      %Exiv2 models metadata as key and value pairs. The concrete class Key is
      used for keys of all types of metadata and provides access to key
      properties and reference data which is available for all metadata types.
      The abstract base class Value defines the interface to access the tag
      value.

      The metadata-specific implementation of the key methods is hidden in
      the implementation, see key.cpp
     */
    class EXIV2API Key1 {
    public:
        //! Shortcut for a %Key1 auto pointer.
        typedef std::auto_ptr<Key1> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor to create a key from a key string. Derives
                 the metadata type from the key string.

          @param key The key string.
          @throw Error if the first part of the key is not a valid family
                 name or the group or tag name parts of the key cannot be
                 parsed.
        */
        explicit Key1(const std::string& key);
        /*!
          @brief Constructor to create an \b Exif key from the tag number and
                 group name.
          @param tag The tag value
          @param groupName The name of the group, i.e., the second part of
                 the Exif key.
          @throw Error if the key cannot be constructed from the tag number
                 and group name.
         */
        Key1(uint16_t tag, const std::string& groupName);
        /*!
          @brief Constructor to create an \b IPTC key from the dataset
                 and record ids.

          @param tag    Tag (dataset) number.
          @param record Record (group) number.

          @throw Error if the schema prefix is not known.
        */
        Key1(uint16_t tag, int record);
        /*!
          @brief Constructor to create an \b XMP key from a schema prefix
                 and a property name.

          @param prefix   Schema prefix name
          @param property Property name

          @throw Error if the schema prefix is not known.
        */
        Key1(const std::string& prefix, const std::string& property);
        //! Copy constructor
        Key1(const Key1& rhs);
        //! Destructor.
        ~Key1();
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator.
        Key1& operator=(const Key1& rhs);
        /*!
          @brief Set the index (unique id of this key within the original
                 metadata, 0 if not set). Only used for Exif metadata.
         */
        void setIdx(int idx);
        //@}

        //! @name Accessors
        //@{
        //! Define an order on keys, usually by family, group, tag (and idx).
        bool operator<(const Key1& rhs) const;
        //! Return the key string. The key string is of the form \c FamilyName.GroupName.TagName.
        std::string key() const;
        //! Return the metadata family name (the first part of the key string).
        const char* familyName() const;
        //! Return the metadata identifier corresponding to the metadata type of the key.
        MetadataId family() const;
        //! Return the group name (the middle part of the key string).
        std::string groupName() const;
        /*!
          @brief Return the group number corresponding to the metadata group
                 of the key.

          The group number is relevant for Exif and IPTC keys. For Exif keys, it
          is an %Exiv2-specific number. For IPTC keys it is the record id. For
          XMP keys the function returns 0.

          @note The group number of a any Exif group may change from one version
                of %Exiv2 to the next, even if they are binary compatible
                versions. Do not hardcode Exif group numbers in an application.
         */          
        int group() const;
        //! Return the tag name (the last part of the key string).
        std::string tagName() const;
        /*!
          @brief Return the tag number corresponding to the metadata tag
                 of the key.

          The tag number is relevant for Exif and IPTC keys. For XMP keys the
          function returns 0.
         */          
        uint16_t tag() const;
        //! Return the tag label (title) of the tag, a short label that is more readable than the tag name (contains spaces).
        std::string tagLabel() const;
        //! Return the description of the tag. A longer tag description.
        std::string tagDesc() const;
        /*!
          @brief Return the default type id for this tag.

          This is the default type for this tag as defined in the tag reference
          tables.  The actual type of a particular metadatum may be different
          and is available through Metadatum::typeId().
         */
        TypeId defaultTypeId() const;
        /*!
          @brief Return the index (unique id of this key within the original
                 metadata, 0 if not set). Only used for Exif metadata.
         */
        int idx() const;
        //@}

    private:
        // Pimpl idiom (enhanced version :)
        Key1Impl* p_;

    }; // class Key1

    //! Output operator for Key1 types
    inline std::ostream& operator<<(std::ostream& os, const Key1& key)
    {
        return os << key.key();
    }

    //! Comparison operator for Key1 types, compares the key strings
    inline bool operator==(const Key1& key1, const Key1& key2)
    {
        return 0 == strcmp(key1.key().c_str(), key2.key().c_str());
    }

}                                       // namespace Exiv2

#endif                                  // #ifndef KEY_HPP_
