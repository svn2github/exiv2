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
  File:      properties.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   13-July-07, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "properties.hpp"
#include "error.hpp"
#include "types.hpp"
#include "value.hpp"
#include "metadatum.hpp"
#include "i18n.h"                // NLS support.

#include <iostream>
#include <iomanip>
#include <sstream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    extern const XmpPropertyInfo xmpTiffInfo[];
    extern const XmpPropertyInfo xmpExifInfo[];

    extern const XmpNsInfo xmpNsInfo[] = {
        // Schemas
        { "http://purl.org/dc/elements/1.1/",             "dc",           0, "Dublin Core schema" },
        { "http://ns.adobe.com/xap/1.0/",                 "xmp",          0, "XMP Basic schema" },
        { "http://ns.adobe.com/xap/1.0/rights/",          "xmpRights",    0, "XMP Rights Management schema" },
        { "http://ns.adobe.com/xap/1.0/mm/",              "xmpMM",        0, "XMP Media Management schema" },
        { "http://ns.adobe.com/xap/1.0/bj/",              "xmpBJ",        0, "XMP Basic Job Ticket schema" },
        { "http://ns.adobe.com/xap/1.0/t/pg/",            "xmpTPg",       0, "XMP Paged-Text schema" },
        { "http://ns.adobe.com/xmp/1.0/DynamicMedia/",    "xmpDM",        0, "XMP Dynamic Media schema" },
        { "http://ns.adobe.com/pdf/1.3/",                 "pdf",          0, "Adobe PDF schema" },
        { "http://ns.adobe.com/photoshop/1.0/",           "photoshop",    0, "Photoshop schema" },
        { "http://ns.adobe.com/camera-raw-settings/1.0/", "crs",          0, "Camera Raw schema" },
        { "http://ns.adobe.com/tiff/1.0/",                "tiff",         xmpTiffInfo, "Exif Schema for TIFF Properties" },
        { "http://ns.adobe.com/exif/1.0/",                "exif",         xmpExifInfo, "Exif schema for Exif-specific Properties" },
        { "http://ns.adobe.com/exif/1.0/aux/",            "aux",          0, "Exif schema for Additional Exif Properties" },
        { "http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/",  "iptc" /*Iptc4xmpCore*/, 0, "IPTC Core schema" },

        // Structures
        { "http://ns.adobe.com/xap/1.0/g/",                   "xapG",    0, "Colorant structure" },
        { "http://ns.adobe.com/xap/1.0/sType/Dimensions#",    "stDim",   0, "Dimensions structure" },
        { "http://ns.adobe.com/xap/1.0/sType/Font#",          "stFnt",   0, "Font structure" },
        { "http://ns.adobe.com/xap/1.0/g/img/",               "xapGImg", 0, "Thumbnail structure" },
        { "http://ns.adobe.com/xap/1.0/sType/ResourceEvent#", "stEvt",   0, "Resource Event structure" },
        { "http://ns.adobe.com/xap/1.0/sType/ResourceRef#",   "stRef",   0, "ResourceRef structure" },
        { "http://ns.adobe.com/xap/1.0/sType/Version#",       "stVer",   0, "Version structure" },
        { "http://ns.adobe.com/xap/1.0/sType/Job#",           "stJob",   0, "Basic Job/Workflow structure" },

        // Qualifiers
        { "http://ns.adobe.com/xmp/Identifier/qual/1.0/", "xmpidq", 0, "Qualifier for xmp:Identifier" }
    };

    extern const XmpPropertyInfo xmpTiffInfo[] = {
        { "XResolution",      "XResolution",      "Rational",    unsignedRational, xmpInternal, "TIFF tag 282, 0x11A. Horizontal resolution in pixels per unit." },
        { "YResolution",      "YResolution",      "Rational",    unsignedRational, xmpInternal, "TIFF tag 283, 0x11B. Vertical resolution in pixels per unit." },
        { "ImageWidth",       "ImageWidth",       "Integer",     unsignedLong,     xmpInternal, "TIFF tag 256, 0x100. Image width in pixels." },
        { "ImageLength",      "ImageLength",      "Integer",     unsignedLong,     xmpInternal, "TIFF tag 257, 0x101. Image height in pixels." },
        { "BitsPerSample",    "BitsPerSample",    "seq Integer", unsignedShort,    xmpInternal, "TIFF tag 258, 0x102. Number of bits per component in each channel." },
        { "Compression",      "Compression",      "Closed Choice of Integer", unsignedShort, xmpInternal, "TIFF tag 259, 0x103. Compression scheme: 1 = uncompressed; 6 = JPEG." },
        { "PhotometricInterpretation", "PhotometricInterpretation", "Closed Choice of Integer", unsignedShort, xmpInternal, "TIFF tag 262, 0x106. Pixel Composition: 2 = RGB; 6 = YCbCr." },
        { "Orientation",      "Orientation",      "Closed Choice of Integer", unsignedShort, xmpInternal, "TIFF tag 274, 0x112. Orientation:"
                                                                                                "1 = 0th row at top, 0th column at left "
                                                                                                "2 = 0th row at top, 0th column at right "
                                                                                                "3 = 0th row at bottom, 0th column at right "
                                                                                                "4 = 0th row at bottom, 0th column at left "
                                                                                                "5 = 0th row at left, 0th column at top "
                                                                                                "6 = 0th row at right, 0th column at top "
                                                                                                "7 = 0th row at right, 0th column at bottom "
                                                                                                "8 = 0th row at left, 0th column at bottom" },
        { "SamplesPerPixel",  "SamplesPerPixel",  "Integer",     unsignedShort,    xmpInternal, "TIFF tag 277, 0x115. Number of components per pixel." },
        { "PlanarConfiguration", "PlanarConfiguration", "Closed Choice of Integer", unsignedShort, xmpInternal, "TIFF tag 284, 0x11C. Data layout:1 = chunky; 2 = planar." },
        { "ReferenceBlackWhite", "ReferenceBlackWhite", "seq Rational", unsignedRational, xmpInternal, "TIFF tag 532, 0x214. Reference black and white point values." },
        { "DateTime",         "DateTime",         "Date",        date,             xmpInternal, "TIFF tag 306, 0x132 (primary) and EXIF tag 37520, "
                                                                                                "0x9290 (subseconds). Date and time of image creation "
                                                                                                "(no time zone in EXIF), stored in ISO 8601 format, not "
                                                                                                "the original EXIF format. This property includes the "
                                                                                                "value for the EXIF SubSecTime attribute. "
                                                                                                "NOTE: This property is stored in XMP as xmp:ModifyDate." },
        { "ImageDescription", "ImageDescription", "Lang Alt",    asciiString,      xmpExternal, "TIFF tag 270, 0x10E. Description of the image. N O T E : This property is stored in XMP as dc:description." },
        { "Make",             "Make",             "ProperName",  asciiString,      xmpInternal, "TIFF tag 271, 0x10F. Manufacturer of recording equipment." },
        { "Model",            "Model",            "ProperName",  asciiString,      xmpInternal, "TIFF tag 272, 0x110. Model name or number of equipment." }
    };

    extern const XmpPropertyInfo xmpExifInfo[] = {
        { "PixelXDimension",  "PixelXDimension",  "Integer",     unsignedLong,     xmpInternal, "EXIF tag 40962, 0xA002. Valid image width, in pixels." },
        { "PixelYDimension",  "PixelYDimension",  "Integer",     unsignedLong,     xmpInternal, "EXIF tag 40963, 0xA003. Valid image height, in pixels." }
    };

    XmpNsInfo::Ns::Ns(const std::string& ns)
        : ns_(ns)
    {
    }

    XmpNsInfo::Prefix::Prefix(const std::string& prefix)
        : prefix_(prefix)
    {
    }

    bool XmpNsInfo::operator==(const XmpNsInfo::Ns& ns) const
    {
        std::string n(ns_);
        return n == ns.ns_;
    }

    bool XmpNsInfo::operator==(const XmpNsInfo::Prefix& prefix) const
    {
        std::string p(prefix_);
        return p == prefix.prefix_;
    }

    bool XmpPropertyInfo::operator==(const std::string& name) const
    {
        std::string n(name_);
        return n == name;
    }

    const char* XmpProperties::propertyTitle(const XmpKey& key)
    {
        return propertyInfo(key)->title_;
    }

    const char* XmpProperties::propertyDesc(const XmpKey& key)
    {
        return propertyInfo(key)->desc_;
    }

    TypeId XmpProperties::propertyType(const XmpKey& key)
    {
        return propertyInfo(key)->typeId_;
    }

    const XmpPropertyInfo* XmpProperties::propertyInfo(const XmpKey& key)
    {
        const XmpPropertyInfo* pl = propertyList(key.groupName());
        if (!pl) throw Error(6, key.groupName()); // Todo: Error number
        const int count = sizeof(pl) / sizeof(pl[0]);
        const XmpPropertyInfo* pi = std::find(pl, pl + count, key.tagName());
        if (pi == pl + count) throw Error(6, key.tagName()); // Todo: Error number
        return pi;
    }

    const char* XmpProperties::ns(const std::string& prefix)
    {
        return nsInfo(prefix)->ns_;
    }

    const char* XmpProperties::nsDesc(const std::string& prefix)
    {
        return nsInfo(prefix)->desc_;
    }

    const XmpPropertyInfo* XmpProperties::propertyList(const std::string& prefix)
    {
        return nsInfo(prefix)->xmpPropertyInfo_;
    }

    const XmpNsInfo* XmpProperties::nsInfo(const std::string& prefix)
    {
        const XmpNsInfo* xn = find(xmpNsInfo, XmpNsInfo::Prefix(prefix));
        if (!xn) throw Error(6, prefix); // Todo: Error number
        return xn;
    }

    const char* XmpProperties::prefix(const std::string& ns)
    {
        const XmpNsInfo* xn = find(xmpNsInfo, XmpNsInfo::Ns(ns));
        return xn ? xn->prefix_ : 0;
    }

    void XmpProperties::printProperties(std::ostream& os, const std::string& prefix)
    {
        const XmpPropertyInfo* pl = propertyList(prefix);
        if (pl) {
            const int ck = sizeof(pl) / sizeof(pl[0]);                
            for (int k = 0; k < ck; ++k) {
                os << pl[k];
            }
        }

    } // XmpProperties::printProperties

    //! @cond IGNORE

    //! Internal Pimpl structure with private members and data of class XmpKey.
    struct XmpKey::Impl {
        Impl();                         //!< Default constructor
        Impl(const std::string& prefix, const std::string& property); //!< Constructor

        /*!
          @brief Parse and convert the \em key string into property and prefix.
                 Updates data members if the string can be decomposed, or throws
                 \em Error.

          @throw Error if the key cannot be decomposed.
        */
        void decomposeKey(const XmpKey* self, const std::string& key);

        // DATA
        static const char* familyName_; //!< "Xmp"

        std::string prefix_;            //!< Prefix
        std::string property_;          //!< Property name
    };
    //! @endcond

    XmpKey::Impl::Impl()
    {
    }

    XmpKey::Impl::Impl(const std::string& prefix, const std::string& property)
        : prefix_(prefix), property_(property)
    {
    }

    const char* XmpKey::Impl::familyName_ = "Xmp";

    XmpKey::XmpKey(const std::string& key)
        : p_(new Impl)
    {
        p_->decomposeKey(this, key);
    }

    XmpKey::XmpKey(const std::string& prefix, const std::string& property)
        : p_(new Impl(prefix, property))
    {
        // Validate prefix and property, throws
        XmpProperties::propertyInfo(*this);
    }

    XmpKey::~XmpKey()
    {
        delete p_;
    }

    XmpKey::XmpKey(const XmpKey& rhs)
        : Key(rhs), p_(new Impl(*rhs.p_))
    {
    }

    XmpKey& XmpKey::operator=(const XmpKey& rhs)
    {
        if (this == &rhs) return *this;
        *p_ = *rhs.p_;
        return *this;
    }

    XmpKey::AutoPtr XmpKey::clone() const
    {
        return AutoPtr(clone_());
    }

    XmpKey* XmpKey::clone_() const
    {
        return new XmpKey(*this);
    }

    std::string XmpKey::key() const
    {
        return std::string(p_->familyName_) + "." + p_->prefix_ + "." + p_->property_;
    }

    const char* XmpKey::familyName() const
    {
        return p_->familyName_;
    }

    std::string XmpKey::groupName() const
    {
        return p_->prefix_;
    }

    std::string XmpKey::tagName() const
    { 
        return p_->property_;
    }

    std::string XmpKey::tagLabel() const
    {
        return XmpProperties::propertyTitle(*this);
    }

    const char* XmpKey::ns() const
    {
        return XmpProperties::ns(p_->prefix_);
    }

    void XmpKey::Impl::decomposeKey(const XmpKey* self, const std::string& key)
    {
        // Get the family name, prefix and property name parts of the key
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos) throw Error(6, key);
        std::string familyName = key.substr(0, pos1);
        if (familyName != std::string(familyName_)) {
            throw Error(6, key);
        }
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos) throw Error(6, key);
        std::string prefix = key.substr(pos0, pos1 - pos0);
        if (prefix == "") throw Error(6, key);
        std::string property = key.substr(pos1 + 1);
        if (property == "") throw Error(6, key);

        property_ = property;
        prefix_ = prefix;

        // Validate prefix and property
        XmpProperties::propertyInfo(*self);

    } // XmpKey::Impl::decomposeKey

    // *************************************************************************
    // free functions
    std::ostream& operator<<(std::ostream& os, const XmpPropertyInfo& property)
    {
        return os << property.name_                       << ",\t"
                  << property.title_                      << ",\t"
                  << property.xmpValueType_               << ",\t"
                  << TypeInfo::typeName(property.typeId_) << ",\t"
                  << ( property.xmpCategory_ == xmpExternal ? "External" : "Internal" ) << ",\t"
                  << property.desc_                       << "\n";
    }

}                                       // namespace Exiv2
