// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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
  File:      exif.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "exif.hpp"
#include "metadatum.hpp"
#include "tags.hpp"
#include "value.hpp"
#include "types.hpp"
#include "error.hpp"
#include "basicio.hpp"
#include "tiffimage.hpp"

// + standard includes
#include <iostream>
#include <sstream>
#include <utility>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <cstdio>

// *****************************************************************************
namespace {
    /*!
      @brief Exif %Thumbnail image. This abstract base class provides the
             interface for the thumbnail image that is optionally embedded in
             the Exif data. This class is used internally by ExifData, it is
             probably not useful for a client as a standalone class.  Instead,
             use an instance of ExifData to access the Exif thumbnail image.
     */
    class Thumbnail {
    public:
        //! Shortcut for a %Thumbnail auto pointer.
        typedef std::auto_ptr<Thumbnail> AutoPtr;

        //! @name Creators
        //@{
        //! Virtual destructor
        virtual ~Thumbnail() {}
        //@}

        //! Factory function to create a thumbnail for the Exif metadata provided.
        static AutoPtr create(const Exiv2::ExifData& exifData);

        //! @name Accessors
        //@{
        /*!
          @brief Return the thumbnail image in a %DataBuf. The caller owns the
                 data buffer and %DataBuf ensures that it will be deleted.
         */
        virtual Exiv2::DataBuf copy(const Exiv2::ExifData& exifData) const =0;
        /*!
          @brief Return the MIME type of the thumbnail ("image/tiff" or
                 "image/jpeg").
         */
        virtual const char* mimeType() const =0;
        /*!
          @brief Return the file extension for the format of the thumbnail
                 (".tif", ".jpg").
         */
        virtual const char* extension() const =0;
        //@}

    }; // class Thumbnail

    //! Exif thumbnail image in TIFF format
    class TiffThumbnail : public Thumbnail {
    public:
        //! Shortcut for a %TiffThumbnail auto pointer.
        typedef std::auto_ptr<TiffThumbnail> AutoPtr;

        //! @name Manipulators
        //@{
        //! Assignment operator.
        TiffThumbnail& operator=(const TiffThumbnail& rhs);
        //@}

        //! @name Accessors
        //@{
        Exiv2::DataBuf copy(const Exiv2::ExifData& exifData) const;
        const char* mimeType() const;
        const char* extension() const;
        //@}

    }; // class TiffThumbnail

    //! Exif thumbnail image in JPEG format
    class JpegThumbnail : public Thumbnail {
    public:
        //! Shortcut for a %JpegThumbnail auto pointer.
        typedef std::auto_ptr<JpegThumbnail> AutoPtr;

        //! @name Manipulators
        //@{
        //! Assignment operator.
        JpegThumbnail& operator=(const JpegThumbnail& rhs);
        //@}

        //! @name Accessors
        //@{
        Exiv2::DataBuf copy(const Exiv2::ExifData& exifData) const;
        const char* mimeType() const;
        const char* extension() const;
        //@}

    }; // class JpegThumbnail

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    /*!
      @brief Set the value of \em exifDatum to \em value. If the object already
             has a value, it is replaced. Otherwise a new ValueType\<T\> value
             is created and set to \em value.

      This is a helper function, called from Exifdatum members. It is meant to
      be used with T = (u)int16_t, (u)int32_t or (U)Rational. Do not use directly.
    */
    template<typename T>
    Exiv2::Exifdatum& setValue(Exiv2::Exifdatum& exifDatum, const T& value)
    {
        std::auto_ptr<Exiv2::ValueType<T> > v
            = std::auto_ptr<Exiv2::ValueType<T> >(new Exiv2::ValueType<T>);
        v->value_.push_back(value);
        exifDatum.value_ = v;
        return exifDatum;
    }

    Exifdatum::Exifdatum(const ExifKey& key, const Value* pValue)
        : key_(key.clone())
    {
        if (pValue) value_ = pValue->clone();
    }

    Exifdatum::~Exifdatum()
    {
    }

    Exifdatum::Exifdatum(const Exifdatum& rhs)
        : Metadatum(rhs)
    {
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy
    }

    std::ostream& Exifdatum::write(std::ostream& os, const ExifData* pMetadata) const
    {
        return ExifTags::printTag(os, tag(), ifdId(), value(), pMetadata);
    }

    const Value& Exifdatum::value() const
    {
        if (value_.get() == 0) throw Error(8);
        return *value_;
    }

    Exifdatum& Exifdatum::operator=(const Exifdatum& rhs)
    {
        if (this == &rhs) return *this;
        Metadatum::operator=(rhs);

        key_.reset();
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy

        value_.reset();
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy

        return *this;
    } // Exifdatum::operator=

    Exifdatum& Exifdatum::operator=(const std::string& value)
    {
        setValue(value);
        return *this;
    }

    Exifdatum& Exifdatum::operator=(const uint16_t& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const uint32_t& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const URational& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const int16_t& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const int32_t& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const Rational& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const Value& value)
    {
        setValue(&value);
        return *this;
    }

    void Exifdatum::setValue(const Value* pValue)
    {
        value_.reset();
        if (pValue) value_ = pValue->clone();
    }

    void Exifdatum::setValue(const std::string& value)
    {
        if (value_.get() == 0) {
            TypeId type = ExifTags::tagType(tag(), ifdId());
            value_ = Value::create(type);
        }
        value_->read(value);
    }

    ExifThumbC::ExifThumbC(const ExifData& exifData)
        : exifData_(exifData)
    {
    }

    DataBuf ExifThumbC::copy() const
    {
        Thumbnail::AutoPtr thumbnail = Thumbnail::create(exifData_);
        if (thumbnail.get() == 0) return DataBuf();
        return thumbnail->copy(exifData_);
    }

    long ExifThumbC::writeFile(const std::string& path) const
    {
        Thumbnail::AutoPtr thumbnail = Thumbnail::create(exifData_);
        if (thumbnail.get() == 0) return 0;
        std::string name = path + thumbnail->extension();
        DataBuf buf(thumbnail->copy(exifData_));
        if (buf.size_ == 0) return 0;
        return Exiv2::writeFile(buf, name);
    }

    const char* ExifThumbC::mimeType() const
    {
        Thumbnail::AutoPtr thumbnail = Thumbnail::create(exifData_);
        if (thumbnail.get() == 0) return "";
        return thumbnail->mimeType();
    }

    const char* ExifThumbC::extension() const
    {
        Thumbnail::AutoPtr thumbnail = Thumbnail::create(exifData_);
        if (thumbnail.get() == 0) return "";
        return thumbnail->extension();
    }

    ExifThumb::ExifThumb(ExifData& exifData)
        : ExifThumbC(exifData), exifData_(exifData)
    {
    }

    void ExifThumb::setJpegThumbnail(
        const std::string& path,
              URational    xres,
              URational    yres,
              uint16_t     unit
    )
    {
        DataBuf thumb = readFile(path); // may throw
        setJpegThumbnail(thumb.pData_, thumb.size_, xres, yres, unit);
    }

    void ExifThumb::setJpegThumbnail(
        const byte*     buf,
              long      size,
              URational xres,
              URational yres,
              uint16_t  unit
    )
    {
        setJpegThumbnail(buf, size);
        exifData_["Exif.Thumbnail.XResolution"] = xres;
        exifData_["Exif.Thumbnail.YResolution"] = yres;
        exifData_["Exif.Thumbnail.ResolutionUnit"] = unit;
    }

    void ExifThumb::setJpegThumbnail(const std::string& path)
    {
        DataBuf thumb = readFile(path); // may throw
        setJpegThumbnail(thumb.pData_, thumb.size_);
    }

    void ExifThumb::setJpegThumbnail(const byte* buf, long size)
    {
        exifData_["Exif.Thumbnail.Compression"] = uint16_t(6);
        Exifdatum& format = exifData_["Exif.Thumbnail.JPEGInterchangeFormat"];
        format = uint32_t(0);
        format.setDataArea(buf, size);
        exifData_["Exif.Thumbnail.JPEGInterchangeFormatLength"] = uint32_t(size);
    }

    void ExifThumb::erase()
    {
        // Delete all Exif.Thumbnail.* (IFD1) metadata
        // Todo: Do it the C++ way(TM)
        ExifMetadata::iterator i = exifData_.begin();
        while (i != exifData_.end()) {
            if (i->ifdId() == ifd1Id) {
                i = exifData_.erase(i);
            }
            else {
                ++i;
            }
        }
    }

    Exifdatum& ExifData::operator[](const std::string& key)
    {
        ExifKey exifKey(key);
        iterator pos = findKey(exifKey);
        if (pos == end()) {
            add(Exifdatum(exifKey));
            pos = findKey(exifKey);
        }
        return *pos;
    }

    void ExifData::add(const ExifKey& key, const Value* pValue)
    {
        add(Exifdatum(key, pValue));
    }

    void ExifData::add(const Exifdatum& exifdatum)
    {
        // allow duplicates
        exifMetadata_.push_back(exifdatum);
    }

    ExifData::const_iterator ExifData::findKey(const ExifKey& key) const
    {
        return std::find_if(exifMetadata_.begin(), exifMetadata_.end(),
                            FindMetadatumByKey(key.key()));
    }

    ExifData::iterator ExifData::findKey(const ExifKey& key)
    {
        return std::find_if(exifMetadata_.begin(), exifMetadata_.end(),
                            FindMetadatumByKey(key.key()));
    }

    void ExifData::clear()
    {
        exifMetadata_.clear();
    }

    void ExifData::sortByKey()
    {
        std::sort(exifMetadata_.begin(), exifMetadata_.end(), cmpMetadataByKey);
    }

    void ExifData::sortByTag()
    {
        std::sort(exifMetadata_.begin(), exifMetadata_.end(), cmpMetadataByTag);
    }

    ExifData::iterator ExifData::erase(ExifData::iterator pos)
    {
        return exifMetadata_.erase(pos);
    }

    ByteOrder ExifParser::decode(
              ExifData& exifData,
        const byte*     pData,
              uint32_t  size
    )
    {
        IptcData iptcData;
        XmpData  xmpData;
        ByteOrder bo = TiffParser::decode(exifData,
                                          iptcData,
                                          xmpData,
                                          pData,
                                          size);
#ifndef SUPPRESS_WARNINGS
        if (!iptcData.empty()) {
            std::cerr << "Warning: Ignoring IPTC information encoded in the Exif data.\n";
        }
        if (!xmpData.empty()) {
            std::cerr << "Warning: Ignoring XMP information encoded in the Exif data.\n";
        }
#endif
        return bo;
    } // ExifParser::decode

    WriteMethod ExifParser::encode(
              Blob&     blob,
        const byte*     pData,
              uint32_t  size,
              ByteOrder byteOrder,
        const ExifData& exifData
    )
    {
        static const char* filteredTags[] = {
            "Exif.Image.StripOffsets",
            "Exif.Image.RowsPerStrip",
            "Exif.Image.StripByteCounts"
        };

        ExifData ed = exifData;
        for (unsigned int i = 0; i < EXV_COUNTOF(filteredTags); ++i) {
            ExifData::iterator pos = ed.findKey(ExifKey(filteredTags[i]));
            if (pos != ed.end()) ed.erase(pos);
        }

        const IptcData iptcData;
        const XmpData  xmpData;
        return TiffParser::encode(blob,
                                  pData,
                                  size,
                                  byteOrder,
                                  ed,
                                  iptcData,
                                  xmpData);
    } // ExifParser::encode

}                                       // namespace Exiv2

// *****************************************************************************
// local definitions
namespace {

    //! @cond IGNORE
    Thumbnail::AutoPtr Thumbnail::create(const Exiv2::ExifData& exifData)
    {
        Thumbnail::AutoPtr thumbnail;
        const Exiv2::ExifKey k1("Exif.Thumbnail.Compression");
        Exiv2::ExifData::const_iterator pos = exifData.findKey(k1);
        if (pos != exifData.end()) {
            if (pos->count() == 0) return thumbnail;
            long compression = pos->toLong();
            if (compression == 6) {
                thumbnail = Thumbnail::AutoPtr(new JpegThumbnail);
            }
            else {
                thumbnail = Thumbnail::AutoPtr(new TiffThumbnail);
            }
        }
        else {
            const Exiv2::ExifKey k2("Exif.Thumbnail.JPEGInterchangeFormat");
            pos = exifData.findKey(k2);
            if (pos != exifData.end()) {
                thumbnail = Thumbnail::AutoPtr(new JpegThumbnail);
            }
        }
        return thumbnail;
    }

    const char* TiffThumbnail::mimeType() const
    {
        return "image/tiff";
    }

    const char* TiffThumbnail::extension() const
    {
        return ".tif";
    }

    Exiv2::DataBuf TiffThumbnail::copy(const Exiv2::ExifData& exifData) const
    {
        Exiv2::ExifData thumb;
        // Copy all Thumbnail (IFD1) tags from exifData to Image (IFD0) tags in thumb
        for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != exifData.end(); ++i) {
            if (i->groupName() == "Thumbnail") {
                std::string key = "Exif.Image." + i->tagName();
                thumb.add(Exiv2::ExifKey(key), &i->value());
            }
        }

        Exiv2::Blob blob;
        const Exiv2::IptcData emptyIptc;
        const Exiv2::XmpData  emptyXmp;
        Exiv2::TiffParser::encode(blob, 0, 0, Exiv2::littleEndian, thumb, emptyIptc, emptyXmp);
        return Exiv2::DataBuf(&blob[0], static_cast<long>(blob.size()));
    }

    const char* JpegThumbnail::mimeType() const
    {
        return "image/jpeg";
    }

    const char* JpegThumbnail::extension() const
    {
        return ".jpg";
    }

    Exiv2::DataBuf JpegThumbnail::copy(const Exiv2::ExifData& exifData) const
    {
        Exiv2::ExifKey key("Exif.Thumbnail.JPEGInterchangeFormat");
        Exiv2::ExifData::const_iterator format = exifData.findKey(key);
        if (format == exifData.end()) return Exiv2::DataBuf();
        return format->dataArea();
    }
    //! @endcond
}
