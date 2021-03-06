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
  File:      nikonmn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   17-May-04, ahu: created
             25-May-04, ahu: combined all Nikon formats in one component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#include "types.hpp"
#include "nikonmn.hpp"
#include "makernote.hpp"
#include "value.hpp"
#include "image.hpp"

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

    const Nikon1MakerNote::RegisterMakerNote Nikon1MakerNote::register_;

    // Nikon1 MakerNote Tag Info
    static const MakerNote::MnTagInfo nikon1MnTagInfo[] = {
        MakerNote::MnTagInfo(0x0001, "Version", "Nikon Makernote version"),
        MakerNote::MnTagInfo(0x0002, "ISOSpeed", "ISO speed setting"),
        MakerNote::MnTagInfo(0x0003, "ColorMode", "Color mode"),
        MakerNote::MnTagInfo(0x0004, "Quality", "Image quality setting"),
        MakerNote::MnTagInfo(0x0005, "WhiteBalance", "White balance"),
        MakerNote::MnTagInfo(0x0006, "Sharpening", "Image sharpening setting"),
        MakerNote::MnTagInfo(0x0007, "Focus", "Focus mode"),
        MakerNote::MnTagInfo(0x0008, "Flash", "Flash mode"),
        MakerNote::MnTagInfo(0x000f, "ISOSelection", "ISO selection"),
        MakerNote::MnTagInfo(0x0080, "ImageAdjustment", "Image adjustment setting"),
        MakerNote::MnTagInfo(0x0082, "Adapter", "Adapter used"),
        MakerNote::MnTagInfo(0x0085, "FocusDistance", "Manual focus distance"),
        MakerNote::MnTagInfo(0x0086, "DigitalZoom", "Digital zoom setting"),
        MakerNote::MnTagInfo(0x0088, "AFFocusPos", "AF focus position"),
        // End of list marker
        MakerNote::MnTagInfo(0xffff, "(UnknownNikon1MnTag)", "Unknown Nikon1MakerNote tag")
    };

    Nikon1MakerNote::Nikon1MakerNote(bool alloc)
        : IfdMakerNote(nikon1MnTagInfo, alloc), ifdItem_("Nikon1")
    {
    }

    Nikon1MakerNote::AutoPtr Nikon1MakerNote::clone(bool alloc) const
    {
        return AutoPtr(clone_(alloc));
    }

    Nikon1MakerNote* Nikon1MakerNote::clone_(bool alloc) const 
    {
        return new Nikon1MakerNote(alloc);
    }

    std::ostream& Nikon1MakerNote::printTag(std::ostream& os, 
                                            uint16_t tag, 
                                            const Value& value) const
    {
        switch (tag) {
        case 0x0002: print0x0002(os, value); break;
        case 0x0007: print0x0007(os, value); break;
        case 0x0085: print0x0085(os, value); break;
        case 0x0086: print0x0086(os, value); break;
        case 0x0088: print0x0088(os, value); break;
        default:
            // All other tags (known or unknown) go here
            os << value;
            break;
        }
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0002(std::ostream& os,
                                               const Value& value)
    {
        if (value.count() > 1) {
            os << value.toLong(1);
        }
        else {
            os << "(" << value << ")"; 
        }
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0007(std::ostream& os,
                                               const Value& value)
    {
        std::string focus = value.toString(); 
        if      (focus == "AF-C  ") os << "Continuous autofocus";
        else if (focus == "AF-S  ") os << "Single autofocus";
        else                      os << "(" << value << ")";
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0085(std::ostream& os,
                                               const Value& value)
    {
        Rational distance = value.toRational();
        if (distance.first == 0) {
            os << "Unknown";
        }
        else if (distance.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(2)
               << (float)distance.first / distance.second
               << " m";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0086(std::ostream& os,
                                               const Value& value)
    {
        Rational zoom = value.toRational();
        if (zoom.first == 0) {
            os << "Not used";
        }
        else if (zoom.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << (float)zoom.first / zoom.second
               << "x";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0088(std::ostream& os,
                                               const Value& value)
    {
        if (value.count() > 1) {
            switch (value.toLong(1)) {
            case 0: os << "Center"; break;
            case 1: os << "Top"; break;
            case 2: os << "Bottom"; break;
            case 3: os << "Left"; break;
            case 4: os << "Right"; break;
            default: os << "(" << value << ")"; break;
            }
        }
        else {
            os << "(" << value << ")"; 
        }
        return os;
    }

    const Nikon2MakerNote::RegisterMakerNote Nikon2MakerNote::register_;

    // Nikon2 MakerNote Tag Info
    static const MakerNote::MnTagInfo nikon2MnTagInfo[] = {
        MakerNote::MnTagInfo(0x0003, "Quality", "Image quality setting"),
        MakerNote::MnTagInfo(0x0004, "ColorMode", "Color mode"),
        MakerNote::MnTagInfo(0x0005, "ImageAdjustment", "Image adjustment setting"),
        MakerNote::MnTagInfo(0x0006, "ISOSpeed", "ISO speed setting"),
        MakerNote::MnTagInfo(0x0007, "WhiteBalance", "White balance"),
        MakerNote::MnTagInfo(0x0008, "Focus", "Focus mode"),
        MakerNote::MnTagInfo(0x000a, "DigitalZoom", "Digital zoom setting"),
        MakerNote::MnTagInfo(0x000b, "Adapter", "Adapter used"),
        // End of list marker
        MakerNote::MnTagInfo(0xffff, "(UnknownNikon2MnTag)", "Unknown Nikon2MakerNote tag")
    };

    Nikon2MakerNote::Nikon2MakerNote(bool alloc)
        : IfdMakerNote(nikon2MnTagInfo, alloc), ifdItem_("Nikon2")
    {
        byte buf[] = {
            'N', 'i', 'k', 'o', 'n', '\0', 0x00, 0x01
        };
        readHeader(buf, 8, byteOrder_);
    }

    int Nikon2MakerNote::readHeader(const byte* buf,
                                    long len, 
                                    ByteOrder byteOrder)
    {
        if (len < 8) return 1;

        header_.alloc(8);
        memcpy(header_.pData_, buf, header_.size_);
        adjOffset_ = 8;
        return 0;
    }

    int Nikon2MakerNote::checkHeader() const
    {
        int rc = 0;
        // Check the Nikon prefix
        if (   header_.size_ < 8
            || std::string(reinterpret_cast<char*>(header_.pData_), 6) 
                    != std::string("Nikon\0", 6)) {
            rc = 2;
        }
        return rc;
    }

    Nikon2MakerNote::AutoPtr Nikon2MakerNote::clone(bool alloc) const
    {
        return AutoPtr(clone_(alloc));
    }

    Nikon2MakerNote* Nikon2MakerNote::clone_(bool alloc) const 
    {
        AutoPtr makerNote(new Nikon2MakerNote(alloc)); 
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    std::ostream& Nikon2MakerNote::printTag(std::ostream& os, 
                                            uint16_t tag, 
                                            const Value& value) const
    {
        switch (tag) {
        case 0x0003: print0x0003(os, value); break;
        case 0x0004: print0x0004(os, value); break;
        case 0x0005: print0x0005(os, value); break;
        case 0x0006: print0x0006(os, value); break;
        case 0x0007: print0x0007(os, value); break;
        case 0x000a: print0x000a(os, value); break;
        default:
            // All other tags (known or unknown) go here
            os << value;
            break;
        }
        return os;
    }

    std::ostream& Nikon2MakerNote::print0x0003(std::ostream& os,
                                               const Value& value)
    {
        long quality = value.toLong();
        switch (quality) {
        case 1: os << "VGA Basic"; break;
        case 2: os << "VGA Normal"; break;
        case 3: os << "VGA Fine"; break;
        case 4: os << "SXGA Basic"; break;
        case 5: os << "SXGA Normal"; break;
        case 6: os << "SXGA Fine"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon2MakerNote::print0x0004(std::ostream& os,
                                               const Value& value)
    {
        long color = value.toLong();
        switch (color) {
        case 1: os << "Color"; break;
        case 2: os << "Monochrome"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon2MakerNote::print0x0005(std::ostream& os,
                                               const Value& value)
    {
        long adjustment = value.toLong();
        switch (adjustment) {
        case 0: os << "Normal"; break;
        case 1: os << "Bright+"; break;
        case 2: os << "Bright-"; break;
        case 3: os << "Contrast+"; break;
        case 4: os << "Contrast-"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon2MakerNote::print0x0006(std::ostream& os,
                                               const Value& value)
    {
        long iso = value.toLong();
        switch (iso) {
        case 0: os << "80"; break;
        case 2: os << "160"; break;
        case 4: os << "320"; break;
        case 5: os << "100"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon2MakerNote::print0x0007(std::ostream& os,
                                               const Value& value)
    {
        long wb = value.toLong();
        switch (wb) {
        case 0: os << "Auto"; break;
        case 1: os << "Preset"; break;
        case 2: os << "Daylight"; break;
        case 3: os << "Incandescent"; break;
        case 4: os << "Fluorescent"; break;
        case 5: os << "Cloudy"; break;
        case 6: os << "Speedlight"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon2MakerNote::print0x000a(std::ostream& os,
                                               const Value& value)
    {
        Rational zoom = value.toRational();
        if (zoom.first == 0) {
            os << "Not used";
        }
        else if (zoom.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << (float)zoom.first / zoom.second
               << "x";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    const Nikon3MakerNote::RegisterMakerNote Nikon3MakerNote::register_;

    // Nikon3 MakerNote Tag Info
    static const MakerNote::MnTagInfo nikon3MnTagInfo[] = {
        MakerNote::MnTagInfo(0x0001, "Version", "Nikon Makernote version"),
        MakerNote::MnTagInfo(0x0002, "ISOSpeed", "ISO speed used"),
        MakerNote::MnTagInfo(0x0003, "ColorMode", "Color mode"),
        MakerNote::MnTagInfo(0x0004, "Quality", "Image quality setting"),
        MakerNote::MnTagInfo(0x0005, "WhiteBalance", "White balance"),
        MakerNote::MnTagInfo(0x0006, "Sharpening", "Image sharpening setting"),
        MakerNote::MnTagInfo(0x0007, "Focus", "Focus mode"),
        MakerNote::MnTagInfo(0x0008, "FlashSetting", "Flash setting"),
        MakerNote::MnTagInfo(0x0009, "FlashMode", "Flash mode"),
        MakerNote::MnTagInfo(0x000b, "WhiteBalanceBias", "White balance bias"),
        MakerNote::MnTagInfo(0x000c, "ColorBalance1", "Color balance 1"),
        MakerNote::MnTagInfo(0x000e, "ExposureDiff", "Exposure difference"),
        MakerNote::MnTagInfo(0x000f, "ISOSelection", "ISO selection"),
        MakerNote::MnTagInfo(0x000b, "DataDump", "Data dump"),
        MakerNote::MnTagInfo(0x0011, "ThumbOffset", "Thumbnail IFD offset"),
        MakerNote::MnTagInfo(0x0012, "FlashComp", "Flash compensation setting"),
        MakerNote::MnTagInfo(0x0013, "ISOSetting", "ISO speed setting"),
        MakerNote::MnTagInfo(0x0016, "ImageBoundry", "Image boundry"),
        MakerNote::MnTagInfo(0x0018, "FlashBracketComp", "Flash bracket compensation applied"),
        MakerNote::MnTagInfo(0x0019, "ExposureBracketComp", "AE bracket compensation applied"),
        MakerNote::MnTagInfo(0x0080, "ImageAdjustment", "Image adjustment setting"),
        MakerNote::MnTagInfo(0x0081, "ToneComp", "Tone compensation setting (contrast)"),
        MakerNote::MnTagInfo(0x0082, "AuxiliaryLens", "Auxiliary lens (adapter)"),
        MakerNote::MnTagInfo(0x0083, "LensType", "Lens type"),
        MakerNote::MnTagInfo(0x0084, "Lens", "Lens"),
        MakerNote::MnTagInfo(0x0085, "FocusDistance", "Manual focus distance"),
        MakerNote::MnTagInfo(0x0086, "DigitalZoom", "Digital zoom setting"),
        MakerNote::MnTagInfo(0x0087, "FlashType", "Type of flash used"),
        MakerNote::MnTagInfo(0x0088, "AFFocusPos", "AF focus position"),
        MakerNote::MnTagInfo(0x0089, "Bracketing", "Bracketing"),
        MakerNote::MnTagInfo(0x008c, "NEFCurve1", "NEF curve 1"),
        MakerNote::MnTagInfo(0x008d, "ColorMode", "Color mode"),
        MakerNote::MnTagInfo(0x008f, "SceneMode", "Scene mode"),
        MakerNote::MnTagInfo(0x0090, "LightingType", "Lighting type"),
        MakerNote::MnTagInfo(0x0092, "HueAdjustment", "Hue adjustment"),
        MakerNote::MnTagInfo(0x0094, "Saturation", "Saturation adjustment"),
        MakerNote::MnTagInfo(0x0095, "NoiseReduction", "Noise reduction"),
        MakerNote::MnTagInfo(0x0096, "NEFCurve2", "NEF curve 2"),
        MakerNote::MnTagInfo(0x0097, "ColorBalance2", "Color balance 2"),
        MakerNote::MnTagInfo(0x0099, "NEFThumbnailSize", "NEF thumbnail size"),
        MakerNote::MnTagInfo(0x00a0, "SerialNumber", "Camera serial number"),
        MakerNote::MnTagInfo(0x00a7, "ShutterCount", "Number of shots taken by camera"),
        MakerNote::MnTagInfo(0x00a9, "ImageOptimization", "Image optimization"),
        MakerNote::MnTagInfo(0x00aa, "Saturation", "Saturation"),
        MakerNote::MnTagInfo(0x00ab, "VariProgram", "Vari program"),
        MakerNote::MnTagInfo(0x0e00, "PrintIM", "Print image matching"),
        // End of list marker
        MakerNote::MnTagInfo(0xffff, "(UnknownNikon3MnTag)", "Unknown Nikon3MakerNote tag")
    };

    Nikon3MakerNote::Nikon3MakerNote(bool alloc)
        : IfdMakerNote(nikon3MnTagInfo, alloc), ifdItem_("Nikon3")
    {
        absOffset_ = false;
        byte buf[] = {
            'N', 'i', 'k', 'o', 'n', '\0', 
            0x02, 0x10, 0x00, 0x00, 0x4d, 0x4d, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08
        };
        readHeader(buf, 18, byteOrder_);
    }

    int Nikon3MakerNote::read(const byte* buf,
                              long len, 
                              ByteOrder byteOrder, 
                              long offset)
    {
        int rc = IfdMakerNote::read(buf, len, byteOrder, offset);
        if (rc) return rc;

        // Todo: Add the tags and thumbnail from the embedded thumbnail IFD
        //       Accessing them is easy, but we need support for more than
        //       one IfdId in makernotes to get it working.
        Ifd thumbIfd(makerIfdId, 0, false);
        rc = ifd_.readSubIfd(thumbIfd, buf+10, len-10, byteOrder, 0x0011);
        if (rc) {
            std::cerr << "Didn't work :(\n";  
        }
        else {
            thumbIfd.print(std::cout);
        }
        return 0;
    }

    int Nikon3MakerNote::readHeader(const byte* buf,
                                  long len, 
                                  ByteOrder byteOrder)
    {
        if (len < 18) return 1;

        header_.alloc(18);
        memcpy(header_.pData_, buf, header_.size_);
        TiffHeader tiffHeader;
        tiffHeader.read(header_.pData_ + 10);
        byteOrder_ = tiffHeader.byteOrder();
        adjOffset_ = tiffHeader.offset();
        return 0;
    }

    int Nikon3MakerNote::checkHeader() const
    {
        int rc = 0;
        // Check the Nikon prefix
        if (   header_.size_ < 18
            || std::string(reinterpret_cast<char*>(header_.pData_), 6) 
                    != std::string("Nikon\0", 6)) {
            rc = 2;
        }
        return rc;
    }

    Nikon3MakerNote::AutoPtr Nikon3MakerNote::clone(bool alloc) const
    {
        return AutoPtr(clone_(alloc));
    }

    Nikon3MakerNote* Nikon3MakerNote::clone_(bool alloc) const 
    {
        AutoPtr makerNote(new Nikon3MakerNote(alloc)); 
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    std::ostream& Nikon3MakerNote::printTag(std::ostream& os, 
                                            uint16_t tag, 
                                            const Value& value) const
    {
        switch (tag) {
        case 0x0002: print0x0002(os, value); break;
        case 0x0012: print0x0012(os, value); break;
        case 0x0013: print0x0002(os, value); break; // use 0x0002 print fct
        case 0x0018: print0x0012(os, value); break; // use 0x0012 print fct
        case 0x0084: print0x0084(os, value); break;
        case 0x0087: print0x0087(os, value); break;
        case 0x0088: print0x0088(os, value); break;
        case 0x0089: print0x0089(os, value); break;
        default:
            // All other tags (known or unknown) go here
            os << value;
            break;
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0002(std::ostream& os,
                                               const Value& value)
    {
        if (value.count() > 1) {
            os << value.toLong(1);
        }
        else {
            os << "(" << value << ")"; 
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0012(std::ostream& os, 
                                               const Value& value)
    {
        // From the PHP JPEG Metadata Toolkit
        long fec = value.toLong();
        switch (fec) {
        case 0x06: os << "+1.0 EV"; break;
        case 0x04: os << "+0.7 EV"; break;
        case 0x03: os << "+0.5 EV"; break;
        case 0x02: os << "+0.3 EV"; break;
        case 0x00: os << "0.0 EV"; break;
        case 0xfe: os << "-0.3 EV"; break;
        case 0xfd: os << "-0.5 EV"; break;
        case 0xfc: os << "-0.7 EV"; break;
        case 0xfa: os << "-1.0 EV"; break;
        case 0xf8: os << "-1.3 EV"; break;
        case 0xf7: os << "-1.5 EV"; break;
        case 0xf6: os << "-1.7 EV"; break;
        case 0xf4: os << "-2.0 EV"; break;
        case 0xf2: os << "-2.3 EV"; break;
        case 0xf1: os << "-2.5 EV"; break;
        case 0xf0: os << "-2.7 EV"; break;
        case 0xee: os << "-3.0 EV"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0084(std::ostream& os,
                                               const Value& value)
    {
        if (value.count() == 4) {
            long len1 = value.toLong(0);
            long len2 = value.toLong(1);
            Rational fno1 = value.toRational(2);
            Rational fno2 = value.toRational(3);
            os << len1;
            if (len2 != len1) {
                os << "-" << len2;
            }
            os << "mm " 
               << "F" << (float)fno1.first / fno1.second;
            if (fno2 != fno1) {
                os << "-" << (float)fno2.first / fno2.second;
            }
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0087(std::ostream& os,
                                               const Value& value)
    {
        // From Exiftool
        long flash = value.toLong();
        switch (flash) {
        case 0: os << "Not used"; break;
        case 8: os << "Fired, commander mode"; break;
        case 9: os << "Fired, TTL mode"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0088(std::ostream& os,
                                               const Value& value)
    {
        // Mappings taken from Exiftool 
        long afpos = value.toLong();
        switch (afpos) {
        case 0x0000: os << "Center"; break;
        case 0x0100: os << "Top"; break;
        case 0x0200: os << "Bottom"; break;
        case 0x0300: os << "Left"; break;
        case 0x0400: os << "Right"; break;
        
        // D70
        case 0x00001: os << "Single area, center"; break;
        case 0x10002: os << "Single area, top"; break;
        case 0x20004: os << "Single area, bottom"; break;
        case 0x30008: os << "Single area, left"; break;
        case 0x40010: os << "Single area, right"; break;

        case 0x1000001: os << "Dynamic area, center"; break;
        case 0x1010002: os << "Dynamic area, top"; break;
        case 0x1020004: os << "Dynamic area, bottom"; break;
        case 0x1030008: os << "Dynamic area, left"; break;
        case 0x1040010: os << "Dynamic area, right"; break;

        case 0x2000001: os << "Closest subject, center"; break;
        case 0x2010002: os << "Closest subject, top"; break;
        case 0x2020004: os << "Closest subject, bottom"; break;
        case 0x2030008: os << "Closest subject, left"; break;
        case 0x2040010: os << "Closest subject, right"; break;

        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0089(std::ostream& os,
                                               const Value& value)
    {
        // From Exiftool
        long b = value.toLong();
        switch (b) {
        case 0x00: os << "Single"; break;
        case 0x01: os << "Continuous"; break;
        case 0x02: os << "Delay"; break;
        case 0x03: os << "Remote with delay"; break;
        case 0x04: os << "Remote"; break;
        case 0x16: os << "Exposure bracketing"; break;
        case 0x64: os << "White balance bracketing"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createNikonMakerNote(bool alloc,
                                            const byte* buf, 
                                            long len, 
                                            ByteOrder byteOrder, 
                                            long offset)
    {
        // If there is no "Nikon" string it must be Nikon1 format
        if (len < 6 || std::string(reinterpret_cast<const char*>(buf), 6) 
                    != std::string("Nikon\0", 6)) {
            return MakerNote::AutoPtr(new Nikon1MakerNote(alloc));
        }
        // If the "Nikon" string is not followed by a TIFF header, we assume
        // Nikon2 format
        TiffHeader tiffHeader;
        if (   len < 18 
            || tiffHeader.read(buf + 10) != 0 || tiffHeader.tag() != 0x002a) {
            return MakerNote::AutoPtr(new Nikon2MakerNote(alloc)); 
        }
        // Else we have a Nikon3 makernote
        return MakerNote::AutoPtr(new Nikon3MakerNote(alloc)); 
    }

}                                       // namespace Exiv2
