// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*
  File:      exivsimple.cpp
  Version:   $Rev$
  Author(s): Brad Schick <brad@robotbattle.com>
  History:   12-Nov-04, brad: created
 */
// *****************************************************************************

#include "stdafx.h"
#include "exivsimple.h"
#include "image.hpp"
#include "exif.hpp"
#include "iptc.hpp"
#include <cassert>

struct ImageWrapper
{
    Exiv2::Image::AutoPtr image;
};

// Returns 0 if failed.
EXIVSIMPLE_API HIMAGE OpenImage(const char *file)
{
    assert(file);
    ImageWrapper *imgWrap = new ImageWrapper;

    // See if file exists. Sorry for very bad error handling
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(file)) {
        return 0;
    }

    imgWrap->image = Exiv2::ImageFactory::open(file);
    if (imgWrap->image.get() == 0) {
        return 0;
    }

    // Load existing metadata
    if (imgWrap->image->readMetadata()) {
        delete imgWrap;
        imgWrap = 0;
    }

    return (HIMAGE)imgWrap;
}

EXIVSIMPLE_API void FreeImage(HIMAGE img)
{
    if (img) {
        ImageWrapper *imgWrap = (ImageWrapper*)img;
        delete imgWrap;
    }
}

EXIVSIMPLE_API int SaveImage(HIMAGE img)
{
    assert(img);
    ImageWrapper *imgWrap = (ImageWrapper*)img;
    return imgWrap->image->writeMetadata();
}

// This is weird because iptc and exif have not been "unified". Once
// they are unified, this DLL should not have to know
// about either... just generic images, keys, values, etc.
//
// buffsize should be the total size of *buff (including space for null)
// Note that if there is more than one entry (for some IPTC datasets) this
// returns the first one found. Currently no way to get the others.
EXIVSIMPLE_API int ReadMeta(HIMAGE img, const char *key, char *buff, int buffsize)
{
    assert(img && key && buff);
    if (img==0 || key==0 || buff==0 || buffsize==0) return -1;
    ImageWrapper *imgWrap = (ImageWrapper*)img;
    int rc = 2;

    Exiv2::IptcData &iptcData = imgWrap->image->iptcData();
    Exiv2::ExifData &exifData = imgWrap->image->exifData();

    try {
        // First try iptc
        Exiv2::IptcKey iptcKey(key);
        rc = 1;
        Exiv2::IptcData::const_iterator iter = iptcData.findKey(iptcKey);
        if (iter != iptcData.end()) {
            strncpy(buff, iter->value().toString().c_str(), buffsize);
            buff[buffsize-1] = 0;
            rc = 0;
        }
    } 
    catch(const Exiv2::Error&) {
    }

    if (rc) {
        // No iptc value, so try exif
        try {
            Exiv2::ExifKey exifKey(key);
            rc = 1;
            Exiv2::ExifData::const_iterator iter = exifData.findKey(exifKey);
            if (iter != exifData.end()) {
                strncpy(buff, iter->value().toString().c_str(), buffsize);
                buff[buffsize-1] = 0;
                rc = 0;
            }
        }
        catch(const Exiv2::Error&) {
        }
    }

    return rc;
}

// Overwrites existing value if found, otherwise creates a new one.
// Passing invalidTypeId causes the type to be guessed.
// Guessing types is accurate for IPTC, but not for EXIF.
EXIVSIMPLE_API int ModifyMeta(HIMAGE img, const char *key, const char *val, DllTypeId type)
{
    assert(img && key && val);
    if (img==0 || key==0 || val==0) return -1;
    ImageWrapper *imgWrap = (ImageWrapper*)img;
    int rc = 2;

    Exiv2::IptcData &iptcData = imgWrap->image->iptcData();
    Exiv2::ExifData &exifData = imgWrap->image->exifData();

    std::string data(val);
    // if data starts and ends with quotes, remove them
    if (data.at(0) == '\"' && data.at(data.size()-1) == '\"') {
        data = data.substr(1, data.size()-2);
    }

    try {
        Exiv2::IptcKey iptcKey(key);
        rc = 1;

        if (type == invalidTypeId)
            type = (DllTypeId)Exiv2::IptcDataSets::dataSetType(iptcKey.tag(), iptcKey.record());
        Exiv2::Value::AutoPtr value = Exiv2::Value::create((Exiv2::TypeId)type);
        value->read(data);

        Exiv2::IptcData::iterator iter = iptcData.findKey(iptcKey);
        if (iter != iptcData.end()) {
            iter->setValue(value.get());
            rc = 0;
        }
        else {
            rc = iptcData.add(iptcKey, value.get());
        }
    } 
    catch(const Exiv2::Error&) {
    }

    if (rc) {
        // Failed with iptc, so try exif
        try {
            Exiv2::ExifKey exifKey(key);
            rc = 1;

            // No way to get value type for exif... string is the most common
            if (type == invalidTypeId)
                type = asciiString;
            Exiv2::Value::AutoPtr value = Exiv2::Value::create((Exiv2::TypeId)type);
            value->read(data);

            Exiv2::ExifData::iterator iter = exifData.findKey(exifKey);
            if (iter != exifData.end()) {
                iter->setValue(value.get());
                rc = 0;
            }
            else {
                exifData.add(exifKey, value.get());
                rc = 0;
            }
        }
        catch(const Exiv2::Error&) {
        }
    }

    return rc;
}

// Always creates a new metadata entry.
// Passing invalidTypeId causes the type to be guessed.
// Guessing types is accurate for IPTC, but not for EXIF.
EXIVSIMPLE_API int AddMeta(HIMAGE img, const char *key, const char *val, DllTypeId type)
{
    assert(img && key && val);
    if (img==0 || key==0 || val==0) return -1;
    ImageWrapper *imgWrap = (ImageWrapper*)img;
    int rc = 2;

    Exiv2::IptcData &iptcData = imgWrap->image->iptcData();
    Exiv2::ExifData &exifData = imgWrap->image->exifData();

    std::string data(val);
    // if data starts and ends with quotes, remove them
    if (data.at(0) == '\"' && data.at(data.size()-1) == '\"') {
        data = data.substr(1, data.size()-2);
    }

    try {
        Exiv2::IptcKey iptcKey(key);
        rc = 1;

        if (type == invalidTypeId)
            type = (DllTypeId)Exiv2::IptcDataSets::dataSetType(iptcKey.tag(), iptcKey.record());
        Exiv2::Value::AutoPtr value = Exiv2::Value::create((Exiv2::TypeId)type);
        value->read(data);

        rc = iptcData.add(iptcKey, value.get());
    } 
    catch(const Exiv2::Error&) {
    }

    if (rc) {
        // Failed with iptc, so try exif
        try {
            Exiv2::ExifKey exifKey(key);
            rc = 1;

            // No way to get value type for exif... string is the most common
            if (type == invalidTypeId)
                type = asciiString;
            Exiv2::Value::AutoPtr value = Exiv2::Value::create((Exiv2::TypeId)type);
            value->read(data);

            exifData.add(exifKey, value.get());
            rc = 0;
        }
        catch(const Exiv2::Error&) {
        }
    }

    return rc;
}

// If multiple entires exist, this only remove the first one
// found. Call multiple times to remove many.
EXIVSIMPLE_API int RemoveMeta(HIMAGE img, const char *key)
{
    assert(img && key);
    if (img==0 || key==0) return -1;
    ImageWrapper *imgWrap = (ImageWrapper*)img;
    int rc = 2;

    Exiv2::IptcData &iptcData = imgWrap->image->iptcData();
    Exiv2::ExifData &exifData = imgWrap->image->exifData();

    try {
        Exiv2::IptcKey iptcKey(key);
        rc = 1;
        Exiv2::IptcData::iterator iter = iptcData.findKey(iptcKey);
        if (iter != iptcData.end()) {
            iptcData.erase(iter);
            rc = 0;
        }
    } 
    catch(const Exiv2::Error&) {
    }

    if (rc) {
        // No iptc value, so try exif
        try {
            Exiv2::ExifKey exifKey(key);
            rc = 1;
            Exiv2::ExifData::iterator iter = exifData.findKey(exifKey);
            if (iter != exifData.end()) {
                exifData.erase(iter);
                rc = 0;
            }
        }
        catch(const Exiv2::Error&) {
        }
    }

    return rc;
}

EXIVSIMPLE_API int EnumMeta(HIMAGE img, METAENUMPROC proc, void *user)
{
    assert(img && proc);
    if (img==0 || proc==0) return -1;
    ImageWrapper *imgWrap = (ImageWrapper*)img;
    bool more = true;

    Exiv2::IptcData &iptcData = imgWrap->image->iptcData();
    Exiv2::ExifData &exifData = imgWrap->image->exifData();

    Exiv2::IptcData::const_iterator iend = iptcData.end();
    for (Exiv2::IptcData::const_iterator i = iptcData.begin(); 
            i != iend && more; ++i) {
        more = proc(i->key().c_str(), i->value().toString().c_str(), user);
    }

    Exiv2::ExifData::const_iterator eend = exifData.end();
    for (Exiv2::ExifData::const_iterator e = exifData.begin();
            e != eend && more; ++e) {
        more = proc(e->key().c_str(), e->value().toString().c_str(), user);
    }

    return 0;
}


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

