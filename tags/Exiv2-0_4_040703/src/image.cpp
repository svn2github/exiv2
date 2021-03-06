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
  Version:   $Name:  $ $Revision: 1.17 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.17 $ $RCSfile: image.cpp,v $")

// *****************************************************************************
// included header files
#include "image.hpp"
#include "types.hpp"

// + standard includes
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>                               // for rename, remove
#ifdef _MSC_VER
#include <process.h>
typedef int pid_t;
#else
#include <sys/types.h>                          // for getpid
#include <unistd.h>                             // for getpid
#endif

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    ImageFactory* ImageFactory::pInstance_ = 0;

    ImageFactory& ImageFactory::instance()
    {
        if (0 == pInstance_) {
            pInstance_ = new ImageFactory;
        }
        return *pInstance_;
    } // ImageFactory::instance

    void ImageFactory::registerImage(Image::Type type, Image* pImage)
    {
        Registry::iterator i = registry_.find(type);
        if (i != registry_.end()) {
            delete i->second;
        }
        registry_[type] = pImage;
    } // ImageFactory::registerImage

    ImageFactory::ImageFactory()
    {
        // Register a prototype of each known image
        registerImage(Image::none, 0);
        registerImage(Image::jpeg, new JpegImage);
    } // ImageFactory c'tor

    Image* ImageFactory::create(std::istream& is) const
    {
        Registry::const_iterator b = registry_.begin();
        Registry::const_iterator e = registry_.end();
        for (Registry::const_iterator i = b; i != e; ++i)
        {
            if (i->second != 0 && i->second->isThisType(is)) {
                Image* pImage = i->second;
                return pImage->clone();
            }
        }
        return 0;
    } // ImageFactory::create

    Image* ImageFactory::create(Image::Type type) const
    {
        Registry::const_iterator i = registry_.find(type);
        if (i != registry_.end() && i->second != 0) {
            Image* pImage = i->second;
            return pImage->clone();
        }
        return 0;
    } // ImageFactory::create

    JpegImage::JpegImage()
        : sizeExifData_(0), pExifData_(0)
    {
    }

    JpegImage::JpegImage(const JpegImage& rhs)
        : Image(rhs), sizeExifData_(0), pExifData_(0)
    {
        char* newExifData = 0;
        if (rhs.sizeExifData_ > 0) {
            char* newExifData = new char[rhs.sizeExifData_];
            memcpy(newExifData, rhs.pExifData_, rhs.sizeExifData_);
        }
        pExifData_ = newExifData;
        sizeExifData_ = rhs.sizeExifData_;
    }

    JpegImage& JpegImage::operator=(const JpegImage& rhs)
    {
        if (this == &rhs) return *this;
        char* newExifData = 0;
        if (rhs.sizeExifData_ > 0) {
            char* newExifData = new char[rhs.sizeExifData_];
            memcpy(newExifData, rhs.pExifData_, rhs.sizeExifData_);
        }
        Image::operator=(rhs);
        pExifData_ = newExifData;
        sizeExifData_ = rhs.sizeExifData_;
        return *this;        
    }

    JpegImage::~JpegImage()
    {
        delete[] pExifData_;
    }

    const uint16 JpegImage::soi_    = 0xffd8;
    const uint16 JpegImage::app0_   = 0xffe0;
    const uint16 JpegImage::app1_   = 0xffe1;
    const char JpegImage::exifId_[] = "Exif\0\0";
    const char JpegImage::jfifId_[] = "JFIF\0";

    int JpegImage::readExifData(const std::string& path)
    {
        std::ifstream file(path.c_str(), std::ios::binary);
        if (!file) return -1;
        return readExifData(file);
    }

    // Todo: implement this properly: skip unknown APP0 and APP1 segments
    int JpegImage::readExifData(std::istream& is)
    {
        // Check if this is a JPEG image in the first place
        if (!isThisType(is, true)) {
            if (!is.good()) return 1;
            return 2;
        }

        // Read and check section marker and size
        char tmpbuf[10];
        is.read(tmpbuf, 10);
        if (!is.good()) return 1;
        uint16 marker = getUShort(tmpbuf, bigEndian);
        uint16 size = getUShort(tmpbuf + 2, bigEndian);
        if (size < 8) return 3;
        if (marker == app0_ && memcmp(tmpbuf + 4, jfifId_, 5) == 0) {
            // Skip the remainder of the JFIF APP0 segment
            is.seekg(size - 8, std::ios::cur);
            // Read the beginning of the next segment
            is.read(tmpbuf, 10);
            if (!is.good()) return 1;
            marker = getUShort(tmpbuf, bigEndian);
            size = getUShort(tmpbuf + 2, bigEndian);
        }
        if (!(marker == app1_ && memcmp(tmpbuf + 4, exifId_, 6) == 0)) return 3;

        // Read the rest of the APP1 field (Exif data)
        long sizeExifData = size - 8;
        pExifData_ = new char[sizeExifData];
        is.read(pExifData_, sizeExifData);
        if (!is.good()) {
            delete[] pExifData_;
            pExifData_ = 0;
            return 1;
        }
        // Finally, set the size and offset of the Exif data buffer
        sizeExifData_ = sizeExifData;

        return 0;
    } // JpegImage::readExifData

    int JpegImage::eraseExifData(const std::string& path) const
    {
        // Open input file
        std::ifstream is(path.c_str(), std::ios::binary);
        if (!is) return -1;

        // Write the output to a temporary file
        pid_t pid = getpid();
        std::string tmpname = path + toString(pid);
        std::ofstream os(tmpname.c_str(), std::ios::binary);
        if (!os) return -3;

        int rc = eraseExifData(os, is);
        os.close();
        is.close();
        if (rc == 0) {
            // Workaround for MinGW rename that does not overwrite existing files
            if (remove(path.c_str()) != 0) rc = -4;
        }
        if (rc == 0) {
            // rename temporary file
            if (rename(tmpname.c_str(), path.c_str()) == -1) rc = -4;
        }
        if (rc != 0) {
            // remove temporary file
            remove(tmpname.c_str());
        }

        return rc;
    } // JpegImage::eraseExifData

    // Todo: implement this properly: skip unknown APP0 and APP1 segments
    int JpegImage::eraseExifData(std::ostream& os, std::istream& is) const
    {
        // Check if this is a JPEG image in the first place
        if (!isThisType(is, true)) {
            if (!is.good()) return 1;
            return 2;
        }
        // Read and check section marker and size
        char tmpbuf[11];
        is.read(tmpbuf, 10);
        if (!is.good()) return 1;
        uint16 marker = getUShort(tmpbuf, bigEndian);
        uint16 size = getUShort(tmpbuf + 2, bigEndian);
        if (size < 8) return 3;

        const long defaultJfifSize = 9;
        static const char defaultJfifData[] = 
            { 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00 };
        DataBuf jfif;

        if (marker == app0_ && memcmp(tmpbuf + 4, jfifId_, 5) == 0) {
            // Read the remainder of the JFIF APP0 segment
            is.seekg(-1, std::ios::cur);
            jfif.alloc(size - 7);
            is.read(jfif.pData_, jfif.size_);
            if (!is.good()) return 1;
            // Read the beginning of the next segment
            is.read(tmpbuf, 10);
            if (!is.good()) return 1;
            marker = getUShort(tmpbuf, bigEndian);
            size = getUShort(tmpbuf + 2, bigEndian);
        }
        // No Exif data found
        if (!(marker == app1_ && memcmp(tmpbuf + 4, exifId_, 6) == 0)) return 3;
        // Skip the rest of the Exif APP1 segment
        is.seekg(size - 8, std::ios::cur);
        if (!is.good()) return 1;

        // Write SOI and APP0 markers, size of APP0 field
        us2Data(tmpbuf, soi_, bigEndian);
        us2Data(tmpbuf + 2, app0_, bigEndian);
        if (jfif.pData_) {
            us2Data(tmpbuf + 4, static_cast<uint16>(7 + jfif.size_), bigEndian);
        }
        else {
            us2Data(tmpbuf + 4, static_cast<uint16>(7 + defaultJfifSize), bigEndian);
        }
        memcpy(tmpbuf + 6, jfifId_, 5);
        os.write(tmpbuf, 11);
        // Write JFIF APP0 data, use that from the input stream if available
        if (jfif.pData_) {
            os.write(jfif.pData_, jfif.size_);
        }
        else {
            os.write(defaultJfifData, defaultJfifSize);
        }
        if (!os.good()) return 4;

        // Copy the rest of the input stream
        os.flush();
        is >> os.rdbuf();
        if (!os.good()) return 4;

        return 0;
    } // JpegImage::eraseExifData

    int JpegImage::writeExifData(const std::string& path) const
    {
        // Open the input file
        std::ifstream is(path.c_str(), std::ios::binary);
        if (!is) return -1;

        // Write the output to a temporary file
        pid_t pid = getpid();
        std::string tmpname = path + toString(pid);
        std::ofstream os(tmpname.c_str(), std::ios::binary);
        if (!os) return -3;

        int rc = writeExifData(os, is);
        os.close();
        is.close();
        if (rc == 0) {
            // Workaround for MinGW rename that does not overwrite existing files
            if (remove(path.c_str()) != 0) rc = -4;
        }
        if (rc == 0) {
            // rename temporary file
            if (rename(tmpname.c_str(), path.c_str()) == -1) rc = -4;
        }
        if (rc != 0) {
            // remove temporary file
            remove(tmpname.c_str());
        }

        return rc;
    } // JpegImage::writeExifData

    // Todo: implement this properly: skip unknown APP0 and APP1 segments
    int JpegImage::writeExifData(std::ostream& os, std::istream& is) const
    {
        // Check if this is a JPEG image in the first place
        if (!isThisType(is, true)) {
            if (!is.good()) return 1;
            return 2;
        }

        // Read and check section marker and size
        char tmpbuf[10];
        is.read(tmpbuf, 10);
        if (!is.good()) return 1;
        uint16 marker = getUShort(tmpbuf, bigEndian);
        uint16 size = getUShort(tmpbuf + 2, bigEndian);
        if (size < 8) return 3;

        bool validFile = false;
        DataBuf jfif;
        if (marker == app0_ && memcmp(tmpbuf + 4, jfifId_, 5) == 0) {
            // Read the remainder of the JFIF APP0 segment
            is.seekg(-1, std::ios::cur);
            jfif.alloc(size -7);
            is.read(jfif.pData_, jfif.size_);
            if (!is.good()) return 1;
            // Read the beginning of the next segment
            is.read(tmpbuf, 10);
            if (!is.good()) return 1;
            marker = getUShort(tmpbuf, bigEndian);
            size = getUShort(tmpbuf + 2, bigEndian);
            validFile = true;
        }
        if (marker == app1_ && memcmp(tmpbuf + 4, exifId_, 6) == 0) {
            // Skip the rest of the Exif APP1 segment
            is.seekg(size - 8, std::ios::cur);
            if (!is.good()) return 1;
            validFile = true;
        }
        else {
            is.seekg(-10, std::ios::cur);
        }
        if (!validFile) return 5;
        // Write SOI marker
        us2Data(tmpbuf, soi_, bigEndian);
        os.write(tmpbuf, 2);
        if (!os.good()) return 4;
        if (jfif.pData_) {
            // Write APP0 marker, size of APP0 field and JFIF data
            us2Data(tmpbuf, app0_, bigEndian);
            us2Data(tmpbuf + 2, static_cast<uint16>(7 + jfif.size_), bigEndian);
            memcpy(tmpbuf + 4, jfifId_, 5);
            os.write(tmpbuf, 9);
            os.write(jfif.pData_, jfif.size_);
            if (!os.good()) {
                return 4;
            }
        }
        // Write APP1 marker, size of APP1 field, Exif id and Exif data
        us2Data(tmpbuf, app1_, bigEndian);
        us2Data(tmpbuf + 2, static_cast<uint16>(sizeExifData_ + 8), bigEndian);
        memcpy(tmpbuf + 4, exifId_, 6);
        os.write(tmpbuf, 10);
        os.write(pExifData_, sizeExifData_);
        if (!os.good()) return 4;
        // Copy rest of the stream
        os.flush();
        is >> os.rdbuf();
        if (!os.good()) return 4;

        return 0;
    } // JpegImage::writeExifData

    void JpegImage::setExifData(const char* buf, long size)
    {
        sizeExifData_ = size;
        delete[] pExifData_;
        pExifData_ = new char[size];
        memcpy(pExifData_, buf, size);
    }

    Image* JpegImage::clone() const
    {
        return new JpegImage(*this);
    }

    bool JpegImage::isThisType(std::istream& is, bool advance) const
    {
        char c;
        is.get(c);
        if (!is.good()) return false;
        if (static_cast<char>((soi_ & 0xff00) >> 8) != c) {
            is.unget();
            return false;
        }
        is.get(c);
        if (!is.good()) return false;
        if (static_cast<char>(soi_ & 0x00ff) != c) {
            is.seekg(-2, std::ios::cur);
            return false;
        }
        if (advance == false) is.seekg(-2, std::ios::cur);
        return true;
    }

    TiffHeader::TiffHeader(ByteOrder byteOrder) 
        : byteOrder_(byteOrder), tag_(0x002a), offset_(0x00000008)
    {
    }

    int TiffHeader::read(const char* buf)
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

    long TiffHeader::copy(char* buf) const
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

    const uint16 ExvFile::app1_   = 0xffe1;
    const char ExvFile::exifId_[] = "Exif\0\0";

    ExvFile::ExvFile()
        : sizeExifData_(0), pExifData_(0)
    {
    } // ExvFile default constructor

    ExvFile::ExvFile(const ExvFile& rhs)
        : sizeExifData_(0), pExifData_(0)
    {
        char* newExifData = 0;
        if (rhs.sizeExifData_ > 0) {
            char* newExifData = new char[rhs.sizeExifData_];
            memcpy(newExifData, rhs.pExifData_, rhs.sizeExifData_);
        }
        pExifData_ = newExifData;
        sizeExifData_ = rhs.sizeExifData_;
    } // ExvFile copy constructor

    ExvFile& ExvFile::operator=(const ExvFile& rhs)
    {
        if (this == &rhs) return *this;
        char* newExifData = 0;
        if (rhs.sizeExifData_ > 0) {
            char* newExifData = new char[rhs.sizeExifData_];
            memcpy(newExifData, rhs.pExifData_, rhs.sizeExifData_);
        }
        pExifData_ = newExifData;
        sizeExifData_ = rhs.sizeExifData_;
        return *this;
    } // ExvFile::operator=

    ExvFile::~ExvFile()
    {
        delete[] pExifData_;
    } // ExvFile destructor

    int ExvFile::readExifData(const std::string& path)
    {
        std::ifstream file(path.c_str(), std::ios::binary);
        if (!file) return -1;
        return readExifData(file);
    } // ExvFile::readExifData

    int ExvFile::readExifData(std::istream& is)
    {
        // Check if this is an Exiv2 file in the first place
        if (!isThisType(is, false)) {
            if (!is.good()) return 1;
            return 2;
        }
        // Read the first ten bytes and extract the size
        char tmpbuf[10];
        is.read(tmpbuf, 10);
        if (!is.good()) return 1;
        uint16 size = getUShort(tmpbuf + 2, bigEndian);
        if (size < 8) return 3;

        // Read the rest of the APP1 field (Exif data)
        long sizeExifData = size - 8;
        char* pExifData = new char[sizeExifData];
        is.read(pExifData, sizeExifData);
        if (!is.good()) {
            delete[] pExifData;
            return 1;
        }
        sizeExifData_ = sizeExifData;
        pExifData_ = pExifData;

        return 0;
    } // ExvFile::readExifData

    void ExvFile::setExifData(const char* buf, long size)
    {
        sizeExifData_ = size;
        delete[] pExifData_;
        pExifData_ = new char[size];
        memcpy(pExifData_, buf, size);
    } // ExvFile::setExifData

    bool ExvFile::isThisType(std::istream& is, bool advance)
    {
        // Read and check section marker and size
        char tmpbuf[10];
        is.read(tmpbuf, 10);
        if (!is.good()) return false;
        bool rc = true;
        uint16 marker = getUShort(tmpbuf, bigEndian);
        uint16 size = getUShort(tmpbuf + 2, bigEndian);
        if (size < 8) rc = false;
        if (!(marker == app1_ && memcmp(tmpbuf + 4, exifId_, 6) == 0)) {
            rc = false;
        }
        if (!advance) is.seekg(-10, std::ios::cur);
        return rc;
    } // ExvFile::isThisType

    int ExvFile::writeExifData(const std::string& path) const
    {
        std::ofstream os(path.c_str(), std::ios::binary);
        if (!os) return -1;
        // Write APP1 marker, size of APP1 field, Exif id and Exif data
        char tmpbuf[10];
        us2Data(tmpbuf, app1_, bigEndian);
        us2Data(tmpbuf + 2, static_cast<uint16>(sizeExifData_ + 8), bigEndian);
        memcpy(tmpbuf + 4, exifId_, 6);
        os.write(tmpbuf, 10);
        os.write(pExifData_, sizeExifData_);
        if (!os.good()) return 4;
        return 0;
    } // ExvFile::writeExifData

}                                       // namespace Exiv2
