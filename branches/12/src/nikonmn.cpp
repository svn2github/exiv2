// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
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
  File:      nikonmn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   17-May-04, ahu: created
             25-May-04, ahu: combined all Nikon formats in one component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "nikonmn.hpp"
#include "makernote.hpp"
#include "value.hpp"
#include "image.hpp"
#include "tags.hpp"
#include "error.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    // Roger Larsson: My guess is that focuspoints will follow autofocus sensor
    // module. Note that relative size and position will vary depending on if
    // "wide" or not
    //! Focus points for Nikon cameras, used for Nikon 1 and Nikon 3 makernotes.
    static const char *nikonFocuspoints[] = {
        "Center",
        "Top",
        "Bottom",
        "Left",
        "Right",
        "Upper left",
        "Upper right",
        "Lower left",
        "Lower right",
        "Leftmost",
        "Rightmost"
    };

    //! @cond IGNORE
    Nikon1MakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("NIKON*", "*", createNikonMakerNote);
        MakerNoteFactory::registerMakerNote(
            nikon1IfdId, MakerNote::AutoPtr(new Nikon1MakerNote));

        ExifTags::registerMakerTagInfo(nikon1IfdId, tagInfo_);
    }
    //! @endcond

    // Nikon1 MakerNote Tag Info
    const TagInfo Nikon1MakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Version", "Version", "Nikon Makernote version", nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x0002, "ISOSpeed", "ISOSpeed", "ISO speed setting", nikon1IfdId, makerTags, unsignedShort, print0x0002),
        TagInfo(0x0003, "ColorMode", "ColorMode", "Color mode", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0004, "Quality", "Quality", "Image quality setting", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0005, "WhiteBalance", "WhiteBalance", "White balance", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0006, "Sharpening", "Sharpening", "Image sharpening setting", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "Focus", "Focus", "Focus mode", nikon1IfdId, makerTags, asciiString, print0x0007),
        TagInfo(0x0008, "Flash", "Flash", "Flash mode", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000a, "0x000a", "0x000a", "Unknown", nikon1IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x000f, "ISOSelection", "ISOSelection", "ISO selection", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0010, "DataDump", "DataDump", "Data dump", nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x0080, "ImageAdjustment", "ImageAdjustment", "Image adjustment setting", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0082, "Adapter", "Adapter", "Adapter used", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0085, "FocusDistance", "FocusDistance", "Manual focus distance", nikon1IfdId, makerTags, unsignedRational, print0x0085),
        TagInfo(0x0086, "DigitalZoom", "DigitalZoom", "Digital zoom setting", nikon1IfdId, makerTags, unsignedRational, print0x0086),
        TagInfo(0x0088, "AFFocusPos", "AFFocusPos", "AF focus position", nikon1IfdId, makerTags, undefined, print0x0088),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikon1MnTag)", "(UnknownNikon1MnTag)", "Unknown Nikon1MakerNote tag", nikon1IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon1MakerNote::tagList()
    {
        return tagInfo_;
    }

    Nikon1MakerNote::Nikon1MakerNote(bool alloc)
        : IfdMakerNote(nikon1IfdId, alloc)
    {
    }

    Nikon1MakerNote::Nikon1MakerNote(const Nikon1MakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    Nikon1MakerNote::AutoPtr Nikon1MakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    Nikon1MakerNote* Nikon1MakerNote::create_(bool alloc) const
    {
        return new Nikon1MakerNote(alloc);
    }

    Nikon1MakerNote::AutoPtr Nikon1MakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    Nikon1MakerNote* Nikon1MakerNote::clone_() const
    {
        return new Nikon1MakerNote(*this);
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
            unsigned long focusPoint = value.toLong(1);

            os << value.toLong(0) << "; ";
            switch (focusPoint) {
            // Could use array nikonFokuspoints
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
                os << nikonFocuspoints[focusPoint];
                break;
            default:
                os << value;
                if (focusPoint < sizeof(nikonFocuspoints)/sizeof(nikonFocuspoints[0]))
                    os << " guess " << nikonFocuspoints[focusPoint];
                break;
            }
        }
        else {
            os << value;
        }
        return os;
    }

    //! @cond IGNORE
    Nikon2MakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote(
            nikon2IfdId, MakerNote::AutoPtr(new Nikon2MakerNote));

        ExifTags::registerMakerTagInfo(nikon2IfdId, tagInfo_);
    }
    //! @endcond

    //! Quality, tag 0x0003
    extern const TagDetails nikon2Quality[] = {
        { 1, "VGA Basic"   },
        { 2, "VGA Normal"  },
        { 3, "VGA Fine"    },
        { 4, "SXGA Basic"  },
        { 5, "SXGA Normal" },
        { 6, "SXGA Fine"   }
    };

    //! ColorMode, tag 0x0004
    extern const TagDetails nikon2ColorMode[] = {
        { 1, "Color"      },
        { 2, "Monochrome" }
    };

    //! ImageAdjustment, tag 0x0005
    extern const TagDetails nikon2ImageAdjustment[] = {
        { 0, "Normal"    },
        { 1, "Bright+"   },
        { 2, "Bright-"   },
        { 3, "Contrast+" },
        { 4, "Contrast-" }
    };

    //! ISOSpeed, tag 0x0006
    extern const TagDetails nikon2IsoSpeed[] = {
        { 0, "80"  },
        { 2, "160" },
        { 4, "320" },
        { 5, "100" }
    };

    //! WhiteBalance, tag 0x0007
    extern const TagDetails nikon2WhiteBalance[] = {
        { 0, "Auto"         },
        { 1, "Preset"       },
        { 2, "Daylight"     },
        { 3, "Incandescent" },
        { 4, "Fluorescent"  },
        { 5, "Cloudy"       },
        { 6, "Speedlight"   }
    };

    // Nikon2 MakerNote Tag Info
    const TagInfo Nikon2MakerNote::tagInfo_[] = {
        TagInfo(0x0002, "0x0002", "0x0002", "Unknown", nikon2IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0003, "Quality", "Quality", "Image quality setting", nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2Quality)),
        TagInfo(0x0004, "ColorMode", "ColorMode", "Color mode", nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2ColorMode)),
        TagInfo(0x0005, "ImageAdjustment", "ImageAdjustment", "Image adjustment setting", nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2ImageAdjustment)),
        TagInfo(0x0006, "ISOSpeed", "ISOSpeed", "ISO speed setting", nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2IsoSpeed)),
        TagInfo(0x0007, "WhiteBalance", "WhiteBalance", "White balance", nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2WhiteBalance)),
        TagInfo(0x0008, "Focus", "Focus", "Focus mode", nikon2IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0009, "0x0009", "0x0009", "Unknown", nikon2IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000a, "DigitalZoom", "DigitalZoom", "Digital zoom setting", nikon2IfdId, makerTags, unsignedRational, print0x000a),
        TagInfo(0x000b, "Adapter", "Adapter", "Adapter used", nikon2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0f00, "0x0f00", "0x0f00", "Unknown", nikon2IfdId, makerTags, unsignedLong, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikon2MnTag)", "(UnknownNikon2MnTag)", "Unknown Nikon2MakerNote tag", nikon2IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon2MakerNote::tagList()
    {
        return tagInfo_;
    }

    Nikon2MakerNote::Nikon2MakerNote(bool alloc)
        : IfdMakerNote(nikon2IfdId, alloc)
    {
        byte buf[] = {
            'N', 'i', 'k', 'o', 'n', '\0', 0x00, 0x01
        };
        readHeader(buf, 8, byteOrder_);
    }

    Nikon2MakerNote::Nikon2MakerNote(const Nikon2MakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    int Nikon2MakerNote::readHeader(const byte* buf,
                                    long        len,
                                    ByteOrder   /*byteOrder*/)
    {
        if (len < 8) return 1;

        header_.alloc(8);
        memcpy(header_.pData_, buf, header_.size_);
        start_ = 8;
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

    Nikon2MakerNote::AutoPtr Nikon2MakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    Nikon2MakerNote* Nikon2MakerNote::create_(bool alloc) const
    {
        AutoPtr makerNote(new Nikon2MakerNote(alloc));
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    Nikon2MakerNote::AutoPtr Nikon2MakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    Nikon2MakerNote* Nikon2MakerNote::clone_() const
    {
        return new Nikon2MakerNote(*this);
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

    //! @cond IGNORE
    Nikon3MakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote(
            nikon3IfdId, MakerNote::AutoPtr(new Nikon3MakerNote));

        ExifTags::registerMakerTagInfo(nikon3IfdId, tagInfo_);
    }
    //! @endcond

    //! FlashComp, tag 0x0012
    extern const TagDetails nikon3FlashComp[] = {
        // From the PHP JPEG Metadata Toolkit
        { 0x06, "+1.0 EV" },
        { 0x04, "+0.7 EV" },
        { 0x03, "+0.5 EV" },
        { 0x02, "+0.3 EV" },
        { 0x00,  "0.0 EV" },
        { 0xfe, "-0.3 EV" },
        { 0xfd, "-0.5 EV" },
        { 0xfc, "-0.7 EV" },
        { 0xfa, "-1.0 EV" },
        { 0xf8, "-1.3 EV" },
        { 0xf7, "-1.5 EV" },
        { 0xf6, "-1.7 EV" },
        { 0xf4, "-2.0 EV" },
        { 0xf2, "-2.3 EV" },
        { 0xf1, "-2.5 EV" },
        { 0xf0, "-2.7 EV" },
        { 0xee, "-3.0 EV" }
    };

    //! FlashType, tag 0x0087
    extern const TagDetails nikon3FlashType[] = {
        // From Exiftool
        { 0, "Not used"              },
        { 8, "Fired, commander mode" },
        { 9, "Fired, TTL mode"       }
    };

    //! Bracketing, tag 0x0089
    extern const TagDetails nikon3Bracketing[] = {
        // From Exiftool
        { 0x00, "Single"                   },
        { 0x01, "Continuous"               },
        { 0x02, "Delay"                    },
        { 0x03, "Remote with delay"        },
        { 0x04, "Remote"                   },
        { 0x16, "Exposure bracketing"      },
        { 0x64, "White balance bracketing" }
    };

    // Nikon3 MakerNote Tag Info
    const TagInfo Nikon3MakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Version", "Version", "Nikon Makernote version", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0002, "ISOSpeed", "ISOSpeed", "ISO speed used", nikon3IfdId, makerTags, unsignedShort, print0x0002),
        TagInfo(0x0003, "ColorMode", "ColorMode", "Color mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0004, "Quality", "Quality", "Image quality setting", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0005, "WhiteBalance", "WhiteBalance", "White balance", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0006, "Sharpening", "Sharpening", "Image sharpening setting", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "Focus", "Focus", "Focus mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0008, "FlashSetting", "FlashSetting", "Flash setting", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0009, "FlashMode", "FlashMode", "Flash mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000b, "WhiteBalanceBias", "WhiteBalanceBias", "White balance bias", nikon3IfdId, makerTags, signedShort, printValue),
//        TagInfo(0x000c, "ColorBalance1", "ColorBalance1", "Color balance 1", nikon3IfdId, makerTags, xxx, printValue),
        TagInfo(0x000d, "0x000d", "0x000d", "Unknown", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x000e, "ExposureDiff", "ExposureDiff", "Exposure difference", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x000f, "ISOSelection", "ISOSelection", "ISO selection", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0010, "DataDump", "DataDump", "Data dump", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0011, "ThumbOffset", "ThumbOffset", "Thumbnail IFD offset", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0012, "FlashComp", "FlashComp", "Flash compensation setting", nikon3IfdId, makerTags, undefined, EXV_PRINT_TAG(nikon3FlashComp)),
        TagInfo(0x0013, "ISOSetting", "ISOSetting", "ISO speed setting", nikon3IfdId, makerTags, unsignedShort, print0x0002), // use 0x0002 print fct
        TagInfo(0x0016, "ImageBoundry", "ImageBoundry", "Image boundry", nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0017, "0x0017", "0x0017", "Unknown", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0018, "FlashBracketComp", "FlashBracketComp", "Flash bracket compensation applied", nikon3IfdId, makerTags, undefined, EXV_PRINT_TAG(nikon3FlashComp)), // use 0x0012 print fct
        TagInfo(0x0019, "ExposureBracketComp", "ExposureBracketComp", "AE bracket compensation applied", nikon3IfdId, makerTags, signedRational, printValue),
        TagInfo(0x0080, "ImageAdjustment", "ImageAdjustment", "Image adjustment setting", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0081, "ToneComp", "ToneComp", "Tone compensation setting (contrast)", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0082, "AuxiliaryLens", "AuxiliaryLens", "Auxiliary lens (adapter)", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0083, "LensType", "LensType", "Lens type", nikon3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(0x0084, "Lens", "Lens", "Lens", nikon3IfdId, makerTags, unsignedRational, print0x0084),
        TagInfo(0x0085, "FocusDistance", "FocusDistance", "Manual focus distance", nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0086, "DigitalZoom", "DigitalZoom", "Digital zoom setting", nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0087, "FlashType", "FlashType", "Type of flash used", nikon3IfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikon3FlashType)),
        TagInfo(0x0088, "AFFocusPos", "AFFocusPos", "AF focus position", nikon3IfdId, makerTags, undefined, print0x0088),
        TagInfo(0x0089, "Bracketing", "Bracketing", "Bracketing", nikon3IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon3Bracketing)),
        TagInfo(0x008a, "0x008a", "0x008a", "Unknown", nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x008b, "LensFStops", "LensFStops", "Number of lens stops", nikon3IfdId, makerTags, undefined, print0x008b),
        TagInfo(0x008c, "ToneCurve", "ToneCurve", "Tone curve", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x008d, "ColorMode", "ColorMode", "Color mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x008f, "SceneMode", "SceneMode", "Scene mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0090, "LightingType", "LightingType", "Lighting type", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0091, "0x0091", "0x0091", "Unknown", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0092, "HueAdjustment", "HueAdjustment", "Hue adjustment", nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x0094, "Saturation", "Saturation", "Saturation adjustment", nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x0095, "NoiseReduction", "NoiseReduction", "Noise reduction", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0096, "CompressionCurve", "CompressionCurve", "Compression curve", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0097, "ColorBalance2", "ColorBalance2", "Color balance 2", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0098, "LensData", "LensData", "Lens data", nikon3IfdId, makerTags, undefined, print0x0098),
        TagInfo(0x0099, "NEFThumbnailSize", "NEFThumbnailSize", "NEF thumbnail size", nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x009a, "SensorPixelSize", "SensorPixelSize", "Sensor pixel size", nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x009b, "0x009b", "0x009b", "Unknown", nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x009f, "0x009f", "0x009f", "Unknown", nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x00a0, "SerialNumber", "SerialNumber", "Camera serial number", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00a2, "0x00a2", "0x00a2", "Unknown", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a3, "0x00a3", "0x00a3", "Unknown", nikon3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(0x00a5, "0x00a5", "0x00a5", "Unknown", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a6, "0x00a6", "0x00a6", "Unknown", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a7, "ShutterCount", "ShutterCount", "Number of shots taken by camera", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a8, "0x00a8", "0x00a8", "Unknown", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x00a9, "ImageOptimization", "ImageOptimization", "Image optimization", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00aa, "Saturation", "Saturation", "Saturation", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00ab, "VariProgram", "VariProgram", "Vari program", nikon3IfdId, makerTags, asciiString, printValue),
//        TagInfo(0x0e00, "PrintIM", "PrintIM", "Print image matching", nikon3IfdId, makerTags, xxx, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikon3MnTag)", "(UnknownNikon3MnTag)", "Unknown Nikon3MakerNote tag", nikon3IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagList()
    {
        return tagInfo_;
    }

    Nikon3MakerNote::Nikon3MakerNote(bool alloc)
        : IfdMakerNote(nikon3IfdId, alloc)
    {
        absShift_ = false;
        byte buf[] = {
            'N', 'i', 'k', 'o', 'n', '\0',
            0x02, 0x10, 0x00, 0x00, 0x4d, 0x4d, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08
        };
        readHeader(buf, 18, byteOrder_);
    }

    Nikon3MakerNote::Nikon3MakerNote(const Nikon3MakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    int Nikon3MakerNote::readHeader(const byte* buf,
                                    long        len,
                                    ByteOrder   /*byteOrder*/)
    {
        if (len < 18) return 1;

        header_.alloc(18);
        memcpy(header_.pData_, buf, header_.size_);
        TiffHeader tiffHeader;
        tiffHeader.read(header_.pData_ + 10);
        byteOrder_ = tiffHeader.byteOrder();
        start_ = 10 + tiffHeader.offset();
        shift_ = 10;
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

    Nikon3MakerNote::AutoPtr Nikon3MakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    Nikon3MakerNote* Nikon3MakerNote::create_(bool alloc) const
    {
        AutoPtr makerNote(new Nikon3MakerNote(alloc));
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    Nikon3MakerNote::AutoPtr Nikon3MakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    Nikon3MakerNote* Nikon3MakerNote::clone_() const
    {
        return new Nikon3MakerNote(*this);
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
            os << "mm ";
            std::ostringstream oss;
            oss.copyfmt(os);
            os << "F" << std::setprecision(2)
               << static_cast<float>(fno1.first) / fno1.second;
            if (fno2 != fno1) {
                os << "-" << std::setprecision(2)
                   << static_cast<float>(fno2.first) / fno2.second;
            }
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0088(std::ostream& os,
                                               const Value& value)
    {
        if (value.size() != 4) { // Size is 4 even for those who map this way...
            os << "(" << value << ")";
        }
        else {
            // Mapping by Roger Larsson
            unsigned focusmetering = value.toLong(0);
            unsigned focuspoint = value.toLong(1);
            unsigned focusused = (value.toLong(2) << 8) + value.toLong(3);
            enum {standard, wide} combination = standard;
            const unsigned focuspoints =   sizeof(nikonFocuspoints)
                                         / sizeof(nikonFocuspoints[0]);

            if (focusmetering == 0 && focuspoint == 0 && focusused == 0) {
                // Special case, in Manual focus and with Nikon compacts
                // this indicates that the field has no meaning.
                // But when acually in "Single area, Center" this can mean
                // that focus was not found (try this in AF-C mode)
                // TODO: handle the meaningful case (interacts with other fields)
                os << "N/A";
                return os;
            }

            switch (focusmetering) {
            case 0x00: os << "Single area"; break; // D70, D200
            case 0x01: os << "Dynamic area"; break; // D70, D200
            case 0x02: os << "Closest subject"; break; // D70, D200
            case 0x03: os << "Group dynamic-AF"; break; // D200
            case 0x04: os << "Single area (wide)"; combination = wide; break; // D200
            case 0x05: os << "Dynamic area (wide)"; combination = wide; break; // D200
            default: os << "(" << focusmetering << ")"; break;
            }

            char sep = ';';
            if (focusmetering != 0x02) { //  No user selected point for Closest subject
                os << sep << ' ';

                // What focuspoint did the user select?
                if (focuspoint < focuspoints) {
                    os << nikonFocuspoints[focuspoint];
                    // TODO: os << position[fokuspoint][combination]
                }
                else
                    os << "(" << focuspoint << ")";

                sep = ',';
            }

            // What fokuspoints(!) did the camera use? add if differs
            if (focusused == 0)
                os << sep << " none";
            else if (focusused != 1U<<focuspoint) {
                // selected point was not the actually used one
                // (Roger Larsson: my interpretation, verify)
                os << sep;
                for (unsigned fpid=0; fpid<focuspoints; fpid++)
                    if (focusused & 1<<fpid)
                        os << ' ' << nikonFocuspoints[fpid];
            }

            os << " used";
        }

        return os;
    }

    std::ostream& Nikon3MakerNote::print0x008b(std::ostream& os,
                                               const Value& value)
    {
        // Decoded by Robert Rottmerhusen <email@rottmerhusen.com>
        if (value.size() != 4) return os << "(" << value << ")";
        float a = value.toFloat(0);
        long  b = value.toLong(1);
        long  c = value.toLong(2);
        if (c == 0) return os << "(" << value << ")";
        return os << a * b / c;
    }

    std::ostream& Nikon3MakerNote::print0x0098(std::ostream& os,
                                               const Value& value)
    {
        return os << value;
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createNikonMakerNote(bool        alloc,
                                            const byte* buf,
                                            long        len,
                                            ByteOrder   /*byteOrder*/,
                                            long        /*offset*/)
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
