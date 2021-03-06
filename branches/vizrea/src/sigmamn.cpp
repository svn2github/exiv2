// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005 Andreas Huggel <ahuggel@gmx.net>
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
  File:      sigmamn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   02-Apr-04, ahu: created
  Credits:   Sigma and Foveon MakerNote implemented according to the specification
             in "SIGMA and FOVEON EXIF MakerNote Documentation" by Foveon.
             <http://www.x3f.info/technotes/FileDocs/MakerNoteDoc.html>
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#include "types.hpp"
#include "sigmamn.hpp"
#include "makernote.hpp"
#include "value.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>

// Define DEBUG_MAKERNOTE to output debug information to std::cerr
#undef DEBUG_MAKERNOTE

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! @cond IGNORE
    SigmaMakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("SIGMA", "*", createSigmaMakerNote); 
        MakerNoteFactory::registerMakerNote("FOVEON", "*", createSigmaMakerNote);
        MakerNoteFactory::registerMakerNote(
            sigmaIfdId, MakerNote::AutoPtr(new SigmaMakerNote));

        ExifTags::registerMakerTagInfo(sigmaIfdId, tagInfo_);
    }
    //! @endcond

    // Sigma (Foveon) MakerNote Tag Info
    const TagInfo SigmaMakerNote::tagInfo_[] = {
        TagInfo(0x0002, "SerialNumber", "Camera serial number", sigmaIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0003, "DriveMode", "Drive Mode", sigmaIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0004, "ResolutionMode", "Resolution Mode", sigmaIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0005, "AutofocusMode", "Autofocus mode", sigmaIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0006, "FocusSetting", "Focus setting", sigmaIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "WhiteBalance", "White balance", sigmaIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0008, "ExposureMode", "Exposure mode", sigmaIfdId, makerTags, asciiString, print0x0008),
        TagInfo(0x0009, "MeteringMode", "Metering mode", sigmaIfdId, makerTags, asciiString, print0x0009),
        TagInfo(0x000a, "LensRange", "Lens focal length range", sigmaIfdId, makerTags, asciiString, printValue),
        TagInfo(0x000b, "ColorSpace", "Color space", sigmaIfdId, makerTags, asciiString, printValue),
        TagInfo(0x000c, "Exposure", "Exposure", sigmaIfdId, makerTags, asciiString, printStripLabel),
        TagInfo(0x000d, "Contrast", "Contrast", sigmaIfdId, makerTags, asciiString, printStripLabel),
        TagInfo(0x000e, "Shadow", "Shadow", sigmaIfdId, makerTags, asciiString, printStripLabel),
        TagInfo(0x000f, "Highlight", "Highlight", sigmaIfdId, makerTags, asciiString, printStripLabel),
        TagInfo(0x0010, "Saturation", "Saturation", sigmaIfdId, makerTags, asciiString, printStripLabel),
        TagInfo(0x0011, "Sharpness", "Sharpness", sigmaIfdId, makerTags, asciiString, printStripLabel),
        TagInfo(0x0012, "FillLight", "X3 Fill light", sigmaIfdId, makerTags, asciiString, printStripLabel),
        TagInfo(0x0014, "ColorAdjustment", "Color adjustment", sigmaIfdId, makerTags, asciiString, printStripLabel),
        TagInfo(0x0015, "AdjustmentMode", "Adjustment mode", sigmaIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0016, "Quality", "Quality", sigmaIfdId, makerTags, asciiString, printStripLabel),
        TagInfo(0x0017, "Firmware", "Firmware", sigmaIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0018, "Software", "Software", sigmaIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0019, "AutoBracket", "Auto bracket", sigmaIfdId, makerTags, asciiString, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownSigmaMakerNoteTag)", "Unknown SigmaMakerNote tag", sigmaIfdId, makerTags, invalidTypeId, printValue)
    };

    SigmaMakerNote::SigmaMakerNote(bool alloc)
        : IfdMakerNote(sigmaIfdId, alloc)
    {
        byte buf[] = {
            'S', 'I', 'G', 'M', 'A', '\0', '\0', '\0', 0x01, 0x00
        };
        readHeader(buf, 10, byteOrder_);
    }

    SigmaMakerNote::SigmaMakerNote(const SigmaMakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    int SigmaMakerNote::readHeader(const byte* buf,
                                   long len, 
                                   ByteOrder byteOrder)
    {
        if (len < 10) return 1;

        // Copy the header. My one and only Sigma sample has two undocumented
        // extra bytes (0x01, 0x00) between the ID string and the start of the
        // Makernote IFD. So we copy 10 bytes into the header.
        header_.alloc(10);
        memcpy(header_.pData_, buf, header_.size_);
        // Adjust the offset of the IFD for the prefix
        adjOffset_ = 10;
        return 0;
    }

    int SigmaMakerNote::checkHeader() const
    {
        int rc = 0;
        // Check the SIGMA or FOVEON prefix
        if (   header_.size_ < 10
            || std::string(reinterpret_cast<char*>(header_.pData_), 8) 
                        != std::string("SIGMA\0\0\0", 8)
            && std::string(reinterpret_cast<char*>(header_.pData_), 8) 
                        != std::string("FOVEON\0\0", 8)) {
            rc = 2;
        }
        return rc;
    }

    SigmaMakerNote::AutoPtr SigmaMakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    SigmaMakerNote* SigmaMakerNote::create_(bool alloc) const
    {
        AutoPtr makerNote = AutoPtr(new SigmaMakerNote(alloc));
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    SigmaMakerNote::AutoPtr SigmaMakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    SigmaMakerNote* SigmaMakerNote::clone_() const
    {
        return new SigmaMakerNote(*this);
    }

    std::ostream& SigmaMakerNote::printStripLabel(std::ostream& os,
                                                  const Value& value)
    {
        std::string v = value.toString();
        std::string::size_type pos = v.find(':');
        if (pos != std::string::npos) {
            if (v[pos + 1] == ' ') ++pos;
            v = v.substr(pos + 1);
        }
        return os << v;
    }

    std::ostream& SigmaMakerNote::print0x0008(std::ostream& os,
                                              const Value& value)
    {
        switch (value.toString()[0]) {
        case 'P': os << "Program"; break;
        case 'A': os << "Aperture priority"; break;
        case 'S': os << "Shutter priority"; break;
        case 'M': os << "Manual"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& SigmaMakerNote::print0x0009(std::ostream& os,
                                              const Value& value)
    {
        switch (value.toString()[0]) {
        case 'A': os << "Average"; break;
        case 'C': os << "Center"; break;
        case '8': os << "8-Segment"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createSigmaMakerNote(bool alloc,
                                            const byte* buf, 
                                            long len, 
                                            ByteOrder byteOrder, 
                                            long offset)
    {
        return MakerNote::AutoPtr(new SigmaMakerNote(alloc));
    }

}                                       // namespace Exiv2
