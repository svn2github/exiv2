// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2007 Andreas Huggel <ahuggel@gmx.net>
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
  File:      properties.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   13-July-07, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "properties.hpp"
#include "tags.hpp"
#include "error.hpp"
#include "types.hpp"
#include "value.hpp"
#include "metadatum.hpp"
#include "i18n.h"                // NLS support.

#include <iostream>
#include <iomanip>
#include <sstream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    extern const XmpPropertyInfo xmpTiffInfo[];
    extern const XmpPropertyInfo xmpExifInfo[];

    extern const XmpNsInfo xmpNsInfo[] = {
        // Schemas
        { "http://purl.org/dc/elements/1.1/",             "dc",           0, "Dublin Core schema" },
        { "http://ns.adobe.com/xap/1.0/",                 "xmp",          0, "XMP Basic schema" },
        { "http://ns.adobe.com/xap/1.0/rights/",          "xmpRights",    0, "XMP Rights Management schema" },
        { "http://ns.adobe.com/xap/1.0/mm/",              "xmpMM",        0, "XMP Media Management schema" },
        { "http://ns.adobe.com/xap/1.0/bj/",              "xmpBJ",        0, "XMP Basic Job Ticket schema" },
        { "http://ns.adobe.com/xap/1.0/t/pg/",            "xmpTPg",       0, "XMP Paged-Text schema" },
        { "http://ns.adobe.com/xmp/1.0/DynamicMedia/",    "xmpDM",        0, "XMP Dynamic Media schema" },
        { "http://ns.adobe.com/pdf/1.3/",                 "pdf",          0, "Adobe PDF schema" },
        { "http://ns.adobe.com/photoshop/1.0/",           "photoshop",    0, "Photoshop schema" },
        { "http://ns.adobe.com/camera-raw-settings/1.0/", "crs",          0, "Camera Raw schema" },
        { "http://ns.adobe.com/tiff/1.0/",                "tiff",         xmpTiffInfo, "Exif Schema for TIFF Properties" },
        { "http://ns.adobe.com/exif/1.0/",                "exif",         xmpExifInfo, "Exif schema for Exif-specific Properties" },
        { "http://ns.adobe.com/exif/1.0/aux/",            "aux",          0, "Exif schema for Additional Exif Properties" },
        { "http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/",  "iptc" /*Iptc4xmpCore*/, 0, "IPTC Core schema" },

        // Structures
        { "http://ns.adobe.com/xap/1.0/g/",                   "xapG",    0, "Colorant structure" },
        { "http://ns.adobe.com/xap/1.0/sType/Dimensions#",    "stDim",   0, "Dimensions structure" },
        { "http://ns.adobe.com/xap/1.0/sType/Font#",          "stFnt",   0, "Font structure" },
        { "http://ns.adobe.com/xap/1.0/g/img/",               "xapGImg", 0, "Thumbnail structure" },
        { "http://ns.adobe.com/xap/1.0/sType/ResourceEvent#", "stEvt",   0, "Resource Event structure" },
        { "http://ns.adobe.com/xap/1.0/sType/ResourceRef#",   "stRef",   0, "ResourceRef structure" },
        { "http://ns.adobe.com/xap/1.0/sType/Version#",       "stVer",   0, "Version structure" },
        { "http://ns.adobe.com/xap/1.0/sType/Job#",           "stJob",   0, "Basic Job/Workflow structure" },

        // Qualifiers
        { "http://ns.adobe.com/xmp/Identifier/qual/1.0/", "xmpidq", 0, "Qualifier for xmp:Identifier" }
    };

    extern const XmpPropertyInfo xmpTiffInfo[] = {
        { "ImageWidth",       "ImageWidth",       "Integer",     unsignedLong,     xmpInternal, "TIFF tag 256, 0x100. Image width in pixels." },
        { "ImageLength",      "ImageLength",      "Integer",     unsignedLong,     xmpInternal, "TIFF tag 257, 0x101. Image height in pixels." },
        { "BitsPerSample",    "BitsPerSample",    "seq Integer", unsignedShort,    xmpInternal, "TIFF tag 258, 0x102. Number of bits per component in each channel." },
        { "Compression",      "Compression",      "Closed Choice of Integer", unsignedShort, xmpInternal, "TIFF tag 259, 0x103. Compression scheme: 1 = uncompressed; 6 = JPEG." },
        { "PhotometricInterpretation", "PhotometricInterpretation", "Closed Choice of Integer", unsignedShort, xmpInternal, "TIFF tag 262, 0x106. Pixel Composition: 2 = RGB; 6 = YCbCr." },
        { "Orientation",      "Orientation",      "Closed Choice of Integer", unsignedShort, xmpInternal, "TIFF tag 274, 0x112. Orientation:"
                                                                                                "1 = 0th row at top, 0th column at left "
                                                                                                "2 = 0th row at top, 0th column at right "
                                                                                                "3 = 0th row at bottom, 0th column at right "
                                                                                                "4 = 0th row at bottom, 0th column at left "
                                                                                                "5 = 0th row at left, 0th column at top "
                                                                                                "6 = 0th row at right, 0th column at top "
                                                                                                "7 = 0th row at right, 0th column at bottom "
                                                                                                "8 = 0th row at left, 0th column at bottom" },
        { "SamplesPerPixel",  "SamplesPerPixel",  "Integer",     unsignedShort,    xmpInternal, "TIFF tag 277, 0x115. Number of components per pixel." },
        { "PlanarConfiguration", "PlanarConfiguration", "Closed Choice of Integer", unsignedShort, xmpInternal, "TIFF tag 284, 0x11C. Data layout:1 = chunky; 2 = planar." },
        { "YCbCrSubSampling", "YCbCrSubSampling", "Closed Choice of seq Integer", unsignedShort, xmpInternal, "TIFF tag 530, 0x212. Sampling ratio of chrominance "
                                                                                                "components: [2, 1] = YCbCr4:2:2; [2, 2] = YCbCr4:2:0" },
        { "YCbCrPositioning", "YCbCrPositioning", "Closed Choice of Integer", unsignedShort, xmpInternal, "TIFF tag 531, 0x213. Position of chrominance vs. "
                                                                                                "luminance components: 1 = centered; 2 = co-sited." },
        { "XResolution",      "XResolution",      "Rational",    unsignedRational, xmpInternal, "TIFF tag 282, 0x11A. Horizontal resolution in pixels per unit." },
        { "YResolution",      "YResolution",      "Rational",    unsignedRational, xmpInternal, "TIFF tag 283, 0x11B. Vertical resolution in pixels per unit." },
        { "ResolutionUnit",   "ResolutionUnit",   "Closed Choice of Integer", unsignedShort, xmpInternal, "TIFF tag 296, 0x128. Unit used for XResolution and "
                                                                                                "YResolution. Value is one of: 2 = inches; 3 = centimeters." },
        { "TransferFunction", "TransferFunction", "seq Integer", unsignedShort,    xmpInternal, "TIFF tag 301, 0x12D. Transfer function for image "
                                                                                                "described in tabular style with 3 * 256 entries." },
        { "WhitePoint",       "WhitePoint",       "seq Rational", unsignedRational, xmpInternal, "TIFF tag 318, 0x13E. Chromaticity of white point." },
        { "PrimaryChromaticities", "PrimaryChromaticities", "seq Rational", unsignedRational, xmpInternal, "TIFF tag 319, 0x13F. Chromaticity of the three primary colors." },
        { "YCbCrCoefficients", "YCbCrCoefficients", "seq Rational", unsignedRational, xmpInternal, "TIFF tag 529, 0x211. Matrix coefficients for RGB to YCbCr transformation." },
        { "ReferenceBlackWhite", "ReferenceBlackWhite", "seq Rational", unsignedRational, xmpInternal, "TIFF tag 532, 0x214. Reference black and white point values." },
        { "DateTime",         "DateTime",         "Date",        date,             xmpInternal, "TIFF tag 306, 0x132 (primary) and EXIF tag 37520, "
                                                                                                "0x9290 (subseconds). Date and time of image creation "
                                                                                                "(no time zone in EXIF), stored in ISO 8601 format, not "
                                                                                                "the original EXIF format. This property includes the "
                                                                                                "value for the EXIF SubSecTime attribute. "
                                                                                                "NOTE: This property is stored in XMP as xmp:ModifyDate." },
        { "ImageDescription", "ImageDescription", "Lang Alt",    asciiString,      xmpExternal, "TIFF tag 270, 0x10E. Description of the image. Note: This property is stored in XMP as dc:description." },
        { "Make",             "Make",             "ProperName",  asciiString,      xmpInternal, "TIFF tag 271, 0x10F. Manufacturer of recording equipment." },
        { "Model",            "Model",            "ProperName",  asciiString,      xmpInternal, "TIFF tag 272, 0x110. Model name or number of equipment." },
        { "Software",         "Software",         "AgentName",   asciiString,      xmpInternal, "TIFF tag 305, 0x131. Software or firmware used to generate image. "
                                                                                                "Note: This property is stored in XMP as xmp:CreatorTool. " },
        { "Artist",           "Artist",           "ProperName",  asciiString,      xmpExternal, "TIFF tag 315, 0x13B. Camera owner, photographer or image creator. "
                                                                                                "Note: This property is stored in XMP as the first item in the dc:creator array." },
        { "Copyright",        "Copyright",        "Lang Alt",    asciiString,      xmpExternal, "TIFF tag 33432, 0x8298. Copyright information. "
                                                                                                "Note: This property is stored in XMP as dc:rights." },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    //! exif:ColorSpace
    extern const TagDetails xmpExifColorSpace[] = {
        { 1,     N_("sRGB")         },
        { 65535, N_("uncalibrated") }
    };

    //! exif:ComponentsConfiguration
    extern const TagDetails xmpExifComponentsConfiguration[] = {
        { 0, "does not exist" },
        { 1, "Y"              },
        { 2, "Cb"             },
        { 3, "Cr"             },
        { 4, "R"              },
        { 5, "G"              },
        { 6, "B"              }
    };

    //! exif:ExposureProgram
    extern const TagDetails xmpExifExposureProgram[] = {
        { 0, N_("not defined")       },
        { 1, N_("Manual")            },
        { 2, N_("Normal program")    },
        { 3, N_("Aperture priority") },
        { 4, N_("Shutter priority")  },
        { 5, N_("Creative program")  },
        { 6, N_("Action program")    },
        { 7, N_("Portrait mode")     },
        { 8, N_("Landscape mode")    }
    };

    //! exif:MeteringMode
    extern const TagDetails xmpExifMeteringMode[] = {
        { 0, N_("unknown")              },
        { 1, N_("Average")               },
        { 2, N_("CenterWeightedAverage") },
        { 3, N_("Spot")                  },
        { 4, N_("MultiSpot")             },
        { 5, N_("Pattern")               },
        { 6, N_("Partial")               },
        { 255, N_("other")               }
    };

    //! exif:LightSource
    extern const TagDetails xmpExifLightSource[] = {
        {   0, N_("unknown")                                 },
        {   1, N_("Daylight")                                },
        {   2, N_("Fluorescent")                             },
        {   3, N_("Tungsten")                                },
        {   4, N_("Flash")                                   },
        {   9, N_("Fine weather")                            },
        {  10, N_("Cloudy weather")                          },
        {  11, N_("Shade")                                   },
        {  12, N_("Daylight fluorescent (D 5700 - 7100K)")   },
        {  13, N_("Day white fluorescent (N 4600 - 5400K)")  },
        {  14, N_("Cool white fluorescent (W 3900 - 4500K)") },
        {  15, N_("White fluorescent (WW 3200 - 3700K)")     },
        {  17, N_("Standard light A")                        },
        {  18, N_("Standard light B")                        },
        {  19, N_("Standard light C")                        },
        {  20, N_("D55")                                     },
        {  21, N_("D65")                                     },
        {  22, N_("D75")                                     },
        {  23, N_("D50")                                     },
        {  24, N_("ISO studio tungsten")                     },
        { 255, N_("other")                                   }
    };

    //! exif:FocalPlaneResolutionUnit
    extern const TagDetails xmpExifFocalPlaneResolutionUnit[] = {
        { 2, N_("inches")      },
        { 3, N_("centimeters") }
    };

    //! exif:SensingMethod
    extern const TagDetails xmpExifSensingMethod[] = {
        { 1, N_("Not defined")                    },
        { 2, N_("One-chip color area sensor")     },
        { 3, N_("Two-chip color area sensor")     },
        { 4, N_("Three-chip color area sensor")   },
        { 5, N_("Color sequential area sensor")   },
        { 7, N_("Trilinear sensor")               },
        { 8, N_("Color sequential linear sensor") }
    };

    //! exif:FileSource
    extern const TagDetails xmpExifFileSource[] = {
        { 3, N_("DSC") }
    };

    //! exif:SceneType
    extern const TagDetails xmpExifSceneType[] = {
        { 1, N_("directly photographed image") }
    };

    //! exif:CustomRendered
    extern const TagDetails xmpExifCustomRendered[] = {
        { 0, N_("Normal process") },
        { 1, N_("Custom process") }
    };

    //! exif:ExposureMode
    extern const TagDetails xmpExifExposureMode[] = {
        { 0, N_("Auto exposure")   },
        { 1, N_("Manual exposure") },
        { 2, N_("Auto bracket")    }
    };

    //! exif:WhiteBalance
    extern const TagDetails xmpExifWhiteBalance[] = {
        { 0, N_("Auto white balance")   },
        { 1, N_("Manual white balance") }
    };

    //! exif:SceneCaptureType
    extern const TagDetails xmpExifSceneCaptureType[] = {
        { 0, N_("Standard")    },
        { 1, N_("Landscape")   },
        { 2, N_("Portrait")    },
        { 3, N_("Night scene") }
    };

    //! exif:GainControl
    extern const TagDetails xmpExifGainControl[] = {
        { 0, N_("None")           },
        { 1, N_("Low gain up")    },
        { 2, N_("High gain up")   },
        { 3, N_("Low gain down")  },
        { 4, N_("High gain down") }
    };

    //! exif:Contrast, exif:Sharpness
    extern const TagDetails xmpExifNormalSoftHard[] = {
        { 0, N_("Normal") },
        { 1, N_("Soft")   },
        { 2, N_("Hard")   }
    };

    //! exif:Saturation
    extern const TagDetails xmpExifSaturation[] = {
        { 0, N_("Normal")          },
        { 1, N_("Low saturation")  },
        { 2, N_("High saturation") }
    };

    //! exif:SubjectDistanceRange
    extern const TagDetails xmpExifSubjectDistanceRange[] = {
        { 0, N_("Unknown")      },
        { 1, N_("Macro")        },
        { 2, N_("Close view")   },
        { 3, N_("Distant view") }
    };

    //! exif:GPSAltitudeRef
    extern const TagDetails xmpExifGPSAltitudeRef[] = {
        { 0, N_("Above sea level") },
        { 1, N_("Below sea level") }
    };

    //! exif:GPSStatus
    extern const TagDetails xmpExifGPSStatus[] = {
        { 'A', N_("measurement in progress")         },
        { 'V', N_("measurement is interoperability") }
    };

    //! exif:GPSMeasureMode
    extern const TagDetails xmpExifGPSMeasureMode[] = {
        { 2, N_("two-dimensional measurement")   },
        { 3, N_("three-dimensional measurement") }
    };

    //! exif:GPSSpeedRef
    extern const TagDetails xmpExifGPSSpeedRef[] = {
        { 'K', N_("kilometers per hour") },
        { 'M', N_("miles per hour")      },
        { 'N', N_("knots")               }
    };

    //! exif:GPSTrackRef, exif:GPSImgDirectionRef, exif:GPSDestBearingRef
    extern const TagDetails xmpExifGPSDirection[] = {
        { 'T', N_("true direction")     },
        { 'M', N_("magnetic direction") }
    };

    //! exif:GPSDestDistanceRef
    extern const TagDetails xmpExifGPSDestDistanceRef[] = {
        { 'K', N_("kilometers") },
        { 'M', N_("miles")      },
        { 'N', N_("knots")      }
    };

    //! exif:GPSDifferential
    extern const TagDetails xmpExifGPSDifferential[] = {
        { 0, N_("Without correction") },
        { 1, N_("Correction applied") }
    };

    extern const XmpPropertyInfo xmpExifInfo[] = {
        { "ExifVersion",      "ExifVersion",      "Closed Choice of Text", asciiString, xmpInternal, "EXIF tag 36864, 0x9000. EXIF version number." },
        { "FlashpixVersion",  "FlashpixVersion",  "Closed Choice of Text", asciiString, xmpInternal, "EXIF tag 40960, 0xA000. Version of FlashPix." },
        { "ColorSpace",       "ColorSpace",       "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 40961, 0xA001. Color space information" },
        { "ComponentsConfiguration", "ComponentsConfiguration", "Closed Choice of seq Integer", unsignedShort, xmpInternal, "EXIF tag 37121, 0x9101. Configuration of components in data: 4 5 6 0 (if RGB compressed data), 1 2 3 0 (other cases)." },
        { "CompressedBitsPerPixel", "CompressedBitsPerPixel", "Rational", unsignedRational, xmpInternal, "EXIF tag 37122, 0x9102. Compression mode used for a compressed image is indicated in unit bits per pixel." },
        { "PixelXDimension",  "PixelXDimension",  "Integer",     unsignedLong,     xmpInternal, "EXIF tag 40962, 0xA002. Valid image width, in pixels." },
        { "PixelYDimension",  "PixelYDimension",  "Integer",     unsignedLong,     xmpInternal, "EXIF tag 40963, 0xA003. Valid image height, in pixels." },
        { "UserComment",      "UserComment",      "Lang Alt",    asciiString,      xmpExternal, "EXIF tag 37510, 0x9286. Comments from user." },
        { "RelatedSoundFile", "RelatedSoundFile", "Text",        asciiString,      xmpInternal, "EXIF tag 40964, 0xA004. An \"8.3\" file name for the related sound file." },
        { "DateTimeOriginal", "DateTimeOriginal", "Date",        asciiString,      xmpInternal, "EXIF tags 36867, 0x9003 (primary) and 37521, 0x9291 (subseconds). Date and time when original image was generated, in ISO 8601 format. Includes the EXIF SubSecTimeOriginal data." },
        { "DateTimeDigitized", "DateTimeDigitized", "Date",      asciiString,      xmpInternal, "EXIF tag 36868, 0x9004 (primary) and 37522, 0x9292 (subseconds). Date and time when "
                                                                                                "image was stored as digital data, can be the same as DateTimeOriginal if originally "
                                                                                                "stored in digital form. Stored in ISO 8601 format. Includes the EXIF SubSecTimeDigitized data." },
        { "ExposureTime",     "ExposureTime",     "Rational",    unsignedRational, xmpInternal, "EXIF tag 33434, 0x829A. Exposure time in seconds." },
        { "FNumber",          "FNumber",          "Rational",    unsignedRational, xmpInternal, "EXIF tag 33437, 0x829D. F number." },
        { "ExposureProgram",  "ExposureProgram",  "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 34850, 0x8822. Class of program used for exposure." },
        { "SpectralSensitivity", "SpectralSensitivity", "Text",  asciiString,      xmpInternal, "EXIF tag 34852, 0x8824. Spectral sensitivity of each channel." },
        { "ISOSpeedRatings",  "ISOSpeedRatings",  "seq Integer", unsignedShort,    xmpInternal, "EXIF tag 34855, 0x8827. ISO Speed and ISO Latitude of the input device as specified in ISO 12232." },
        { "OECF",             "OECF",             "OECF/SFR",    asciiString,      xmpInternal, "EXIF tag 34856, 0x8828. Opto-Electoric Conversion Function as specified in ISO 14524." },
        { "ShutterSpeedValue", "ShutterSpeedValue", "Rational",  signedRational,   xmpInternal, "EXIF tag 37377, 0x9201. Shutter speed, unit is APEX. See Annex C of the EXIF specification." },
        { "ApertureValue",    "ApertureValue",    "Rational",    unsignedRational, xmpInternal, "EXIF tag 37378, 0x9202. Lens aperture, unit is APEX." },
        { "BrightnessValue",  "BrightnessValue",  "Rational",    signedRational,   xmpInternal, "EXIF tag 37379, 0x9203. Brightness, unit is APEX." },
        { "ExposureBiasValue", "ExposureBiasValue", "Rational",  signedRational,   xmpInternal, "EXIF tag 37380, 0x9204. Exposure bias, unit is APEX." },
        { "MaxApertureValue", "MaxApertureValue", "Rational",    unsignedRational, xmpInternal, "EXIF tag 37381, 0x9205. Smallest F number of lens, in APEX." },
        { "SubjectDistance",  "SubjectDistance",  "Rational",    unsignedRational, xmpInternal, "EXIF tag 37382, 0x9206. Distance to subject, in meters." },
        { "MeteringMode",     "MeteringMode",     "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 37383, 0x9207. Metering mode." },
        { "LightSource",      "LightSource",      "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 37384, 0x9208. Light source." },
        { "Flash",            "Flash",            "Flash",       asciiString,      xmpInternal, "EXIF tag 37385, 0x9209. Strobe light (flash) source data." },
        { "FocalLength",      "FocalLength",      "Rational",    unsignedRational, xmpInternal, "EXIF tag 37386, 0x920A. Focal length of the lens, in millimeters." },
        { "SubjectArea",      "SubjectArea",      "seq Integer", unsignedShort,    xmpInternal, "EXIF tag 37396, 0x9214. The location and area of the main subject in the overall scene." },
        { "FlashEnergy",      "FlashEnergy",      "Rational",    unsignedRational, xmpInternal, "EXIF tag 41483, 0xA20B. Strobe energy during image capture." },
        { "SpatialFrequencyResponse", "SpatialFrequencyResponse", "OECF/SFR", asciiString, xmpInternal, "EXIF tag 41484, 0xA20C. Input device spatial frequency table and SFR values as specified in ISO 12233." },
        { "FocalPlaneXResolution", "FocalPlaneXResolution", "Rational", unsignedRational, xmpInternal, "EXIF tag 41486, 0xA20E. Horizontal focal resolution, measured pixels per unit." },
        { "FocalPlaneYResolution", "FocalPlaneYResolution", "Rational", unsignedRational, xmpInternal, "EXIF tag 41487, 0xA20F. Vertical focal resolution, measured in pixels per unit." },
        { "FocalPlaneResolutionUnit", "FocalPlaneResolutionUnit", "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 41488, 0xA210. Unit used for FocalPlaneXResolution and FocalPlaneYResolution." },
        { "SubjectLocation",  "SubjectLocation",  "seq Integer", unsignedShort,    xmpInternal, "EXIF tag 41492, 0xA214. Location of the main subject of the scene. The first value is the "
                                                                                                "horizontal pixel and the second value is the vertical pixel at which the main subject appears." },
        { "ExposureIndex",    "ExposureIndex",    "Rational",    unsignedRational, xmpInternal, "EXIF tag 41493, 0xA215. Exposure index of input device." },
        { "SensingMethod",    "SensingMethod",    "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 41495, 0xA217. Image sensor type on input device." },
        { "FileSource",       "FileSource",       "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 41728, 0xA300. Indicates image source." },
        { "SceneType",        "SceneType",        "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 41729, 0xA301. Indicates the type of scene." },
        { "CFAPattern",       "CFAPattern",       "CFAPattern",  asciiString,      xmpInternal, "EXIF tag 41730, 0xA302. Color filter array geometric pattern of the image sense." },
        { "CustomRendered",   "CustomRendered",   "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 41985, 0xA401. Indicates the use of special processing on image data." },
        { "ExposureMode",     "ExposureMode",     "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 41986, 0xA402. Indicates the exposure mode set when the image was shot." },
        { "WhiteBalance",     "WhiteBalance",     "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 41987, 0xA403. Indicates the white balance mode set when the image was shot." },
        { "DigitalZoomRatio", "DigitalZoomRatio", "Rational",    unsignedRational, xmpInternal, "EXIF tag 41988, 0xA404. Indicates the digital zoom ratio when the image was shot." },
        { "FocalLengthIn35mmFilm", "FocalLengthIn35mmFilm", "Integer", unsignedShort, xmpInternal, "EXIF tag 41989, 0xA405. Indicates the equivalent focal length assuming a 35mm film "
                                                                                                "camera, in mm. A value of 0 means the focal length is unknown. Note that this tag differs from the FocalLength tag." },
        { "SceneCaptureType", "SceneCaptureType", "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 41990, 0xA406. Indicates the type of scene that was shot." },
        { "GainControl",      "GainControl", "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 41991, 0xA407. Indicates the degree of overall image gain adjustment." },
        { "Contrast",         "Contrast", "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 41992, 0xA408. Indicates the direction of contrast processing applied by the camera." },
        { "Saturation",       "Saturation", "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 41993, 0xA409. Indicates the direction of saturation processing applied by the camera." },
        { "Sharpness",        "Sharpness", "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 41994, 0xA40A. Indicates the direction of sharpness processing applied by the camera." },
        { "DeviceSettingDescription", "DeviceSettingDescription", "DeviceSettings", asciiString, xmpInternal, "EXIF tag 41995, 0xA40B. Indicates information on the picture-taking conditions of a particular camera model." },
        { "SubjectDistanceRange", "SubjectDistanceRange", "Closed Choice of Integer", unsignedShort, xmpInternal, "EXIF tag 41996, 0xA40C. Indicates the distance to the subject." },
        { "ImageUniqueID",    "ImageUniqueID",    "Text",        asciiString,      xmpInternal, "EXIF tag 42016, 0xA420. An identifier assigned uniquely to each image. It is recorded as a 32 "
                                                                                                "character ASCII string, equivalent to hexadecimal notation and 128-bit fixed length." },
        { "GPSVersionID",     "GPSVersionID",     "Text",        asciiString,      xmpInternal, "GPS tag 0, 0x00. A decimal encoding of each of the four EXIF bytes with period separators. The current value is \"2.0.0.0\"." },
        { "GPSLatitude",      "GPSLatitude",      "GPSCoordinate", asciiString,    xmpInternal, "GPS tag 2, 0x02 (position) and 1, 0x01 (North/South). Indicates latitude." },
        { "GPSLongitude",     "GPSLongitude",     "GPSCoordinate", asciiString,    xmpInternal, "GPS tag 4, 0x04 (position) and 3, 0x03 (East/West). Indicates longitude." },
        { "GPSAltitudeRef",   "GPSAltitudeRef",   "Closed Choice of Integer", unsignedByte, xmpInternal, "GPS tag 5, 0x5. Indicates whether the altitude is above or below sea level." },
        { "GPSAltitude",      "GPSAltitude",      "Rational",    unsignedRational, xmpInternal, "GPS tag 6, 0x06. Indicates altitude in meters." },
        { "GPSTimeStamp",     "GPSTimeStamp",     "Date",        asciiString,      xmpInternal, "GPS tag 29 (date), 0x1D, and, and GPS tag 7 (time), 0x07. Time stamp of GPS data, in Coordinated Universal Time. "
                                                                                                "Note: The GPSDateStamp tag is new in EXIF 2.2. The GPS timestamp in EXIF 2.1 does not include a date. If not present, "
                                                                                                "the date component for the XMP should be taken from exif:DateTimeOriginal, or if that is "
                                                                                                "also lacking from exif:DateTimeDigitized. If no date is available, do not write exif:GPSTimeStamp to XMP." },
        { "GPSSatellites",    "GPSSatellites",    "Text",        asciiString,      xmpInternal, "GPS tag 8, 0x08. Satellite information, format is unspecified." },
        { "GPSStatus",        "GPSStatus",        "Closed Choice of Text", asciiString, xmpInternal, "GPS tag 9, 0x09. Status of GPS receiver at image creation time." },
        { "GPSMeasureMode",   "GPSMeasureMode",   "Text",        asciiString,      xmpInternal, "GPS tag 10, 0x0A. GPS measurement mode, Text type." },
        { "GPSDOP",           "GPSDOP",           "Rational",    unsignedRational, xmpInternal, "GPS tag 11, 0x0B. Degree of precision for GPS data." },
        { "GPSSpeedRef",      "GPSSpeedRef",      "Closed Choice of Text", asciiString, xmpInternal, "GPS tag 12, 0x0C. Units used to speed measurement." },
        { "GPSSpeed",         "GPSSpeed",         "Rational",    unsignedRational, xmpInternal, "GPS tag 13, 0x0D. Speed of GPS receiver movement." },
        { "GPSTrackRef",      "GPSTrackRef",      "Closed Choice of Text", asciiString, xmpInternal, "GPS tag 14, 0x0E. Reference for movement direction." },
        { "GPSTrack",         "GPSTrack",         "Rational",    unsignedRational, xmpInternal, "GPS tag 15, 0x0F. Direction of GPS movement, values range from 0 to 359.99." },
        { "GPSImgDirectionRef", "GPSImgDirectionRef", "Closed Choice of Text", asciiString, xmpInternal, "GPS tag 16, 0x10. Reference for movement direction." },
        { "GPSImgDirection",  "GPSImgDirection",  "Rational",    unsignedRational, xmpInternal, "GPS tag 17, 0x11. Direction of image when captured, values range from 0 to 359.99." },
        { "GPSMapDatum",      "GPSMapDatum",      "Text",        asciiString,      xmpInternal, "GPS tag 18, 0x12. Geodetic survey data." },
        { "GPSDestLatitude",  "GPSDestLatitude",  "GPSCoordinate", asciiString,    xmpInternal, "GPS tag 20, 0x14 (position) and 19, 0x13 (North/South). Indicates destination latitude." },
        { "GPSDestLongitude", "GPSDestLongitude", "GPSCoordinate", asciiString,    xmpInternal, "GPS tag 22, 0x16 (position) and 21, 0x15 (East/West). Indicates destination longitude." },
        { "GPSDestBearingRef", "GPSDestBearingRef", "Closed Choice of Text", asciiString, xmpInternal, "GPS tag 23, 0x17. Reference for movement direction." },
        { "GPSDestBearing",   "GPSDestBearing",   "Rational",    unsignedRational, xmpInternal, "GPS tag 24, 0x18. Destination bearing, values from 0 to 359.99." },
        { "GPSDestDistanceRef", "GPSDestDistanceRef", "Closed Choice  of Text", asciiString, xmpInternal, "GPS tag 25, 0x19. Units used for speed measurement." },
        { "GPSDestDistance",  "GPSDestDistance",  "Rational",    unsignedRational, xmpInternal, "GPS tag 26, 0x1A. Distance to destination." },
        { "GPSProcessingMethod", "GPSProcessingMethod", "Text",  asciiString,      xmpInternal, "GPS tag 27, 0x1B. A character string recording the name of the method used for location finding." },
        { "GPSAreaInformation", "GPSAreaInformation", "Text",    asciiString,      xmpInternal, "GPS tag 28, 0x1C. A character string recording the name of the GPS area." },
        { "GPSDifferential",  "GPSDifferential",  "Closed Choice of Integer", unsignedShort, xmpInternal, "GPS tag 30, 0x1E. Indicates whether differential correction is applied to the GPS receiver." },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    XmpNsInfo::Ns::Ns(const std::string& ns)
        : ns_(ns)
    {
    }

    XmpNsInfo::Prefix::Prefix(const std::string& prefix)
        : prefix_(prefix)
    {
    }

    bool XmpNsInfo::operator==(const XmpNsInfo::Ns& ns) const
    {
        std::string n(ns_);
        return n == ns.ns_;
    }

    bool XmpNsInfo::operator==(const XmpNsInfo::Prefix& prefix) const
    {
        std::string p(prefix_);
        return p == prefix.prefix_;
    }

    bool XmpPropertyInfo::operator==(const std::string& name) const
    {
        std::string n(name_);
        return n == name;
    }

    const char* XmpProperties::propertyTitle(const XmpKey& key)
    {
        return propertyInfo(key)->title_;
    }

    const char* XmpProperties::propertyDesc(const XmpKey& key)
    {
        return propertyInfo(key)->desc_;
    }

    TypeId XmpProperties::propertyType(const XmpKey& key)
    {
        return propertyInfo(key)->typeId_;
    }

    const XmpPropertyInfo* XmpProperties::propertyInfo(const XmpKey& key)
    {
        const XmpPropertyInfo* pl = propertyList(key.groupName());
        if (!pl) throw Error(36, key.groupName());
        const XmpPropertyInfo* pi = 0;
        for (int i = 0; pl[i].name_ != 0; ++i) {
            if (std::string(pl[i].name_) == key.tagName()) {
                pi = pl + i;
                break;
            }
        }
        if (!pi) throw Error(38, key.groupName(), key.tagName());
        return pi;
    }

    const char* XmpProperties::ns(const std::string& prefix)
    {
        return nsInfo(prefix)->ns_;
    }

    const char* XmpProperties::nsDesc(const std::string& prefix)
    {
        return nsInfo(prefix)->desc_;
    }

    const XmpPropertyInfo* XmpProperties::propertyList(const std::string& prefix)
    {
        return nsInfo(prefix)->xmpPropertyInfo_;
    }

    const XmpNsInfo* XmpProperties::nsInfo(const std::string& prefix)
    {
        const XmpNsInfo* xn = find(xmpNsInfo, XmpNsInfo::Prefix(prefix));
        if (!xn) throw Error(35, prefix);
        return xn;
    }

    const char* XmpProperties::prefix(const std::string& ns)
    {
        const XmpNsInfo* xn = find(xmpNsInfo, XmpNsInfo::Ns(ns));
        return xn ? xn->prefix_ : 0;
    }

    void XmpProperties::printProperties(std::ostream& os, const std::string& prefix)
    {
        const XmpPropertyInfo* pl = propertyList(prefix);
        if (pl) {
            const int ck = sizeof(pl) / sizeof(pl[0]);                
            for (int k = 0; k < ck; ++k) {
                os << pl[k];
            }
        }

    } // XmpProperties::printProperties

    //! @cond IGNORE

    //! Internal Pimpl structure with private members and data of class XmpKey.
    struct XmpKey::Impl {
        Impl();                         //!< Default constructor
        Impl(const std::string& prefix, const std::string& property); //!< Constructor

        /*!
          @brief Parse and convert the \em key string into property and prefix.
                 Updates data members if the string can be decomposed, or throws
                 \em Error.

          @throw Error if the key cannot be decomposed.
        */
        void decomposeKey(const XmpKey* self, const std::string& key);

        // DATA
        static const char* familyName_; //!< "Xmp"

        std::string prefix_;            //!< Prefix
        std::string property_;          //!< Property name
    };
    //! @endcond

    XmpKey::Impl::Impl()
    {
    }

    XmpKey::Impl::Impl(const std::string& prefix, const std::string& property)
        : prefix_(prefix), property_(property)
    {
    }

    const char* XmpKey::Impl::familyName_ = "Xmp";

    XmpKey::XmpKey(const std::string& key)
        : p_(new Impl)
    {
        p_->decomposeKey(this, key);
    }

    XmpKey::XmpKey(const std::string& prefix, const std::string& property)
        : p_(new Impl(prefix, property))
    {
        // Validate prefix and property, throws
        XmpProperties::propertyInfo(*this);
    }

    XmpKey::~XmpKey()
    {
        delete p_;
    }

    XmpKey::XmpKey(const XmpKey& rhs)
        : Key(rhs), p_(new Impl(*rhs.p_))
    {
    }

    XmpKey& XmpKey::operator=(const XmpKey& rhs)
    {
        if (this == &rhs) return *this;
        *p_ = *rhs.p_;
        return *this;
    }

    XmpKey::AutoPtr XmpKey::clone() const
    {
        return AutoPtr(clone_());
    }

    XmpKey* XmpKey::clone_() const
    {
        return new XmpKey(*this);
    }

    std::string XmpKey::key() const
    {
        return std::string(p_->familyName_) + "." + p_->prefix_ + "." + p_->property_;
    }

    const char* XmpKey::familyName() const
    {
        return p_->familyName_;
    }

    std::string XmpKey::groupName() const
    {
        return p_->prefix_;
    }

    std::string XmpKey::tagName() const
    { 
        return p_->property_;
    }

    std::string XmpKey::tagLabel() const
    {
        return XmpProperties::propertyTitle(*this);
    }

    const char* XmpKey::ns() const
    {
        return XmpProperties::ns(p_->prefix_);
    }

    void XmpKey::Impl::decomposeKey(const XmpKey* self, const std::string& key)
    {
        // Get the family name, prefix and property name parts of the key
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos) throw Error(6, key);
        std::string familyName = key.substr(0, pos1);
        if (familyName != std::string(familyName_)) {
            throw Error(6, key);
        }
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos) throw Error(6, key);
        std::string prefix = key.substr(pos0, pos1 - pos0);
        if (prefix == "") throw Error(6, key);
        std::string property = key.substr(pos1 + 1);
        if (property == "") throw Error(6, key);

        property_ = property;
        prefix_ = prefix;

        // Validate prefix and property
        XmpProperties::propertyInfo(*self);

    } // XmpKey::Impl::decomposeKey

    // *************************************************************************
    // free functions
    std::ostream& operator<<(std::ostream& os, const XmpPropertyInfo& property)
    {
        return os << property.name_                       << ",\t"
                  << property.title_                      << ",\t"
                  << property.xmpValueType_               << ",\t"
                  << TypeInfo::typeName(property.typeId_) << ",\t"
                  << ( property.xmpCategory_ == xmpExternal ? "External" : "Internal" ) << ",\t"
                  << property.desc_                       << "\n";
    }

}                                       // namespace Exiv2
