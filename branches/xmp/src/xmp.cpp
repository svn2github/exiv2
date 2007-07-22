// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2007 Andreas Huggel <ahuggel@gmx.net>
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
  File:      xmp.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   13-July-07, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "xmp.hpp"
#include "types.hpp"
#include "error.hpp"
#include "value.hpp"
#include "properties.hpp"

// + standard includes
#include <iostream>
#include <algorithm>
#include <cassert>

// *****************************************************************************
namespace {
    //! Unary predicate that matches an Xmpdatum by key
    class FindXmpdatum {
    public:
        //! Constructor, initializes the object with key
        FindXmpdatum(const Exiv2::XmpKey& key)
            : key_(key.key()) {}
        /*!
          @brief Returns true if prefix and property of the argument
                 Xmpdatum are equal to that of the object.
        */
        bool operator()(const Exiv2::Xmpdatum& xmpdatum) const
            { return key_ == xmpdatum.key(); }

    private:
        std::string key_;

    }; // class FindXmpdatum
}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    Xmpdatum::Xmpdatum(const XmpKey& key,
                       const Value* pValue)
        : key_(key.clone())
    {
        if (pValue) value_ = pValue->clone();
    }

    Xmpdatum::Xmpdatum(const Xmpdatum& rhs)
        : Metadatum(rhs)
    {
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy
    }

    Xmpdatum::~Xmpdatum()
    {
    }

    std::string Xmpdatum::key() const
    {
        return key_.get() == 0 ? "" : key_->key(); 
    }

    std::string Xmpdatum::groupName() const
    {
        return key_.get() == 0 ? "" : key_->groupName();
    }

    std::string Xmpdatum::tagName() const
    {
        return key_.get() == 0 ? "" : key_->tagName();
    }

    std::string Xmpdatum::tagLabel() const
    {
        return key_.get() == 0 ? "" : key_->tagLabel();
    }

    uint16_t Xmpdatum::tag() const
    {
        return key_.get() == 0 ? 0 : key_->tag();
    }

    TypeId Xmpdatum::typeId() const
    {
        return value_.get() == 0 ? invalidTypeId : value_->typeId();
    }

    const char* Xmpdatum::typeName() const
    {
        return TypeInfo::typeName(typeId());
    }

    long Xmpdatum::count() const
    {
        return value_.get() == 0 ? 0 : value_->count();
    }

    long Xmpdatum::size() const
    {
        return value_.get() == 0 ? 0 : value_->size();
    }

    std::string Xmpdatum::toString() const
    {
        return value_.get() == 0 ? "" : value_->toString();
    }

    long Xmpdatum::toLong(long n) const
    {
        return value_.get() == 0 ? -1 : value_->toLong(n);
    }

    float Xmpdatum::toFloat(long n) const
    {
        return value_.get() == 0 ? -1 : value_->toFloat(n); 
    }

    Rational Xmpdatum::toRational(long n) const
    {
        return value_.get() == 0 ? Rational(-1, 1) : value_->toRational(n);
    }

    Value::AutoPtr Xmpdatum::getValue() const
    {
        return value_.get() == 0 ? Value::AutoPtr(0) : value_->clone(); 
    }

    const Value& Xmpdatum::value() const
    {
        if (value_.get() == 0) throw Error(8);
        return *value_;
    }

    long Xmpdatum::copy(byte* /*buf*/, ByteOrder /*byteOrder*/) const
    {
        throw Error(34, "Xmpdatum::copy");
        return 0;
    }

    Xmpdatum& Xmpdatum::operator=(const Xmpdatum& rhs)
    {
        if (this == &rhs) return *this;
        Metadatum::operator=(rhs);

        key_.reset();
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy

        value_.reset();
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy

        return *this;
    } // Xmpdatum::operator=

    Xmpdatum& Xmpdatum::operator=(const uint16_t& value)
    {
        UShortValue::AutoPtr v(new UShortValue);
        v->value_.push_back(value);
        value_ = v;
        return *this;
    }

    Xmpdatum& Xmpdatum::operator=(const std::string& value)
    {
        setValue(value);
        return *this;
    }

    Xmpdatum& Xmpdatum::operator=(const Value& value)
    {
        setValue(&value);
        return *this;
    }

    void Xmpdatum::setValue(const Value* pValue)
    {
        value_.reset();
        if (pValue) value_ = pValue->clone();
    }

    void Xmpdatum::setValue(const std::string& value)
    {
        // Todo: What's the correct default? Adjust doc
        if (value_.get() == 0) {
            assert(0 != key_.get());
            TypeId type = XmpProperties::propertyType(*key_.get());
            value_ = Value::create(type);
        }
        value_->read(value);
    }

    Xmpdatum& XmpData::operator[](const std::string& key)
    {
        XmpKey xmpKey(key);
        iterator pos = findKey(xmpKey);
        if (pos == end()) {
            add(Xmpdatum(xmpKey));
            pos = findKey(xmpKey);
        }
        return *pos;
    }

    int XmpData::load(const byte* buf, long len)
    {
        xmpMetadata_.clear();

        // Todo: Implement me!

        return 0;
    } // XmpData::load

    DataBuf XmpData::copy() const
    {
        DataBuf buf;

        // Todo: Implement me!

        return buf;
    } // XmpData::copy

    int XmpData::add(const XmpKey& key, Value* value)
    {
        return add(Xmpdatum(key, value));
    }

    int XmpData::add(const Xmpdatum& xmpDatum)
    {
        xmpMetadata_.push_back(xmpDatum);
        return 0;
    }

    XmpData::const_iterator XmpData::findKey(const XmpKey& key) const
    {
        return std::find_if(xmpMetadata_.begin(), xmpMetadata_.end(),
                            FindXmpdatum(key));
    }

    XmpData::iterator XmpData::findKey(const XmpKey& key)
    {
        return std::find_if(xmpMetadata_.begin(), xmpMetadata_.end(),
                            FindXmpdatum(key));
    }

    void XmpData::clear()
    {
        xmpMetadata_.clear();
    }

    void XmpData::sortByKey()
    {
        std::sort(xmpMetadata_.begin(), xmpMetadata_.end(), cmpMetadataByKey);
    }

    XmpData::const_iterator XmpData::begin() const 
    {
        return xmpMetadata_.begin(); 
    }

    XmpData::const_iterator XmpData::end() const
    {
        return xmpMetadata_.end(); 
    }

    bool XmpData::empty() const 
    {
        return count() == 0;
    }

    long XmpData::count() const
    {
        return static_cast<long>(xmpMetadata_.size());
    }

    XmpData::iterator XmpData::begin()
    {
        return xmpMetadata_.begin();
    }

    XmpData::iterator XmpData::end()
    {
        return xmpMetadata_.end();
    }

    XmpData::iterator XmpData::erase(XmpData::iterator pos)
    {
        return xmpMetadata_.erase(pos);
    }

    // *************************************************************************
    // free functions
    std::ostream& operator<<(std::ostream& os, const Xmpdatum& md)
    {
        return os << md.value();
    }

}                                       // namespace Exiv2
