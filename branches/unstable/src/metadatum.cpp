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

    Metadatum::Metadatum()
    {
    }

    Metadatum::Metadatum(const Metadatum& /*rhs*/)
    {
    }

    Metadatum::~Metadatum()
    {
    }

    Metadatum& Metadatum::operator=(const Metadatum& /*rhs*/)
    {
        return *this;
    }

    std::string Metadatum::print(const ExifData* pMetadata) const
    {
        std::ostringstream os;
        write(os, pMetadata);
        return os.str();
    }

    bool cmpTag1ByTag(const Tag1& lhs, const Tag1& rhs)
    {
        return lhs.tag() < rhs.tag();
    }

    bool cmpTag1ByKey(const Tag1& lhs, const Tag1& rhs)
    {
        return lhs.key() < rhs.key();
    }

}                                       // namespace Exiv2

