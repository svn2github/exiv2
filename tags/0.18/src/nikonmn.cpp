// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
 *
 * Lens database to decode Exif.Nikon3.LensData
 * Copyright (C) 2005-2008 Robert Rottmerhusen <lens_id@rottmerhusen.com>
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
             Gilles Caulier (gc) <caulier.gilles@kdemail.net>
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
#include "value.hpp"
#include "image.hpp"
#include "tags.hpp"
#include "error.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>

// *****************************************************************************
namespace {
    using namespace Exiv2;
    /*!
      @brief Decrypt Nikon data (Ref [4])
      @param pData Pointer to the data to decrypt
      @param size Size of the data buffer
      @return 0 if successful (and decrypted data in buf)
     */
    int decryptNikonData(byte* pData, uint32_t size, const ExifData& exifData);
    /*!
      @brief Decrypt Nikon data (Ref [4], [7])
      @param pData Pointer to the data to decrypt
      @param size Size of the data buffer
      @param count Shutter count key
      @param serial Serial number key
     */
    void decrypt(byte* pData, uint32_t size, uint32_t count, uint32_t serial);
}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! Focus area for Nikon cameras.
    static const char *nikonFocusarea[] = {
        N_("Single area"),
        N_("Dynamic area"),
        N_("Dynamic area, closest subject"),
        N_("Group dynamic"),
        N_("Single area (wide)"),
        N_("Dynamic area (wide")
    };

    // Roger Larsson: My guess is that focuspoints will follow autofocus sensor
    // module. Note that relative size and position will vary depending on if
    // "wide" or not
    //! Focus points for Nikon cameras, used for Nikon 1 and Nikon 3 makernotes.
    static const char *nikonFocuspoints[] = {
        N_("Center"),
        N_("Top"),
        N_("Bottom"),
        N_("Left"),
        N_("Right"),
        N_("Upper-left"),
        N_("Upper-right"),
        N_("Lower-left"),
        N_("Lower-right"),
        N_("Left-most"),
        N_("Right-most")
    };

    //! FlashComp, tag 0x0012
    extern const TagDetails nikonFlashComp[] = {
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

    //! ColorSpace, tag 0x001e
    extern const TagDetails nikonColorSpace[] = {
        { 1, N_("sRGB")      },
        { 2, N_("Adobe RGB") }
    };

    //! FlashMode, tag 0x0087
    extern const TagDetails nikonFlashMode[] = {
        { 0, N_("Did not fire")         },
        { 1, N_("Fire, manual")         },
        { 7, N_("Fire, external")       },
        { 8, N_("Fire, commander mode") },
        { 9, N_("Fire, TTL mode")       }
    };

    //! ShootingMode, tag 0x0089
    extern const TagDetails nikonShootingMode[] = {
        {  1, N_("Continuous")               },
        {  2, N_("Delay")                    },
        {  4, N_("PC control")               },
        {  8, N_("Exposure bracketing")      },
        { 16, N_("Unused LE-NR slowdown")    },
        { 32, N_("White balance bracketing") },
        { 64, N_("IR control")               }
    };

    //! AutoBracketRelease, tag 0x008a
    extern const TagDetails nikonAutoBracketRelease[] = {
        { 0, N_("None")           },
        { 1, N_("Auto release")   },
        { 2, N_("Manual release") }
    };

    //! HighISONoiseReduction, tag 0x00b1
    extern const TagDetails nikonHighISONoiseReduction[] = {
        { 0, N_("Off")                  },
        { 1, N_("On for ISO 1600/3200") },
        { 2, N_("Weak")                 },
        { 4, N_("Normal")               },
        { 6, N_("Strong")               }
    };

    // Nikon1 MakerNote Tag Info
    const TagInfo Nikon1MakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Version", N_("Version"),
                N_("Nikon Makernote version"),
                nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x0002, "ISOSpeed", N_("ISO Speed"),
                N_("ISO speed setting"),
                nikon1IfdId, makerTags, unsignedShort, print0x0002),
        TagInfo(0x0003, "ColorMode", N_("Color Mode"),
                N_("Color mode"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0004, "Quality", N_("Quality"),
                N_("Image quality setting"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0005, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0006, "Sharpening", N_("Sharpening"),
                N_("Image sharpening setting"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "Focus", N_("Focus"),
                N_("Focus mode"),
                nikon1IfdId, makerTags, asciiString, print0x0007),
        TagInfo(0x0008, "FlashSetting", N_("Flash Setting"),
                N_("Flash setting"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000a, "0x000a", "0x000a",
                N_("Unknown"),
                nikon1IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x000f, "ISOSelection", N_("ISO Selection"),
                N_("ISO selection"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0010, "DataDump", N_("Data Dump"),
                N_("Data dump"),
                nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x0080, "ImageAdjustment", N_("Image Adjustment"),
                N_("Image adjustment setting"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0082, "AuxiliaryLens", N_("Auxiliary Lens"),
                N_("Auxiliary lens (adapter)"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0085, "FocusDistance", N_("Focus Distance"),
                N_("Manual focus distance"),
                nikon1IfdId, makerTags, unsignedRational, print0x0085),
        TagInfo(0x0086, "DigitalZoom", N_("Digital Zoom"),
                N_("Digital zoom setting"),
                nikon1IfdId, makerTags, unsignedRational, print0x0086),
        TagInfo(0x0088, "AFFocusPos", N_("AF Focus Position"),
                N_("AF focus position information"),
                nikon1IfdId, makerTags, undefined, print0x0088),

        // End of list marker
        TagInfo(0xffff, "(UnknownNikon1MnTag)", "(UnknownNikon1MnTag)",
                N_("Unknown Nikon1MakerNote tag"),
                nikon1IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon1MakerNote::tagList()
    {
        return tagInfo_;
    }

    std::ostream& Nikon1MakerNote::print0x0002(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
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
                                               const Value& value,
                                               const ExifData*)
    {
        std::string focus = value.toString();
        if      (focus == "AF-C  ") os << _("Continuous autofocus");
        else if (focus == "AF-S  ") os << _("Single autofocus");
        else                        os << "(" << value << ")";
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0085(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        Rational distance = value.toRational();
        if (distance.first == 0) {
            os << _("Unknown");
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
                                               const Value& value,
                                               const ExifData*)
    {
        Rational zoom = value.toRational();
        if (zoom.first == 0) {
            os << _("Not used");
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
                                               const Value& value,
                                               const ExifData*)
    {
        if (value.count() >= 1) {
            unsigned long focusArea = value.toLong(0);
            os << nikonFocusarea[focusArea] ;
        }
        if (value.count() >= 2) {
            os << "; ";
            unsigned long focusPoint = value.toLong(1);

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
                    os << " " << _("guess") << " " << nikonFocuspoints[focusPoint];
                break;
            }
        }
        if (value.count() >= 3) {
            unsigned long focusPointsUsed1 = value.toLong(2);
            unsigned long focusPointsUsed2 = value.toLong(3);

            if (focusPointsUsed1 != 0 && focusPointsUsed2 != 0)
            {
                os << "; [";

                if (focusPointsUsed1 & 1)
                    os << nikonFocuspoints[0] << " ";
                if (focusPointsUsed1 & 2)
                    os << nikonFocuspoints[1] << " ";
                if (focusPointsUsed1 & 4)
                    os << nikonFocuspoints[2] << " ";
                if (focusPointsUsed1 & 8)
                    os << nikonFocuspoints[3] << " ";
                if (focusPointsUsed1 & 16)
                    os << nikonFocuspoints[4] << " ";
                if (focusPointsUsed1 & 32)
                    os << nikonFocuspoints[5] << " ";
                if (focusPointsUsed1 & 64)
                    os << nikonFocuspoints[6] << " ";
                if (focusPointsUsed1 & 128)
                    os << nikonFocuspoints[7] << " ";

                if (focusPointsUsed2 & 1)
                    os << nikonFocuspoints[8] << " ";
                if (focusPointsUsed2 & 2)
                    os << nikonFocuspoints[9] << " ";
                if (focusPointsUsed2 & 4)
                    os << nikonFocuspoints[10] << " ";

                os << "]";
            }
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    //! Quality, tag 0x0003
    extern const TagDetails nikon2Quality[] = {
        { 1, N_("VGA Basic")   },
        { 2, N_("VGA Normal")  },
        { 3, N_("VGA Fine")    },
        { 4, N_("SXGA Basic")  },
        { 5, N_("SXGA Normal") },
        { 6, N_("SXGA Fine")   }
    };

    //! ColorMode, tag 0x0004
    extern const TagDetails nikon2ColorMode[] = {
        { 1, N_("Color")      },
        { 2, N_("Monochrome") }
    };

    //! ImageAdjustment, tag 0x0005
    extern const TagDetails nikon2ImageAdjustment[] = {
        { 0, N_("Normal")    },
        { 1, N_("Bright+")   },
        { 2, N_("Bright-")   },
        { 3, N_("Contrast+") },
        { 4, N_("Contrast-") }
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
        { 0, N_("Auto")         },
        { 1, N_("Preset")       },
        { 2, N_("Daylight")     },
        { 3, N_("Incandescent") },
        { 4, N_("Fluorescent")  },
        { 5, N_("Cloudy")       },
        { 6, N_("Speedlight")   }
    };

    // Nikon2 MakerNote Tag Info
    const TagInfo Nikon2MakerNote::tagInfo_[] = {
        TagInfo(0x0002, "0x0002", "0x0002",
                N_("Unknown"),
                nikon2IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0003, "Quality", N_("Quality"),
                N_("Image quality setting"),
                nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2Quality)),
        TagInfo(0x0004, "ColorMode", N_("Color Mode"),
                N_("Color mode"),
                nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2ColorMode)),
        TagInfo(0x0005, "ImageAdjustment", N_("Image Adjustment"),
                N_("Image adjustment setting"),
                nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2ImageAdjustment)),
        TagInfo(0x0006, "ISOSpeed", N_("ISO Speed"),
                N_("ISO speed setting"),
                nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2IsoSpeed)),
        TagInfo(0x0007, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2WhiteBalance)),
        TagInfo(0x0008, "Focus", N_("Focus Mode"),
                N_("Focus mode"),
                nikon2IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0009, "0x0009", "0x0009",
                N_("Unknown"),
                nikon2IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000a, "DigitalZoom", N_("Digital Zoom"),
                N_("Digital zoom setting"),
                nikon2IfdId, makerTags, unsignedRational, print0x000a),
        TagInfo(0x000b, "AuxiliaryLens", N_("Auxiliary Lens"),
                N_("Auxiliary lens (adapter)"),
                nikon2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0f00, "0x0f00", "0x0f00",
                N_("Unknown"),
                nikon2IfdId, makerTags, unsignedLong, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikon2MnTag)", "(UnknownNikon2MnTag)",
                N_("Unknown Nikon2MakerNote tag"),
                nikon2IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon2MakerNote::tagList()
    {
        return tagInfo_;
    }

    std::ostream& Nikon2MakerNote::print0x000a(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        Rational zoom = value.toRational();
        if (zoom.first == 0) {
            os << _("Not used");
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

    // Nikon3 MakerNote Tag Info
    const TagInfo Nikon3MakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Version", N_("Version"),
                N_("Nikon Makernote version"),
                nikon3IfdId, makerTags, undefined, printExifVersion),
        TagInfo(0x0002, "ISOSpeed", N_("ISO Speed"),
                N_("ISO speed setting"),
                nikon3IfdId, makerTags, unsignedShort, print0x0002),
        TagInfo(0x0003, "ColorMode", N_("Color Mode"),
                N_("Color mode"),
                nikon3IfdId, makerTags, asciiString, printValue),
         TagInfo(0x0004, "Quality", N_("Quality"),
                N_("Image quality setting"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0005, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0006, "Sharpening", N_("Sharpening"),
                N_("Image sharpening setting"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "Focus", N_("Focus"),
                N_("Focus mode"),
                nikon3IfdId, makerTags, asciiString, print0x0007),
        TagInfo(0x0008, "FlashSetting", N_("Flash Setting"),
                N_("Flash setting"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0009, "FlashDevice", N_("Flash Device"),
                N_("Flash device"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000a, "0x000a", "0x000a",
                N_("Unknown"),
                nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x000b, "WhiteBalanceBias", N_("White Balance Bias"),
                N_("White balance bias"),
                nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x000c, "ColorBalance1", N_("Color Balance 1"),
                N_("Color balance settings 1"),
                nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x000d, "ProgramShift", N_("Program Shift"),
                N_("Program shift"),
                nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x000e, "ExposureDiff", N_("Exposure Difference"),
                N_("Exposure difference"),
                nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x000f, "ISOSelection", N_("ISO Selection"),
                N_("ISO selection"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0010, "DataDump", N_("Data Dump"),
                N_("Data dump"),
                nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0011, "Preview", N_("Pointer to a preview image"),
                N_("Offset to an IFD containing a preview image"),
                nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0012, "FlashComp", N_("Flash Comp"),
                N_("Flash compensation setting"),
                nikon3IfdId, makerTags, undefined, EXV_PRINT_TAG(nikonFlashComp)),
        TagInfo(0x0013, "ISOSettings", N_("ISO Settings"),
                N_("ISO setting"),
                nikon3IfdId, makerTags, unsignedShort, print0x0002), // use 0x0002 print fct
        TagInfo(0x0016, "ImageBoundary", N_("Image Boundary"),
                N_("Image boundary"),
                nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0017, "0x0017", "0x0017",
                N_("Unknown"),
                nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0018, "FlashBracketComp", N_("Flash Bracket Comp"),
                N_("Flash bracket compensation applied"),
                nikon3IfdId, makerTags, undefined, EXV_PRINT_TAG(nikonFlashComp)), // use 0x0012 print fct
        TagInfo(0x0019, "ExposureBracketComp", N_("Exposure Bracket Comp"),
                N_("AE bracket compensation applied"),
                nikon3IfdId, makerTags, signedRational, printValue),
        TagInfo(0x001a, "ImageProcessing", N_("Image Processing"),
                N_("Image processing"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x001b, "CropHiSpeed", N_("Crop High Speed"),
                N_("Crop high speed"),
                nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001d, "SerialNumber", N_("Serial Number"),
                N_("Serial Number"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x001e, "ColorSpace", N_("Color Space"),
                N_("Color space"),
                nikon3IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonColorSpace)),
        TagInfo(0x0080, "ImageAdjustment", N_("Image Adjustment"),
                N_("Image adjustment setting"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0081, "ToneComp", N_("Tone Compensation"),
                N_("Tone compensation"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0082, "AuxiliaryLens", N_("Auxiliary Lens"),
                N_("Auxiliary lens (adapter)"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0083, "LensType", N_("Lens Type"),
                N_("Lens type"),
                nikon3IfdId, makerTags, unsignedByte, print0x0083),
        TagInfo(0x0084, "Lens", N_("Lens"),
                N_("Lens"),
                nikon3IfdId, makerTags, unsignedRational, print0x0084),
        TagInfo(0x0085, "FocusDistance", N_("Focus Distance"),
                N_("Manual focus distance"),
                nikon3IfdId, makerTags, unsignedRational, print0x0085),
        TagInfo(0x0086, "DigitalZoom", N_("Digital Zoom"),
                N_("Digital zoom setting"),
                nikon3IfdId, makerTags, unsignedRational, print0x0086),
        TagInfo(0x0087, "FlashMode", N_("Flash Mode"),
                N_("Mode of flash used"),
                nikon3IfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonFlashMode)),
        TagInfo(0x0088, "AFFocusPos", N_("AF Focus Position"),
                N_("AF focus position information"),
                nikon3IfdId, makerTags, undefined, print0x0088),
        TagInfo(0x0089, "ShootingMode", N_("Shooting Mode"),
                N_("Shooting mode"),
                nikon3IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonShootingMode)),
        TagInfo(0x008a, "AutoBracketRelease", N_("Auto Bracket Release"),
                N_("Auto bracket release"),
                nikon3IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonAutoBracketRelease)),
        TagInfo(0x008b, "LensFStops", N_("Lens FStops"),
                N_("Lens FStops"),
                nikon3IfdId, makerTags, undefined, print0x008b),
        TagInfo(0x008c, "ToneCurve", N_("Tone Curve"),
                N_("Tone curve"),
                nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x008d, "ColorMode", N_("Color Mode"),
                N_("Color mode"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x008f, "SceneMode", N_("Scene Mode"),
                N_("Scene mode"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0090, "LightSource", N_("Light Source"),
                N_("Light source"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0091, "0x0091", "0x0091",
                N_("Unknown"),
                nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0092, "HueAdjustment", N_("Hue Adjustment"),
                N_("Hue adjustment"),
                nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x0094, "Saturation", N_("Saturation"),
                N_("Saturation"),
                nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x0095, "NoiseReduction", N_("Noise Reduction"),
                N_("Noise reduction"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0096, "CompressionCurve", N_("Compression Curve"),
                N_("Compression curve"),
                nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0097, "ColorBalance2", N_("Color Balance 2"),
                N_("Color balance settings 2"),
                nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0098, "LensData", N_("Lens Data"),
                N_("Lens data settings"),
                nikon3IfdId, makerTags, undefined, print0x0098),
        TagInfo(0x0099, "RawImageCenter", N_("Raw Image Center"),
                N_("Raw image center"),
                nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x009a, "SensorPixelSize", N_("Sensor Pixel Size"),
                N_("Sensor pixel size"),
                nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x009b, "0x009b", "0x009b",
                N_("Unknown"),
                nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x009f, "0x009f", "0x009f",
                N_("Unknown"),
                nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x00a0, "SerialNO", N_("Serial NO"),
                N_("Camera serial number, usually starts with \"NO= \""),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00a2, "ImageDataSize", N_("Image Data Size"),
                N_("Image data size"),
                nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a3, "0x00a3", "0x00a3",
                N_("Unknown"),
                nikon3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(0x00a5, "ImageCount", N_("Image Count"),
                N_("Image count"),
                nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a6, "DeleteImageCount", N_("Delete Image Count"),
                N_("Delete image count"),
                nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a7, "ShutterCount", N_("Shutter Count"),
                N_("Number of shots taken by camera"),
                nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a8, "0x00a8", "0x00a8",
                N_("Unknown"),
                nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x00a9, "ImageOptimization", N_("Image Optimization"),
                N_("Image optimization"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00aa, "Saturation", N_("Saturation"),
                N_("Saturation"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00ab, "VariProgram", N_("Program Variation"),
                N_("Program variation"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00ac, "ImageStabilization", N_("Image Stabilization"),
                N_("Image stabilization"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00ad, "AFResponse", N_("AF Response"),
                N_("AF response"),
                nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00b1, "HighISONoiseReduction", N_("High ISO Noise Reduction"),
                N_("High ISO Noise Reduction"),
                nikon3IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonHighISONoiseReduction)),
        TagInfo(0x0e00, "PrintIM", N_("Print IM"),
                N_("PrintIM information"),
                nikon3IfdId, makerTags, undefined, printValue),

        // TODO: Add Capture Data decoding implementation.
        TagInfo(0x0e01, "CaptureData", N_("Capture Data"),
                N_("Capture data"),
                nikon3IfdId, makerTags, undefined, printValue),

        TagInfo(0x0e09, "CaptureVersion", N_("Capture Version"),
                N_("Capture version"),
                nikon3IfdId, makerTags, asciiString, printValue),

        // TODO: Add Capture Offsets decoding implementation.
        TagInfo(0x0e0e, "CaptureOffsets", N_("Capture Offsets"),
                N_("Capture offsets"),
                nikon3IfdId, makerTags, undefined, printValue),

        // End of list marker
        TagInfo(0xffff, "(UnknownNikon3MnTag)", "(UnknownNikon3MnTag)",
                N_("Unknown Nikon3MakerNote tag"),
                nikon3IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagList()
    {
        return tagInfo_;
    }

    std::ostream& Nikon3MakerNote::print0x0002(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        if (value.count() > 1) {
            os << value.toLong(1);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0007(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        std::string focus = value.toString();
        if      (focus == "AF-C  ") os << _("Continuous autofocus");
        else if (focus == "AF-S  ") os << _("Single autofocus");
        else                      os << "(" << value << ")";
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0083(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        long lensType = value.toLong();

        bool valid=false;
        if (lensType & 1)
        {
            os << "MF ";
            valid=true;
        }
        if (lensType & 2)
        {
            os << "D ";
            valid=true;
        }
        if (lensType & 4)
        {
            os << "G ";
            valid=true;
        }
        if (lensType & 8)
        {
            os << "VR";
            valid=true;
        }

        if (!valid)
            os << "(" << lensType << ")";

        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0084(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        if (   value.count() != 4
            || value.toRational(0).second == 0
            || value.toRational(1).second == 0) {
            os << "(" << value << ")";
            return os;
        }
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
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0085(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        Rational distance = value.toRational();
        if (distance.first == 0) {
            os << _("Unknown");
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

    std::ostream& Nikon3MakerNote::print0x0086(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        Rational zoom = value.toRational();
        if (zoom.first == 0) {
            os << _("Not used");
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

    std::ostream& Nikon3MakerNote::print0x0088(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
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
            case 0x00: os << _("Single area");          break; // D70, D200
            case 0x01: os << _("Dynamic area");         break; // D70, D200
            case 0x02: os << _("Closest subject");      break; // D70, D200
            case 0x03: os << _("Group dynamic-AF");     break; // D200
            case 0x04: os << _("Single area (wide)");   combination = wide; break; // D200
            case 0x05: os << _("Dynamic area (wide)");  combination = wide; break; // D200
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
                os << sep << " " << _("none");
            else if (focusused != 1U<<focuspoint) {
                // selected point was not the actually used one
                // (Roger Larsson: my interpretation, verify)
                os << sep;
                for (unsigned fpid=0; fpid<focuspoints; fpid++)
                    if (focusused & 1<<fpid)
                        os << ' ' << nikonFocuspoints[fpid];
            }

            os << " " << _("used");
        }

        return os;
    }

    std::ostream& Nikon3MakerNote::print0x008b(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        // Decoded by Robert Rottmerhusen <email@rottmerhusen.com>
        if (   value.size() != 4
            || value.typeId() != undefined) {
            return os << "(" << value << ")";
        }
        float a = value.toFloat(0);
        long  b = value.toLong(1);
        long  c = value.toLong(2);
        if (c == 0) return os << "(" << value << ")";
        return os << a * b / c;
    }

    std::ostream& Nikon3MakerNote::print0x0098(std::ostream& os,
                                               const Value& value,
                                               const ExifData* metadata)
    {
#ifdef EXV_HAVE_LENSDATA
//#-----------------------------------------
//# List of AF F-Mount lenses - version 3.82
//#-----------------------------------------
//#
//# created by Robert Rottmerhusen 2005 - 2008
//# http://www.rottmerhusen.com (lens_id@rottmerhusen.com)
//#
//# with great help of Hiroshi Kamisaka (many new lenses)
//# http://homepage3.nifty.com/kamisaka/
//#
//# three misidentified lenses (ID like the a Nikkor):
//#    "Sigma" "105mm F2.8 EX DG Macro";
//#    "Tokina" "AT-X 235 AF PRO / AF 20-35mm f/2.8";
//#    "Tokina" "AT-X 124 AF PRO DX II / 12-24mm f/4";
//#
//# for use in non-commercial, GPL or open source software only!
//# please contact me for adding lenses or use in commercial software.
//#
//#"data from TAG 0x98" "ltyp""manuf" "lens name from manuf";
//#
//#------------------------------------------------------------------------------------------------
//# Nikkor lenses by their LensID
//#------------------------------------------------------------------------------------------------
//#
static const struct {unsigned char lid,stps,focs,focl,aps,apl,lfw, ltype; const char *manuf, *lensname;}
fmountlens[] = {
{0x01,0x58,0x50,0x50,0x14,0x14,0x02,0x00, "Nikon", "AF Nikkor 50mm f/1.8"},
{0x01,0x00,0x00,0x00,0x00,0x00,0x02,0x00, "Nikon", "AF Teleconverter TC-16A 1.6x"},
{0x01,0x00,0x00,0x00,0x00,0x00,0x08,0x00, "Nikon", "AF Teleconverter TC-16A 1.6x"},
{0x02,0x42,0x44,0x5C,0x2A,0x34,0x02,0x00, "Nikon", "AF Zoom-Nikkor 35-70mm f/3.3-4.5"},
{0x02,0x42,0x44,0x5C,0x2A,0x34,0x08,0x00, "Nikon", "AF Zoom-Nikkor 35-70mm f/3.3-4.5"},
{0x03,0x48,0x5C,0x81,0x30,0x30,0x02,0x00, "Nikon", "AF Zoom-Nikkor 70-210mm f/4"},
{0x04,0x48,0x3C,0x3C,0x24,0x24,0x03,0x00, "Nikon", "AF Nikkor 28mm f/2.8"},
{0x05,0x54,0x50,0x50,0x0C,0x0C,0x04,0x00, "Nikon", "AF Nikkor 50mm f/1.4"},
{0x06,0x54,0x53,0x53,0x24,0x24,0x06,0x00, "Nikon", "AF Micro-Nikkor 55mm f/2.8"},
{0x07,0x40,0x3C,0x62,0x2C,0x34,0x03,0x00, "Nikon", "AF Zoom-Nikkor 28-85mm f/3.5-4.5"},
{0x08,0x40,0x44,0x6A,0x2C,0x34,0x04,0x00, "Nikon", "AF Zoom-Nikkor 35-105mm f/3.5-4.5"},
{0x09,0x48,0x37,0x37,0x24,0x24,0x04,0x00, "Nikon", "AF Nikkor 24mm f/2.8"},
{0x0A,0x48,0x8E,0x8E,0x24,0x24,0x03,0x00, "Nikon", "AF Nikkor 300mm f/2.8 IF-ED"},
{0x0B,0x48,0x7C,0x7C,0x24,0x24,0x05,0x00, "Nikon", "AF Nikkor 180mm f/2.8 IF-ED"},
//#0C
{0x0D,0x40,0x44,0x72,0x2C,0x34,0x07,0x00, "Nikon", "AF Zoom-Nikkor 35-135mm f/3.5-4.5"},
{0x0E,0x48,0x5C,0x81,0x30,0x30,0x05,0x00, "Nikon", "AF Zoom-Nikkor 70-210mm f/4"},
{0x0F,0x58,0x50,0x50,0x14,0x14,0x05,0x00, "Nikon", "AF Nikkor 50mm f/1.8 N"},
{0x10,0x48,0x8E,0x8E,0x30,0x30,0x08,0x00, "Nikon", "AF Nikkor 300mm f/4 IF-ED"},
{0x11,0x48,0x44,0x5C,0x24,0x24,0x08,0x00, "Nikon", "AF Zoom-Nikkor 35-70mm f/2.8"},
{0x12,0x48,0x5C,0x81,0x30,0x3C,0x09,0x00, "Nikon", "AF Nikkor 70-210mm f/4-5.6"},
{0x13,0x42,0x37,0x50,0x2A,0x34,0x0B,0x00, "Nikon", "AF Zoom-Nikkor 24-50mm f/3.3-4.5"},
{0x14,0x48,0x60,0x80,0x24,0x24,0x0B,0x00, "Nikon", "AF Zoom-Nikkor 80-200mm f/2.8 ED"},
{0x15,0x4C,0x62,0x62,0x14,0x14,0x0C,0x00, "Nikon", "AF Nikkor 85mm f/1.8"},
//#16
{0x17,0x3C,0xA0,0xA0,0x30,0x30,0x11,0x00, "Nikon", "Nikkor 500mm f/4 P ED IF"},
{0x18,0x40,0x44,0x72,0x2C,0x34,0x0E,0x00, "Nikon", "AF Zoom-Nikkor 35-135mm f/3.5-4.5 N"},
//#19
{0x1A,0x54,0x44,0x44,0x18,0x18,0x11,0x00, "Nikon", "AF Nikkor 35mm f/2"},
{0x1B,0x44,0x5E,0x8E,0x34,0x3C,0x10,0x00, "Nikon", "AF Zoom-Nikkor 75-300mm f/4.5-5.6"},
{0x1C,0x48,0x30,0x30,0x24,0x24,0x12,0x00, "Nikon", "AF Nikkor 20mm f/2.8"},
{0x1D,0x42,0x44,0x5C,0x2A,0x34,0x12,0x00, "Nikon", "AF Zoom-Nikkor 35-70mm f/3.3-4.5 N"},
{0x1E,0x54,0x56,0x56,0x24,0x24,0x13,0x00, "Nikon", "AF Micro-Nikkor 60mm f/2.8"},
{0x1F,0x54,0x6A,0x6A,0x24,0x24,0x14,0x00, "Nikon", "AF Micro-Nikkor 105mm f/2.8"},
{0x20,0x48,0x60,0x80,0x24,0x24,0x15,0x00, "Nikon", "AF Zoom-Nikkor 80-200mm f/2.8 ED"},
{0x21,0x40,0x3C,0x5C,0x2C,0x34,0x16,0x00, "Nikon", "AF Zoom-Nikkor 28-70mm f/3.5-4.5"},
{0x22,0x48,0x72,0x72,0x18,0x18,0x16,0x00, "Nikon", "AF DC-Nikkor 135mm f/2"},
{0x23,0x30,0xBE,0xCA,0x3C,0x48,0x17,0x00, "Nikon", "Zoom-Nikkor 1200-1700mm f/5.6-8 P ED IF"},
{0x24,0x48,0x60,0x80,0x24,0x24,0x1A,0x02, "Nikon", "AF Zoom-Nikkor 80-200mm f/2.8D ED"},
{0x25,0x48,0x44,0x5c,0x24,0x24,0x1B,0x02, "Nikon", "AF Zoom-Nikkor 35-70mm f/2.8D"},
{0x25,0x48,0x44,0x5c,0x24,0x24,0x52,0x02, "Nikon", "AF Zoom-Nikkor 35-70mm f/2.8D"},
//#26
{0x27,0x48,0x8E,0x8E,0x24,0x24,0x1D,0x02, "Nikon", "AF-I Nikkor 300mm f/2.8D IF-ED"},
{0x27,0x48,0x8E,0x8E,0x24,0x24,0xF1,0x02, "Nikon", "AF-I Nikkor 300mm f/2.8D IF-ED + TC-14E"},
{0x27,0x48,0x8E,0x8E,0x24,0x24,0xE1,0x02, "Nikon", "AF-I Nikkor 300mm f/2.8D IF-ED + TC-17E"},
{0x27,0x48,0x8E,0x8E,0x24,0x24,0xF2,0x02, "Nikon", "AF-I Nikkor 300mm f/2.8D IF-ED + TC-20E"},
{0x28,0x3C,0xA6,0xA6,0x30,0x30,0x1D,0x02, "Nikon", "AF-I Nikkor 600mm f/4D IF-ED"},
//#29
{0x2A,0x54,0x3C,0x3C,0x0C,0x0C,0x26,0x02, "Nikon", "AF Nikkor 28mm f/1.4D"},
{0x2B,0x3C,0x44,0x60,0x30,0x3C,0x1F,0x02, "Nikon", "AF Zoom-Nikkor 35-80mm f/4-5.6D"},
{0x2C,0x48,0x6A,0x6A,0x18,0x18,0x27,0x02, "Nikon", "AF DC-Nikkor 105mm f/2D"},
{0x2D,0x48,0x80,0x80,0x30,0x30,0x21,0x02, "Nikon", "AF Micro-Nikkor 200mm f/4D IF-ED"},
{0x2E,0x48,0x5C,0x82,0x30,0x3C,0x28,0x02, "Nikon", "AF Nikkor 70-210mm f/4-5.6D"},
{0x2F,0x48,0x30,0x44,0x24,0x24,0x29,0x02, "Nikon", "AF Zoom-Nikkor 20-35mm f/2.8D IF"},
{0x30,0x48,0x98,0x98,0x24,0x24,0x24,0x02, "Nikon", "AF-I Nikkor 400mm f/2.8D IF-ED"},
{0x31,0x54,0x56,0x56,0x24,0x24,0x25,0x02, "Nikon", "AF Micro-Nikkor 60mm f/2.8D"},
{0x32,0x54,0x6A,0x6A,0x24,0x24,0x35,0x02, "Nikon", "AF Micro-Nikkor 105mm f/2.8D"},
{0x33,0x48,0x2D,0x2D,0x24,0x24,0x31,0x02, "Nikon", "AF Nikkor 18mm f/2.8D"},
{0x34,0x48,0x29,0x29,0x24,0x24,0x32,0x02, "Nikon", "AF Fisheye Nikkor 16mm f/2.8D"},
{0x35,0x3C,0xA0,0xA0,0x30,0x30,0x33,0x02, "Nikon", "AF-I Nikkor 500mm f/4D IF-ED"},
{0x36,0x48,0x37,0x37,0x24,0x24,0x34,0x02, "Nikon", "AF Nikkor 24mm f/2.8D"},
{0x37,0x48,0x30,0x30,0x24,0x24,0x36,0x02, "Nikon", "AF Nikkor 20mm f/2.8D"},
{0x38,0x4C,0x62,0x62,0x14,0x14,0x37,0x02, "Nikon", "AF Nikkor 85mm f/1.8D"},
//#39
{0x3A,0x40,0x3C,0x5C,0x2C,0x34,0x39,0x02, "Nikon", "AF Zoom-Nikkor 28-70mm f/3.5-4.5D"},
{0x3B,0x48,0x44,0x5C,0x24,0x24,0x3A,0x02, "Nikon", "AF Zoom-Nikkor 35-70mm f/2.8D N"},
{0x3C,0x48,0x60,0x80,0x24,0x24,0x3B,0x02, "Nikon", "AF Zoom-Nikkor 80-200mm f/2.8D ED"},
{0x3D,0x3C,0x44,0x60,0x30,0x3C,0x3E,0x02, "Nikon", "AF Zoom-Nikkor 35-80mm f/4-5.6D"},
{0x3E,0x48,0x3C,0x3C,0x24,0x24,0x3D,0x02, "Nikon", "AF Nikkor 28mm f/2.8D"},
{0x3F,0x40,0x44,0x6A,0x2C,0x34,0x45,0x02, "Nikon", "AF Zoom-Nikkor 35-105mm f/3.5-4.5D"},
//#40
{0x41,0x48,0x7c,0x7c,0x24,0x24,0x43,0x02, "Nikon", "AF Nikkor 180mm f/2.8D IF-ED"},
{0x42,0x54,0x44,0x44,0x18,0x18,0x44,0x02, "Nikon", "AF Nikkor 35mm f/2D"},
{0x43,0x54,0x50,0x50,0x0C,0x0C,0x46,0x02, "Nikon", "AF Nikkor 50mm f/1.4D"},
{0x44,0x44,0x60,0x80,0x34,0x3C,0x47,0x02, "Nikon", "AF Nikkor 80-200mm f/4.5-5.6D"},
{0x45,0x40,0x3C,0x60,0x2C,0x3C,0x48,0x02, "Nikon", "AF Zoom-Nikkor 28-80mm F/3.5-5.6D"},
{0x46,0x3C,0x44,0x60,0x30,0x3C,0x49,0x02, "Nikon", "AF Zoom-Nikkor 35-80mm f/4-5.6D N"},
{0x47,0x42,0x37,0x50,0x2A,0x34,0x4A,0x02, "Nikon", "AF Zoom-Nikkor 24-50mm f/3.3-4.5D"},
{0x48,0x48,0x8E,0x8E,0x24,0x24,0x4B,0x02, "Nikon", "AF-S Nikkor 300mm f/2.8D IF-ED"},
{0x49,0x3C,0xA6,0xA6,0x30,0x30,0x4C,0x02, "Nikon", "AF-S Nikkor 600mm f/4D IF-ED"},
{0x49,0x3C,0xA6,0xA6,0x30,0x30,0xF1,0x02, "Nikon", "AF-S Nikkor 600mm f/4D IF-ED + TC-14E"},
{0x49,0x3C,0xA6,0xA6,0x30,0x30,0xF2,0x02, "Nikon", "AF-S Nikkor 600mm f/4D IF-ED + TC-20E"},
{0x4A,0x54,0x62,0x62,0x0C,0x0C,0x4D,0x02, "Nikon", "AF Nikkor 85mm f/1.4D IF"},
{0x4B,0x3C,0xA0,0xA0,0x30,0x30,0x4E,0x02, "Nikon", "AF-S Nikkor 500mm f/4D IF-ED"},
{0x4B,0x3C,0xA0,0xA0,0x30,0x30,0xF1,0x02, "Nikon", "AF-S Nikkor 500mm f/4D IF-ED + TC-14E"},
{0x4B,0x3C,0xA0,0xA0,0x30,0x30,0xF2,0x02, "Nikon", "AF-S Nikkor 500mm f/4D IF-ED + TC-20E"},
{0x4C,0x40,0x37,0x6E,0x2C,0x3C,0x4F,0x02, "Nikon", "AF Zoom-Nikkor 24-120mm f/3.5-5.6D IF"},
{0x4D,0x40,0x3C,0x80,0x2C,0x3C,0x62,0x02, "Nikon", "AF Zoom-Nikkor 28-200mm f/3.5-5.6D IF"},
{0x4E,0x48,0x72,0x72,0x18,0x18,0x51,0x02, "Nikon", "AF DC-Nikkor 135mm f/2D"},
{0x4F,0x40,0x37,0x5C,0x2C,0x3C,0x53,0x06, "Nikon", "IX-Nikkor 24-70mm f/3.5-5.6"},
//#50
//#51
//#52
{0x53,0x48,0x60,0x80,0x24,0x24,0x60,0x02, "Nikon", "AF Zoom-Nikkor 80-200mm f/2.8D ED"},
{0x54,0x44,0x5C,0x7C,0x34,0x3C,0x58,0x02, "Nikon", "AF Zoom-Micro Nikkor 70-180mm f/4.5-5.6D ED"},
//#55
{0x56,0x48,0x5C,0x8E,0x30,0x3C,0x5A,0x02, "Nikon", "AF Zoom-Nikkor 70-300mm f/4-5.6D ED"},
//#57
//#58
{0x59,0x48,0x98,0x98,0x24,0x24,0x5D,0x02, "Nikon", "AF-S Nikkor 400mm f/2.8D IF-ED"},
{0x5A,0x3C,0x3E,0x56,0x30,0x3C,0x5E,0x06, "Nikon", "IX-Nikkor 30-60mm f/4-5.6"},
{0x5B,0x44,0x56,0x7C,0x34,0x3C,0x5F,0x06, "Nikon", "IX-Nikkor 60-180mm f/4.5-5.6"},
//#5C
{0x5D,0x48,0x3C,0x5C,0x24,0x24,0x63,0x02, "Nikon", "AF-S Zoom-Nikkor 28-70mm f/2.8D IF-ED"},
{0x5E,0x48,0x60,0x80,0x24,0x24,0x64,0x02, "Nikon", "AF-S Zoom-Nikkor 80-200mm f/2.8D IF-ED"},
{0x5F,0x40,0x3C,0x6A,0x2C,0x34,0x65,0x02, "Nikon", "AF Zoom-Nikkor 28-105mm f/3.5-4.5D IF"},
{0x60,0x40,0x3C,0x60,0x2C,0x3C,0x66,0x02, "Nikon", "AF Zoom-Nikkor 28-80mm f/3.5-5.6D"},
{0x61,0x44,0x5E,0x86,0x34,0x3C,0x67,0x02, "Nikon", "AF Zoom-Nikkor 75-240mm f/4.5-5.6D"},
//#62
{0x63,0x48,0x2B,0x44,0x24,0x24,0x68,0x02, "Nikon", "AF-S Nikkor 17-35mm f/2.8D IF-ED"},
{0x64,0x00,0x62,0x62,0x24,0x24,0x6A,0x02, "Nikon", "PC Micro-Nikkor 85mm f/2.8D"},
{0x65,0x44,0x60,0x98,0x34,0x3C,0x6B,0x0A, "Nikon", "AF VR Zoom-Nikkor 80-400mm f/4.5-5.6D ED"},
{0x66,0x40,0x2D,0x44,0x2C,0x34,0x6C,0x02, "Nikon", "AF Zoom-Nikkor 18-35mm f/3.5-4.5D IF-ED"},
{0x67,0x48,0x37,0x62,0x24,0x30,0x6D,0x02, "Nikon", "AF Zoom-Nikkor 24-85mm f/2.8-4D IF"},
{0x68,0x42,0x3C,0x60,0x2A,0x3C,0x6E,0x06, "Nikon", "AF Zoom-Nikkor 28-80mm f/3.3-5.6G"},
{0x69,0x48,0x5C,0x8E,0x30,0x3C,0x6F,0x06, "Nikon", "AF Zoom-Nikkor 70-300mm f/4-5.6G"},
{0x6A,0x48,0x8E,0x8E,0x30,0x30,0x70,0x02, "Nikon", "AF-S Nikkor 300mm f/4D IF-ED"},
{0x6B,0x48,0x24,0x24,0x24,0x24,0x71,0x02, "Nikon", "AF Nikkor ED 14mm f/2.8D"},
//#6C                72
{0x6D,0x48,0x8E,0x8E,0x24,0x24,0x73,0x02, "Nikon", "AF-S Nikkor 300mm f/2.8D IF-ED II"},
{0x6E,0x48,0x98,0x98,0x24,0x24,0x74,0x02, "Nikon", "AF-S Nikkor 400mm f/2.8D IF-ED II"},
{0x6F,0x3C,0xA0,0xA0,0x30,0x30,0x75,0x02, "Nikon", "AF-S Nikkor 500mm f/4D IF-ED II"},
{0x70,0x3C,0xA6,0xA6,0x30,0x30,0x76,0x02, "Nikon", "AF-S Nikkor 600mm f/4D IF-ED II"},
//#71
{0x72,0x48,0x4C,0x4C,0x24,0x24,0x77,0x00, "Nikon", "Nikkor 45mm f/2.8 P"},
//#73
{0x74,0x40,0x37,0x62,0x2C,0x34,0x78,0x06, "Nikon", "AF-S Zoom-Nikkor 24-85mm f/3.5-4.5G IF-ED"},
{0x75,0x40,0x3C,0x68,0x2C,0x3C,0x79,0x06, "Nikon", "AF Zoom-Nikkor 28-100mm f/3.5-5.6G"},
{0x76,0x58,0x50,0x50,0x14,0x14,0x7A,0x02, "Nikon", "AF Nikkor 50mm f/1.8D"},
{0x77,0x48,0x5C,0x80,0x24,0x24,0x7B,0x0E, "Nikon", "AF-S VR Zoom-Nikkor 70-200mm f/2.8G IF-ED"},
{0x78,0x40,0x37,0x6E,0x2C,0x3C,0x7C,0x0E, "Nikon", "AF-S VR Zoom-Nikkor 24-120mm f/3.5-5.6G IF-ED"},
{0x79,0x40,0x3C,0x80,0x2C,0x3C,0x7F,0x06, "Nikon", "AF Zoom-Nikkor 28-200mm f/3.5-5.6G IF-ED"},
{0x7A,0x3C,0x1F,0x37,0x30,0x30,0x7E,0x06, "Nikon", "AF-S DX Zoom-Nikkor 12-24mm f/4G IF-ED"},
{0x7B,0x48,0x80,0x98,0x30,0x30,0x80,0x0E, "Nikon", "AF-S VR Zoom-Nikkor 200-400mm f/4G IF-ED"},
//#7C                81
{0x7D,0x48,0x2B,0x53,0x24,0x24,0x82,0x06, "Nikon", "AF-S DX Zoom-Nikkor 17-55mm f/2.8G IF-ED"},
//#7E                83
{0x7F,0x40,0x2D,0x5C,0x2C,0x34,0x84,0x06, "Nikon", "AF-S DX Zoom-Nikkor 18-70mm f/3.5-4.5G IF-ED"},
{0x80,0x48,0x1A,0x1A,0x24,0x24,0x85,0x06, "Nikon", "AF DX Fisheye-Nikkor 10.5mm f/2.8G ED"},
{0x81,0x54,0x80,0x80,0x18,0x18,0x86,0x0E, "Nikon", "AF-S VR Nikkor 200mm f/2G IF-ED"},
{0x82,0x48,0x8E,0x8E,0x24,0x24,0x87,0x0E, "Nikon", "AF-S VR Nikkor 300mm f/2.8G IF-ED"},
//#83
//#84
//#85
//#86
//#87
//#88
{0x89,0x3C,0x53,0x80,0x30,0x3C,0x8B,0x06, "Nikon", "AF-S DX Zoom-Nikkor 55-200mm f/4-5.6G ED"},
{0x8A,0x54,0x6A,0x6A,0x24,0x24,0x8C,0x0E, "Nikon", "AF-S VR Micro-Nikkor 105mm f/2.8G IF-ED"},
{0x8B,0x40,0x2D,0x80,0x2C,0x3C,0xFD,0x0E, "Nikon", "AF-S DX VR Zoom-Nikkor 18-200mm f/3.5-5.6G IF-ED"},
{0x8B,0x40,0x2D,0x80,0x2C,0x3C,0x8D,0x0E, "Nikon", "AF-S DX VR Zoom-Nikkor 18-200mm f/3.5-5.6G IF-ED"},
{0x8C,0x40,0x2D,0x53,0x2C,0x3C,0x8E,0x06, "Nikon", "AF-S DX Zoom-Nikkor 18-55mm f/3.5-5.6G ED"},
{0x8D,0x44,0x5C,0x8E,0x34,0x3C,0x8F,0x0E, "Nikon", "AF-S VR Zoom-Nikkor 70-300mm f/4.5-5.6G IF-ED"},
//#8E                90
{0x8F,0x40,0x2D,0x72,0x2C,0x3C,0x91,0x06, "Nikon", "AF-S DX Zoom-Nikkor 18-135mm f/3.5-5.6G IF-ED"},
{0x90,0x3B,0x53,0x80,0x30,0x3C,0x92,0x0E, "Nikon", "AF-S DX VR Zoom-Nikkor 55-200mm f/4-5.6G IF-ED"},
//#91                93
{0x92,0x48,0x24,0x37,0x24,0x24,0x94,0x06, "Nikon", "AF-S Zoom-Nikkor 14-24mm f/2.8G ED"},
{0x93,0x48,0x37,0x5C,0x24,0x24,0x95,0x06, "Nikon", "AF-S Zoom-Nikkor 24-70mm f/2.8G ED"},
{0x94,0x40,0x2D,0x53,0x2C,0x3C,0x96,0x06, "Nikon", "AF-S DX Zoom-Nikkor 18-55mm f/3.5-5.6G ED II"},
{0x95,0x4C,0x37,0x37,0x2C,0x2C,0x97,0x02, "Nikon", "PC-E Nikkor 24mm f/3.5D ED"},
{0x95,0x00,0x37,0x37,0x2C,0x2C,0x97,0x06, "Nikon", "PC-E Nikkor 24mm f/3.5D ED"},
{0x96,0x48,0x98,0x98,0x24,0x24,0x98,0x0E, "Nikon", "AF-S VR Nikkor 400mm f/2.8G ED"},
{0x97,0x3C,0xA0,0xA0,0x30,0x30,0x99,0x0E, "Nikon", "AF-S VR Nikkor 500mm f/4G ED"},
{0x98,0x3C,0xA6,0xA6,0x30,0x30,0x9A,0x0E, "Nikon", "AF-S VR Nikkor 600mm f/4G ED"},
{0x99,0x40,0x29,0x62,0x2C,0x3C,0x9B,0x0E, "Nikon", "AF-S DX VR Zoom-Nikkor 16-85mm f/3.5-5.6G ED"},
{0x9A,0x40,0x2D,0x53,0x2C,0x3C,0x9C,0x0E, "Nikon", "AF-S DX VR Zoom-Nikkor 18-55mm f/3.5-5.6G"},
{0x9B,0x54,0x4C,0x4C,0x24,0x24,0x9D,0x02, "Nikon", "PC-E Micro Nikkor 45mm f/2.8D ED"},
{0x9B,0x00,0x4C,0x4C,0x24,0x24,0x9D,0x06, "Nikon", "PC-E Micro Nikkor 45mm f/2.8D ED"},
{0x9C,0x54,0x56,0x56,0x24,0x24,0x9E,0x06, "Nikon", "AF-S Micro Nikkor 60mm f/2.8G ED"},
{0x9D,0x54,0x62,0x62,0x24,0x24,0x9F,0x02, "Nikon", "PC-E Micro Nikkor 85mm f/2.8D"},
{0x9D,0x00,0x62,0x62,0x24,0x24,0x9F,0x06, "Nikon", "PC-E Micro Nikkor 85mm f/2.8D"},
{0x9E,0x40,0x2D,0x6A,0x2C,0x3C,0xA0,0x0E, "Nikon", "AF-S DX VR Zoom-Nikkor 18-105mm f/3.5-5.6G ED"},
//#9F                A1
{0xA0,0x54,0x50,0x50,0x0C,0x0C,0xA2,0x06, "Nikon", "AF-S Nikkor 50mm f/1.4G"},
//#
//#------------------------------------------------------------------------------------------------
//# Sigma lenses by focal length, first fixed then zoom lenses
//#------------------------------------------------------------------------------------------------
//#
{0xFE,0x47,0x00,0x00,0x24,0x24,0x4B,0x06, "Sigma", "4.5mm F2.8 EX DC HSM Circular Fisheye"},
{0x26,0x48,0x11,0x11,0x30,0x30,0x1C,0x02, "Sigma", "8mm F4 EX Circular Fisheye"},
{0x79,0x40,0x11,0x11,0x2C,0x2C,0x1C,0x06, "Sigma", "8mm F3.5 EX Circular Fisheye"},
{0xDC,0x48,0x19,0x19,0x24,0x24,0x4B,0x06, "Sigma", "10mm F2.8 EX DC HSM Fisheye"},
{0x02,0x3F,0x24,0x24,0x2C,0x2C,0x02,0x00, "Sigma", "14mm F3.5"},
{0x48,0x48,0x24,0x24,0x24,0x24,0x4B,0x02, "Sigma", "14mm F2.8 EX Aspherical HSM"},
{0x26,0x48,0x27,0x27,0x24,0x24,0x1C,0x02, "Sigma", "15mm F2.8 EX Diagonal Fisheye"},
{0x26,0x58,0x31,0x31,0x14,0x14,0x1C,0x02, "Sigma", "20mm F1.8 EX DG Aspherical RF"},
{0x26,0x58,0x37,0x37,0x14,0x14,0x1C,0x02, "Sigma", "24mm F1.8 EX DG Aspherical MACRO"},
{0x02,0x46,0x37,0x37,0x25,0x25,0x02,0x00, "Sigma", "24mm F2.8 Macro"},
{0x26,0x58,0x3C,0x3C,0x14,0x14,0x1C,0x02, "Sigma", "28mm F1.8 EX DG DF"},
{0x48,0x54,0x3E,0x3E,0x0C,0x0C,0x4B,0x06, "Sigma", "30mm F1.4 EX DC HSM"},
{0xF8,0x54,0x3E,0x3E,0x0C,0x0C,0x4B,0x06, "Sigma", "30mm F1.4 EX DC HSM"},
{0xDE,0x54,0x50,0x50,0x0C,0x0C,0x4B,0x06, "Sigma", "50mm F1.4 EX DG HSM"},
{0x32,0x54,0x50,0x50,0x24,0x24,0x35,0x02, "Sigma", "Macro 50mm F2.8 EX DG"},
{0x79,0x48,0x5C,0x5C,0x24,0x24,0x1C,0x06, "Sigma", "Macro 70mm F2.8 EX DG"},
{0x02,0x48,0x65,0x65,0x24,0x24,0x02,0x00, "Sigma", "90mm F2.8 Macro"},
//#"32 54 6A 6A 24 24 35" "02" "Sigma" "Macro 105mm F2.8 EX DG";
{0xE5,0x54,0x6A,0x6A,0x24,0x24,0x35,0x02, "Sigma", "Macro 105mm F2.8 EX DG"},
{0x48,0x48,0x76,0x76,0x24,0x24,0x4B,0x06, "Sigma", "APO Macro 150mm F2.8 EX DG HSM"},
{0xF5,0x48,0x76,0x76,0x24,0x24,0x4B,0x06, "Sigma", "APO Macro 150mm F2.8 EX DG HSM"},
{0x48,0x4C,0x7C,0x7C,0x2C,0x2C,0x4B,0x02, "Sigma", "180mm F3.5 EX DG Macro"},
{0x48,0x4C,0x7D,0x7D,0x2C,0x2C,0x4B,0x02, "Sigma", "APO MACRO 180mm F3.5 EX DG HSM"},
{0x48,0x54,0x8E,0x8E,0x24,0x24,0x4B,0x02, "Sigma", "APO 300mm F2.8 EX DG HSM"},
{0xFB,0x54,0x8E,0x8E,0x24,0x24,0x4B,0x02, "Sigma", "APO 300mm F2.8 EX DG HSM"},
{0x26,0x48,0x8E,0x8E,0x30,0x30,0x1C,0x02, "Sigma", "APO TELE MACRO 300mm F4"},
{0x02,0x2F,0x98,0x98,0x3D,0x3D,0x02,0x00, "Sigma", "400mm F5.6 APO"},
{0x02,0x37,0xA0,0xA0,0x34,0x34,0x02,0x00, "Sigma", "APO 500mm F4.5"},
{0x48,0x44,0xA0,0xA0,0x34,0x34,0x4B,0x02, "Sigma", "APO 500mm F4.5 EX HSM"},
//#
{0x48,0x3C,0x19,0x31,0x30,0x3C,0x4B,0x06, "Sigma", "10-20mm F4-5.6 EX DC HSM"},
{0xF9,0x3C,0x19,0x31,0x30,0x3C,0x4B,0x06, "Sigma", "10-20mm F4-5.6 EX DC HSM"},
{0x48,0x38,0x1F,0x37,0x34,0x3C,0x4B,0x06, "Sigma", "12-24mm F4.5-5.6 EX Aspherical DG HSM"},
{0x26,0x40,0x27,0x3F,0x2C,0x34,0x1C,0x02, "Sigma", "15-30mm F3.5-4.5 EX Aspherical DG DF"},
{0x48,0x48,0x2B,0x44,0x24,0x30,0x4B,0x06, "Sigma", "17-35mm F2.8-4 EX DG  Aspherical HSM"},
{0x26,0x54,0x2B,0x44,0x24,0x30,0x1C,0x02, "Sigma", "17-35mm F2.8-4 EX Aspherical"},
{0x7A,0x47,0x2B,0x5C,0x24,0x34,0x4B,0x06, "Sigma", "17-70mm F2.8-4.5 DC Macro Asp. IF HSM"},
{0x7F,0x48,0x2B,0x5C,0x24,0x34,0x1C,0x06, "Sigma", "17-70mm F2.8-4.5 DC Macro Asp. IF"},
{0x26,0x40,0x2D,0x44,0x2B,0x34,0x1C,0x02, "Sigma", "18-35 F3.5-4.5 Aspherical"},
{0x26,0x48,0x2D,0x50,0x24,0x24,0x1C,0x06, "Sigma", "18-50mm F2.8 EX DC"},
{0x7F,0x48,0x2D,0x50,0x24,0x24,0x1C,0x06, "Sigma", "18-50mm F2.8 EX DC MACRO"},
{0x7A,0x48,0x2D,0x50,0x24,0x24,0x4B,0x06, "Sigma", "18-50mm F2.8 EX DC HSM"},
{0x26,0x40,0x2D,0x50,0x2C,0x3C,0x1C,0x06, "Sigma", "18-50mm F3.5-5.6 DC"},
{0x7A,0x40,0x2D,0x50,0x2C,0x3C,0x4B,0x06, "Sigma", "18-50mm F3.5-5.6 DC HSM"},
{0x26,0x40,0x2D,0x70,0x2B,0x3C,0x1C,0x06, "Sigma", "18-125mm F3.5-5.6 DC"},
{0xCD,0x3D,0x2D,0x70,0x2E,0x3C,0x4B,0x06, "Sigma", "18-125mm F3.8-5.6 DC OS HSM"},
{0x26,0x40,0x2D,0x80,0x2C,0x40,0x1C,0x06, "Sigma", "18-200mm F3.5-6.3 DC"},
{0xED,0x40,0x2D,0x80,0x2C,0x40,0x4B,0x0E, "Sigma", "18-200mm F3.5-6.3 DC OS HSM"},
{0x26,0x48,0x31,0x49,0x24,0x24,0x1C,0x02, "Sigma", "20-40mm F2.8"},
{0x26,0x48,0x37,0x56,0x24,0x24,0x1C,0x02, "Sigma", "24-60mm F2.8 EX DG"},
{0xB6,0x48,0x37,0x56,0x24,0x24,0x1C,0x02, "Sigma", "24-60mm F2.8 EX DG"},
{0x26,0x54,0x37,0x5C,0x24,0x24,0x1C,0x02, "Sigma", "24-70mm F2.8 EX DG Macro"},
{0x67,0x54,0x37,0x5C,0x24,0x24,0x1C,0x02, "Sigma", "24-70mm F2.8 EX DG Macro"},
{0x26,0x40,0x37,0x5C,0x2C,0x3C,0x1C,0x02, "Sigma", "24-70mm F3.5-5.6 Aspherical HF"},
{0x26,0x54,0x37,0x73,0x24,0x34,0x1C,0x02, "Sigma", "24-135mm F2.8-4.5"},
{0x02,0x46,0x3C,0x5C,0x25,0x25,0x02,0x00, "Sigma", "28-70mm F2.8"},
{0x26,0x54,0x3C,0x5C,0x24,0x24,0x1C,0x02, "Sigma", "28-70mm F2.8 EX"},
{0x26,0x48,0x3C,0x5C,0x24,0x24,0x1C,0x06, "Sigma", "28-70mm F2.8 EX DG"},
{0x26,0x48,0x3C,0x5C,0x24,0x30,0x1C,0x02, "Sigma", "28-70mm F2.8-4"},
{0x02,0x3F,0x3C,0x5C,0x2D,0x35,0x02,0x00, "Sigma", "28-70mm F3.5-4.5 UC"},
{0x26,0x40,0x3C,0x60,0x2C,0x3C,0x1C,0x02, "Sigma", "28-80mm F3.5-5.6 Mini Zoom Macro II Aspherical"},
{0x26,0x40,0x3C,0x65,0x2C,0x3C,0x1C,0x02, "Sigma", "28-90mm F3.5-5.6 Macro"},
{0x26,0x48,0x3C,0x6A,0x24,0x30,0x1C,0x02, "Sigma", "28-105mm F2.8-4 Aspherical"},
{0x26,0x3E,0x3C,0x6A,0x2E,0x3C,0x1C,0x02, "Sigma", "28-105mm F3.8-5.6 UC-III Aspherical IF"},
{0x26,0x40,0x3C,0x80,0x2C,0x3C,0x1C,0x02, "Sigma", "28-200mm F3.5-5.6 Compact Aspherical Hyperzoom Macro"},
{0x26,0x40,0x3C,0x80,0x2B,0x3C,0x1C,0x02, "Sigma", "28-200mm F3.5-5.6 Compact Aspherical Hyperzoom Macro"},
{0x26,0x41,0x3C,0x8E,0x2C,0x40,0x1C,0x02, "Sigma", "28-300mm F3.5-6.3 DG Macro"},
{0x26,0x40,0x3C,0x8E,0x2C,0x40,0x1C,0x02, "Sigma", "28-300mm F3.5-6.3 Macro"},
{0x02,0x40,0x44,0x73,0x2B,0x36,0x02,0x00, "Sigma", "35-135mm F3.5-4.5 a"},
{0x7A,0x47,0x50,0x76,0x24,0x24,0x4B,0x06, "Sigma", "50-150mm F2.8 EX APO DC HSM"},
{0xFD,0x47,0x50,0x76,0x24,0x24,0x4B,0x06, "Sigma", "50-150mm F2.8 EX APO DC HSM II"},
{0x48,0x3C,0x50,0xA0,0x30,0x40,0x4B,0x02, "Sigma", "APO 50-500mm F4-6.3 EX HSM"},
{0x26,0x3C,0x54,0x80,0x30,0x3C,0x1C,0x06, "Sigma", "55-200mm F4-5.6 DC"},
{0x7A,0x3B,0x53,0x80,0x30,0x3C,0x4B,0x06, "Sigma", "55-200mm F4-5.6 DC HSM"},
{0x48,0x54,0x5C,0x80,0x24,0x24,0x4B,0x02, "Sigma", "70-200mm F2.8 EX APO IF HSM"},
{0xEE,0x48,0x5C,0x80,0x24,0x24,0x4B,0x06, "Sigma", "70-200mm F2.8 EX APO DG MACRO HSM II"},
{0x02,0x46,0x5C,0x82,0x25,0x25,0x02,0x00, "Sigma", "70-210mm F2.8 APO"},
{0x26,0x3C,0x5C,0x82,0x30,0x3C,0x1C,0x02, "Sigma", "70-210mm F4-5.6 UC-II"},
{0x26,0x3C,0x5C,0x8E,0x30,0x3C,0x1C,0x02, "Sigma", "70-300mm F4-5.6 DG Macro"},
{0x56,0x3C,0x5C,0x8E,0x30,0x3C,0x1C,0x02, "Sigma", "70-300mm F4-5.6 APO Macro Super II"},
{0xE0,0x3C,0x5C,0x8E,0x30,0x3C,0x4B,0x06, "Sigma", "70-300mm F4-5.6 APO DG Macro HSM"},
{0x02,0x37,0x5E,0x8E,0x35,0x3D,0x02,0x00, "Sigma", "75-300mm F4.5-5.6 APO"},
{0x02,0x3A,0x5E,0x8E,0x32,0x3D,0x02,0x00, "Sigma", "75-300mm F4.0-5.6"},
{0x77,0x44,0x61,0x98,0x34,0x3C,0x7B,0x0E, "Sigma", "80-400mm f4.5-5.6 EX OS"},
{0x48,0x48,0x68,0x8E,0x30,0x30,0x4B,0x02, "Sigma", "APO 100-300mm F4 EX IF HSM"},
{0x48,0x54,0x6F,0x8E,0x24,0x24,0x4B,0x02, "Sigma", "APO 120-300mm F2.8 EX DG HSM"},
{0xCF,0x38,0x6E,0x98,0x34,0x3C,0x4B,0x0E, "Sigma", "APO 120-400mm F4.5-5.6 DG OS HSM"},
{0x26,0x44,0x73,0x98,0x34,0x3C,0x1C,0x02, "Sigma", "135-400mm F4.5-5.6 APO Aspherical"},
{0xCE,0x34,0x76,0xA0,0x38,0x40,0x4B,0x0E, "Sigma", "APO 150-500 mm F5-6.3 DG OS HSM"},
{0x26,0x40,0x7B,0xA0,0x34,0x40,0x1C,0x02, "Sigma", "APO 170-500mm F5-6.3 Aspherical RF"},
{0x48,0x3C,0x8E,0xB0,0x3C,0x3C,0x4B,0x02, "Sigma", "APO 300-800 F5.6 EX DG HSM"},
{0x48,0x3C,0xB0,0xB0,0x3C,0x3C,0x4B,0x02, "Sigma", "APO 800mm f/5.6 EX HSM"},
//#
//#------------------------------------------------------------------------------------------------
//# Tamron lenses by focal length, first fixed then zoom lenses
//#------------------------------------------------------------------------------------------------
//#
//#"1E 5D 64 64 20 20 13" "00" "Tamron" "SP AF 90mm F/2.5 (52E)";
{0x32,0x53,0x64,0x64,0x24,0x24,0x35,0x02, "Tamron", "SP AF 90mm F/2.8 Di Macro 1:1 (272E)"},
{0xF8,0x55,0x64,0x64,0x24,0x24,0x84,0x06, "Tamron", "SP AF 90mm F/2.8 Di MACRO 1:1"},
{0x00,0x4C,0x7C,0x7C,0x2C,0x2C,0x00,0x02, "Tamron", "SP AF 180mm F3.5 Di Model B01"},
//#
{0xF6,0x3F,0x18,0x37,0x2C,0x34,0x84,0x06, "Tamron", "SP AF 10-24mm F/3.5-4.5 Di II LD Aspherical (IF)"},
{0x00,0x36,0x1C,0x2D,0x34,0x3C,0x00,0x06, "Tamron", "SP AF 11-18mm F/4.5-5.6 Di II LD Aspherical (IF)"},
{0x07,0x46,0x2B,0x44,0x24,0x30,0x03,0x02, "Tamron", "SP AF 17-35mm F/2.8-4 Di LD Aspherical (IF)"},
{0x00,0x53,0x2B,0x50,0x24,0x24,0x00,0x06, "Tamron", "SP AF 17-50mm F2.8 (A16)"},
{0x00,0x54,0x2B,0x50,0x24,0x24,0x00,0x06, "Tamron", "SP AF 17-50mm F/2.8 XR Di II LD Aspherical [IF] (A16NII)"},
{0x00,0x3F,0x2D,0x80,0x2B,0x40,0x00,0x06, "Tamron", "AF 18-200mm F/3.5-6.3 XR Di II LD Aspherical (IF)"},
{0x00,0x3F,0x2D,0x80,0x2C,0x40,0x00,0x06, "Tamron", "AF 18-200mm F/3.5-6.3 XR Di II LD Aspherical (IF) Macro"},
{0x00,0x40,0x2D,0x88,0x2C,0x40,0x62,0x06, "Tamron", "AF 18-250mm F/3.5-6.3 Di II LD Aspherical (IF) Macro"},
{0x00,0x40,0x2D,0x88,0x2C,0x40,0x00,0x06, "Tamron", "AF 18-250mm F/3.5-6.3 Di II LD Aspherical (IF) Macro (A18NII)"},
{0xF5,0x40,0x2C,0x8A,0x2C,0x40,0x40,0x0E, "Tamron", "AF 18-270mm F/3.5-6.3 Di II VC LD Aspherical [IF] Macro"},
{0x07,0x40,0x2F,0x44,0x2C,0x34,0x03,0x02, "Tamron", "AF 19-35mm F/3.5-4.5 N"},
{0x07,0x40,0x30,0x45,0x2D,0x35,0x03,0x02, "Tamron", "AF 19-35mm F/3.5-4.5"},
{0x00,0x49,0x30,0x48,0x22,0x2B,0x00,0x02, "Tamron", "SP AF 20-40mm F/2.7-3.5"},
{0x0E,0x4A,0x31,0x48,0x23,0x2D,0x0E,0x02, "Tamron", "SP AF 20-40mm F/2.7-3.5"},
{0x45,0x41,0x37,0x72,0x2C,0x3C,0x48,0x02, "Tamron", "SP AF 24-135mm F/3.5-5.6 AD Aspherical (IF) Macro"},
{0x33,0x54,0x3C,0x5E,0x24,0x24,0x62,0x02, "Tamron", "SP AF 28-75mm F/2.8 XR Di LD Aspherical (IF) Macro"},
{0xFA,0x54,0x3C,0x5E,0x24,0x24,0x84,0x06, "Tamron", "SP AF 28-75mm F/2.8 XR Di LD Aspherical [IF] MACRO"},
{0x10,0x3D,0x3C,0x60,0x2C,0x3C,0xD2,0x02, "Tamron", "AF 28-80mm F/3.5-5.6 Aspherical"},
{0x45,0x3D,0x3C,0x60,0x2C,0x3C,0x48,0x02, "Tamron", "AF 28-80mm F/3.5-5.6 Aspherical"},
{0x00,0x48,0x3C,0x6A,0x24,0x24,0x00,0x02, "Tamron", "SP AF 28-105mm f/2.8"},
{0x0B,0x3E,0x3D,0x7F,0x2F,0x3D,0x0E,0x02, "Tamron", "AF 28-200mm F/3.8-5.6D"},
{0x0B,0x3E,0x3D,0x7F,0x2F,0x3D,0x0E,0x00, "Tamron", "AF 28-200mm F/3.8-5.6"},
{0x4D,0x41,0x3C,0x8E,0x2B,0x40,0x62,0x02, "Tamron", "AF 28-300mm F/3.5-6.3 XR Di LD Aspherical (IF)"},
{0x4D,0x41,0x3C,0x8E,0x2C,0x40,0x62,0x02, "Tamron", "AF 28-300mm F/3.5-6.3 XR LD Aspherical (IF)"},
{0xF9,0x40,0x3C,0x8E,0x2C,0x40,0x40,0x0E, "Tamron", "AF 28-300mm F/3.5-6.3 XR Di VC LD Aspherical [IF] MACRO"},
{0x00,0x47,0x53,0x80,0x30,0x3C,0x00,0x06, "Tamron", "AF 55-200mm F/4-5.6 Di II LD"},
{0xF7,0x53,0x5C,0x80,0x24,0x24,0x84,0x06, "Tamron", "SP AF 70-200mm F/2.8 Di LD [IF] MACRO"},
{0x69,0x48,0x5C,0x8E,0x30,0x3C,0x6F,0x02, "Tamron", "AF 70-300mm F/4-5.6 LD Macro 1:2"},
{0x00,0x48,0x5C,0x8E,0x30,0x3C,0x00,0x06, "Tamron", "AF 70-300mm F/4-5.6 Di LD MACRO 1:2"},
{0x20,0x3C,0x80,0x98,0x3D,0x3D,0x1E,0x02, "Tamron", "AF 200-400mm F/5.6 LD IF"},
{0x00,0x3E,0x80,0xA0,0x38,0x3F,0x00,0x02, "Tamron", "SP AF 200-500mm F/5-6.3 Di LD (IF)"},
{0x00,0x3F,0x80,0xA0,0x38,0x3F,0x00,0x02, "Tamron", "SP AF 200-500mm F/5-6.3 Di"},
//#
//#------------------------------------------------------------------------------------------------
//# Tokina Lenses by focal length, first fixed then zoom lenses
//#------------------------------------------------------------------------------------------------
//#
{0x00,0x40,0x2B,0x2B,0x2C,0x2C,0x00,0x02, "Tokina", "AT-X 17 AF PRO / AF 17mm f/3.5"},
{0x00,0x47,0x44,0x44,0x24,0x24,0x00,0x06, "Tokina", "AT-X M35 PRO DX / 35mm f/2.8"},
{0x00,0x54,0x68,0x68,0x24,0x24,0x00,0x02, "Tokina", "AT-X M100 PRO D / 100mm f/2.8"},
{0x00,0x54,0x8E,0x8E,0x24,0x24,0x00,0x02, "Tokina", "AT-X 300 AF PRO / 300mm f/2.8"},
//#
{0x00,0x40,0x18,0x2B,0x2C,0x34,0x00,0x06, "Tokina", "AT-X 107 DX Fisheye / 10-17mm f/3.5-4.5"},
{0x00,0x48,0x1C,0x29,0x24,0x24,0x00,0x06, "Tokina", "AT-X 116 PRO DX / 11-16mm f/2.8"},
{0x00,0x3C,0x1F,0x37,0x30,0x30,0x00,0x06, "Tokina", "AT-X 124 AF PRO DX / 12-24mm f/4"},
//#"7A 3C 1F 37 30 30 7E" "06" "Tokina" "AT-X 124 AF PRO DX II / 12-24mm f/4";
{0x00,0x48,0x29,0x50,0x24,0x24,0x00,0x06, "Tokina", "AT-X 165 PRO DX / 16-50mm f/2.8"},
//#"2F 48 30 44 24 24 29" "02" "Tokina" "AT-X 235 AF PRO / 20-35mm f/2.8";
//#                            "Tokina" "AT-X 270 AF PRO II / 28-70mm f/2.6-2.8"
{0x25,0x48,0x3C,0x5C,0x24,0x24,0x1B,0x02, "Tokina", "AT-X 287 AF PRO SV / 28-70mm f/2.8"},
{0x07,0x48,0x3C,0x5C,0x24,0x24,0x03,0x00, "Tokina", "AT-X AF 28-70mm f/2.8"},
{0x00,0x48,0x3C,0x60,0x24,0x24,0x00,0x02, "Tokina", "AT-X 280 AF PRO / 28-80mm f/2.8 Aspherical"},
{0x00,0x48,0x50,0x72,0x24,0x24,0x00,0x06, "Tokina", "AT-X 535 PRO DX / 50-135mm f/2.8"},
{0x14,0x54,0x60,0x80,0x24,0x24,0x0B,0x00, "Tokina", "AT-X 828 AF / AF 80-200mm f/2.8"},
{0x24,0x44,0x60,0x98,0x34,0x3C,0x1A,0x02, "Tokina", "AT-X 840 AF II / AF 80-400mm f/4.5-5.6"},
{0x00,0x44,0x60,0x98,0x34,0x3C,0x00,0x02, "Tokina", "AT-X 840 D / 80-400mm f/4.5-5.6"},
{0x14,0x48,0x68,0x8E,0x30,0x30,0x0B,0x00, "Tokina", "AT-X 340 AF / 100-300mm f/4"},
//#
//#------------------------------------------------------------------------------------------------
//# Lenses from various other brands
//#------------------------------------------------------------------------------------------------
//#
{0x00,0x54,0x56,0x56,0x30,0x30,0x00,0x00, "Coastal Optical Systems", "60mm 1:4 UV-VIS-IR Macro Apo"},
{0x00,0x54,0x48,0x48,0x18,0x18,0x00,0x00, "Voigtlander", "Ultron SL2 40mm F/2 SL II Aspherical"},
{0x00,0x54,0x55,0x55,0x0C,0x0C,0x00,0x00, "Voigtlander", "Nokton SL2 58mm F/1.4 SL II"},
{0x07,0x3E,0x30,0x43,0x2D,0x35,0x03,0x00, "Soligor", "AF Zoom 19-35mm 1:3.5-4.5"},
{0x03,0x43,0x5C,0x81,0x35,0x35,0x02,0x00, "Soligor", "AF C/D Zoom UMCS 70-210mm 1:4.5"},
{0x07,0x36,0x3D,0x5F,0x2C,0x3C,0x03,0x00, "Cosina", "AF Zoom 28-80mm F/3.5-5.6 MC Macro"},
{0x12,0x36,0x5C,0x81,0x35,0x3D,0x09,0x00, "Cosina", "AF Zoom 70-210mm F/4.5-5.6 MC Macro"},
{0x06,0x3F,0x68,0x68,0x2C,0x2C,0x06,0x00, "Cosina", "AF 100mm F/3.5 Macro"},
{0x2F,0x40,0x30,0x44,0x2C,0x34,0x29,0x02, "Unknown", "20-35mm F/3.5-4.5D"},
{0x1E,0x5D,0x64,0x64,0x20,0x20,0x13,0x00, "Unknown", "90mm F/2.5"},
{0x12,0x3B,0x68,0x8D,0x3D,0x43,0x09,0x02, "Unknown", "100-290mm F5.6-6.7"},
//#
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01, "Manual Lens", "No CPU"},
//#
{0,0,0,0,0,0,0,0, NULL, NULL}
};

        if (value.typeId() != undefined) return os << value;

        DataBuf lens(value.size());
        // ByteOrder is only to satisfy the interface
        value.copy(lens.pData_, invalidByteOrder);

        int idx = 0;
        if (0 == memcmp(lens.pData_, "0100", 4)) {
            idx = 6;
        }
        else if (0 == memcmp(lens.pData_, "0101", 4)) {
            idx = 11;
        }
        else if (   0 == memcmp(lens.pData_, "0201", 4)
                 || 0 == memcmp(lens.pData_, "0202", 4)) {
            if (metadata) {
                // The decrypt algorithm requires access to serial number
                // and shutter count tags
                decryptNikonData(lens.pData_ + 4, lens.size_ - 4, *metadata);
            }
            idx = 11;
        }
        if (idx == 0 || lens.size_ < idx + 7) {
            // Unknown version or not enough data
            return os << value;
        }
        for (int i = 0; fmountlens[i].lensname != NULL; ++i) {
            if (   lens.pData_[idx]   == fmountlens[i].lid
                && lens.pData_[idx+1] == fmountlens[i].stps
                && lens.pData_[idx+2] == fmountlens[i].focs
                && lens.pData_[idx+3] == fmountlens[i].focl
                && lens.pData_[idx+4] == fmountlens[i].aps
                && lens.pData_[idx+5] == fmountlens[i].apl
                && lens.pData_[idx+6] == fmountlens[i].lfw) {
                // Lens found in database
                return os << fmountlens[i].manuf << " " << fmountlens[i].lensname;
            }
        }
        // Lens not found in database
        return os << value;
#else
        return os << value;
#endif // EXV_HAVE_LENSDATA
    }

}                                       // namespace Exiv2

// *****************************************************************************
// local definitions
namespace {

    int decryptNikonData(byte* pData, uint32_t size, const ExifData& exifData)
    {
        ExifKey key("Exif.Nikon3.ShutterCount");
        ExifData::const_iterator pos = exifData.findKey(key);
        if (pos == exifData.end() || pos->count() == 0) return 1;
        uint32_t count = static_cast<uint32_t>(pos->toLong());
        key = ExifKey("Exif.Nikon3.SerialNumber");
        pos = exifData.findKey(key);
        if (pos == exifData.end() || pos->count() == 0) return 2;
        bool ok(false);
        uint32_t serial = stringTo<uint32_t>(pos->toString(), ok);
        if (!ok) {
            key = ExifKey("Exif.Image.Model");
            pos = exifData.findKey(key);
            if (pos == exifData.end() || pos->count() == 0) return 3;
            std::string model = pos->toString();
            if (model.find("D50") != std::string::npos) {
                serial = 0x22;
            }
            else {
                serial = 0x60;
            }
        }
        decrypt(pData, size, count, serial);
        return 0;
    }

    void decrypt(byte* pData, uint32_t size, uint32_t count, uint32_t serial)
    {
        static const byte xlat[2][256] = {
            { 0xc1,0xbf,0x6d,0x0d,0x59,0xc5,0x13,0x9d,0x83,0x61,0x6b,0x4f,0xc7,0x7f,0x3d,0x3d,
              0x53,0x59,0xe3,0xc7,0xe9,0x2f,0x95,0xa7,0x95,0x1f,0xdf,0x7f,0x2b,0x29,0xc7,0x0d,
              0xdf,0x07,0xef,0x71,0x89,0x3d,0x13,0x3d,0x3b,0x13,0xfb,0x0d,0x89,0xc1,0x65,0x1f,
              0xb3,0x0d,0x6b,0x29,0xe3,0xfb,0xef,0xa3,0x6b,0x47,0x7f,0x95,0x35,0xa7,0x47,0x4f,
              0xc7,0xf1,0x59,0x95,0x35,0x11,0x29,0x61,0xf1,0x3d,0xb3,0x2b,0x0d,0x43,0x89,0xc1,
              0x9d,0x9d,0x89,0x65,0xf1,0xe9,0xdf,0xbf,0x3d,0x7f,0x53,0x97,0xe5,0xe9,0x95,0x17,
              0x1d,0x3d,0x8b,0xfb,0xc7,0xe3,0x67,0xa7,0x07,0xf1,0x71,0xa7,0x53,0xb5,0x29,0x89,
              0xe5,0x2b,0xa7,0x17,0x29,0xe9,0x4f,0xc5,0x65,0x6d,0x6b,0xef,0x0d,0x89,0x49,0x2f,
              0xb3,0x43,0x53,0x65,0x1d,0x49,0xa3,0x13,0x89,0x59,0xef,0x6b,0xef,0x65,0x1d,0x0b,
              0x59,0x13,0xe3,0x4f,0x9d,0xb3,0x29,0x43,0x2b,0x07,0x1d,0x95,0x59,0x59,0x47,0xfb,
              0xe5,0xe9,0x61,0x47,0x2f,0x35,0x7f,0x17,0x7f,0xef,0x7f,0x95,0x95,0x71,0xd3,0xa3,
              0x0b,0x71,0xa3,0xad,0x0b,0x3b,0xb5,0xfb,0xa3,0xbf,0x4f,0x83,0x1d,0xad,0xe9,0x2f,
              0x71,0x65,0xa3,0xe5,0x07,0x35,0x3d,0x0d,0xb5,0xe9,0xe5,0x47,0x3b,0x9d,0xef,0x35,
              0xa3,0xbf,0xb3,0xdf,0x53,0xd3,0x97,0x53,0x49,0x71,0x07,0x35,0x61,0x71,0x2f,0x43,
              0x2f,0x11,0xdf,0x17,0x97,0xfb,0x95,0x3b,0x7f,0x6b,0xd3,0x25,0xbf,0xad,0xc7,0xc5,
              0xc5,0xb5,0x8b,0xef,0x2f,0xd3,0x07,0x6b,0x25,0x49,0x95,0x25,0x49,0x6d,0x71,0xc7 },
            { 0xa7,0xbc,0xc9,0xad,0x91,0xdf,0x85,0xe5,0xd4,0x78,0xd5,0x17,0x46,0x7c,0x29,0x4c,
              0x4d,0x03,0xe9,0x25,0x68,0x11,0x86,0xb3,0xbd,0xf7,0x6f,0x61,0x22,0xa2,0x26,0x34,
              0x2a,0xbe,0x1e,0x46,0x14,0x68,0x9d,0x44,0x18,0xc2,0x40,0xf4,0x7e,0x5f,0x1b,0xad,
              0x0b,0x94,0xb6,0x67,0xb4,0x0b,0xe1,0xea,0x95,0x9c,0x66,0xdc,0xe7,0x5d,0x6c,0x05,
              0xda,0xd5,0xdf,0x7a,0xef,0xf6,0xdb,0x1f,0x82,0x4c,0xc0,0x68,0x47,0xa1,0xbd,0xee,
              0x39,0x50,0x56,0x4a,0xdd,0xdf,0xa5,0xf8,0xc6,0xda,0xca,0x90,0xca,0x01,0x42,0x9d,
              0x8b,0x0c,0x73,0x43,0x75,0x05,0x94,0xde,0x24,0xb3,0x80,0x34,0xe5,0x2c,0xdc,0x9b,
              0x3f,0xca,0x33,0x45,0xd0,0xdb,0x5f,0xf5,0x52,0xc3,0x21,0xda,0xe2,0x22,0x72,0x6b,
              0x3e,0xd0,0x5b,0xa8,0x87,0x8c,0x06,0x5d,0x0f,0xdd,0x09,0x19,0x93,0xd0,0xb9,0xfc,
              0x8b,0x0f,0x84,0x60,0x33,0x1c,0x9b,0x45,0xf1,0xf0,0xa3,0x94,0x3a,0x12,0x77,0x33,
              0x4d,0x44,0x78,0x28,0x3c,0x9e,0xfd,0x65,0x57,0x16,0x94,0x6b,0xfb,0x59,0xd0,0xc8,
              0x22,0x36,0xdb,0xd2,0x63,0x98,0x43,0xa1,0x04,0x87,0x86,0xf7,0xa6,0x26,0xbb,0xd6,
              0x59,0x4d,0xbf,0x6a,0x2e,0xaa,0x2b,0xef,0xe6,0x78,0xb6,0x4e,0xe0,0x2f,0xdc,0x7c,
              0xbe,0x57,0x19,0x32,0x7e,0x2a,0xd0,0xb8,0xba,0x29,0x00,0x3c,0x52,0x7d,0xa8,0x49,
              0x3b,0x2d,0xeb,0x25,0x49,0xfa,0xa3,0xaa,0x39,0xa7,0xc5,0xa7,0x50,0x11,0x36,0xfb,
              0xc6,0x67,0x4a,0xf5,0xa5,0x12,0x65,0x7e,0xb0,0xdf,0xaf,0x4e,0xb3,0x61,0x7f,0x2f }
        };
        byte key = 0;
        for (int i = 0; i < 4; ++i) {
            key ^= (count >> (i*8)) & 0xff;
        }
        byte ci = xlat[0][serial & 0xff];
        byte cj = xlat[1][key];
        byte ck = 0x60;
        for (uint32_t i = 0; i < size; ++i) {
            cj += ci * ck++;
            pData[i] ^= cj;
        }
    }

}
