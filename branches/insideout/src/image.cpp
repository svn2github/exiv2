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
  File:      image.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Brad Schick (brad) <brad@robotbattle.com>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
             19-Jul-04, brad: revamped to be more flexible and support Iptc
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#ifdef HAVE_CONFIG_H
# include <config.h>
#else
# ifdef _MSC_VER
#  include <config_win32.h>
# endif
#endif

#include "image.hpp"
#include "error.hpp"

// Ensure registration with factory
#include "jpgimage.hpp"

// + standard includes
#include <cstdio>
#include <cstring>
#include <cstdio>                               // for rename, remove
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
# define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#endif
#ifdef HAVE_PROCESS_H
# include <process.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>                            // for getpid, stat
#endif

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    ImageFactory::Registry* ImageFactory::registry_;

    void ImageFactory::init()
    {
        if (0 == registry_) {
            registry_ = new Registry;
        }
    } // ImageFactory::instance

    void ImageFactory::registerImage(Image::Type type, 
                NewInstanceFct newInst, IsThisTypeFct isType)
    {
        init();
        assert (newInst && isType);
        (*registry_)[type] = ImageFcts(newInst, isType);
    } // ImageFactory::registerImage

    Image::Type ImageFactory::getType(const std::string& path)
    {
        init();
        FileIo file(path);
        if (file.open("rb") != 0) return Image::none;

        Image::Type type = Image::none;
        Registry::const_iterator b = registry_->begin();
        Registry::const_iterator e = registry_->end();
        for (Registry::const_iterator i = b; i != e; ++i)
        {
            if (i->second.isThisType(file, false)) {
                type = i->first;
                break;
            }
        }
        return type;
    } // ImageFactory::getType

    Image::AutoPtr ImageFactory::open(const std::string& path)
    {
        init();
        Image::AutoPtr image;
        FileIo file(path);
        if (file.open("rb") != 0) return image;

        Registry::const_iterator b = registry_->begin();
        Registry::const_iterator e = registry_->end();
        for (Registry::const_iterator i = b; i != e; ++i)
        {
            if (i->second.isThisType(file, false)) {
                image = i->second.newInstance(path, false);
                break;
            }
        }
        return image;
    } // ImageFactory::open

    Image::AutoPtr ImageFactory::create(Image::Type type, 
                                        const std::string& path)
    {
        init();
        Registry::const_iterator i = registry_->find(type);
        if (i != registry_->end()) {
            return i->second.newInstance(path, true);
        }
        return Image::AutoPtr();
    } // ImageFactory::create


    TiffHeader::TiffHeader(ByteOrder byteOrder) 
        : byteOrder_(byteOrder), tag_(0x002a), offset_(0x00000008)
    {
    }

    int TiffHeader::read(const byte* buf)
    {
        if (buf[0] == 0x49 && buf[1] == 0x49) {
            byteOrder_ = littleEndian;
        }
        else if (buf[0] == 0x4d && buf[1] == 0x4d) {
            byteOrder_ = bigEndian;
        }
        else {
            return 1;
        }
        tag_ = getUShort(buf+2, byteOrder_);
        offset_ = getULong(buf+4, byteOrder_);
        return 0;
    }

    long TiffHeader::copy(byte* buf) const
    {
        switch (byteOrder_) {
        case littleEndian:
            buf[0] = 0x49;
            buf[1] = 0x49;
            break;
        case bigEndian:
            buf[0] = 0x4d;
            buf[1] = 0x4d;
            break;
        case invalidByteOrder:
            // do nothing
            break;
        }
        us2Data(buf+2, 0x002a, byteOrder_);
        ul2Data(buf+4, 0x00000008, byteOrder_);
        return size();
    } // TiffHeader::copy

// *****************************************************************************
// free functions

    bool fileExists(const std::string& path, bool ct)
    {
        struct stat buf;
        int ret = stat(path.c_str(), &buf);
        if (0 != ret)                    return false;
        if (ct && !S_ISREG(buf.st_mode)) return false;
        return true;
    } // fileExists

}                                       // namespace Exiv2
