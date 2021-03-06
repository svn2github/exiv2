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
  File:      panasonicmn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   11-Jun-04, ahu: created
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "panasonicmn.hpp"
#include "makernote.hpp"
#include "value.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! @cond IGNORE
    PanasonicMakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("Panasonic", "*", createPanasonicMakerNote);
        MakerNoteFactory::registerMakerNote(
            panasonicIfdId, MakerNote::AutoPtr(new PanasonicMakerNote));

        ExifTags::registerMakerTagInfo(panasonicIfdId, tagInfo_);
    }
    //! @endcond

    //! Quality, tag 0x0001
    extern const TagDetails panasonicQuality[] = {
        { 2, "High"      },
        { 3, "Standard"  },
        { 6, "Very High" },
        { 7, "Raw"       }
    };

    //! WhiteBalance, tag 0x0003
    extern const TagDetails panasonicWhiteBalance[] = {
        {  1, "Auto"            },
        {  2, "Daylight"        },
        {  3, "Cloudy"          },
        {  4, "Halogen"         },
        {  5, "Manual"          },
        {  8, "Flash"           },
        { 10, "Black and White" }
    };

    //! FocusMode, tag 0x0007
    extern const TagDetails panasonicFocusMode[] = {
        {  1, "Auto" },
        {  2, "Manual" }
    };

    //! ImageStabilizer, tag 0x001a
    extern const TagDetails panasonicImageStabilizer[] = {
        {  2, "On, Mode 1" },
        {  3, "Off"        },
        {  4, "On, Mode 2" }
    };

    //! Macro, tag 0x001c
    extern const TagDetails panasonicMacro[] = {
        { 1, "On"  },
        { 2, "Off" }
    };

    //! ShootingMode, tag 0x001f
    extern const TagDetails panasonicShootingMode[] = {
        {  1, "Normal"            },
        {  2, "Portrait"          },
        {  3, "Scenery"           },
        {  4, "Sports"            },
        {  5, "Night Portrait"    },
        {  6, "Program"           },
        {  7, "Aperture Priority" },
        {  8, "Shutter Priority"  },
        {  9, "Macro"             },
        { 11, "Manual"            },
        { 13, "Panning"           },
        { 18, "Fireworks"         },
        { 19, "Party"             },
        { 20, "Snow"              },
        { 21, "Night Scenery"     }
    };

    //! Audio, tag 0x0020
    extern const TagDetails panasonicAudio[] = {
        { 1, "Yes" },
        { 2, "No"  }
    };

    //! ColorEffect, tag 0x0028
    extern const TagDetails panasonicColorEffect[] = {
        { 1, "Off"             },
        { 2, "Warm"            },
        { 3, "Cool"            },
        { 4, "Black and White" },
        { 5, "Sepia"           }
    };

    //! Contrast, tag 0x002c
    extern const TagDetails panasonicContrast[] = {
        {     0, "Standard" },
        {     1, "Low"      },
        {     2, "High"     },
        { 0x100, "Low"      },
        { 0x110, "Standard" },
        { 0x120, "High"     }
    };

    //! NoiseReduction, tag 0x002d
    extern const TagDetails panasonicNoiseReduction[] = {
        { 0, "Standard" },
        { 1, "Low"      },
        { 2, "High"     }
    };

    // Panasonic MakerNote Tag Info
    const TagInfo PanasonicMakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Quality", "Quality", "Image Quality", panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicQuality)),
        TagInfo(0x0002, "FirmwareVersion", "FirmwareVersion", "Firmware version", panasonicIfdId, makerTags, undefined, printValue),
        TagInfo(0x0003, "WhiteBalance", "WhiteBalance", "White balance setting", panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicWhiteBalance)),
        TagInfo(0x0004, "0x0004", "0x0004", "Unknown", panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "FocusMode", "FocusMode", "Focus mode", panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicFocusMode)),
        TagInfo(0x000f, "SpotMode", "SpotMode", "Spot mode", panasonicIfdId, makerTags, unsignedByte, print0x000f),
        TagInfo(0x001a, "ImageStabilizer", "ImageStabilizer", "Image stabilizer", panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicImageStabilizer)),
        TagInfo(0x001c, "Macro", "Macro", "Macro mode", panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicMacro)),
        TagInfo(0x001f, "ShootingMode", "ShootingMode", "Shooting mode", panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicShootingMode)),
        TagInfo(0x0020, "Audio", "Audio", "Audio", panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicAudio)),
        TagInfo(0x0021, "DataDump", "DataDump", "Data dump", panasonicIfdId, makerTags, undefined, printValue),
        TagInfo(0x0022, "0x0022", "0x0022", "Unknown", panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0023, "WhiteBalanceBias", "WhiteBalanceBias", "White balance adjustment", panasonicIfdId, makerTags, unsignedShort, print0x0023),
        TagInfo(0x0024, "FlashBias", "FlashBias", "Flash bias", panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0025, "SerialNumber", "SerialNumber", "Serial number", panasonicIfdId, makerTags, undefined, printValue),
        TagInfo(0x0026, "0x0026", "0x0026", "Unknown", panasonicIfdId, makerTags, undefined, printValue),
        TagInfo(0x0027, "0x0027", "0x0027", "Unknown", panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0028, "ColorEffect", "ColorEffect", "Color effect", panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicColorEffect)),
        TagInfo(0x0029, "0x0029", "0x0029", "Unknown", panasonicIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x002a, "0x002a", "0x002a", "Unknown", panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x002b, "0x002b", "0x002b", "Unknown", panasonicIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x002c, "Contrast", "Contrast", "Contrast setting", panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicContrast)),
        TagInfo(0x002d, "NoiseReduction", "NoiseReduction", "Noise reduction", panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicNoiseReduction)),
        TagInfo(0x002e, "0x002e", "0x002e", "Unknown", panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x002f, "0x002f", "0x002f", "Unknown", panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0030, "0x0030", "0x0030", "Unknown", panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0031, "0x0031", "0x0031", "Unknown", panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0032, "0x0032", "0x0032", "Unknown", panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x4449, "0x4449", "0x4449", "Unknown", panasonicIfdId, makerTags, undefined, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownPanasonicMakerNoteTag)", "(UnknownPanasonicMakerNoteTag)", "Unknown PanasonicMakerNote tag", panasonicIfdId, makerTags, invalidTypeId, printValue)
    };

    PanasonicMakerNote::PanasonicMakerNote(bool alloc)
        : IfdMakerNote(panasonicIfdId, alloc, false)
    {
        byte buf[] = {
            'P', 'a', 'n', 'a', 's', 'o', 'n', 'i', 'c', 0x00, 0x00, 0x00
        };
        readHeader(buf, 12, byteOrder_);
    }

    PanasonicMakerNote::PanasonicMakerNote(const PanasonicMakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    int PanasonicMakerNote::readHeader(const byte* buf,
                                       long        len,
                                       ByteOrder   /*byteOrder*/)
    {
        if (len < 12) return 1;

        header_.alloc(12);
        memcpy(header_.pData_, buf, header_.size_);
        // Adjust the offset of the IFD for the prefix
        start_ = 12;
        return 0;
    }

    int PanasonicMakerNote::checkHeader() const
    {
        int rc = 0;
        // Check the Panasonic prefix
        if (   header_.size_ < 12
            || std::string(reinterpret_cast<char*>(header_.pData_), 9)
               != std::string("Panasonic", 9)) {
            rc = 2;
        }
        return rc;
    }

    PanasonicMakerNote::AutoPtr PanasonicMakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    PanasonicMakerNote* PanasonicMakerNote::create_(bool alloc) const
    {
        AutoPtr makerNote(new PanasonicMakerNote(alloc));
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    PanasonicMakerNote::AutoPtr PanasonicMakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    PanasonicMakerNote* PanasonicMakerNote::clone_() const
    {
        return new PanasonicMakerNote(*this);
    }

    std::ostream& PanasonicMakerNote::print0x000f(std::ostream& os,
                                                  const Value& value)
    {
        if (value.count() < 2 || value.typeId() != unsignedByte) {
            return os << value;
        }
        long l0 = value.toLong(0);
        if (l0 == 1) os << "On";
        else if (l0 == 16) os << "Off";
        else os << value;
        return os;
    } // PanasonicMakerNote::print0x000f

    std::ostream& PanasonicMakerNote::print0x0023(std::ostream& os,
                                                  const Value& value)
    {
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1)
           << value.toLong() / 3 << " EV";
        os.copyfmt(oss);

        return os;

    } // PanasonicMakerNote::print0x0023

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createPanasonicMakerNote(bool        alloc,
                                                const byte* /*buf*/,
                                                long        /*len*/,
                                                ByteOrder   /*byteOrder*/,
                                                long        /*offset*/)
    {
        return MakerNote::AutoPtr(new PanasonicMakerNote(alloc));
    }

}                                       // namespace Exiv2
