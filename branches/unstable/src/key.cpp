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
/*
  File:      key.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   31-Aug-10, ahu: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "key.hpp"
#include "tags_int.hpp"
#include "datasets.hpp"
#include "properties.hpp"
#include "error.hpp"
#include "types.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <cstring>

// *****************************************************************************
namespace {

    //! Get the family name from a key string, throw if not successful
    std::string getFamily(const std::string& key)
    {
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos || pos1 == 0) throw Exiv2::Error(6, key);
        return key.substr(0, pos1);
    }

    //! Get the group name from a key string, throw if not successful
    std::string getGroup(const std::string& key)
    {
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos || pos1 == 0) throw Exiv2::Error(6, key);
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos || pos1 - pos0 == 0) throw Exiv2::Error(6, key);
        return key.substr(pos0, pos1 - pos0);
    }

    //! Get the tag name from a key string, throw if not successful
    std::string getTag(const std::string& key)
    {
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos || pos1 == 0) throw Exiv2::Error(6, key);
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos || pos1 - pos0 == 0) throw Exiv2::Error(6, key);
        if (pos1 + 1 == std::string::npos) throw Exiv2::Error(6, key);
        return key.substr(pos1 + 1);
    }

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

/*
  HOWTO ADD A KEY FOR A NEW TYPE OF METADATA:
  - add new Key1 constructor(s) if necessary. That should be the only change
    needed in the header file, i.e., the published API
  - subclass Key1Impl
  - add a row to the Key1Impl::metadataInfo_[] array
  - add a NewKey::create method (see XmpKey1::create, ExifKey1::create, IptcKey1::create)

  TODO: 
  - Remove setMetadataId make the id a const and set it during construction
    in the Key1Impl c'tor
  - should group be uint16_t?
  - test/optmize: cachegrind, strtok, inline
  - Make reference data accessible through keys, like for XMP properties
 */

    /*!
      @brief %Internal Pimpl structure with private members and common data of
             class Key1.  This is an abstract base class, subclassed for each
             type of metadata. It has a factory method to create the subclasses.
     */
    class Key1Impl {
        // Prevent assignment: not implemented
        Key1Impl& operator=(const Key1Impl& rhs);

    public:
        //! @name Creators
        //@{
        //! Default constructor
        Key1Impl();
        //! Virtual destructor
        virtual ~Key1Impl();
        //@}

        //! @name Manipulators
        //@{
        //! Set the metadata type (family) of the key
        void setMetadataId(MetadataId metadataId) { metadataId_ = metadataId; }
        //! Set the index
        void setIdx(int idx) { idx_ = idx; }
        //@}

        //! @name Accessors
        //@{
        //! Return the key string.
        virtual std::string key() const =0;
        //! Return the metadata family name (the first part of the key string).
        const char* familyName() const;
        //! Return the metadata identifier corresponding to the metadata type of the key.
        MetadataId family() const { return metadataId_; }
        //! Return the group name (the middle part of the key string).
        virtual std::string groupName() const =0;
        //! Return the group number corresponding to the metadata group of the key.
        virtual int group() const =0;
        //! Return the tag name (the last part of the key string).
        virtual std::string tagName() const =0;
        //! Return the tag number corresponding to the metadata tag of the key.
        virtual uint16_t tag() const =0;
        //! Return the tag label (title) of the tag.
        virtual std::string tagLabel() const =0;
        //! Return the description of the tag.
        virtual std::string tagDesc() const =0;
        //! Return the default type id for this tag.
        virtual TypeId defaultTypeId() const =0;
        //! Virtual copy constructor, returns a pointer to a (deep) copy of the object.
        virtual Key1Impl* clone() const =0;
        //! Return the index. Only used for Exif metadata.
        int idx() const { return idx_; }
        //@}

        //! Factory method to create the correct subclass from a key string
        static Key1Impl* create(const std::string& key);

    private:
        // DATA
        /*!
          @brief Type for a function which creates a Key1Impl subclass. Caller must
                 delete the object.
         */
        typedef Key1Impl* (*KeyFct)(const std::string& key);

        //! Metadata type reference table
        struct MetadataInfo;
        static const MetadataInfo metadataInfo_[];

        MetadataId metadataId_;         //!< The metadata type (family) of the key
        int idx_;                       //!< Unique id of the key in the image (only used for Exif metadata)

    }; // class Key1Impl

    //! Data structure for the rows of the metadata type info table
    struct Key1Impl::MetadataInfo {
        //! Comparison operator to compare a MetadataInfo with a family name string
        bool operator==(const std::string& familyName) const;
        //! Comparison operator to compare a MetadataInfo with a metadata id
        bool operator==(MetadataId metadataId) const;

        // DATA
        MetadataId metadataId_;         //!< Metadata id 
        const char* familyName_;        //!< Family name 
        KeyFct keyFct_;                 //!< Function to create a key from a key string

    }; // struct MetadataInfo

    //! Exif key
    class ExifKey1 : public Key1Impl {
        // Prevent assignment: not implemented
        ExifKey1& operator=(const ExifKey1& rhs);

    public:
        //! @name Creators
        //@{
        //! Constructor taking the Exif \em tag and group name
        ExifKey1(uint16_t tag, const std::string& groupName);
        //@}

        //! @name Accessors
        //@{
        virtual std::string key() const;
        virtual std::string groupName() const;
        virtual int group() const;
        virtual std::string tagName() const;
        virtual uint16_t tag() const;
        virtual std::string tagLabel() const;
        virtual std::string tagDesc() const;
        virtual TypeId defaultTypeId() const;
        virtual Key1Impl* clone() const;
        //@}

        /*!
          @brief Factory method to create an Exif key from the key string.
                 Throws Error if the key cannot be parsed.
         */
        static Key1Impl* create(const std::string& key);

    private:
        // DATA
        const TagInfo* tagInfo_;        //!< Tag info
        uint16_t tag_;                  //!< Tag number
        Internal::IfdId groupId_;       //!< Group id
        std::string groupName_;         //!< Group name
        std::string key_;               //!< %Key

    }; // class ExifKey1

    //! IPTC key
    class IptcKey1 : public Key1Impl {
        // Prevent assignment: not implemented
        IptcKey1& operator=(const IptcKey1& rhs);

    public:
        //! @name Creators
        //@{
        //! Constructor taking the IPTC \em dataset (tag) and \em record (group) ids
        IptcKey1(uint16_t dataset, int record);
        //@}

        //! @name Accessors
        //@{
        virtual std::string key() const;
        virtual std::string groupName() const;
        virtual int group() const;
        virtual std::string tagName() const;
        virtual uint16_t tag() const;
        virtual std::string tagLabel() const;
        virtual std::string tagDesc() const;
        virtual TypeId defaultTypeId() const;
        virtual Key1Impl* clone() const;
        //@}

        /*!
          @brief Factory method to create an IPTC key from the key string.
                 Throws Error if the key cannot be parsed.
         */
        static Key1Impl* create(const std::string& key);

    private:
        // DATA
        uint16_t dataset_;              //!< Dataset id
        int record_;                    //!< Record id
        std::string key_;               //!< %Key

    }; // class IptcKey1

    //! XMP key
    class XmpKey1 : public Key1Impl {
        // Prevent assignment: not implemented
        XmpKey1& operator=(const XmpKey1& rhs);

    public:
        //! @name Creators
        //@{
        //! Constructor taking the XMP \em prefix (group name) and \em property (tag name)
        XmpKey1(const std::string& prefix, const std::string& property);
        //@}

        //! @name Accessors
        //@{
        virtual std::string key() const;
        virtual std::string groupName() const;
        virtual int group() const;
        virtual std::string tagName() const;
        virtual uint16_t tag() const;
        virtual std::string tagLabel() const;
        virtual std::string tagDesc() const;
        virtual TypeId defaultTypeId() const;
        virtual Key1Impl* clone() const;
        //@}

        /*!
          @brief Factory method to create an XMP key from the key string.
                 Throws Error if the key cannot be parsed.
         */
        static Key1Impl* create(const std::string& key);

    private:
        // DATA
        std::string prefix_;            //!< Prefix
        std::string property_;          //!< Property name

    }; // class XmpKey1

    const Key1Impl::MetadataInfo Key1Impl::metadataInfo_[] = {
        { mdExif, "Exif",  ExifKey1::create },
        { mdIptc, "Iptc",  IptcKey1::create },
        { mdXmp,  "Xmp",   XmpKey1::create  }
    };

    Key1Impl* Key1Impl::create(const std::string& key)
    {
        const MetadataInfo* mi = find(metadataInfo_, getFamily(key));
        if (mi == 0) throw Error(6, key);
        return mi->keyFct_(key);
    }

    const char* Key1Impl::familyName() const
    {
        return find(metadataInfo_, metadataId_)->familyName_;
    }

    bool Key1Impl::MetadataInfo::operator==(const std::string& familyName) const
    {
        return 0 == strcmp(familyName.c_str(), familyName_);
    }

    bool Key1Impl::MetadataInfo::operator==(MetadataId metadataId) const
    {
        return metadataId_ == metadataId;
    }

    Key1Impl::Key1Impl()
        : metadataId_(mdNone), idx_(0)
    {
    }

    Key1Impl::~Key1Impl()
    {
    }

    ExifKey1::ExifKey1(uint16_t tag, const std::string& groupName)
        : tagInfo_(0), tag_(0), groupId_(Internal::ifdIdNotSet)
    {
        Internal::IfdId ifdId = Internal::groupId(groupName);
        // Todo: Test if this condition can be removed
        if (!Internal::isExifIfd(ifdId) && !Internal::isMakerIfd(ifdId)) {
            throw Error(23, ifdId);
        }
        const TagInfo* ti = Internal::tagInfo(tag, ifdId);
        if (ti == 0) {
            throw Error(23, ifdId);
        }
        setMetadataId(mdExif);
        tagInfo_ = ti;
        tag_ = tag;
        groupId_ = ifdId;
        groupName_ = groupName;
        key_ = std::string(familyName()) + "." + groupName_ + "." + tagName();
    }

    std::string ExifKey1::key() const
    {
        return key_;
    }

    std::string ExifKey1::groupName() const
    {
        return groupName_;
    }

    int ExifKey1::group() const
    {
        return groupId_;
    }

    std::string ExifKey1::tagName() const
    {
        if (tagInfo_ != 0 && tagInfo_->tag_ != 0xffff) {
            return tagInfo_->name_;
        }
        std::ostringstream os;
        os << "0x" << std::setw(4) << std::setfill('0') << std::right
           << std::hex << tag_;
        return os.str();
    }

    uint16_t ExifKey1::tag() const
    {
        return tag_;
    }

    std::string ExifKey1::tagLabel() const
    {
        if (tagInfo_ == 0 || tagInfo_->tag_ == 0xffff) return "";
        return _(tagInfo_->title_);
    }

    std::string ExifKey1::tagDesc() const
    {
        if (tagInfo_ == 0 || tagInfo_->tag_ == 0xffff) return "";
        return _(tagInfo_->desc_);
    }

    TypeId ExifKey1::defaultTypeId() const
    {
        // Todo: Fix me! Make unknownTag accessible from here
//        if (tagInfo_ == 0) return unknownTag.typeId_;
        if (tagInfo_ == 0) return asciiString;
        return tagInfo_->typeId_;
    }

    Key1Impl* ExifKey1::clone() const
    {
        return new ExifKey1(*this);
    }

    Key1Impl* ExifKey1::create(const std::string& key)
    {
        // Todo: Can be optimized (see old code)
        const std::string g = getGroup(key);
        return new ExifKey1(Internal::tagNumber(getTag(key), Internal::groupId(g)), g);
    }

    IptcKey1::IptcKey1(uint16_t dataset, int record)
        : dataset_(dataset), record_(record)
    {
        setMetadataId(mdIptc);
        key_ =   std::string(familyName())
               + "." + IptcDataSets::recordName(record_)
               + "." + IptcDataSets::dataSetName(dataset_, record_);
    }

    std::string IptcKey1::key() const
    {
        return key_;
    }

    std::string IptcKey1::groupName() const
    {
        return IptcDataSets::recordName(record_);
    }

    int IptcKey1::group() const
    {
        return record_;
    }

    std::string IptcKey1::tagName() const
    {
        return IptcDataSets::dataSetName(dataset_, record_);
    }

    uint16_t IptcKey1::tag() const
    {
        return dataset_;
    }

    std::string IptcKey1::tagLabel() const
    {
        return IptcDataSets::dataSetTitle(dataset_, record_);
    }

    std::string IptcKey1::tagDesc() const
    {
        return IptcDataSets::dataSetDesc(dataset_, record_);
    }

    TypeId IptcKey1::defaultTypeId() const
    {
        return IptcDataSets::dataSetType(dataset_, record_);
    }

    Key1Impl* IptcKey1::clone() const
    {
        return new IptcKey1(*this);
    }

    Key1Impl* IptcKey1::create(const std::string& key)
    {
        int record = IptcDataSets::recordId(getGroup(key));
        return new IptcKey1(IptcDataSets::dataSet(getTag(key), record), record);
    }

    XmpKey1::XmpKey1(const std::string& prefix, const std::string& property)
    {
        // Validate prefix
        if (XmpProperties::ns(prefix).empty()) throw Error(46, prefix);

        setMetadataId(mdXmp);
        prefix_ = prefix;
        property_ = property;
    }

    std::string XmpKey1::key() const
    {
        return std::string(familyName()) + "." + prefix_ + "." + property_;
    }

    std::string XmpKey1::groupName() const
    {
        return prefix_;
    }

    int XmpKey1::group() const
    {
        return 0;
    }

    std::string XmpKey1::tagName() const
    {
        return property_;
    }

    uint16_t XmpKey1::tag() const
    {
        return 0;
    }

    std::string XmpKey1::tagLabel() const
    {
        const char* pt = XmpProperties::propertyTitle(prefix_, property_);
        if (!pt) return tagName();
        return pt;
    }

    std::string XmpKey1::tagDesc() const
    {
        const char* td = XmpProperties::propertyDesc(prefix_, property_);
        if (!td) return "";
        return td;
    }

    TypeId XmpKey1::defaultTypeId() const
    {
        return XmpProperties::propertyType(prefix_, property_);
    }

    Key1Impl* XmpKey1::clone() const
    {
        return new XmpKey1(*this);
    }

    Key1Impl* XmpKey1::create(const std::string& key)
    {
        return new XmpKey1(getGroup(key), getTag(key));
    }

    Key1::Key1(const std::string& key)
        : p_(Key1Impl::create(key))
    {
    }

    Key1::Key1(uint16_t tag, const std::string& groupName)
        : p_(new ExifKey1(tag, groupName))
    {
    }

    Key1::Key1(const std::string& prefix, const std::string& property)
        : p_(new XmpKey1(prefix, property))
    {
    }

    Key1::Key1(uint16_t tag, int record)
        : p_(new IptcKey1(tag, record))
    {
    }

    Key1::Key1(const Key1& rhs)
        : p_(rhs.p_->clone())
    {
    }

    Key1::~Key1()
    {
        delete p_;
    }

    Key1& Key1::operator=(const Key1& rhs)
    {
        if (this == &rhs) return *this;
        Key1Impl* p = rhs.p_->clone();
        delete p_;
        p_ = p;
        return *this;
    }

    void Key1::setIdx(int idx)
    {
        p_->setIdx(idx);
    }

    std::string Key1::key() const
    {
        return p_->key();
    }

    const char* Key1::familyName() const
    {
        return p_->familyName();
    }

    MetadataId Key1::family() const
    {
        return p_->family();
    }

    std::string Key1::groupName() const
    {
        return p_->groupName();
    }

    int Key1::group() const
    {
        return p_->group();
    }

    std::string Key1::tagName() const
    {
        return p_->tagName();
    }

    uint16_t Key1::tag() const
    {
        return p_->tag();
    }

    std::string Key1::tagLabel() const
    {
        return p_->tagLabel();
    }
    
    std::string Key1::tagDesc() const
    {
        return p_->tagDesc();
    }

    TypeId Key1::defaultTypeId() const
    {
        return p_->defaultTypeId();
    }

    int Key1::idx() const
    {
        return p_->idx();
    }
}                                       // namespace Exiv2
