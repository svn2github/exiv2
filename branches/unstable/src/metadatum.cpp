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
/*
  File:      metadatum.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Brad Schick (brad) <brad@robotbattle.com>
  History:   26-Jan-04, ahu: created
             31-Jul-04, brad: isolated as a component
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "metadatum.hpp"
#include "error.hpp"

// Todo: added for hacked write() only
#include "tags_int.hpp"
#include "properties.hpp"

// + standard includes
#include <iostream>
#include <iomanip>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    Tag1::Tag1(const Key1& key, const Value* pValue)
        : key_(key), pValue_(0)
    {
        if (pValue) pValue_ = pValue->clone().release();  // deep copy
    }

    Tag1::Tag1(const Tag1& rhs)
        : key_(rhs.key_), pValue_(0)
    {
        if (rhs.pValue_ != 0) pValue_ = rhs.pValue_->clone().release(); // deep copy
    }

    Tag1::~Tag1()
    {
        delete pValue_;
    }

    Tag1& Tag1::operator=(const Tag1& rhs)
    {
        if (this == &rhs) return *this;
        Value::AutoPtr p;
        if (rhs.pValue_) p = rhs.pValue_->clone(); // deep copy
        key_ = rhs.key_;
        delete pValue_;
        pValue_ = p.release();
        return *this;
    }

    Tag1& Tag1::operator=(const uint16_t& value)
    {
        // Todo: Fix ugly hack!
        if (key_.family() == mdXmp) return operator=(Exiv2::toString(value));
        setValueHelper(value);
        return *this;
    }

    Tag1& Tag1::operator=(const uint32_t& value)
    {
        // Todo: Fix ugly hack!
        if (key_.family() == mdXmp) return operator=(Exiv2::toString(value));
        setValueHelper(value);
        return *this;
    }

    Tag1& Tag1::operator=(const URational& value)
    {
        // Todo: Fix ugly hack!
        if (key_.family() == mdXmp) return operator=(Exiv2::toString(value));
        setValueHelper(value);
        return *this;
    }

    Tag1& Tag1::operator=(const int16_t& value)
    {
        // Todo: Fix ugly hack!
        if (key_.family() == mdXmp) return operator=(Exiv2::toString(value));
        setValueHelper(value);
        return *this;
    }

    Tag1& Tag1::operator=(const int32_t& value)
    {
        // Todo: Fix ugly hack!
        if (key_.family() == mdXmp) return operator=(Exiv2::toString(value));
        setValueHelper(value);
        return *this;
    }

    Tag1& Tag1::operator=(const Rational& value)
    {
        // Todo: Fix ugly hack!
        if (key_.family() == mdXmp) return operator=(Exiv2::toString(value));
        setValueHelper(value);
        return *this;
    }

    Tag1& Tag1::operator=(const std::string& value)
    {
        setValue(value);
        return *this;
    }

    Tag1& Tag1::operator=(const Value& value)
    {
        setValue(&value);
        return *this;
    }

    void Tag1::setValue(const Value* pValue)
    {
        Value::AutoPtr p;
        if (pValue) p = pValue->clone();
        delete pValue_;
        pValue_ = p.release();
    }

    int Tag1::setValue(const std::string& buf)
    {
        if (pValue_ == 0) {
            TypeId type = key_.defaultTypeId();
            pValue_ = Value::create(type).release();
        }
        return pValue_->read(buf);
    }

    int Tag1::setDataArea(const byte* buf, long len)
    {
        return pValue_ == 0 ? -1 : pValue_->setDataArea(buf, len);
    }

    std::string Tag1::print(const ExifData* pMetadata) const
    {
        std::ostringstream os;
        write(os, pMetadata);
        return os.str();
    }

//---
    std::ostream& Tag1::write(std::ostream& os, const ExifData* pMetadata) const
    {
        // Todo: remove the #include when this is solved properly

        switch (key_.family()) {
        case mdIptc:
            // IPTC version - should IPTC also have print functions? Maybe they do now - check later!
            return os << value();
            break;
        case mdXmp:
            // XMP version
            return XmpProperties::printProperty(os, key(), value());
            break;
        case mdExif:
            // Exif version
            if (value().count() == 0) return os;
            PrintFct fct = printValue;
            const TagInfo* ti = tagInfo(tag(), static_cast<IfdId>(group()));
            if (ti != 0) fct = ti->printFct_;
            return fct(os, value(), pMetadata);
            break;
        }
    }

//---
    const Value& Tag1::value() const
    {
        if (pValue_ == 0) throw Error(8);
        return *pValue_;
    }

// ---------------------------------- Metadata (begin)
/*
  Issues with the Metadata implementation based on std::multiset
  --------------------------------------------------------------
  
  iterator is const_iterator
  The GNU C++ library's std::multiset has only a const_iterator to prevent any
  modification of the key once added to the multiset.  In Exiv2 that's
  conceptually not an issue though, as Tag consists of a Key and a Value of
  which only the Value should be modifiable after creation. However, check Key,
  Tag assignment operators and Key::setIdx().
  Apparently, the C++ standard is going in the same direction. See
  http://stackoverflow.com/questions/2038453/c-standard-unexpected-const-iterator-in-multiset

  Sorting and comparison operation
  std::multiset is an ordered container and allows only one sorting
  algorithm. However, Exiv2 doesn't sort tags in makernote IFDs currently but
  makes it a point to maintain the order as read from the image.

  Exif keys use an index to remember the order of equal tags from the original
  metadata. If the index is used in the sorting algorithm, it will not be
  possible for a user to find an Exif tag which has a non-zero index unless he
  knows the index.
  TEST: find() with index

  On the other hand, the ordered container allows an elegant way to access
  tags by family, using begin(family) and end(family).
 
  Miscellaneous
  Add erase(Key) and maybe a wrapper for std::multiset::equal_range

  So what about keeping it a list but making sure new elements are inserted at
  the correct place? - Not safe: user can easily use begin()/end() and destroy
  the sorting order.


  => use multimap instead of multiset

  - solves iterator problem

  => use a running number as index for all tags, which is set on read but not
     used for comparison. Make sure the running number is only set in the c'tor

  - solves find() issue
  - sort issue can be overcome too if necessary by copying the tags to a list before
    serializing them and re-sorting them using the index where needed.

*/

    Tag1& Metadata::operator[](const std::string& key)
    {
/*
  The GNU C++ library has this in stl_multiset.h:

      // DR 103. set::iterator is required to be modifiable,
      // but this allows modification of keys.
      typedef typename _Rep_type::const_iterator            iterator;

  which means the following code only compiles with a const-cast:

        std::multiset<long> m;
        std::multiset<long>::iterator i = m.find(3);
        long& l = const_cast<long&>(*i);

*/
        Key1 k(key);
        iterator pos = find(k);
        if (pos == end()) {
            pos = add(Tag1(k));
        }
        return const_cast<Tag1&>(*pos);
    }

    Metadata::iterator Metadata::add(const Tag1& tag)
    {
        return metadata_.insert(tag);
    }

    Metadata::iterator Metadata::begin(MetadataId family)
    {
        if (family == mdNone) return metadata_.begin();
        return metadata_.upper_bound(Tag1(Key1::min(family)));
    }

    Metadata::iterator Metadata::end(MetadataId family)
    {
        if (family == mdNone) return metadata_.end();
        return metadata_.lower_bound(Tag1(Key1::max(family)));
    }

    Metadata::iterator Metadata::find(const Key1& key)
    {
        // Todo: What about the index?
        return metadata_.find(Tag1(key));
    }

    Metadata::const_iterator Metadata::begin(MetadataId family) const
    {
        if (family == mdNone) return metadata_.begin();
        return metadata_.upper_bound(Tag1(Key1::min(family)));
    }

    Metadata::const_iterator Metadata::end(MetadataId family) const
    {
        if (family == mdNone) return metadata_.end();
        return metadata_.lower_bound(Tag1(Key1::max(family)));
    }

    Metadata::const_iterator Metadata::find(const Key1& key) const
    {
        // Todo: What about the index?
        return metadata_.find(Tag1(key));
    }

// ---------------------------------- Metadata (end)

    bool cmpTag1ByTag(const Tag1& lhs, const Tag1& rhs)
    {
        return lhs.tag() < rhs.tag();
    }

    bool cmpTag1ByKey(const Tag1& lhs, const Tag1& rhs)
    {
        return lhs.key() < rhs.key();
    }

}                                       // namespace Exiv2
