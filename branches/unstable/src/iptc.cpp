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
  File:      iptc.cpp
  Version:   $Rev$
  Author(s): Brad Schick (brad) <brad@robotbattle.com>
  History:   31-July-04, brad: created
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "iptc.hpp"
#include "types.hpp"
#include "error.hpp"
#include "value.hpp"
#include "datasets.hpp"
#include "jpgimage.hpp"

// + standard includes
#include <iostream>
#include <algorithm>
#include <iterator>

// *****************************************************************************
namespace {
    /*!
      @brief Read a single dataset payload and create a new metadata entry.

      @param iptcData IPTC metadata container to add the dataset to
      @param dataSet  DataSet number
      @param record   Record Id
      @param data     Pointer to the first byte of dataset payload
      @param sizeData Length in bytes of dataset payload
      @return 0 if successful.
     */
    int readData(
              Exiv2::IptcData& iptcData,
              uint16_t         dataSet,
              uint16_t         record,
        const Exiv2::byte*     data,
              uint32_t         sizeData
    );

    //! Unary predicate that matches an Tag1 with given record and dataset
    class FindTag1 {
    public:
        //! Constructor, initializes the object with the record and dataset id
        FindTag1(uint16_t dataset, uint16_t record)
            : dataset_(dataset), record_(record) {}
        /*!
          @brief Returns true if the record and dataset id of the argument
                Tag1 is equal to that of the object.
        */
        bool operator()(const Exiv2::Tag1& iptcdatum) const
        {
            return dataset_ == iptcdatum.tag() && record_ == iptcdatum.group();
        }

    private:
        // DATA
        uint16_t dataset_;
        uint16_t record_;

    }; // class FindTag1
}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    Tag1& IptcData::operator[](const std::string& key)
    {
        Key1 iptcKey(key);
        iterator pos = findKey(iptcKey);
        if (pos == end()) {
            add(Tag1(iptcKey));
            pos = findKey(iptcKey);
        }
        return *pos;
    }

    long IptcData::size() const
    {
        long newSize = 0;
        const_iterator iter = iptcMetadata_.begin();
        const_iterator end = iptcMetadata_.end();
        for ( ; iter != end; ++iter) {
            // marker, record Id, dataset num, first 2 bytes of size
            newSize += 5;
            long dataSize = iter->size();
            newSize += dataSize;
            if (dataSize > 32767) {
                // extended dataset (we always use 4 bytes)
                newSize += 4;
            }
        }
        return newSize;
    } // IptcData::size

    int IptcData::add(const Key1& key, Value* value)
    {
        return add(Tag1(key, value));
    }

    int IptcData::add(const Tag1& iptcDatum)
    {
        if (!IptcDataSets::dataSetRepeatable(
               iptcDatum.tag(), iptcDatum.group()) &&
               findId(iptcDatum.tag(), iptcDatum.group()) != end()) {
             return 6;
        }
        // allow duplicates
        iptcMetadata_.push_back(iptcDatum);
        return 0;
    }

    IptcData::const_iterator IptcData::findKey(const Key1& key) const
    {
        return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(),
                            FindTag1(key.tag(), key.group()));
    }

    IptcData::iterator IptcData::findKey(const Key1& key)
    {
        return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(),
                            FindTag1(key.tag(), key.group()));
    }

    IptcData::const_iterator IptcData::findId(uint16_t dataset, uint16_t record) const
    {
        return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(),
                            FindTag1(dataset, record));
    }

    IptcData::iterator IptcData::findId(uint16_t dataset, uint16_t record)
    {
        return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(),
                            FindTag1(dataset, record));
    }

    void IptcData::sortByKey()
    {
        std::sort(iptcMetadata_.begin(), iptcMetadata_.end(), cmpTag1ByKey);
    }

    void IptcData::sortByTag()
    {
        std::sort(iptcMetadata_.begin(), iptcMetadata_.end(), cmpTag1ByTag);
    }

    IptcData::iterator IptcData::erase(IptcData::iterator pos)
    {
        return iptcMetadata_.erase(pos);
    }

    const char *IptcData::detectCharset() const
    {
        const_iterator pos = findKey(Key1("Iptc.Envelope.CharacterSet"));
        if (pos != end()) {
            const std::string value = pos->toString();
            if (pos->value().ok()) {
                if (value == "\033%G") return "UTF-8";
                // other values are probably not practically relevant
            }
        }

        bool ascii = true;
        bool utf8 = true;

        for (pos = begin(); pos != end(); ++pos) {
            std::string value = pos->toString();
            if (pos->value().ok()) {
                int seqCount = 0;
                std::string::iterator i;
                for (i = value.begin(); i != value.end(); ++i) {
                    char c = *i;
                    if (seqCount) {
                        if ((c & 0xc0) != 0x80) {
                            utf8 = false;
                            break;
                        }
                        --seqCount;
                    }
                    else {
                        if (c & 0x80) ascii = false;
                        else continue; // ascii character

                        if      ((c & 0xe0) == 0xc0) seqCount = 1;
                        else if ((c & 0xf0) == 0xe0) seqCount = 2;
                        else if ((c & 0xf8) == 0xf0) seqCount = 3;
                        else if ((c & 0xfc) == 0xf8) seqCount = 4;
                        else if ((c & 0xfe) == 0xfc) seqCount = 5;
                        else {
                            utf8 = false;
                            break;
                        }
                    }
                }
                if (seqCount) utf8 = false; // unterminated seq
                if (!utf8) break;
            }
        }

        if (ascii) return "ASCII";
        if (utf8) return "UTF-8";
        return NULL;
    }

    const byte IptcParser::marker_ = 0x1C;          // Dataset marker

    int IptcParser::decode(
              IptcData& iptcData,
        const byte*     pData,
              uint32_t  size
    )
    {
#ifdef DEBUG
        std::cerr << "IptcParser::decode, size = " << size << "\n";
#endif
        const byte* pRead = pData;
        iptcData.clear();

        uint16_t record = 0;
        uint16_t dataSet = 0;
        uint32_t sizeData = 0;
        byte extTest = 0;

        while (pRead + 3 < pData + size) {
            // First byte should be a marker. If it isn't, scan forward and skip
            // the chunk bytes present in some images. This deviates from the
            // standard, which advises to treat such cases as errors.
            if (*pRead++ != marker_) continue;
            record = *pRead++;
            dataSet = *pRead++;

            extTest = *pRead;
            if (extTest & 0x80) {
                // extended dataset
                uint16_t sizeOfSize = (getUShort(pRead, bigEndian) & 0x7FFF);
                if (sizeOfSize > 4) return 5;
                pRead += 2;
                sizeData = 0;
                for (; sizeOfSize > 0; --sizeOfSize) {
                    sizeData |= *pRead++ << (8 *(sizeOfSize-1));
                }
            }
            else {
                // standard dataset
                sizeData = getUShort(pRead, bigEndian);
                pRead += 2;
            }
            if (pRead + sizeData <= pData + size) {
                int rc = 0;
                if ((rc = readData(iptcData, dataSet, record, pRead, sizeData)) != 0) {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to read IPTC dataset "
                                << Key1(dataSet, record)
                                << " (rc = " << rc << "); skipped.\n";
#endif
                }
            }
#ifndef SUPPRESS_WARNINGS
            else {
                EXV_WARNING << "IPTC dataset " << Key1(dataSet, record)
                            << " has invalid size " << sizeData << "; skipped.\n";
            }
#endif
            pRead += sizeData;
        }

        return 0;
    } // IptcParser::decode

    /*!
      @brief Compare two iptc items by record. Return true if the record of
             lhs is less than that of rhs.

      This is a helper function for IptcParser::encode().
     */
    bool cmpIptcdataByRecord(const Tag1& lhs, const Tag1& rhs)
    {
        return lhs.group() < rhs.group();
    }

    DataBuf IptcParser::encode(const IptcData& iptcData)
    {
        DataBuf buf(iptcData.size());
        byte *pWrite = buf.pData_;

        // Copy the iptc data sets and sort them by record but preserve the order of datasets
        IptcMetadata sortedIptcData;
        std::copy(iptcData.begin(), iptcData.end(), std::back_inserter(sortedIptcData));
        std::stable_sort(sortedIptcData.begin(), sortedIptcData.end(), cmpIptcdataByRecord);

        IptcData::const_iterator iter = sortedIptcData.begin();
        IptcData::const_iterator end = sortedIptcData.end();
        for ( ; iter != end; ++iter) {
            // marker, record Id, dataset num
            *pWrite++ = marker_;
            *pWrite++ = static_cast<byte>(iter->group());
            *pWrite++ = static_cast<byte>(iter->tag());

            // extended or standard dataset?
            long dataSize = iter->size();
            if (dataSize > 32767) {
                // always use 4 bytes for extended length
                uint16_t sizeOfSize = 4 | 0x8000;
                us2Data(pWrite, sizeOfSize, bigEndian);
                pWrite += 2;
                ul2Data(pWrite, dataSize, bigEndian);
                pWrite += 4;
            }
            else {
                us2Data(pWrite, static_cast<uint16_t>(dataSize), bigEndian);
                pWrite += 2;
            }
            pWrite += iter->value().copy(pWrite, bigEndian);
        }

        return buf;
    } // IptcParser::encode

}                                       // namespace Exiv2

// *****************************************************************************
// local definitions
namespace {

    int readData(
              Exiv2::IptcData& iptcData,
              uint16_t         dataSet,
              uint16_t         record,
        const Exiv2::byte*     data,
              uint32_t         sizeData
    )
    {
        Exiv2::Value::AutoPtr value;
        Exiv2::TypeId type = Exiv2::IptcDataSets::dataSetType(dataSet, record);
        value = Exiv2::Value::create(type);
        int rc = value->read(data, sizeData, Exiv2::bigEndian);
        if (0 == rc) {
            Exiv2::Key1 key(dataSet, record);
            iptcData.add(key, value.get());
        }
        else if (1 == rc) {
            // If the first attempt failed, try with a string value
            value = Exiv2::Value::create(Exiv2::string);
            rc = value->read(data, sizeData, Exiv2::bigEndian);
            if (0 == rc) {
                Exiv2::Key1 key(dataSet, record);
                iptcData.add(key, value.get());
            }
        }
        return rc;
    }

}
