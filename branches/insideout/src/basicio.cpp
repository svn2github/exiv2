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
  File:      basicio.cpp
  Version:   $Rev$
  Author(s): Brad Schick (brad) <brad@robotbattle.com>
  History:   04-Dec-04, brad: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// Define DEBUG_MAKERNOTE to output debug information to std::cerr
#undef DEBUG_MAKERNOTE

// *****************************************************************************
// included header files
#include "basicio.hpp"
#include "types.hpp"

// + standard includes
#include <cassert>


// *****************************************************************************
// class member definitions
namespace Exiv2 {

    FileIo::FileIo(const std::string& path) : path_(path)
    {
        fp_ = 0;
    }
        
    FileIo::~FileIo()
    {
        close();
    }

    long FileIo::write(const byte* data, long wcount )
    {
        assert(fp_ != 0);
        return (long)fwrite(data, 1, wcount, fp_);
    }

    long FileIo::write(BasicIo& src)
    {
        assert(fp_ != 0);
        if (static_cast<BasicIo*>(this)==&src) return 0;
        if (!src.isopen()) return 0;

        byte buf[4096];
        long readCount = 0;
        long writeCount = 0;
        long writeTotal = 0;
        while ((readCount = src.read(buf, sizeof(buf)))) {
            writeTotal += writeCount = (long)fwrite(buf, 1, readCount, fp_);
            if (writeCount != readCount) {
                // try to reset back to where write stopped
                src.seek(writeCount-readCount, BasicIo::cur);
                break;
            }
        }

        return writeTotal;
    }

    int FileIo::putb(byte data)
    {
        assert(fp_ != 0);
        return putc(data, fp_);
    }
    
    int FileIo::seek(long offset, BasicIo::Position pos)
    {
        assert(fp_ != 0);
        int fileSeek;
        if (pos == BasicIo::cur) {
            fileSeek = SEEK_CUR;
        }
        else if (pos == BasicIo::beg) {
            fileSeek = SEEK_SET;
        }
        else {
            assert(pos == BasicIo::end);
            fileSeek = SEEK_END;
        }
        
        return fseek(fp_, offset, fileSeek);
    }
        
    long FileIo::tell() const
    {
        assert(fp_ != 0);
        return ftell(fp_);
    }

    int FileIo::open()
    {
        // Default open is in read-write binary mode
        return open("r+b");
    }

    int FileIo::open(const std::string& mode)
    {
        if (fp_ != 0) {
            fclose(fp_);
        }

        fp_ = fopen(path_.c_str(), mode.c_str());
        if (!fp_) return 1;
        return 0;
    }

    bool FileIo::isopen() const
    {
        return fp_ != 0;
    }
    
    int FileIo::close()
    {
        if (fp_ != 0) fclose(fp_);
        fp_= 0;
        return 0;
    }
        
    DataBuf FileIo::read(long rcount)
    {
        assert(fp_ != 0);
        DataBuf buf(rcount);
        long readCount = read(buf.pData_, buf.size_);
        buf.size_ = readCount;
        return buf;
    }

    long FileIo::read(byte* buf, long rcount)
    {
        assert(fp_ != 0);
        return (long)fread(buf, 1, rcount, fp_);
    }

    int FileIo::getb()
    {
        assert(fp_ != 0);
        return getc(fp_);
    }

    int FileIo::error() const
    {
        assert(fp_ != 0);
        return ferror(fp_);
    }
    
    bool FileIo::eof() const
    {
        assert(fp_ != 0);
        return feof(fp_) != 0;
    }


    MemIo::MemIo(const byte* data, long size)
    {
        // If copying data is too slow it might be worth
        // creating a readonly memio variant
        data_.reserve(size);
        data_.assign(data, data+size);
        idx_ = 0;
    }

    MemIo::MemIo(const ByteVector& data)
    {
        // If copying data is too slow it might be worth
        // creating a readonly memio variant
        data_.reserve(data.size());
        data_.assign(data.begin(), data.end());
        idx_ = 0;
    }
        
    void MemIo::CheckSize(long wcount)
    {
        ByteVector::size_type need = wcount + idx_;
        if (need > data_.size()) {
            data_.resize(need);
        }
    }

    long MemIo::write(const byte* data, long wcount )
    {
        CheckSize(wcount);
        // According to Josuttis 6.2.3 this is safe
        memcpy(&data_[idx_], data, wcount);
        idx_ += wcount;
        return wcount;
    }

    long MemIo::write(BasicIo& src)
    {
        if (static_cast<BasicIo*>(this)==&src) return 0;
        if (!src.isopen()) return 0;
        
        byte buf[4096];
        long readCount = 0;
        long writeTotal = 0;
        while ((readCount = src.read(buf, sizeof(buf)))) {
            write(buf, readCount);
            writeTotal += readCount;
        }

        return writeTotal;
    }

    int MemIo::putb(byte data)
    {
        CheckSize(1);
        data_[idx_++] = data;
        return data;
    }
    
    int MemIo::seek(long offset, BasicIo::Position pos)
    {
        ByteVector::size_type newIdx;
        
        if (pos == BasicIo::cur ) {
            newIdx = idx_ + offset;
        }
        else if (pos == BasicIo::beg) {
            newIdx = offset;
        }
        else {
            assert(pos == BasicIo::end);
            newIdx = data_.size() + offset;
        }

        if (newIdx < 0 || newIdx > data_.size()) return 1;
        idx_ = newIdx;
        return 0;
    }

    long MemIo::tell() const
    {
        return (long)idx_;
    }
    
    int MemIo::open()
    {
        return 0;
    }

    bool MemIo::isopen() const
    {
        return true;
    }
    
    int MemIo::close()
    {
        return 0;
    }
        
    DataBuf MemIo::read(long rcount)
    {
        DataBuf buf(rcount);
        long readCount = read(buf.pData_, buf.size_);
        buf.size_ = readCount;
        return buf;
    }

    long MemIo::read(byte* buf, long rcount)
    {
        long avail = (long)(data_.size() - idx_);
        long allow = std::min(rcount, avail);
        
        // According to Josuttis 6.2.3 this is safe
        memcpy(buf, &data_[idx_], allow);
        idx_ += allow;
        return allow;
    }

    int MemIo::getb()
    {
        if (idx_ == data_.size())
            return EOF;
        return data_[idx_++];
    }

    int MemIo::error() const
    {
        return 0;
    }
    
    bool MemIo::eof() const
    {
        return idx_ == data_.size();
    }

}                                       // namespace Exiv2
