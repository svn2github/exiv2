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
/*!
  @file    basicio.hpp
  @brief   Simple IO abstraction
  @version $Rev$
  @author  Brad Schick (brad) 
           <a href="mailto:brad@robotbattle.com">brad@robotbattle.com</a>
  @date    04-Dec-04, brad: created
 */
#ifndef BASICIO_HPP_
#define BASICIO_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "error.hpp"

// + standard includes
#include <string>
#include <vector>
#include <cstdio>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    /*!
      @brief Interface for simple byte* based IO.
     */
    class BasicIo
    {
    public:
        typedef std::auto_ptr<BasicIo> AutoPtr;
        enum Position { beg, cur, end };
        //! @name Creators
        //@{
        //! Destructor
        virtual ~BasicIo() {}
        //@}

        //! @name Manipulators
        //@{
        virtual long write(const byte* data, long wcount) = 0;
        virtual long write(BasicIo& src) = 0;
        virtual int transfer(BasicIo& src) = 0;
        virtual int putb(byte data) = 0;
        virtual int seek(long offset, Position pos) = 0;
        virtual int open() = 0;
        virtual int close() = 0;
        //@}

        //! @name Accessors
        //@{
        virtual DataBuf read(long rcount) = 0;
        virtual long read(byte *buf, long rcount) = 0;
        virtual int getb() = 0;
        virtual long tell() const = 0;
        virtual bool isopen() const = 0;
        virtual int error() const = 0;
        virtual bool eof() const = 0;
        virtual BasicIo::AutoPtr temporary() const = 0;
        //@}

    protected:
        //! @name Creators
        //@{
        //! Default Constructor
        BasicIo() {}
        //@}
    }; // class BasicIo

    /*!
      @brief Utility class that closes a BasicIo instance upon destruction.
            Its primary use is to be a stack variable in functions that need to
            ensure BasicIo instances get closed. Useful when functions return
            errors from many locations.
     */
    class IoCloser {
    public:
        //! @name Creators
        //@{
        //! Constructor, takes a file stream pointer 
        IoCloser(BasicIo &bio) : bio_(bio) {}
        //! Destructor, closes the file
        ~IoCloser() { close(); }
        //@}

        //! @name Manipulators
        //@{
        //! Close the file
        void close() { if (bio_.isopen()) bio_.close(); }
        //@}

        // DATA
        //! The BasicIo pointer
        BasicIo &bio_; 
    private:
        // Not implemented
        //! Copy constructor
        IoCloser(const IoCloser&);
        //! Assignment operator
        IoCloser& operator=(const IoCloser&);
    }; // class IoCloser


    class FileIo : public BasicIo
    {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructors
         */
        FileIo(const std::string& path);
        //! Destructor
        virtual ~FileIo();
        //@}

        //! @name Manipulators
        //@{
        virtual long write(const byte* data, long wcount);
        virtual long write(BasicIo& src);
        virtual int transfer(BasicIo& src);
        virtual int putb(byte data);
        virtual int seek(long offset, BasicIo::Position pos);
        virtual int open();
        int open( const std::string& mode);
        virtual int close();
        //@}

        //! @name Accessors
        //@{
        virtual DataBuf read(long rcount);
        virtual long read(byte *buf, long rcount);
        virtual int getb();
        virtual long tell() const;
        virtual bool isopen() const;
        virtual int error() const;
        virtual bool eof() const;
        virtual BasicIo::AutoPtr temporary() const;
        //@}
        
    private:
        // NOT IMPLEMENTED
        //! Copy constructor
        FileIo(FileIo& rhs);
        //! Assignment operator
        FileIo& operator=(const FileIo& rhs);

        // Enumeration
        enum OpMode { opRead, opWrite, opSeek };

        // DATA
        std::string path_;
        std::string openMode_;
        FILE *fp_;
        OpMode opMode_;
    }; // class FileIo

    class MemIo : public BasicIo
    {
    public:
        typedef std::vector<byte> ByteVector;
        //! @name Creators
        //@{
        /*!
          @brief Constructors
         */
        MemIo() { idx_ = 0; }
        MemIo(const byte* data, long size);
        MemIo(const ByteVector& data);
        //! Destructor
        virtual ~MemIo() {}
        //@}

        //! @name Manipulators
        //@{
        virtual long write(const byte* data, long wcount);
        virtual long write(BasicIo& src);
        virtual int transfer(BasicIo& src);
        virtual int putb(byte data);
        virtual int seek(long offset, BasicIo::Position pos);
        virtual int open();
        int open( const std::string& mode);
        virtual int close();
        //@}

        //! @name Accessors
        //@{
        virtual DataBuf read(long rcount);
        virtual long read(byte *buf, long rcount);
        virtual int getb();
        virtual long tell() const;
        virtual bool isopen() const;
        virtual int error() const;
        virtual bool eof() const;
        virtual BasicIo::AutoPtr temporary() const;
        //@}
    private:
        // NOT IMPLEMENTED
        //! Copy constructor
        MemIo(MemIo& rhs);
        //! Assignment operator
        MemIo& operator=(const MemIo& rhs);
        
        // DATA
        ByteVector data_;
        ByteVector::size_type idx_;

        //METHODS
        void checkSize(long wcount);
    }; // class MemIo    
}                                       // namespace Exiv2

#endif                                  // #ifndef BASICIO_HPP_
