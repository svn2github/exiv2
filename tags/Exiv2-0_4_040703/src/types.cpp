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
  File:      types.cpp
  Version:   $Name:  $ $Revision: 1.9 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.9 $ $RCSfile: types.cpp,v $")

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <utility>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    TypeInfoTable::TypeInfoTable(TypeId typeId, const char* name, long size)
        : typeId_(typeId), name_(name), size_(size)
    {
    }

    //! Lookup list of supported IFD type information
    const TypeInfoTable TypeInfo::typeInfoTable_[] = {
        TypeInfoTable(invalidTypeId,    "Invalid",     0),
        TypeInfoTable(unsignedByte,     "Byte",        1),
        TypeInfoTable(asciiString,      "Ascii",       1),
        TypeInfoTable(unsignedShort,    "Short",       2),
        TypeInfoTable(unsignedLong,     "Long",        4),
        TypeInfoTable(unsignedRational, "Rational",    8),
        TypeInfoTable(invalid6,         "Invalid (6)", 1),
        TypeInfoTable(undefined,        "Undefined",   1),
        TypeInfoTable(signedShort,      "SShort",      2),
        TypeInfoTable(signedLong,       "SLong",       4),
        TypeInfoTable(signedRational,   "SRational",   8)
    };

    const char* TypeInfo::typeName(TypeId typeId)
    {
        return typeInfoTable_[ typeId < lastTypeId ? typeId : 0 ].name_;
    }

    long TypeInfo::typeSize(TypeId typeId)
    {
        return typeInfoTable_[ typeId < lastTypeId ? typeId : 0 ].size_;
    }

    // *************************************************************************
    // free functions

    uint16 getUShort(const char* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return (unsigned char)buf[1] << 8 | (unsigned char)buf[0];
        }
        else {
            return (unsigned char)buf[0] << 8 | (unsigned char)buf[1];
        }
    }

    uint32 getULong(const char* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return   (unsigned char)buf[3] << 24 | (unsigned char)buf[2] << 16 
                   | (unsigned char)buf[1] <<  8 | (unsigned char)buf[0];
        }
        else {
            return   (unsigned char)buf[0] << 24 | (unsigned char)buf[1] << 16 
                   | (unsigned char)buf[2] <<  8 | (unsigned char)buf[3];
        }
    }

    URational getURational(const char* buf, ByteOrder byteOrder)
    {
        uint32 nominator = getULong(buf, byteOrder);
        uint32 denominator = getULong(buf + 4, byteOrder);
        return std::make_pair(nominator, denominator);
    }

    int16 getShort(const char* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return (unsigned char)buf[1] << 8 | (unsigned char)buf[0];
        }
        else {
            return (unsigned char)buf[0] << 8 | (unsigned char)buf[1];
        }
    }

    int32 getLong(const char* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return   (unsigned char)buf[3] << 24 | (unsigned char)buf[2] << 16 
                   | (unsigned char)buf[1] <<  8 | (unsigned char)buf[0];
        }
        else {
            return   (unsigned char)buf[0] << 24 | (unsigned char)buf[1] << 16 
                   | (unsigned char)buf[2] <<  8 | (unsigned char)buf[3];
        }
    }

    Rational getRational(const char* buf, ByteOrder byteOrder)
    {
        int32 nominator = getLong(buf, byteOrder);
        int32 denominator = getLong(buf + 4, byteOrder);
        return std::make_pair(nominator, denominator);
    }

    long us2Data(char* buf, uint16 s, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  (char)(s & 0x00ff);
            buf[1] = (char)((s & 0xff00) >> 8);
        }
        else {
            buf[0] = (char)((s & 0xff00) >> 8);
            buf[1] =  (char)(s & 0x00ff);
        }
        return 2;
    }

    long ul2Data(char* buf, uint32 l, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  (char)(l & 0x000000ff);
            buf[1] = (char)((l & 0x0000ff00) >> 8);
            buf[2] = (char)((l & 0x00ff0000) >> 16);
            buf[3] = (char)((l & 0xff000000) >> 24);
        }
        else {
            buf[0] = (char)((l & 0xff000000) >> 24);
            buf[1] = (char)((l & 0x00ff0000) >> 16);
            buf[2] = (char)((l & 0x0000ff00) >> 8);
            buf[3] =  (char)(l & 0x000000ff);
        }
        return 4;
    }

    long ur2Data(char* buf, URational l, ByteOrder byteOrder)
    {
        long o = ul2Data(buf, l.first, byteOrder);
        o += ul2Data(buf+o, l.second, byteOrder);
        return o;
    }

    long s2Data(char* buf, int16 s, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  (char)(s & 0x00ff);
            buf[1] = (char)((s & 0xff00) >> 8);
        }
        else {
            buf[0] = (char)((s & 0xff00) >> 8);
            buf[1] =  (char)(s & 0x00ff);
        }
        return 2;
    }

    long l2Data(char* buf, int32 l, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  (char)(l & 0x000000ff);
            buf[1] = (char)((l & 0x0000ff00) >> 8);
            buf[2] = (char)((l & 0x00ff0000) >> 16);
            buf[3] = (char)((l & 0xff000000) >> 24);
        }
        else {
            buf[0] = (char)((l & 0xff000000) >> 24);
            buf[1] = (char)((l & 0x00ff0000) >> 16);
            buf[2] = (char)((l & 0x0000ff00) >> 8);
            buf[3] =  (char)(l & 0x000000ff);
        }
        return 4;
    }

    long r2Data(char* buf, Rational l, ByteOrder byteOrder)
    {
        long o = l2Data(buf, l.first, byteOrder);
        o += l2Data(buf+o, l.second, byteOrder);
        return o;
    }

    void hexdump(std::ostream& os, const char* buf, long len, long offset)
    {
        const std::string::size_type pos = 8 + 16 * 3 + 2; 
        const std::string align(pos, ' '); 

        long i = 0;
        while (i < len) {
            os << "  " 
               << std::setw(4) << std::setfill('0') << std::hex 
               << i + offset << "  ";
            std::ostringstream ss;
            do {
                unsigned char c = buf[i];
                os << std::setw(2) << std::setfill('0') 
                   << std::hex << (int)c << " ";
                ss << ((int)c >= 31 && (int)c < 127 ? buf[i] : '.');
            } while (++i < len && i%16 != 0);
            std::string::size_type width = 9 + ((i-1)%16 + 1) * 3;
            os << (width > pos ? "" : align.substr(width)) << ss.str() << "\n";
        }
        os << std::dec << std::setfill(' ');
    } // hexdump

    int gcd(int a, int b)
    {
        int temp;
        if (a < b) {
            temp = a;
            a = b; 
            b = temp; 
        }
        while ((temp = a % b) != 0) {
            a = b;
            b = temp;
        }
        return b;
    } // gcd

    long lgcd(long a, long b)
    {
        long temp;
        if (a < b) {
            temp = a;
            a = b; 
            b = temp; 
        }
        while ((temp = a % b) != 0) {
            a = b;
            b = temp;
        }
        return b;
    } // lgcd

}                                       // namespace Exiv2
