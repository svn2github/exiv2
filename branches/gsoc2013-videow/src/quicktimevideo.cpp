// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2013 Andreas Huggel <ahuggel@gmx.net>
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
  File:      quicktimevideo.cpp
  Version:   $Rev$
  Author(s): Abhinav Badola for GSoC 2012 (AB) <mail.abu.to@gmail.com>
  History:   28-Jun-12, AB: created
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "quicktimevideo.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
// + standard includes
#include <cmath>
#include <iostream>
using namespace std;
// *****************************************************************************
// class member definitions
namespace Exiv2 {
namespace Internal {

extern const TagVocabulary qTimeFileType[] =  {
    {   "3g2a", "3GPP2 Media (.3G2) compliant with 3GPP2 C.S0050-0 V1.0" },
    {   "3g2b", "3GPP2 Media (.3G2) compliant with 3GPP2 C.S0050-A V1.0.0" },
    {   "3g2c", "3GPP2 Media (.3G2) compliant with 3GPP2 C.S0050-B v1.0" },
    {   "3ge6", "3GPP (.3GP) Release 6 MBMS Extended Presentations" },
    {   "3ge7", "3GPP (.3GP) Release 7 MBMS Extended Presentations" },
    {   "3gg6", "3GPP Release 6 General Profile" },
    {   "3gp1", "3GPP Media (.3GP) Release 1 (probably non-existent)" },
    {   "3gp2", "3GPP Media (.3GP) Release 2 (probably non-existent)" },
    {   "3gp3", "3GPP Media (.3GP) Release 3 (probably non-existent)" },
    {   "3gp4", "3GPP Media (.3GP) Release 4" },
    {   "3gp5", "3GPP Media (.3GP) Release 5" },
    {   "3gp6", "3GPP Media (.3GP) Release 6 Streaming Servers" },
    {   "3gs7", "3GPP Media (.3GP) Release 7 Streaming Servers" },
    {   "CAEP", "Canon Digital Camera" },
    {   "CDes", "Convergent Design" },
    {   "F4A ", "Audio for Adobe Flash Player 9+ (.F4A)" },
    {   "F4B ", "Audio Book for Adobe Flash Player 9+ (.F4B)" },
    {   "F4P ", "Protected Video for Adobe Flash Player 9+ (.F4P)" },
    {   "F4V ", "Video for Adobe Flash Player 9+ (.F4V)" },
    {   "JP2 ", "JPEG 2000 Image (.JP2) [ISO 15444-1 ?]" },
    {   "JP20", "Unknown, from GPAC samples (prob non-existent)" },
    {   "KDDI", "3GPP2 EZmovie for KDDI 3G cellphones" },
    {   "M4A ", "Apple iTunes AAC-LC (.M4A) Audio" },
    {   "M4B ", "Apple iTunes AAC-LC (.M4B) Audio Book" },
    {   "M4P ", "Apple iTunes AAC-LC (.M4P) AES Protected Audio" },
    {   "M4V ", "Apple iTunes Video (.M4V) Video" },
    {   "M4VH", "Apple TV (.M4V)" },
    {   "M4VP", "Apple iPhone (.M4V)" },
    {   "MPPI", "Photo Player, MAF [ISO/IEC 23000-3]" },
    {   "MSNV", "MPEG-4 (.MP4) for SonyPSP" },
    {   "NDAS", "MP4 v2 [ISO 14496-14] Nero Digital AAC Audio" },
    {   "NDSC", "MPEG-4 (.MP4) Nero Cinema Profile" },
    {   "NDSH", "MPEG-4 (.MP4) Nero HDTV Profile" },
    {   "NDSM", "MPEG-4 (.MP4) Nero Mobile Profile" },
    {   "NDSP", "MPEG-4 (.MP4) Nero Portable Profile" },
    {   "NDSS", "MPEG-4 (.MP4) Nero Standard Profile" },
    {   "NDXC", "H.264/MPEG-4 AVC (.MP4) Nero Cinema Profile" },
    {   "NDXH", "H.264/MPEG-4 AVC (.MP4) Nero HDTV Profile" },
    {   "NDXM", "H.264/MPEG-4 AVC (.MP4) Nero Mobile Profile" },
    {   "NDXP", "H.264/MPEG-4 AVC (.MP4) Nero Portable Profile" },
    {   "NDXS", "H.264/MPEG-4 AVC (.MP4) Nero Standard Profile" },
    {   "NIKO", "Nikon" },
    {   "ROSS", "Ross Video" },
    {   "avc1", "MP4 Base w/ AVC ext [ISO 14496-12:2005]" },
    {   "caqv", "Casio Digital Camera" },
    {   "da0a", "DMB MAF w/ MPEG Layer II aud, MOT slides, DLS, JPG/PNG/MNG images" },
    {   "da0b", "DMB MAF, extending DA0A, with 3GPP timed text, DID, TVA, REL, IPMP" },
    {   "da1a", "DMB MAF audio with ER-BSAC audio, JPG/PNG/MNG images" },
    {   "da1b", "DMB MAF, extending da1a, with 3GPP timed text, DID, TVA, REL, IPMP" },
    {   "da2a", "DMB MAF aud w/ HE-AAC v2 aud, MOT slides, DLS, JPG/PNG/MNG images" },
    {   "da2b", "DMB MAF, extending da2a, with 3GPP timed text, DID, TVA, REL, IPMP" },
    {   "da3a", "DMB MAF aud with HE-AAC aud, JPG/PNG/MNG images" },
    {   "da3b", "DMB MAF, extending da3a w/ BIFS, 3GPP timed text, DID, TVA, REL, IPMP" },
    {   "dmb1", "DMB MAF supporting all the components defined in the specification" },
    {   "dmpf", "Digital Media Project" },
    {   "drc1", "Dirac (wavelet compression), encapsulated in ISO base media (MP4)" },
    {   "dv1a", "DMB MAF vid w/ AVC vid, ER-BSAC aud, BIFS, JPG/PNG/MNG images, TS" },
    {   "dv1b", "DMB MAF, extending dv1a, with 3GPP timed text, DID, TVA, REL, IPMP" },
    {   "dv2a", "DMB MAF vid w/ AVC vid, HE-AAC v2 aud, BIFS, JPG/PNG/MNG images, TS" },
    {   "dv2b", "DMB MAF, extending dv2a, with 3GPP timed text, DID, TVA, REL, IPMP" },
    {   "dv3a", "DMB MAF vid w/ AVC vid, HE-AAC aud, BIFS, JPG/PNG/MNG images, TS" },
    {   "dv3b", "DMB MAF, extending dv3a, with 3GPP timed text, DID, TVA, REL, IPMP" },
    {   "dvr1", "DVB (.DVB) over RTP" },
    {   "dvt1", "DVB (.DVB) over MPEG-2 Transport Stream" },
    {   "isc2", "ISMACryp 2.0 Encrypted File" },
    {   "iso2", "MP4 Base Media v2 [ISO 14496-12:2005]" },
    {   "isom", "MP4 Base Media v1 [IS0 14496-12:2003]" },
    {   "jpm ", "JPEG 2000 Compound Image (.JPM) [ISO 15444-6]" },
    {   "jpx ", "JPEG 2000 with extensions (.JPX) [ISO 15444-2]" },
    {   "mj2s", "Motion JPEG 2000 [ISO 15444-3] Simple Profile" },
    {   "mjp2", "Motion JPEG 2000 [ISO 15444-3] General Profile" },
    {   "mmp4", "MPEG-4/3GPP Mobile Profile (.MP4/3GP) (for NTT)" },
    {   "mp21", "MPEG-21 [ISO/IEC 21000-9]" },
    {   "mp41", "MP4 v1 [ISO 14496-1:ch13]" },
    {   "mp42", "MP4 v2 [ISO 14496-14]" },
    {   "mp71", "MP4 w/ MPEG-7 Metadata [per ISO 14496-12]" },
    {   "mqt ", "Sony / Mobile QuickTime (.MQV) US Patent 7,477,830 (Sony Corp)" },
    {   "niko", "Nikon" },
    {   "odcf", "OMA DCF DRM Format 2.0 (OMA-TS-DRM-DCF-V2_0-20060303-A)" },
    {   "opf2", "OMA PDCF DRM Format 2.1 (OMA-TS-DRM-DCF-V2_1-20070724-C)" },
    {   "opx2", "OMA PDCF DRM + XBS extensions (OMA-TS-DRM_XBS-V1_0-20070529-C)" },
    {   "pana", "Panasonic Digital Camera" },
    {   "qt  ", "Apple QuickTime (.MOV/QT)" },
    {   "sdv ", "SD Memory Card Video" },
    {   "ssc1", "Samsung stereoscopic, single stream" },
    {   "ssc2", "Samsung stereoscopic, dual stream" },
};

extern const TagVocabulary handlerClassTags[] =  {
    {   "dhlr", "Data Handler" },
    {   "mhlr", "Media Handler" }
};

extern const TagVocabulary handlerTypeTags[] =  {
    {   "alis", "Alias Data" },
    {   "crsm", "Clock Reference" },
    {   "hint", "Hint Track" },
    {   "ipsm", "IPMP" },
    {   "m7sm", "MPEG-7 Stream" },
    {   "mdir", "Metadata" },
    {   "mdta", "Metadata Tags" },
    {   "mjsm", "MPEG-J" },
    {   "ocsm", "Object Content" },
    {   "odsm", "Object Descriptor" },
    {   "sdsm", "Scene Description" },
    {   "soun", "Audio Track" },
    {   "text", "Text" },
    {   "tmcd", "Time Code" },
    {   "url ", "URL" },
    {   "vide", "Video Track" }
};

extern const TagVocabulary vendorIDTags[] =  {
    {   "FFMP", "FFmpeg" },
    {   "appl", "Apple" },
    {   "olym", "Olympus" },
    {   "GIC ", "General Imaging Co." },
    {   "fe20", "Olympus (fe20)" },
    {   "pana", "Panasonic" },
    {   "KMPI", "Konica-Minolta" },
    {   "kdak", "Kodak" },
    {   "pent", "Pentax" },
    {   "NIKO", "Nikon" },
    {   "leic", "Leica" },
    {   "pr01", "Olympus (pr01)" },
    {   "SMI ", "Sorenson Media Inc." },
    {   "mino", "Minolta" },
    {   "sany", "Sanyo" },
    {   "ZORA", "Zoran Corporation" },
    {   "niko", "Nikon" }
};

extern const TagVocabulary cameraByteOrderTags[] =  {
    {   "II", "Little-endian (Intel, II)" },
    {   "MM", "Big-endian (Motorola, MM)" }
};

extern const TagDetails graphicsModetags[] = {
    {   0x0, "srcCopy" },
    {   0x1, "srcOr" },
    {   0x2, "srcXor" },
    {   0x3, "srcBic" },
    {   0x4, "notSrcCopy" },
    {   0x5, "notSrcOr" },
    {   0x6, "notSrcXor" },
    {   0x7, "notSrcBic" },
    {   0x8, "patCopy" },
    {   0x9, "patOr" },
    {   0xa, "patXor" },
    {   0xb, "patBic" },
    {   0xc, "notPatCopy" },
    {   0xd, "notPatOr" },
    {   0xe, "notPatXor" },
    {   0xf, "notPatBic" },
    {   0x20, "blend" },
    {   0x21, "addPin" },
    {   0x22, "addOver" },
    {   0x23, "subPin" },
    {   0x24, "transparent" },
    {   0x25, "addMax" },
    {   0x26, "subOver" },
    {   0x27, "addMin" },
    {   0x31, "grayishTextOr" },
    {   0x32, "hilite" },
    {   0x40, "ditherCopy" },
    {   0x100, "Alpha" },
    {   0x101, "White Alpha" },
    {   0x102, "Pre-multiplied Black Alpha" },
    {   0x110, "Component Alpha" }
};


extern const TagVocabulary userDatatags[] = {
    {   "AllF", "PlayAllFrames" },
    {   "CNCV", "CompressorVersion" },
    {   "CNFV", "FirmwareVersion" },
    {   "CNMN", "Model" },
    {   "CNTH", "CanonCNTH" },
    {   "DcMD", "DcMD" },
    {   "FFMV", "FujiFilmFFMV" },
    {   "INFO", "SamsungINFO" },
    {   "LOOP", "LoopStyle" },
    {   "MMA0", "MinoltaMMA0" },
    {   "MMA1", "MinoltaMMA1" },
    {   "MVTG", "FujiFilmMVTG" },
    {   "NCDT", "NikonNCDT" },
    {   "PANA", "PanasonicPANA" },
    {   "PENT", "PentaxPENT" },
    {   "PXMN", "MakerNotePentax5b" },
    {   "PXTH", "PentaxPreview" },
    {   "QVMI", "CasioQVMI" },
    {   "SDLN", "PlayMode" },
    {   "SelO", "PlaySelection" },
    {   "TAGS", "KodakTags/KonicaMinoltaTags/MinoltaTags/NikonTags/OlympusTags/PentaxTags/SamsungTags/SanyoMOV/SanyoMP4" },
    {   "WLOC", "WindowLocation" },
    {   "XMP_", "XMP" },
    {   "Xtra", "Xtra" },
    {   "hinf", "HintTrackInfo" },
    {   "hinv", "HintVersion" },
    {   "hnti", "Hint" },
    {   "meta", "Meta" },
    {   "name", "Name" },
    {   "ptv ", "PrintToVideo" },
    {   "scrn", "OlympusPreview" },
    {   "thmb", "MakerNotePentax5a/OlympusThumbnail" },
};

extern const TagVocabulary userDataReferencetags[] = {
    {   "CNCV", "Xmp.video.CompressorVersion" },
    {   "CNFV", "Xmp.video.FirmwareVersion" },
    {   "CNMN", "Xmp.video.Model" },
    {   "NCHD", "Xmp.video.MakerNoteType" },
    {   "WLOC", "Xmp.video.WindowLocation" },
    {   "SDLN", "Xmp.video.PlayMode" },
    {   "FFMV", "Xmp.video.StreamName" },
    {   "SelO", "Xmp.video.PlaySelection" },
    {   "name", "Xmp.video.Name" },
    {   "vndr", "Xmp.video.Vendor" },
    {   " ART", "Xmp.video.Artist" },
    {   " alb", "Xmp.video.Album" },
    {   " arg", "Xmp.video.Arranger" },
    {   " ark", "Xmp.video.ArrangerKeywords" },
    {   " cmt", "Xmp.video.Comment" },
    {   " cok", "Xmp.video.ComposerKeywords" },
    {   " com", "Xmp.video.Composer" },
    {   " cpy", "Xmp.video.Copyright" },
    {   " day", "Xmp.video.CreateDate" },
    {   " dir", "Xmp.video.Director" },
    {   " ed1", "Xmp.video.Edit1" },
    {   " ed2", "Xmp.video.Edit2" },
    {   " ed3", "Xmp.video.Edit3" },
    {   " ed4", "Xmp.video.Edit4" },
    {   " ed5", "Xmp.video.Edit5" },
    {   " ed6", "Xmp.video.Edit6" },
    {   " ed7", "Xmp.video.Edit7" },
    {   " ed8", "Xmp.video.Edit8" },
    {   " ed9", "Xmp.video.Edit9" },
    {   " enc", "Xmp.video.Encoder" },
    {   " fmt", "Xmp.video.Format" },
    {   " gen", "Xmp.video.Genre" },
    {   " grp", "Xmp.video.Grouping" },
    {   " inf", "Xmp.video.Information" },
    {   " isr", "Xmp.video.ISRCCode" },
    {   " lab", "Xmp.video.RecordLabelName" },
    {   " lal", "Xmp.video.RecordLabelURL" },
    {   " lyr", "Xmp.video.Lyrics" },
    {   " mak", "Xmp.video.Make" },
    {   " mal", "Xmp.video.MakerURL" },
    {   " mod", "Xmp.video.Model" },
    {   " nam", "Xmp.video.Title" },
    {   " pdk", "Xmp.video.ProducerKeywords" },
    {   " phg", "Xmp.video.RecordingCopyright" },
    {   " prd", "Xmp.video.Producer" },
    {   " prf", "Xmp.video.Performers" },
    {   " prk", "Xmp.video.PerformerKeywords" },
    {   " prl", "Xmp.video.PerformerURL" },
    {   " req", "Xmp.video.Requirements" },
    {   " snk", "Xmp.video.SubtitleKeywords" },
    {   " snm", "Xmp.video.Subtitle" },
    {   " src", "Xmp.video.SourceCredits" },
    {   " swf", "Xmp.video.SongWriter" },
    {   " swk", "Xmp.video.SongWriterKeywords" },
    {   " swr", "Xmp.video.SoftwareVersion" },
    {   " too", "Xmp.video.Encoder" },
    {   " trk", "Xmp.video.Track" },
    {   " wrt", "Xmp.video.Composer" },
    {   " xyz", "Xmp.video.GPSCoordinates" },
    {   "CMbo", "Xmp.video.CameraByteOrder" },
    {   "Cmbo", "Xmp.video.CameraByteOrder" },
};

extern const TagDetails NikonNCTGTags[] = {
    {       0x0001, "Xmp.video.Make" },
    {       0x0002, "Xmp.video.Model" },
    {       0x0003, "Xmp.video.Software" },
    {       0x0011, "Xmp.video.CreationDate" },
    {       0x0012, "Xmp.video.DateTimeOriginal" },
    {       0x0013, "Xmp.video.FrameCount" },
    {       0x0016, "Xmp.video.FrameRate" },
    {       0x0022, "Xmp.video.FrameWidth" },
    {       0x0023, "Xmp.video.FrameHeight" },
    {       0x0032, "Xmp.audio.channelType" },
    {       0x0033, "Xmp.audio.BitsPerSample" },
    {       0x0034, "Xmp.audio.sampleRate" },
    {    0x1108822, "Xmp.video.ExposureProgram" },
    {    0x1109204, "Xmp.video.ExposureCompensation" },
    {    0x1109207, "Xmp.video.MeteringMode" },
    {    0x110a434, "Xmp.video.LensModel" },
    {    0x1200000, "Xmp.video.GPSVersionID" },
    {    0x1200001, "Xmp.video.GPSLatitudeRef" },
    {    0x1200002, "Xmp.video.GPSLatitude" },
    {    0x1200003, "Xmp.video.GPSLongitudeRef" },
    {    0x1200004, "Xmp.video.GPSLongitude" },
    {    0x1200005, "Xmp.video.GPSAltitudeRef" },
    {    0x1200006, "Xmp.video.GPSAltitude" },
    {    0x1200007, "Xmp.video.GPSTimeStamp" },
    {    0x1200008, "Xmp.video.GPSSatellites" },
    {    0x1200010, "Xmp.video.GPSImgDirectionRef" },
    {    0x1200011, "Xmp.video.GPSImgDirection" },
    {    0x1200012, "Xmp.video.GPSMapDatum" },
    {    0x120001d, "Xmp.video.GPSDateStamp" },
    {    0x2000001, "Xmp.video.MakerNoteVersion" },
    {    0x2000005, "Xmp.video.WhiteBalance" },
    {    0x200000b, "Xmp.video.WhiteBalanceFineTune" },
    {    0x200001e, "Xmp.video.ColorSpace" },
    {    0x2000023, "Xmp.video.PictureControlData" },
    {    0x2000024, "Xmp.video.WorldTime" },
    {    0x200002c, "Xmp.video.UnknownInfo" },
    {    0x2000032, "Xmp.video.UnknownInfo2" },
    {    0x2000039, "Xmp.video.LocationInfo" },
    {    0x2000083, "Xmp.video.LensType" },
    {    0x2000084, "Xmp.video.LensModel" },
    {    0x20000ab, "Xmp.video.VariProgram" },
};

extern const TagDetails NikonColorSpace[] = {
    {       1, "sRGB" },
    {       2, "Adobe RGB" },
};

extern const TagVocabulary NikonGPS_Latitude_Longitude_ImgDirection_Reference[] = {
    {   "N", "North" },
    {   "S", "South" },
    {   "E", "East" },
    {   "W", "West" },
    {   "M", "Magnetic North" },
    {   "T", "True North" },
};

extern const TagDetails NikonGPSAltitudeRef[] = {
    {   0, "Above Sea Level" },
    {   1, "Below Sea Level" },
};

extern const TagDetails NikonExposureProgram[] = {
    {   0, "Not Defined" },
    {   1, "Manual" },
    {   2, "Program AE" },
    {   3, "Aperture-priority AE" },
    {   4, "Shutter speed priority AE" },
    {   5, "Creative (Slow speed)" },
    {   6, "Action (High speed)" },
    {   7, "Portrait" },
    {   8, "Landscape" },
};

extern const TagDetails NikonMeteringMode[] = {
    {   0, "Unknown" },
    {   1, "Average" },
    {   2, "Center-weighted average" },
    {   3, "Spot" },
    {   4, "Multi-spot" },
    {   5, "Multi-segment" },
    {   6, "Partial" },
    { 255, "Other" },
};

extern const TagDetails PictureControlAdjust[] = {
    {   0, "Default Settings" },
    {   1, "Quick Adjust" },
    {   2, "Full Control" },
};

//! Contrast and Sharpness
extern const TagDetails NormalSoftHard[] = {
    {   0, "Normal" },
    {   1, "Soft"   },
    {   2, "Hard"   }
};

//! Saturation
extern const TagDetails Saturation[] = {
    {   0, "Normal" },
    {   1, "Low"    },
    {   2, "High"   }
};

//! YesNo, used for DaylightSavings
extern const TagDetails YesNo[] = {
    {   0, "No"    },
    {   1, "Yes"   }
};

//! DateDisplayFormat
extern const TagDetails DateDisplayFormat[] = {
    {   0, "Y/M/D" },
    {   1, "M/D/Y" },
    {   2, "D/M/Y" }
};

extern const TagDetails FilterEffect[] = {
    {   0x80, "Off" },
    {   0x81, "Yellow"    },
    {   0x82, "Orange"   },
    {   0x83, "Red"   },
    {   0x84, "Green"   },
    {   0xff, "n/a"   },
};

extern const TagDetails ToningEffect[] = {
    {   0x80, "B&W" },
    {   0x81, "Sepia" },
    {   0x82, "Cyanotype" },
    {   0x83, "Red" },
    {   0x84, "Yellow" },
    {   0x85, "Green" },
    {   0x86, "Blue-green" },
    {   0x87, "Blue" },
    {   0x88, "Purple-blue" },
    {   0x89, "Red-purple" },
    {   0xff, "n/a" },
};

extern const TagDetails whiteBalance[] = {
    {   0, "Auto" },
    {   1, "Daylight" },
    {   2, "Shade" },
    {   3, "Fluorescent" },
    {   4, "Tungsten" },
    {   5, "Manual" },
};

enum movieHeaderTags {
    MovieHeaderVersion, CreateDate, ModifyDate, TimeScale, Duration, PreferredRate, PreferredVolume,
    PreviewTime = 18, PreviewDuration,PosterTime, SelectionTime, SelectionDuration, CurrentTime, NextTrackID
};
enum trackHeaderTags {
    TrackHeaderVersion, TrackCreateDate, TrackModifyDate, TrackID, TrackDuration = 5, TrackLayer = 8,
    TrackVolume, ImageWidth = 19, ImageHeight
};
enum mediaHeaderTags {
    MediaHeaderVersion, MediaCreateDate, MediaModifyDate, MediaTimeScale, MediaDuration, MediaLanguageCode
};
enum handlerTags {
    HandlerClass = 1, HandlerType, HandlerVendorID
};
enum videoHeaderTags {
    GraphicsMode = 2, OpColor
};
enum stream {
    Video, Audio, Hint, Null, GenMediaHeader
};
enum imageDescTags {
    codec, VendorID = 4, SourceImageWidth_Height = 7,  XResolution,
    YResolution, CompressorName = 10, BitDepth
};
enum audioDescTags {
    AudioFormat, AudioVendorID = 4, AudioChannels, AudioSampleRate = 7, MOV_AudioFormat = 13
};

/*!
      @brief Function used to check equality of a Tags with a
          particular string (ignores case while comparing).
      @param buf Data buffer that will contain Tag to compare
      @param str char* Pointer to string
      @return Returns true if the buffer value is equal to string.
     */
bool equalsQTimeTag(Exiv2::DataBuf& buf ,const char* str) {
    for(int i = 0; i < 4; ++i)
        if(tolower(buf.pData_[i]) != tolower(str[i]))
            return false;
    return true;
}

/*!
      @brief Function used to ignore Tags and values stored in them,
          since they are not necessary as metadata information
      @param buf Data buffer that will contain Tag to compare
      @return Returns true, if Tag is found in the ignoreList[]
     */
bool ignoreList (Exiv2::DataBuf& buf) {
    const char ignoreList[13][5] = {
        "mdat", "edts", "junk", "iods", "alis", "stsc", "stsz", "stco", "ctts", "stss",
        "skip", "wide", "cmvd",
    };

    for(int i = 0 ; i < 13 ; ++i)
        if(equalsQTimeTag(buf, ignoreList[i]))
            return true;

    return false;
}

/*!
      @brief Function used to ignore Tags, basically Tags which
          contain other tags inside them, since they are not necessary
          as metadata information
      @param buf Data buffer that will contain Tag to compare
      @return Returns true, if Tag is found in the ignoreList[]
     */
bool dataIgnoreList (Exiv2::DataBuf& buf) {
    const char ignoreList[8][5] = {
        "moov", "mdia", "minf", "dinf", "alis", "stbl", "cmov",
        "meta",
    };

    for(int i = 0 ; i < 8 ; ++i)
        if(equalsQTimeTag(buf, ignoreList[i]))
            return true;

    return false;
}

void reverseTagVocabulary(const TagVocabulary inputTagVocabulary[],TagVocabulary  outputTagVocabulary[] ,int size)
{
    for (int i=0; i<size ;i++)
    {
        outputTagVocabulary[i].label_ = inputTagVocabulary[i].voc_;
        outputTagVocabulary[i].voc_ = inputTagVocabulary[i].label_;
    }
}

void reverseTagDetails(const TagDetails inputTagVocabulary[],RevTagDetails  outputTagVocabulary[] ,int size)
{
    for (int i=0; i<size ;i++)
    {
        outputTagVocabulary[i].label_ = inputTagVocabulary[i].label_;
        outputTagVocabulary[i].val_ = inputTagVocabulary[i].val_;
    }
}

const std::vector<ushort> getNumberFromString(const std::string stringData,char seperator )
{
    vector<ushort> shortValues;
    for(int i=0; i<2; i++)
    {
        int counter =0;
        byte tmpValue[4];
        while((counter < stringData.size()) && (stringData[counter] != seperator) && (stringData[counter] != '\0'))
        {
            tmpValue[counter] = stringData[counter];
            counter++;
        }
        shortValues.push_back((short)Exiv2::getShort(tmpValue, bigEndian));
    }
    return shortValues;
}

const std::vector<long> getLongFromString(const std::string stringData,char seperator )
{
    vector<long> longValues;
    for(int i=0; i<2; i++)
    {
        int counter =0;
        byte tmpValue[4];
        while((counter < stringData.size()) && (stringData[counter] != seperator) && (stringData[counter] != '\0'))
        {
            tmpValue[counter] = stringData[counter];
            counter++;
        }
        longValues.push_back((long)Exiv2::getLong(tmpValue, bigEndian));
    }
    return longValues;
}
/*!
      @brief Function used to convert buffer data into 64-bit
          signed integer, information stored in Big Endian format
      @param buf Data buffer that will contain data to be converted
      @return Returns a signed 64-bit integer
     */
int64_t returnBufValue(Exiv2::DataBuf& buf, int n = 4) {
    int64_t temp = 0;
    for(int i = n - 1; i >= 0; i--)
#ifdef _MSC_VER
        temp = temp + static_cast<int64_t>(buf.pData_[i]*(pow(static_cast<float>(256), n-i-1)));
#else
        temp = temp + buf.pData_[i]*(pow((float)256,n-i-1));
#endif

    return temp;
}

DataBuf returnBuf(int64_t intValue,int n=4)
{
    DataBuf buf((unsigned long)(n+1));
    buf.pData_[n] = '\0';
    for(int i = 0; i < n; i++)
    {
#ifdef _MSC_VER
        buf.pData_[n-i-1] = static_cast<byte> (intValue - (intValue/(pow(static_cast<float>(256), n-i))));
        intValue = static_cast<int64_t>(intValue/(pow(static_cast<float>(256), n-i)));
#else
        buf.pData_[n-i-1] = (byte)(intValue - intValue/(pow((float)256,n-i)));
        intValue = (int64_t) (intValue/(pow((float)256,n-i)));
#endif
    }
    return buf;
}
/*!
      @brief Function used to convert buffer data into 64-bit
          unsigned integer, information stored in Big Endian format
      @param buf Data buffer that will contain data to be converted
      @return Returns an unsigned 64-bit integer
     */
uint64_t returnUnsignedBufValue(Exiv2::DataBuf& buf, int n = 4) {
    uint64_t temp = 0;
    for(int i = n-1; i >= 0; i--)
#if _MSC_VER
        temp = temp + static_cast<uint64_t>(buf.pData_[i]*(pow(static_cast<float>(256), n-i-1)));
#else
        temp = temp + buf.pData_[i]*(pow((float)256,n-i-1));
#endif

    return temp;
}

DataBuf returnBuf(uint64_t intValue,int n=4)
{
    DataBuf buf((unsigned long)(n+1));
    buf.pData_[n] = '\0';
    for(int i = 0; i < n; i++)
    {
#ifdef _MSC_VER
        buf.pData_[n-i-1] = static_cast<byte> (intValue - (intValue/(pow(static_cast<float>(256), n-i))));
        intValue = static_cast<uint64_t>(intValue/(pow(static_cast<float>(256), n-i)));
#else
        buf.pData_[n-i-1] = (byte)(intValue - intValue/(pow((float)256,n-i)));
        intValue = (uint64_t) (intValue/(pow((float)256,n-i)));
#endif
    }
    return buf;
}
/*!
      @brief Function used to quicktime files, by checking the
          the tags at the start of the file. If the Tag is any one
          of the tags listed below, then it is of Quicktime Type.
      @param a, b, c, d - characters used to compare
      @return Returns true, if Tag is found in the list qTimeTags
     */
bool isQuickTimeType (char a, char b, char c, char d) {
    char qTimeTags[][5] = {
        "PICT", "free", "ftyp", "junk", "mdat",
        "moov", "pict", "pnot", "skip",  "uuid", "wide"
    };

    for(int i = 0; i <= 10; i++)
        if(a == qTimeTags[i][0] && b == qTimeTags[i][1] && c == qTimeTags[i][2] && d == qTimeTags[i][3])
            return true;
    return false;
}

}}                                      // namespace Internal, Exiv2

namespace Exiv2 {

using namespace Exiv2::Internal;

QuickTimeVideo::QuickTimeVideo(BasicIo::AutoPtr io)
    : Image(ImageType::qtime, mdNone, io)
{
    m_decodeMetadata = true;
    m_modifyMetadata = false;
} // QuickTimeVideo::QuickTimeVideo

std::string QuickTimeVideo::mimeType() const
{
    return "video/quicktime";
}

void QuickTimeVideo::writeMetadata()
{
    if (io_->open() != 0)
    {
        throw Error(9, io_->path(), strError());
    }
    IoCloser closer(*io_);

    doWriteMetadata();

    io_->close();
}

void QuickTimeVideo::doWriteMetadata()
{
    // Ensure that this is the correct image type
    if (!isQTimeType(*io_, false)) {
        if (io_->error() || io_->eof()) throw Error(14);
        throw Error(3, "QuickTime");
    }

    m_decodeMetadata = false;
    m_modifyMetadata = true;
    continueTraversing_ = true;

    while (continueTraversing_) decodeBlock();
}

void QuickTimeVideo::readMetadata()
{
    if (io_->open() != 0) throw Error(9, io_->path(), strError());

    // Ensure that this is the correct image type
    if (!isQTimeType(*io_, false))
    {
        if (io_->error() || io_->eof()) throw Error(14);
        throw Error(3, "QuickTime");
    }

    IoCloser closer(*io_);
    clearMetadata();
    continueTraversing_ = true;
    height_ = width_ = 1;

    xmpData_["Xmp.video.FileSize"] = (double)io_->size()/(double)1048576;
    xmpData_["Xmp.video.FileName"] = io_->path();
    xmpData_["Xmp.video.MimeType"] = mimeType();

    while (continueTraversing_) decodeBlock();

    aspectRatio();
} // QuickTimeVideo::readMetadata

void QuickTimeVideo::decodeBlock()
{
    const long bufMinSize = 4;
    DataBuf buf(bufMinSize+1);
    unsigned long size = 0;
    buf.pData_[4] = '\0' ;

    std::memset(buf.pData_, 0x0, buf.size_);

    io_->read(buf.pData_, 4);
    if(io_->eof()) {
        continueTraversing_ = false;
        return;
    }

    size = Exiv2::getULong(buf.pData_, bigEndian);
    io_->read(buf.pData_, 4);
    if(size < 8)
        return;

    //      std::cerr<<"\nTag=>"<<buf.pData_<<"     size=>"<<size-8;
    tagDecoder(buf,size-8);
} // QuickTimeVideo::decodeBlock

void QuickTimeVideo::tagDecoder(Exiv2::DataBuf &buf, unsigned long size)
{
    if (ignoreList(buf))
        discard(size);

    else if (dataIgnoreList(buf))
        decodeBlock();

    else if (equalsQTimeTag(buf, "ftyp"))
        fileTypeDecoder(size);

    else if (equalsQTimeTag(buf, "trak"))
        setMediaStream();

    else if (equalsQTimeTag(buf, "mvhd"))
        movieHeaderDecoder(size);

    else if (equalsQTimeTag(buf, "tkhd"))
        trackHeaderDecoder(size);

    else if (equalsQTimeTag(buf, "mdhd"))
        mediaHeaderDecoder(size);

    else if (equalsQTimeTag(buf, "hdlr"))
        handlerDecoder(size);

    else if (equalsQTimeTag(buf, "vmhd"))
        videoHeaderDecoder(size);

    else if (equalsQTimeTag(buf, "udta"))
        userDataDecoder(size);

    else if (equalsQTimeTag(buf, "dref"))
        multipleEntriesDecoder();

    else if (equalsQTimeTag(buf, "stsd"))
        sampleDesc(size);

    else if (equalsQTimeTag(buf, "stts"))
        timeToSampleDecoder();

    else if (equalsQTimeTag(buf, "pnot"))
        previewTagDecoder(size);

    else if (equalsQTimeTag(buf, "tapt"))
        trackApertureTagDecoder(size);

    else if (equalsQTimeTag(buf, "keys"))
        keysTagDecoder(size);

    else if (equalsQTimeTag(buf, "url "))
    {
        if(!m_modifyMetadata)
        {
            io_->read(buf.pData_, size);
            if (currentStream_ == Video)
                xmpData_["Xmp.video.URL"] = Exiv2::toString(buf.pData_);
            else if (currentStream_ == Audio)
                xmpData_["Xmp.audio.URL"] = Exiv2::toString(buf.pData_);
        }
        else
        {
            if (currentStream_ == Video && xmpData_["Xmp.video.URL"].count() >0)
            {
                byte rawURL[size];
                const std::string URL = xmpData_["Xmp.video.URL"].toString();
                for(int j=0; j<size; j++)
                {
                    rawURL[j] = (byte)URL[j];
                }
                io_->write(rawURL,size);
            }
            else if (currentStream_ == Audio && xmpData_["Xmp.audio.URL"].count() >0)
            {
                byte rawURL[size];
                const std::string URL = xmpData_["Xmp.audio.URL"].toString();
                for(int j=0; j<size; j++)
                {
                    rawURL[j] = (byte)URL[j];
                }
                io_->write(rawURL,size);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }
        }
    }

    else if (equalsQTimeTag(buf, "urn "))
    {
        if(!m_modifyMetadata)
        {
            io_->read(buf.pData_, size);
            if (currentStream_ == Video)
                xmpData_["Xmp.video.URN"] = Exiv2::toString(buf.pData_);
            else if (currentStream_ == Audio)
                xmpData_["Xmp.audio.URN"] = Exiv2::toString(buf.pData_);
        }
        else
        {
            if (currentStream_ == Video && xmpData_["Xmp.video.URN"].count() >0)
            {
                byte rawURN[size];
                const std::string URN = xmpData_["Xmp.video.URN"].toString();
                for(int j=0; j<size; j++)
                {
                    rawURN[j] = (byte)URN[j];
                }
                io_->write(rawURN,size);
            }
            else if (currentStream_ == Audio && xmpData_["Xmp.audio.URN"].count() >0)
            {
                byte rawURN[size];
                const std::string URN = xmpData_["Xmp.audio.URN"].toString();
                for(int j=0; j<size; j++)
                {
                    rawURN[j] = (byte)URN[j];
                }
                io_->write(rawURN,size);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }
        }
    }

    else if (equalsQTimeTag(buf, "dcom"))
    {
        if(!m_modifyMetadata)
        {
            io_->read(buf.pData_, size);
            xmpData_["Xmp.video.Compressor"] = Exiv2::toString(buf.pData_);
        }
        else
        {
            byte rawCompressor[size];
            const std::string compressor = xmpData_["Xmp.video.Compressor"].toString();
            for(int j=0; j<size; j++)
            {
                rawCompressor[j] = (byte)compressor[j];
            }
            io_->write(rawCompressor,size);
        }
    }

    else if (equalsQTimeTag(buf, "smhd"))
    {
        io_->seek(4,BasicIo::cur);
        if(!m_modifyMetadata)
        {
            io_->read(buf.pData_, 4);
            xmpData_["Xmp.audio.Balance"] = returnBufValue(buf, 2);
        }
        else
        {
            byte rawBalance[size];
            const std::string balance = xmpData_["Xmp.audio.Balance"].toString();
            for(int j=0; j<size; j++)
            {
                rawBalance[j] = (byte)balance[j];
            }
            io_->write(rawBalance,size);
        }
    }

    else
    {
        discard(size);
    }
} // QuickTimeVideo::tagDecoder

void QuickTimeVideo::discard(unsigned long size)
{
    uint64_t cur_pos = io_->tell();
    io_->seek(cur_pos + size, BasicIo::beg);
} // QuickTimeVideo::discard

void QuickTimeVideo::previewTagDecoder(unsigned long size)
{
    DataBuf buf(4);
    uint64_t cur_pos = io_->tell();
    if(!m_modifyMetadata)
    {
        io_->read(buf.pData_, 4);
        xmpData_["Xmp.video.PreviewDate"] = getULong(buf.pData_, bigEndian);
        io_->read(buf.pData_, 2);
        xmpData_["Xmp.video.PreviewVersion"] = getShort(buf.pData_, bigEndian);

        io_->read(buf.pData_, 4);
        if(equalsQTimeTag(buf, "PICT"))
            xmpData_["Xmp.video.PreviewAtomType"] = "QuickDraw Picture";
        else
            xmpData_["Xmp.video.PreviewAtomType"] = Exiv2::toString(buf.pData_);

        io_->seek(cur_pos + size, BasicIo::beg);
    }
    else
    {
        if(xmpData_["Xmp.video.PreviewDate"].count() >0)
        {
            byte rawPreviewDate[4];
            const long previewDate = xmpData_["Xmp.video.PreviewDate"].toLong();
            memcpy(rawPreviewDate,&previewDate,4);
            io_->write(rawPreviewDate,4);
        }
        else
        {
            io_->seek(4,BasicIo::cur);
        }
        if(xmpData_["Xmp.video.PreviewVersion"].count() >0)
        {
            byte rawPreviewVersion[2];
            const ushort previewVersion = (ushort) xmpData_["Xmp.video.PreviewVersion"].toLong();
            memcpy(rawPreviewVersion,&previewVersion,2);
            io_->write(rawPreviewVersion,2);
        }
        else
        {
            io_->seek(2,BasicIo::cur);
        }
        if(xmpData_["Xmp.video.PreviewAtomType"].count() >0)
        {
            DataBuf buf((unsigned long)5);
            byte rawPreviewAtomType[4] = {'P','I','C','T'};
            const std::string previewAtomType = xmpData_["Xmp.video.PreviewAtomType"].toString();
            for(int j=0; j<4; j++)
            {
                buf.pData_[j] = previewAtomType[j];
            }
            if(!equalsQTimeTag(buf, "PICT"))
            {
                io_->write(buf.pData_,4);
            }
            else
            {
                io_->write(rawPreviewAtomType,4);
            }
        }
        else
        {
            io_->seek(4,BasicIo::cur);
        }
        io_->seek(cur_pos + size, BasicIo::beg);
    }
} // QuickTimeVideo::previewTagDecoder

void QuickTimeVideo::keysTagDecoder(unsigned long size)
{
    DataBuf buf(4);
    if(!m_modifyMetadata)
    {
        uint64_t cur_pos = io_->tell();
        io_->read(buf.pData_, 4);
        xmpData_["Xmp.video.PreviewDate"] = getULong(buf.pData_, bigEndian);
        io_->read(buf.pData_, 2);
        xmpData_["Xmp.video.PreviewVersion"] = getShort(buf.pData_, bigEndian);

        io_->read(buf.pData_, 4);
        if(equalsQTimeTag(buf, "PICT"))
            xmpData_["Xmp.video.PreviewAtomType"] = "QuickDraw Picture";
        else
            xmpData_["Xmp.video.PreviewAtomType"] = Exiv2::toString(buf.pData_);

        io_->seek(cur_pos + size, BasicIo::beg);
    }
    else
    {
        if(xmpData_["Xmp.video.PreviewDate"].count() >0)
        {
            byte rawpreviewDate[4];
            const long previewDate = xmpData_["Xmp.video.PreviewDate"].toLong();
            memcpy(rawpreviewDate,&previewDate,4);
            io_->write(rawpreviewDate,4);
        }
        else
        {
            io_->seek(4,BasicIo::cur);
        }
        if(xmpData_["Xmp.video.PreviewVersion"].count() >0)
        {
            byte rawPreviewVersion[2];
            const ushort previewVersion = (ushort) xmpData_["Xmp.video.PreviewVersion"].toLong();
            memcpy(rawPreviewVersion,&previewVersion,2);
            io_->write(rawPreviewVersion,2);
        }
        else
        {
            io_->seek(2,BasicIo::cur);
        }
        if(xmpData_["Xmp.video.PreviewAtomType"].count() >0)
        {
            DataBuf buf((unsigned long)5);
            byte rawPreviewAtomType[4] = {'P','I','C','T'};
            const std::string previewAtomType = xmpData_["Xmp.video.PreviewAtomType"].toString();
            for(int j=0; j<4; j++)
            {
                buf.pData_[j] = previewAtomType[j];
            }
            if(!equalsQTimeTag(buf, "PICT"))
            {
                io_->write(buf.pData_,4);
            }
            else
            {
                io_->write(rawPreviewAtomType,4);
            }
        }
        else
        {
            io_->seek(4,BasicIo::cur);
        }
    }
} // QuickTimeVideo::keysTagDecoder

void QuickTimeVideo::trackApertureTagDecoder(unsigned long size)
{
    DataBuf buf(4), buf2(2);
    uint64_t cur_pos = io_->tell();
    if(!m_modifyMetadata)
    {
        byte n = 3;
        while(n--)
        {
            io_->seek(static_cast<long>(4), BasicIo::cur); io_->read(buf.pData_, 4);

            if(equalsQTimeTag(buf, "clef")) {
                io_->seek(static_cast<long>(4), BasicIo::cur);
                io_->read(buf.pData_, 2); io_->read(buf2.pData_, 2);
                xmpData_["Xmp.video.CleanApertureWidth"]    =   Exiv2::toString(getUShort(buf.pData_, bigEndian))
                        + "." + Exiv2::toString(getUShort(buf2.pData_, bigEndian));
                io_->read(buf.pData_, 2); io_->read(buf2.pData_, 2);
                xmpData_["Xmp.video.CleanApertureHeight"]   =   Exiv2::toString(getUShort(buf.pData_, bigEndian))
                        + "." + Exiv2::toString(getUShort(buf2.pData_, bigEndian));
            }

            else if(equalsQTimeTag(buf, "prof")) {
                io_->seek(static_cast<long>(4), BasicIo::cur);
                io_->read(buf.pData_, 2); io_->read(buf2.pData_, 2);
                xmpData_["Xmp.video.ProductionApertureWidth"]    =   Exiv2::toString(getUShort(buf.pData_, bigEndian))
                        + "." + Exiv2::toString(getUShort(buf2.pData_, bigEndian));
                io_->read(buf.pData_, 2); io_->read(buf2.pData_, 2);
                xmpData_["Xmp.video.ProductionApertureHeight"]   =   Exiv2::toString(getUShort(buf.pData_, bigEndian))
                        + "." + Exiv2::toString(getUShort(buf2.pData_, bigEndian));
            }

            else if(equalsQTimeTag(buf, "enof")) {
                io_->seek(static_cast<long>(4), BasicIo::cur);
                io_->read(buf.pData_, 2); io_->read(buf2.pData_, 2);
                xmpData_["Xmp.video.EncodedPixelsWidth"]    =   Exiv2::toString(getUShort(buf.pData_, bigEndian))
                        + "." + Exiv2::toString(getUShort(buf2.pData_, bigEndian));
                io_->read(buf.pData_, 2); io_->read(buf2.pData_, 2);
                xmpData_["Xmp.video.EncodedPixelsHeight"]   =   Exiv2::toString(getUShort(buf.pData_, bigEndian))
                        + "." + Exiv2::toString(getUShort(buf2.pData_, bigEndian));
            }
        }
        io_->seek(static_cast<long>(cur_pos + size), BasicIo::beg);
    }
    else
    {
        io_->seek(4,BasicIo::cur);
        io_->read(buf.pData_,4);
        if(xmpData_["Xmp.video.CleanApertureWidth"].count() >0)
        {
            if(equalsQTimeTag(buf, "clef"))
            {
                char rawCleanApertureWidthOne[4]; char rawCleanApertureWidthTwo[2];
                byte rawCleanApertureWidth[6];
                const std::string cleanApertureWidth =  xmpData_["Xmp.video.CleanApertureWidth"].toString();
                vector<ushort> cleanApertureWidthValue = getNumberFromString(cleanApertureWidth,'.');
                const long tmpValue = (long)cleanApertureWidthValue.at(0);
                memcpy(rawCleanApertureWidthOne,&tmpValue,4);
                memcpy(rawCleanApertureWidthTwo,&cleanApertureWidthValue.at(1),2);
                for(int j=0; j<4; j++)
                {
                    rawCleanApertureWidth[j] = (byte)rawCleanApertureWidthOne[j];
                }
                rawCleanApertureWidth[4] =(byte) rawCleanApertureWidthTwo[0];
                rawCleanApertureWidth[5] =(byte) rawCleanApertureWidthTwo[1];
                io_->write(rawCleanApertureWidth,6);
            }
            else
            {
                io_->seek(6,BasicIo::cur);
            }
        }
        else
        {
            io_->seek(6,BasicIo::cur);
        }
        if(xmpData_["Xmp.video.CleanApertureHeight"].count() >0)
        {
            io_->seek(-10,BasicIo::cur);
            io_->read(buf.pData_,4);
            if(equalsQTimeTag(buf, "clef"))
            {
                byte rawCleanApertureHeightOne[4]; byte rawCleanApertureHeightTwo[2];
                const std::string cleanApertureHeight = xmpData_["Xmp.video.CleanApertureHeight"].toString();
                vector<ushort> cleanApertureHeightValue = getNumberFromString(cleanApertureHeight,'.');
                const long tmpValue = (long)cleanApertureHeightValue.at(0);
                memcpy(rawCleanApertureHeightOne,&tmpValue,4);
                memcpy(rawCleanApertureHeightTwo,&cleanApertureHeightValue.at(1),2);
                io_->write(rawCleanApertureHeightOne,4);
                io_->seek(2,BasicIo::cur);
                io_->write(rawCleanApertureHeightTwo,2);
            }
            else
            {
                io_->seek(2,BasicIo::cur);
            }
        }
        else
        {
            io_->seek(2,BasicIo::cur);
        }

        io_->seek(4,BasicIo::cur);
        io_->read(buf.pData_,4);
        if(xmpData_["Xmp.video.ProductionApertureWidth"].count() >0)
        {
            if(equalsQTimeTag(buf, "prof"))
            {
                char rawProductionApertureWidthOne[4]; char rawProductionApertureWidthTwo[2];
                byte rawProductionApertureWidth[6];
                const std::string ProductionApertureWidth = xmpData_["Xmp.video.ProductionApertureWidth"].toString();
                vector<ushort> ProductionApertureWidthValue = getNumberFromString(ProductionApertureWidth,'.');
                const long tmpValue =(long)ProductionApertureWidthValue.at(0);
                memcpy(rawProductionApertureWidthOne,&tmpValue,4);
                memcpy(rawProductionApertureWidthTwo,&ProductionApertureWidthValue.at(1),2);
                for(int j=0; j<4; j++)
                {
                    rawProductionApertureWidth[j] = (byte)rawProductionApertureWidthOne[j];
                }
                rawProductionApertureWidth[4] =(byte) rawProductionApertureWidthTwo[0];
                rawProductionApertureWidth[5] =(byte) rawProductionApertureWidthTwo[1];
                io_->write(rawProductionApertureWidth,6);
            }
            else
            {
                io_->seek(6,BasicIo::cur);
            }
        }
        else
        {
            io_->seek(6,BasicIo::cur);
        }
        if(xmpData_["Xmp.video.ProductionApertureHeight"].count() >0)
        {
            io_->seek(-10,BasicIo::cur);
            io_->read(buf.pData_,4);
            if(equalsQTimeTag(buf, "prof"))
            {
                byte rawProductionApertureHeightOne[4]; byte rawProductionApertureHeightTwo[2];
                const std::string ProductionApertureHeight = xmpData_["Xmp.video.ProductionApertureHeight"].toString();
                vector<ushort> ProductionApertureHeightValue = getNumberFromString(ProductionApertureHeight,'.');
                const long tmpValue = (long)ProductionApertureHeightValue.at(0);
                memcpy(rawProductionApertureHeightOne,&tmpValue,4);
                memcpy(rawProductionApertureHeightTwo,&ProductionApertureHeightValue.at(1),2);
                io_->write(rawProductionApertureHeightOne,4);
                io_->seek(2,BasicIo::cur);
                io_->write(rawProductionApertureHeightTwo,2);
            }
            else
            {
                io_->seek(2,BasicIo::cur);
            }
        }
        else
        {
            io_->seek(2,BasicIo::cur);
        }
        io_->seek(4,BasicIo::cur);
        io_->read(buf.pData_,4);
        if(xmpData_["Xmp.video.EncodedPixelsWidth"].count() >0)
        {
            if(equalsQTimeTag(buf, "prof"))
            {
                char rawEncodedPixelsWidthOne[4]; char rawEncodedPixelsWidthTwo[2];
                byte rawEncodedPixelsWidth[6];
                const std::string EncodedPixelsWidth = xmpData_["Xmp.video.EncodedPixelsWidth"].toString();
                vector<ushort> EncodedPixelsWidthValue = getNumberFromString(EncodedPixelsWidth,'.');
                const long tmpValue = (long)EncodedPixelsWidthValue.at(0);
                memcpy(rawEncodedPixelsWidthOne,&tmpValue,4);
                memcpy(rawEncodedPixelsWidthTwo,&EncodedPixelsWidthValue.at(1),2);
                for(int j=0; j<4; j++)
                {
                    rawEncodedPixelsWidth[j] = (byte)rawEncodedPixelsWidthOne[j];
                }
                rawEncodedPixelsWidth[4] =(byte) rawEncodedPixelsWidthTwo[0];
                rawEncodedPixelsWidth[5] =(byte) rawEncodedPixelsWidthTwo[1];
                io_->write(rawEncodedPixelsWidth,6);
            }
            else
            {
                io_->seek(6,BasicIo::cur);
            }
        }
        else
        {
            io_->seek(6,BasicIo::cur);
        }
        if(xmpData_["Xmp.video.EncodedPixelsHeight"].count() >0)
        {
            io_->seek(-10,BasicIo::cur);
            io_->read(buf.pData_,4);
            if(equalsQTimeTag(buf, "prof"))
            {
                byte rawEncodedPixelsHeightOne[4]; byte rawEncodedPixelsHeightTwo[2];
                const std::string EncodedPixelsHeight = xmpData_["Xmp.video.EncodedPixelsHeight"].toString();
                vector<ushort> EncodedPixelsHeightValue = getNumberFromString(EncodedPixelsHeight,'.');
                const long tmpValue = (long)EncodedPixelsHeightValue.at(0);
                memcpy(rawEncodedPixelsHeightOne,&tmpValue,4);
                memcpy(rawEncodedPixelsHeightTwo,&EncodedPixelsHeightValue.at(1),2);
                io_->write(rawEncodedPixelsHeightOne,4);
                io_->seek(2,BasicIo::cur);
                io_->write(rawEncodedPixelsHeightTwo,2);
            }
        }
    }
} // QuickTimeVideo::trackApertureTagDecoder

void QuickTimeVideo::CameraTagsDecoder(unsigned long size_external)
{
    uint64_t cur_pos = io_->tell();
    DataBuf buf(50), buf2(4);
    const TagDetails* td;
    const RevTagDetails* rtd;
    io_->read(buf.pData_, 4);
    if(equalsQTimeTag(buf, "NIKO"))
    {
        io_->seek(cur_pos, BasicIo::beg);
        if(!m_modifyMetadata)
        {
            io_->read(buf.pData_, 24);
            xmpData_["Xmp.video.Make"] = Exiv2::toString(buf.pData_);
            io_->read(buf.pData_, 14);
            xmpData_["Xmp.video.Model"] = Exiv2::toString(buf.pData_);
            io_->read(buf.pData_, 4);
            xmpData_["Xmp.video.ExposureTime"] = "1/" + Exiv2::toString( ceil( getULong(buf.pData_, littleEndian) / (double)10));
            io_->read(buf.pData_, 4); io_->read(buf2.pData_, 4);
            xmpData_["Xmp.video.FNumber"] =  getULong(buf.pData_, littleEndian) / (double)getULong(buf2.pData_, littleEndian) ;
            io_->read(buf.pData_, 4); io_->read(buf2.pData_, 4);
            xmpData_["Xmp.video.ExposureCompensation"] =  getULong(buf.pData_, littleEndian) / (double)getULong(buf2.pData_, littleEndian) ;
            io_->read(buf.pData_, 10); io_->read(buf.pData_, 4);
            td = find(whiteBalance, getULong(buf.pData_, littleEndian));
            if (td)
                xmpData_["Xmp.video.WhiteBalance"] = exvGettext(td->label_);
            io_->read(buf.pData_, 4); io_->read(buf2.pData_, 4);
            xmpData_["Xmp.video.FocalLength"] =  getULong(buf.pData_, littleEndian) / (double)getULong(buf2.pData_, littleEndian) ;
            io_->seek(static_cast<long>(95), BasicIo::cur);
            io_->read(buf.pData_, 48);
            xmpData_["Xmp.video.Software"] = Exiv2::toString(buf.pData_);
            io_->read(buf.pData_, 4);
            xmpData_["Xmp.video.ISO"] = getULong(buf.pData_, littleEndian);
            io_->seek(cur_pos + size_external, BasicIo::beg);
        }
        else
        {
            RevTagDetails revTagDetails[(sizeof(graphicsModetags)/sizeof(graphicsModetags[0]))];
            reverseTagDetails(graphicsModetags,revTagDetails,((sizeof(graphicsModetags)/sizeof(graphicsModetags[0]))));

            if(xmpData_["Xmp.video.Make"].count() >0)
            {
                byte rawMake[24];
                const std::string make = xmpData_["Xmp.video.Make"].toString();
                for(int j=0; j<24; j++)
                {
                    rawMake[j] = make[j];
                }
                io_->write(rawMake,24);
            }
            else
            {
                io_->seek(24,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.Model"].count() >0)
            {
                byte rawModel[14];
                const std::string model = xmpData_["Xmp.video.Model"].toString();
                for(int j=0; j<14; j++)
                {
                    rawModel[j] = model[j];
                }
                io_->write(rawModel,14);
            }
            else
            {
                io_->seek(14,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.ExposureTime"].count() >0)
            {
                byte rawExposureTime[4];
                const std::vector<long> d_exposureTime = getLongFromString(xmpData_["Xmp.video.ExposureTime"].toString(),'/');
                const long tmpValue = (d_exposureTime.at(1)*10);
                memcpy(rawExposureTime,&tmpValue,4);
                io_->write(rawExposureTime,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.FNumber"].count() >0)
            {
                byte rawFNumber[4];
                const long fNumber =(long)((double)xmpData_["Xmp.video.FNumber"].toLong()*(double)getULong(buf2.pData_, littleEndian));
                memcpy(rawFNumber,&fNumber,4);
                io_->write(rawFNumber,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.ExposureCompensation"].count() >0)
            {
                byte rawExposureCompensation[4];
                const long exposureCompensation =(long)((double)xmpData_["Xmp.video.ExposureCompensation"].toLong()
                                                        *(double)getULong(buf2.pData_, littleEndian));
                memcpy(rawExposureCompensation,&exposureCompensation,4);
                io_->write(rawExposureCompensation,4);
                io_->seek(10,BasicIo::cur);
            }
            else
            {
                io_->seek(14,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.WhiteBalance"].count() >0)
            {
                byte rawWhiteBalance[2];
                const std::string whiteBalance = xmpData_["Xmp.video.WhiteBalance"].toString();
                char *whiteBalanceData = (char *)malloc(xmpData_["Xmp.video.WhiteBalance"].size());
                for(int j=0; j<xmpData_["Xmp.video.GraphicsMode"].size(); j++)
                {
                    whiteBalanceData[j] = whiteBalance[j];
                }
                rtd = find(revTagDetails,whiteBalanceData);
                short sWhiteBalance = (short)rtd->val_;
                memcpy(rawWhiteBalance,&sWhiteBalance,2);
                io_->write(rawWhiteBalance,2);}
            else
            {
                io_->seek(4,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.FocalLength"].count() >0)
            {
                byte rawFocalLength[4];
                const long focalLength =(long)((double)xmpData_["Xmp.video.FocalLength"].toLong()
                                               *(double)getULong(buf2.pData_, littleEndian));
                memcpy(rawFocalLength,&focalLength,4);
                io_->write(rawFocalLength,4);
                io_->seek(95,BasicIo::cur);
            }
            else
            {
                io_->seek(99,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.Software"].count() >0)
            {
                byte rawSoftware[48];
                const std::string software = xmpData_["Xmp.video.Software"].toString();
                for(int j=0; j<48; j++)
                {
                    rawSoftware[j] = software[j];
                }
                io_->write(rawSoftware,48);
            }
            else
            {
                io_->seek(48,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.ISO"].count() >0)
            {
                byte rawISO[4];
                const long ISO = xmpData_["Xmp.video.ISO"].toLong();
                memcpy(rawISO,&ISO,4);
                io_->write(rawISO,4);
            }
        }
    }
} // QuickTimeVideo::CameraTagsDecoder

void QuickTimeVideo::userDataDecoder(unsigned long size_external)
{
    uint64_t cur_pos = io_->tell();
    const TagVocabulary* td;
    const TagVocabulary* tv, *tv_internal;

    if(!m_modifyMetadata)
    {
        const long bufMinSize = 100;
        DataBuf buf(bufMinSize);
        unsigned long size = 0, size_internal = size_external;
        std::memset(buf.pData_, 0x0, buf.size_);

        while((size_internal/4 != 0) && (size_internal > 0))
        {

            buf.pData_[4] = '\0' ;
            io_->read(buf.pData_, 4);
            size = Exiv2::getULong(buf.pData_, bigEndian);
            if(size > size_internal)
                break;
            size_internal -= size;
            io_->read(buf.pData_, 4);

            if(buf.pData_[0] == 169)
                buf.pData_[0] = ' ';
            td = find(userDatatags, Exiv2::toString( buf.pData_));

            tv = find(userDataReferencetags, Exiv2::toString( buf.pData_));

            if(size == 0 || (size - 12) <= 0)
                break;

            else if(equalsQTimeTag(buf, "DcMD")  || equalsQTimeTag(buf, "NCDT"))
                userDataDecoder(size - 8);

            else if(equalsQTimeTag(buf, "NCTG"))
                NikonTagsDecoder(size - 8);

            else if(equalsQTimeTag(buf, "TAGS"))
                CameraTagsDecoder(size - 8);

            else if(equalsQTimeTag(buf, "CNCV") || equalsQTimeTag(buf, "CNFV")
                    || equalsQTimeTag(buf, "CNMN") || equalsQTimeTag(buf, "NCHD")
                    || equalsQTimeTag(buf, "FFMV")) {
                io_->read(buf.pData_, size - 8);
                xmpData_[exvGettext(tv->label_)] = Exiv2::toString(buf.pData_);
            }

            else if(equalsQTimeTag(buf, "CMbo") || equalsQTimeTag(buf, "Cmbo")) {
                io_->read(buf.pData_, 2);
                buf.pData_[2] = '\0' ;
                tv_internal = find(cameraByteOrderTags, Exiv2::toString( buf.pData_));

                if (tv_internal)
                    xmpData_[exvGettext(tv->label_)] = exvGettext(tv_internal->label_);
                else
                    xmpData_[exvGettext(tv->label_)] = Exiv2::toString(buf.pData_);
            }

            else if(tv) {
                io_->read(buf.pData_, 4);
                io_->read(buf.pData_, size-12);
                xmpData_[exvGettext(tv->label_)] = Exiv2::toString(buf.pData_);
            }

            else if(td)
                tagDecoder(buf,size-8);
        }

        io_->seek(cur_pos + size_external, BasicIo::beg);
    }
    else
    {
        const long bufMinSize = 100;
        DataBuf buf(bufMinSize);
        unsigned long size = 0, size_internal = size_external;
        std::memset(buf.pData_, 0x0, buf.size_);

        while((size_internal/4 != 0) && (size_internal > 0)) {

            buf.pData_[4] = '\0' ;
            io_->read(buf.pData_, 4);
            size = Exiv2::getULong(buf.pData_, bigEndian);
            if(size > size_internal)
                break;
            size_internal -= size;
            io_->read(buf.pData_, 4);

            if(buf.pData_[0] == 169)
                buf.pData_[0] = ' ';
            td = find(userDatatags, Exiv2::toString( buf.pData_));

            tv = find(userDataReferencetags, Exiv2::toString( buf.pData_));

            if(size == 0 || (size - 12) <= 0)
                break;

            else if(equalsQTimeTag(buf, "DcMD")  || equalsQTimeTag(buf, "NCDT"))
                userDataDecoder(size - 8);

            else if(equalsQTimeTag(buf, "NCTG"))
                NikonTagsDecoder(size - 8);

            else if(equalsQTimeTag(buf, "TAGS"))
                CameraTagsDecoder(size - 8);

            else if(equalsQTimeTag(buf, "CNCV") || equalsQTimeTag(buf, "CNFV")
                    || equalsQTimeTag(buf, "CNMN") || equalsQTimeTag(buf, "NCHD")
                    || equalsQTimeTag(buf, "FFMV"))
            {
                if(xmpData_[exvGettext(tv->label_)].count() > 0)
                {
                    byte rawTagData[size-12];
                    const std::string tagData = xmpData_[exvGettext(tv->label_)].toString();
                    for(int j=0; j <(size-12); j++)
                    {
                        rawTagData[j] = tagData[j];
                    }
                    io_->write(rawTagData,(size-12));
                }
                else
                {
                    io_->seek((size-12),BasicIo::cur);
                }
            }

            else if(equalsQTimeTag(buf, "CMbo") || equalsQTimeTag(buf, "Cmbo"))
            {
                io_->seek(2,BasicIo::cur);
            }

            else if(tv)
            {
                io_->seek(4,BasicIo::cur);
                if(xmpData_[exvGettext(tv->label_)].count() >0)
                {
                    byte rawtagData[size-12];
                    const std::string tagData = xmpData_[exvGettext(tv->label_)].toString();
                    for(int j=0; j<(size-12); j++)
                    {
                        rawtagData[j] = tagData[j];
                    }
                    io_->write(rawtagData,(size-12));
                }
            }
        }
    }
} // QuickTimeVideo::userDataDecoder

void QuickTimeVideo::NikonTagsDecoder(unsigned long size_external)
{
    uint64_t cur_pos = io_->tell();
    DataBuf buf(200), buf2(4+1);
    unsigned long TagID = 0;
    unsigned short dataLength = 0, dataType = 2;
    const TagDetails* td, *td2;
    const RevTagDetails *rtd,*rtd2;

    if(!m_modifyMetadata)
    {
        for(int i = 0 ; i < 100 ; i++)
        {
            io_->read(buf.pData_, 4);
            TagID = Exiv2::getULong(buf.pData_, bigEndian);
            td = find(NikonNCTGTags, TagID);

            io_->read(buf.pData_, 2);
            dataType = Exiv2::getUShort(buf.pData_, bigEndian);

            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, 2);

            if(TagID == 0x2000023) {
                uint64_t local_pos = io_->tell();
                dataLength = Exiv2::getUShort(buf.pData_, bigEndian);
                std::memset(buf.pData_, 0x0, buf.size_);

                io_->read(buf.pData_, 4);   xmpData_["Xmp.video.PictureControlVersion"]  = Exiv2::toString(buf.pData_);
                io_->read(buf.pData_, 20);  xmpData_["Xmp.video.PictureControlName"]     = Exiv2::toString(buf.pData_);
                io_->read(buf.pData_, 20);  xmpData_["Xmp.video.PictureControlBase"]     = Exiv2::toString(buf.pData_);
                io_->read(buf.pData_, 4);   std::memset(buf.pData_, 0x0, buf.size_);

                io_->read(buf.pData_, 1);
                td2 = find(PictureControlAdjust, (int)buf.pData_[0] & 7 );
                if(td2)
                    xmpData_["Xmp.video.PictureControlAdjust"] = exvGettext(td2->label_);
                else
                    xmpData_["Xmp.video.PictureControlAdjust"] = (int)buf.pData_[0] & 7 ;

                io_->read(buf.pData_, 1);
                td2 = find(NormalSoftHard, (int)buf.pData_[0] & 7 );
                if(td2)
                    xmpData_["Xmp.video.PictureControlQuickAdjust"] = exvGettext(td2->label_);

                io_->read(buf.pData_, 1);
                td2 = find(NormalSoftHard, (int)buf.pData_[0] & 7 );
                if(td2)
                    xmpData_["Xmp.video.Sharpness"] = exvGettext(td2->label_);
                else
                    xmpData_["Xmp.video.Sharpness"] = (int)buf.pData_[0] & 7;

                io_->read(buf.pData_, 1);
                td2 = find(NormalSoftHard, (int)buf.pData_[0] & 7 );
                if(td2)
                    xmpData_["Xmp.video.Contrast"] = exvGettext(td2->label_);
                else
                    xmpData_["Xmp.video.Contrast"] = (int)buf.pData_[0] & 7;

                io_->read(buf.pData_, 1);
                td2 = find(NormalSoftHard, (int)buf.pData_[0] & 7 );
                if(td2)
                    xmpData_["Xmp.video.Brightness"] = exvGettext(td2->label_);
                else
                    xmpData_["Xmp.video.Brightness"] = (int)buf.pData_[0] & 7;

                io_->read(buf.pData_, 1);
                td2 = find(Saturation, (int)buf.pData_[0] & 7 );
                if(td2)
                    xmpData_["Xmp.video.Saturation"] = exvGettext(td2->label_);
                else
                    xmpData_["Xmp.video.Saturation"] = (int)buf.pData_[0] & 7;

                io_->read(buf.pData_, 1);
                xmpData_["Xmp.video.HueAdjustment"] = (int)buf.pData_[0] & 7;

                io_->read(buf.pData_, 1);
                td2 = find(FilterEffect, (int)buf.pData_[0]);
                if(td2)
                    xmpData_["Xmp.video.FilterEffect"] = exvGettext(td2->label_);
                else
                    xmpData_["Xmp.video.FilterEffect"] = (int)buf.pData_[0];

                io_->read(buf.pData_, 1);
                td2 = find(ToningEffect, (int)buf.pData_[0]);
                if(td2)
                    xmpData_["Xmp.video.ToningEffect"] = exvGettext(td2->label_);
                else
                    xmpData_["Xmp.video.ToningEffect"] = (int)buf.pData_[0];

                io_->read(buf.pData_, 1);   xmpData_["Xmp.video.ToningSaturation"] = (int)buf.pData_[0];

                io_->seek(local_pos + dataLength, BasicIo::beg);
            }

            else if(TagID == 0x2000024) {
                uint64_t local_pos = io_->tell();
                dataLength = Exiv2::getUShort(buf.pData_, bigEndian);
                std::memset(buf.pData_, 0x0, buf.size_);

                io_->read(buf.pData_, 2);   xmpData_["Xmp.video.TimeZone"] = Exiv2::getShort(buf.pData_, bigEndian);
                io_->read(buf.pData_, 1);
                td2 = find(YesNo, (int)buf.pData_[0]);
                if(td2)
                    xmpData_["Xmp.video.DayLightSavings"] = exvGettext(td2->label_);

                io_->read(buf.pData_, 1);
                td2 = find(DateDisplayFormat, (int)buf.pData_[0]);
                if(td2)
                    xmpData_["Xmp.video.DateDisplayFormat"] = exvGettext(td2->label_);

                io_->seek(local_pos + dataLength, BasicIo::beg);
            }

            else if(dataType == 2 || dataType == 7) {
                dataLength = Exiv2::getUShort(buf.pData_, bigEndian);
                std::memset(buf.pData_, 0x0, buf.size_);

                // Sanity check with an "unreasonably" large number
                if (dataLength > 200) {
#ifndef SUPPRESS_WARNINGS
                    EXV_ERROR << "Xmp.video Nikon Tags, dataLength was found to be larger than 200."
                              << " Entries considered invalid. Not Processed.\n";
#endif
                    io_->seek(io_->tell() + dataLength, BasicIo::beg);
                }
                else
                    io_->read(buf.pData_, dataLength);

                if(td)
                    xmpData_[exvGettext(td->label_)] = Exiv2::toString(buf.pData_);
            }
            else if(dataType == 4)  {
                dataLength = Exiv2::getUShort(buf.pData_, bigEndian) * 4;
                std::memset(buf.pData_, 0x0, buf.size_);
                io_->read(buf.pData_, 4);
                if(td)
                    xmpData_[exvGettext(td->label_)] = Exiv2::toString(Exiv2::getULong( buf.pData_, bigEndian));

                // Sanity check with an "unreasonably" large number
                if (dataLength > 200 || dataLength < 4) {
#ifndef SUPPRESS_WARNINGS
                    EXV_ERROR << "Xmp.video Nikon Tags, dataLength was found to be of inapropriate size."
                              << " Entries considered invalid. Not Processed.\n";
#endif
                    io_->seek(io_->tell() + dataLength - 4, BasicIo::beg);
                }
                else
                    io_->read(buf.pData_, dataLength - 4);
            }
            else if(dataType == 3)  {
                dataLength = Exiv2::getUShort(buf.pData_, bigEndian) * 2;
                std::memset(buf.pData_, 0x0, buf.size_);
                io_->read(buf.pData_, 2);
                if(td)
                    xmpData_[exvGettext(td->label_)] = Exiv2::toString(Exiv2::getUShort( buf.pData_, bigEndian));

                // Sanity check with an "unreasonably" large number
                if (dataLength > 200 || dataLength < 2) {
#ifndef SUPPRESS_WARNINGS
                    EXV_ERROR << "Xmp.video Nikon Tags, dataLength was found to be of inapropriate size."
                              << " Entries considered invalid. Not Processed.\n";
#endif
                    io_->seek(io_->tell() + dataLength - 2, BasicIo::beg);
                }
                else
                    io_->read(buf.pData_, dataLength - 2);
            }
            else if(dataType == 5) {
                dataLength = Exiv2::getUShort(buf.pData_, bigEndian) * 8;
                std::memset(buf.pData_, 0x0, buf.size_);
                io_->read(buf.pData_, 4);
                io_->read(buf2.pData_, 4);
                if(td)
                    xmpData_[exvGettext(td->label_)] = Exiv2::toString((double)Exiv2::getULong( buf.pData_, bigEndian) / (double)Exiv2::getULong( buf2.pData_, bigEndian));

                // Sanity check with an "unreasonably" large number
                if (dataLength > 200 || dataLength < 8) {
#ifndef SUPPRESS_WARNINGS
                    EXV_ERROR << "Xmp.video Nikon Tags, dataLength was found to be of inapropriate size."
                              << " Entries considered invalid. Not Processed.\n";
#endif
                    io_->seek(io_->tell() + dataLength - 8, BasicIo::beg);
                }
                else
                    io_->read(buf.pData_, dataLength - 8);
            }
            else if(dataType == 8) {
                dataLength = Exiv2::getUShort(buf.pData_, bigEndian) * 2;
                std::memset(buf.pData_, 0x0, buf.size_);
                io_->read(buf.pData_, 2);
                io_->read(buf2.pData_, 2);
                if(td)
                    xmpData_[exvGettext(td->label_)] = Exiv2::toString(Exiv2::getUShort( buf.pData_, bigEndian) ) + " " + Exiv2::toString(Exiv2::getUShort( buf2.pData_, bigEndian));

                // Sanity check with an "unreasonably" large number
                if (dataLength > 200 || dataLength < 4) {
#ifndef SUPPRESS_WARNINGS
                    EXV_ERROR << "Xmp.video Nikon Tags, dataLength was found to be of inapropriate size."
                              << " Entries considered invalid. Not Processed.\n";
#endif
                    io_->seek(io_->tell() + dataLength - 4, BasicIo::beg);
                }
                else
                    io_->read(buf.pData_, dataLength - 4);
            }
        }
        io_->seek(cur_pos + size_external, BasicIo::beg);
    }
    else
    {
        io_->read(buf.pData_, 4);
        TagID = Exiv2::getULong(buf.pData_, bigEndian);
        td = find(NikonNCTGTags, TagID);

        io_->read(buf.pData_, 2);
        dataType = Exiv2::getUShort(buf.pData_, bigEndian);

        std::memset(buf.pData_, 0x0, buf.size_);
        io_->read(buf.pData_, 2);

        if(TagID == 0x2000023)
        {
            uint64_t local_pos = io_->tell();
            dataLength = Exiv2::getUShort(buf.pData_, bigEndian);

            if(xmpData_["Xmp.video.PictureControlVersion"].count() >0)
            {
                byte rawPictureControlVersion[4];
                const std::string pictureControlVersion = xmpData_["Xmp.video.PictureControlVersion"].toString();
                for(int j=0; j<4; j++)
                {
                    rawPictureControlVersion[j] = pictureControlVersion[j];
                }
                io_->write(rawPictureControlVersion,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.PictureControlName"].count() >0)
            {
                byte rawPictureControlName[20];
                const std::string pictureControlName = xmpData_["Xmp.video.PictureControlName"].toString();
                for(int j=0; j<20; j++)
                {
                    rawPictureControlName[j] = pictureControlName[j];
                }
                io_->write(rawPictureControlName,20);
            }
            else
            {
                io_->seek(20,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.PictureControlBase"].count() >0)
            {
                byte rawPictureControlBase[20];
                const std::string pictureControlBase = xmpData_["Xmp.video.PictureControlBase"].toString();
                for(int j=0; j<20; j++)
                {
                    rawPictureControlBase[j] = pictureControlBase[j];
                }
                io_->write(rawPictureControlBase,20);
            }
            else
            {
                io_->seek(20,BasicIo::cur);
            }
            io_->seek(4,BasicIo::cur);   std::memset(buf.pData_, 0x0, buf.size_);

            if(xmpData_["Xmp.video.PictureControlAdjust"].count() >0)
            {
                RevTagDetails revTagDetails[(sizeof(PictureControlAdjust)/sizeof(PictureControlAdjust[0]))];
                reverseTagDetails(PictureControlAdjust,revTagDetails,((sizeof(PictureControlAdjust)/sizeof(PictureControlAdjust[0]))));
                char * m_tagData = (char *)malloc(xmpData_["Xmp.video.PictureControlAdjust"].size());
                const std::string m_strData = xmpData_["Xmp.video.PictureControlAdjust"].toString();
                for(int j=0; j<xmpData_["Xmp.video.PictureControlAdjust"].size(); j++)
                {
                    m_tagData[j] = m_strData[j];
                }
                rtd2 = find(revTagDetails,m_tagData);
                if(rtd2)
                {
                    byte rawPictureControlAdjust = (byte)rtd2->val_;
                    io_->write(&rawPictureControlAdjust,1);
                }
                else
                {
                    io_->seek(1,BasicIo::cur);
                }
            }
            else
            {
                io_->seek(1,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.PictureControlQuickAdjust"].count() >0)
            {
                RevTagDetails revTagDetails[(sizeof(NormalSoftHard)/sizeof(NormalSoftHard[0]))];
                reverseTagDetails(NormalSoftHard,revTagDetails,((sizeof(NormalSoftHard)/sizeof(NormalSoftHard[0]))));
                char * m_tagData = (char *)malloc(xmpData_["Xmp.video.PictureControlQuickAdjust"].size());
                const std::string m_strData = xmpData_["Xmp.video.PictureControlQuickAdjust"].toString();
                for(int j=0; j<xmpData_["Xmp.video.PictureControlQuickAdjust"].size(); j++)
                {
                    m_tagData[j] = m_strData[j];
                }
                rtd2 = find(revTagDetails, m_tagData );
                if(rtd2)
                {
                    byte rawPictureControlQuickAdjust = (byte)rtd2->val_;
                    io_->write(&rawPictureControlQuickAdjust,1);
                }
                else
                {
                    io_->seek(1,BasicIo::cur);
                }
            }
            else
            {
                io_->seek(1,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.Sharpness"].count() >0)
            {
                RevTagDetails revTagDetails[(sizeof(NormalSoftHard)/sizeof(NormalSoftHard[0]))];
                reverseTagDetails(NormalSoftHard,revTagDetails,((sizeof(NormalSoftHard)/sizeof(NormalSoftHard[0]))));
                char * m_tagData = (char *)malloc(xmpData_["Xmp.video.Sharpness"].size());
                const std::string m_strData = xmpData_["Xmp.video.Sharpness"].toString();
                for(int j=0; j<xmpData_["Xmp.video.Sharpness"].size(); j++)
                {
                    m_tagData[j] = m_strData[j];
                }
                rtd2 = find(revTagDetails, m_tagData );
                if(rtd2)
                {
                    byte rawSharpness = (byte)rtd2->val_;
                    io_->write(&rawSharpness,1);
                }
                else
                {
                    io_->seek(1,BasicIo::cur);
                }
            }
            else
            {
                io_->seek(1,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.Contrast"].count() >0)
            {
                RevTagDetails revTagDetails[(sizeof(NormalSoftHard)/sizeof(NormalSoftHard[0]))];
                reverseTagDetails(NormalSoftHard,revTagDetails,((sizeof(NormalSoftHard)/sizeof(NormalSoftHard[0]))));
                char * m_tagData = (char *)malloc(xmpData_["Xmp.video.Contrast"].size());
                const std::string m_strData = xmpData_["Xmp.video.Contrast"].toString();
                for(int j=0; j<xmpData_["Xmp.video.Contrast"].size(); j++)
                {
                    m_tagData[j] = m_strData[j];
                }
                rtd2 = find(revTagDetails, m_tagData );
                if(rtd2)
                {
                    byte rawContrast = (byte)rtd2->val_;
                    io_->write(&rawContrast,1);
                }
                else
                {
                    io_->seek(1,BasicIo::cur);
                }
            }
            else
            {
                io_->seek(1,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.Brightness"].count() >0)
            {
                RevTagDetails revTagDetails[(sizeof(NormalSoftHard)/sizeof(NormalSoftHard[0]))];
                reverseTagDetails(NormalSoftHard,revTagDetails,((sizeof(NormalSoftHard)/sizeof(NormalSoftHard[0]))));
                char * m_tagData = (char *)malloc(xmpData_["Xmp.video.Brightness"].size());
                const std::string m_strData = xmpData_["Xmp.video.Brightness"].toString();
                for(int j=0; j<xmpData_["Xmp.video.Brightness"].size(); j++)
                {
                    m_tagData[j] = m_strData[j];
                }
                rtd2 = find(revTagDetails, m_tagData );
                if(rtd2)
                {
                    byte rawBrightness = (byte)rtd2->val_;
                    io_->write(&rawBrightness,1);
                }
                else
                {
                    io_->seek(1,BasicIo::cur);
                }
            }
            else
            {
                io_->seek(1,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.Saturation"].count() >0)
            {
                RevTagDetails revTagDetails[(sizeof(Saturation)/sizeof(Saturation[0]))];
                reverseTagDetails(Saturation,revTagDetails,((sizeof(Saturation)/sizeof(Saturation[0]))));
                char * m_tagData = (char *)malloc(xmpData_["Xmp.video.Saturation"].size());
                const std::string m_strData = xmpData_["Xmp.video.Saturation"].toString();
                for(int j=0; j<xmpData_["Xmp.video.Saturation"].size(); j++)
                {
                    m_tagData[j] = m_strData[j];
                }
                rtd2 = find(revTagDetails, m_tagData );
                if(rtd2)
                {
                    byte rawSaturation = (byte)rtd2->val_;
                    io_->write(&rawSaturation,1);
                }
                else
                {
                    io_->seek(1,BasicIo::cur);
                }
            }
            else
            {
                io_->seek(1,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.HueAdjustment"].count() >0)
            {
                byte rawHueAdjustment = (byte)xmpData_["Xmp.video.HueAdjustment"].toString()[0];
                io_->write(&rawHueAdjustment,1);
            }
            else
            {
                io_->seek(1,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.FilterEffect"].count() >0)
            {
                RevTagDetails revTagDetails[(sizeof(FilterEffect)/sizeof(FilterEffect[0]))];
                reverseTagDetails(FilterEffect,revTagDetails,((sizeof(FilterEffect)/sizeof(FilterEffect[0]))));
                char * m_tagData = (char *)malloc(xmpData_["Xmp.video.FilterEffect"].size());
                const std::string m_strData = xmpData_["Xmp.video.FilterEffect"].toString();
                for(int j=0; j<xmpData_["Xmp.video.FilterEffect"].size(); j++)
                {
                    m_tagData[j] = m_strData[j];
                }
                rtd2 = find(revTagDetails, m_tagData );
                if(rtd2)
                {
                    byte rawFilterEffect = (byte)rtd2->val_;
                    io_->write(&rawFilterEffect,1);
                }
                else
                {
                    io_->seek(1,BasicIo::cur);
                }
            }
            else
            {
                io_->seek(1,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.ToningEffect"].count() >0)
            {
                RevTagDetails revTagDetails[(sizeof(ToningEffect)/sizeof(ToningEffect[0]))];
                reverseTagDetails(ToningEffect,revTagDetails,((sizeof(ToningEffect)/sizeof(ToningEffect[0]))));
                char * m_tagData = (char *)malloc(xmpData_["Xmp.video.ToningEffect"].size());
                const std::string m_strData = xmpData_["Xmp.video.ToningEffect"].toString();
                for(int j=0; j<xmpData_["Xmp.video.ToningEffect"].size(); j++)
                {
                    m_tagData[j] = m_strData[j];
                }
                rtd2 = find(revTagDetails, m_tagData );
                if(rtd2)
                {
                    byte rawToningEffect = (byte)rtd2->val_;
                    io_->write(&rawToningEffect,1);
                }
                else
                {
                    io_->seek(1,BasicIo::cur);
                }
            }
            else
            {
                io_->seek(1,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.ToningSaturation"].count() >0)
            {
                byte rawToningSaturation = (byte)xmpData_["Xmp.video.ToningSaturation"].toString()[0];
                io_->write(&rawToningSaturation,1);
            }
            io_->seek(local_pos + dataLength, BasicIo::beg);
        }

        else if(TagID == 0x2000024)
        {
            uint64_t local_pos = io_->tell();
            dataLength = Exiv2::getUShort(buf.pData_, bigEndian);
            std::memset(buf.pData_, 0x0, buf.size_);

            if(xmpData_["Xmp.video.TimeZone"].count() >0)
            {
                byte rawTimeZone[2];
                ushort timeZone = (short)xmpData_["Xmp.video.TimeZone"].toLong();
                memcpy(rawTimeZone,&timeZone,2);
                io_->write(rawTimeZone,2);
            }
            else
            {
                io_->seek(2,BasicIo::cur);
            }
            if(xmpData_["Xmp.video.DayLightSavings"].count() >0)
            {
                RevTagDetails revTagDetails[(sizeof(YesNo)/sizeof(YesNo[0]))];
                reverseTagDetails(YesNo,revTagDetails,((sizeof(YesNo)/sizeof(YesNo[0]))));
                char * m_tagData = (char *)malloc(xmpData_["Xmp.video.DayLightSavings"].size());
                const std::string m_strData = xmpData_["Xmp.video.DayLightSavings"].toString();
                for(int j=0; j<xmpData_["Xmp.video.DayLightSavings"].size(); j++)
                {
                    m_tagData[j] = m_strData[j];
                }
                rtd2 = find(revTagDetails, m_tagData );
                if(rtd2)
                {
                    byte rawDayLightSavings = (byte)rtd2->val_;
                    io_->write(&rawDayLightSavings,1);
                }
                else
                {
                    io_->seek(1,BasicIo::cur);
                }
            }
            if(xmpData_["Xmp.video.DateDisplayFormat"].count() >0)
            {
                RevTagDetails revTagDetails[(sizeof(DateDisplayFormat)/sizeof(DateDisplayFormat[0]))];
                reverseTagDetails(DateDisplayFormat,revTagDetails,((sizeof(DateDisplayFormat)/sizeof(DateDisplayFormat[0]))));
                char * m_tagData = (char *)malloc(xmpData_["Xmp.video.DateDisplayFormat"].size());
                const std::string m_strData = xmpData_["Xmp.video.DateDisplayFormat"].toString();
                for(int j=0; j<xmpData_["Xmp.video.DateDisplayFormat"].size(); j++)
                {
                    m_tagData[j] = m_strData[j];
                }
                rtd2 = find(revTagDetails, m_tagData );
                if(rtd2)
                {
                    byte rawDateDisplayFormat = (byte)rtd2->val_;
                    io_->write(&rawDateDisplayFormat,1);
                }
                else
                {
                    io_->seek(1,BasicIo::cur);
                }
            }
            io_->seek(local_pos + dataLength, BasicIo::beg);
        }

        else if(dataType == 2 || dataType == 7)
        {
            dataLength = Exiv2::getUShort(buf.pData_, bigEndian);
            std::memset(buf.pData_, 0x0, buf.size_);

            // Sanity check with an "unreasonably" large number
            if (dataLength > 200) {
#ifndef SUPPRESS_WARNINGS
                EXV_ERROR << "Xmp.video Nikon Tags, dataLength was found to be larger than 200."
                          << " Entries considered invalid. Not Processed.\n";
#endif
                io_->seek(io_->tell() + dataLength, BasicIo::beg);
            }
            else
            {
                if(td && (xmpData_[exvGettext(td->label_)].count() >0) && (xmpData_[exvGettext(td->label_)].size() >= dataLength))
                {
                    byte rawTagData[dataLength];
                    const std::string m_strData = xmpData_[exvGettext(td->label_)].toString();
                    for(int j=0; j<dataLength; j++)
                    {
                        rawTagData[j] = m_strData[j];
                    }
                    io_->write(rawTagData,dataLength);
                }
                else
                {
                    io_->seek(dataLength,BasicIo::cur);
                }
            }
        }
        else if(dataType == 4)
        {
            dataLength = Exiv2::getUShort(buf.pData_, bigEndian) * 4;
            if(td && (xmpData_[exvGettext(td->label_)].count() >0))
            {
                byte rawTagData[4];
                const long tagData = xmpData_[exvGettext(td->label_)].toLong();
                memcpy(rawTagData,&tagData,4);
                io_->write(rawTagData,4);
                io_->seek((dataLength-4),BasicIo::cur);
            }
            else
            {
                io_->seek(dataLength,BasicIo::cur);
            }
        }
        else if(dataType == 3)  {
            dataLength = Exiv2::getUShort(buf.pData_, bigEndian) * 2;
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, 2);
            if(td &&(xmpData_[exvGettext(td->label_)].count() >0))
            {
                byte rawTagData[2];
                const ushort tagData = (ushort)xmpData_[exvGettext(td->label_)].toLong();
                memcpy(rawTagData,&tagData,2);
                io_->write(rawTagData,2);
                io_->seek((dataLength-2),BasicIo::cur);
            }
            else
            {
                io_->seek(dataLength,BasicIo::cur);
            }
        }
        else if(dataType == 5)
        {
            dataLength = Exiv2::getUShort(buf.pData_, bigEndian) * 8;
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, 4);
            io_->read(buf2.pData_, 4);
            if(td && (xmpData_[exvGettext(td->label_)].count() >0))
            {
                io_->seek(-8,BasicIo::cur);
                byte rawTagData[4];
                const long tagData = (long)((double)xmpData_[exvGettext(td->label_)].toLong()*
                                            (double)Exiv2::getULong( buf2.pData_, bigEndian));
                memcpy(rawTagData,&tagData,4);
                io_->write(rawTagData,4);
                io_->seek((dataLength-4),BasicIo::cur);
            }
            else
            {
                io_->seek(dataLength,BasicIo::cur);
            }
        }
        else if(dataType == 8)
        {//TODO : writesupport for datatype8
        }
    }
} // QuickTimeVideo::NikonTagsDecoder

void QuickTimeVideo::setMediaStream()
{
    uint64_t current_position = io_->tell();
    DataBuf buf(4+1);

    while(!io_->eof())
    {
        io_->read(buf.pData_, 4);
        if (equalsQTimeTag(buf, "hdlr"))
        {
            io_->seek(8,BasicIo::cur);
            io_->read(buf.pData_, 4);

            if (equalsQTimeTag(buf, "vide"))
                currentStream_ = Video;
            else if(equalsQTimeTag(buf, "soun"))
                currentStream_ = Audio;
            else if (equalsQTimeTag(buf, "hint"))
                currentStream_ = Hint;
            else
                currentStream_ = GenMediaHeader;
            break;
        }
    }
    io_->seek(current_position, BasicIo::beg);
} // QuickTimeVideo::setMediaStream

void QuickTimeVideo::timeToSampleDecoder()
{
    DataBuf buf(4+1);
    io_->read(buf.pData_, 4);
    io_->read(buf.pData_, 4);
    uint64_t noOfEntries, totalframes = 0, timeOfFrames = 0;
    noOfEntries = returnUnsignedBufValue(buf);
    uint64_t temp;

    if(!m_modifyMetadata)
    {
        for(unsigned long i = 1; i <= noOfEntries; i++)
        {
            io_->read(buf.pData_, 4);
            temp = returnBufValue(buf);
            totalframes += temp;
            io_->read(buf.pData_, 4);
            timeOfFrames += temp * returnBufValue(buf);
        }
        if (currentStream_ == Video)
            xmpData_["Xmp.video.FrameRate"] = (double)totalframes * (double)timeScale_ / (double)timeOfFrames;
    }
    else
    {
        for(unsigned long i = 1; i <= noOfEntries; i++)
        {
            io_->seek(8, BasicIo::cur);
        }
        if(xmpData_["Xmp.video.FrameRate"].count() >0)
        {
            //TODO calculation
        }
    }
} // QuickTimeVideo::timeToSampleDecoder

void QuickTimeVideo::sampleDesc(unsigned long size)
{
    DataBuf buf(100);
    uint64_t cur_pos = io_->tell();
    io_->read(buf.pData_, 4);
    io_->read(buf.pData_, 4);
    uint64_t noOfEntries;
    noOfEntries = returnUnsignedBufValue(buf);

    for(unsigned long i = 1; i <= noOfEntries; i++) {
        if (currentStream_ == Video)
            imageDescDecoder();
        else if (currentStream_ == Audio)
            audioDescDecoder();
    }
    io_->seek(cur_pos + size, BasicIo::beg);
} // QuickTimeVideo::sampleDesc

void QuickTimeVideo::audioDescDecoder()
{
    DataBuf buf(40);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';
    io_->read(buf.pData_, 4);
    uint64_t size = 82;

    const TagVocabulary* td;

    if(!m_modifyMetadata)
    {
        for (int i = 0; size/4 != 0 ; size -= 4, i++) {
            io_->read(buf.pData_, 4);
            switch(i) {
            case AudioFormat:
                td = find(qTimeFileType, Exiv2::toString( buf.pData_));
                if(td)
                    xmpData_["Xmp.audio.Compressor"] = exvGettext(td->label_);
                else
                    xmpData_["Xmp.audio.Compressor"] = Exiv2::toString( buf.pData_);
                break;
            case AudioVendorID:
                td = find(vendorIDTags, Exiv2::toString( buf.pData_));
                if(td)
                    xmpData_["Xmp.audio.VendorID"] = exvGettext(td->label_);
                break;
            case AudioChannels:
                xmpData_["Xmp.audio.ChannelType"] = returnBufValue(buf, 2);
                xmpData_["Xmp.audio.BitsPerSample"] = (buf.pData_[2] * 256 + buf.pData_[3]);
                break;
            case AudioSampleRate:
                xmpData_["Xmp.audio.SampleRate"] = returnBufValue(buf, 2) + ((buf.pData_[2] * 256 + buf.pData_[3]) * 0.01);
                break;
            default:
                break;
            }
        }
        io_->read(buf.pData_, static_cast<long>(size % 4));	//cause size is so small, this cast should be right.
    }
    else
    {
        TagVocabulary revTagVocabulary[(sizeof(qTimeFileType)/sizeof(qTimeFileType[0]))];
        reverseTagVocabulary(qTimeFileType,revTagVocabulary,((sizeof(qTimeFileType)/sizeof(qTimeFileType[0]))));
        if(xmpData_["Xmp.audio.Compressor"].count() > 0)
        {
            byte rawCompressor[4];
            td = find(qTimeFileType,xmpData_["Xmp.audio.Compressor"].toString());
            const char* compressor = td->voc_;
            if(td)
            {
                for(int j=0; j<4; j++)
                {
                    rawCompressor[j] = compressor[j];
                }
                io_->write(rawCompressor,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }
        }
        else
        {
            io_->seek(4,BasicIo::cur);
        }
        if(xmpData_["Xmp.audio.VendorID"].count() >0)
        {
            byte rawVendorID[4];
            td = find(qTimeFileType,xmpData_["Xmp.audio.VendorID"].toString());
            const char* vendorID = td->voc_;
            if(td)
            {
                for(int j=0; j<4; j++)
                {
                    rawVendorID[j] = vendorID[j];
                }
                io_->write(rawVendorID,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }
        }
        else
        {
            io_->seek(4,BasicIo::cur);
        }
        if(xmpData_["Xmp.audio.ChannelType"].count() >0)
        {
            byte rawChannelType[2];
            ushort channelType =(ushort) xmpData_["Xmp.audio.ChannelType"].toLong();
            memcpy(rawChannelType,&channelType,2);
            io_->write(rawChannelType,2);
        }
        else
        {
            io_->seek(2,BasicIo::cur);
        }
        if(xmpData_["Xmp.audio.BitsPerSample"].count() >0)
        {
            byte rawBitsPerSample[2];
            ushort bitsPerSample =(ushort) (xmpData_["Xmp.audio.BitsPerSample"].toLong()/256);
            memcpy(rawBitsPerSample,&bitsPerSample,2);
            io_->write(rawBitsPerSample,2);
        }
        else
        {
            io_->seek(2,BasicIo::cur);
        }
        if(xmpData_["Xmp.audio.SampleRate"].count() >0)
        {
            byte rawSampleRate[4];
            byte tmpRawData[2];
            xmpData_["Xmp.audio.SampleRate"].toLong();
            ushort sampleRateBase = (ushort) (xmpData_["Xmp.audio.SampleRate"].toLong()/512);
            memcpy(tmpRawData,&sampleRateBase,2);
            rawSampleRate[0] = tmpRawData[0] ; rawSampleRate[1] = tmpRawData[1];
            //TODO calculations and logic
        }
        else
        {

        }
    }
} // QuickTimeVideo::audioDescDecoder

void QuickTimeVideo::imageDescDecoder()
{
    DataBuf buf(40);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';
    io_->read(buf.pData_, 4);
    uint64_t size = 82;

    const TagVocabulary* td;

    if(!m_modifyMetadata)
    {
        for (int i = 0; size/4 != 0 ; size -= 4, i++)
        {
            io_->read(buf.pData_, 4);

            switch(i) {
            case codec:
                td = find(qTimeFileType, Exiv2::toString( buf.pData_));
                if(td)
                    xmpData_["Xmp.video.Codec"] = exvGettext(td->label_);
                else
                    xmpData_["Xmp.video.Codec"] = Exiv2::toString( buf.pData_);
                break;
            case VendorID:
                td = find(vendorIDTags, Exiv2::toString( buf.pData_));
                if(td)
                    xmpData_["Xmp.video.VendorID"] = exvGettext(td->label_);
                break;
            case SourceImageWidth_Height:
                xmpData_["Xmp.video.SourceImageWidth"] = returnBufValue(buf, 2);
                xmpData_["Xmp.video.SourceImageHeight"] = (buf.pData_[2] * 256 + buf.pData_[3]);
                break;
            case XResolution:
                xmpData_["Xmp.video.XResolution"] = returnBufValue(buf, 2) + ((buf.pData_[2] * 256 + buf.pData_[3]) * 0.01);
                break;
            case YResolution:
                xmpData_["Xmp.video.YResolution"] = returnBufValue(buf, 2) + ((buf.pData_[2] * 256 + buf.pData_[3]) * 0.01);
                io_->read(buf.pData_, 3); size -= 3;
                break;
            case CompressorName:
                io_->read(buf.pData_, 32); size -= 32;
                xmpData_["Xmp.video.Compressor"] = Exiv2::toString( buf.pData_);
                break;
            default:
                break;
            }
        }
        io_->read(buf.pData_, static_cast<long>(size % 4));
        xmpData_["Xmp.video.BitDepth"] = returnBufValue(buf, 1);
    }
    else
    {
        TagVocabulary revTagVocabulary[(sizeof(qTimeFileType)/sizeof(qTimeFileType[0]))];
        reverseTagVocabulary(qTimeFileType,revTagVocabulary,((sizeof(qTimeFileType)/sizeof(qTimeFileType[0]))));

        for (int i = 0; size/4 != 0 ; size -= 4, i++)
        {
            switch(i)
            {
            case codec:
                if(xmpData_["Xmp.video.Codec"].count() >0)
                {
                    td = find(revTagVocabulary, xmpData_["Xmp.video.Codec"].toString());
                    if(td)
                    {
                        byte rawCodec[4];
                        const char *codecName = td->voc_;
                        for(int j=0; j<4 ;j++)
                        {
                            rawCodec[j] = codecName[j];
                        }
                        io_->write(rawCodec,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case VendorID:
                if(xmpData_["Xmp.video.VendorID"].count() >0)
                {
                    td = find(revTagVocabulary, xmpData_["Xmp.video.VendorID"].toString());
                    if(td)
                    {
                        byte rawVendorID[4];
                        const char *vendorID = td->voc_;
                        for(int j=0; j<4 ;j++)
                        {
                            rawVendorID[j] = vendorID[j];
                        }
                        io_->write(rawVendorID,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case SourceImageWidth_Height:
                if(xmpData_["Xmp.video.SourceImageWidth"].count() >0)
                {
                    DataBuf imageWidth = returnBuf((int64_t)xmpData_["Xmp.video.SourceImageWidth"].toLong());
                    io_->write(imageWidth.pData_,2);
                }
                else
                {
                    io_->seek(2,BasicIo::cur);
                }
                if(xmpData_["Xmp.video.SourceImageHeight"].count() >0)
                {
                    DataBuf imageHeight = returnBuf(((int64_t)(xmpData_["Xmp.video.SourceImageWidth"].toLong() -
                                                               xmpData_["Xmp.video.SourceImageHeight"].toLong()/65536)*65536));
                    io_->write(imageHeight.pData_,2);
                }
                else
                {
                    io_->seek(2,BasicIo::cur);
                }
                break;
            case XResolution:
                if(xmpData_["Xmp.video.XResolution"].count() >0)
                {
                    cout << "here" << endl;
                    DataBuf tmpBuf((unsigned long)5);
                    tmpBuf.pData_[3] ='\0';
                    DataBuf imageWidth = returnBuf((int64_t)xmpData_["Xmp.video.XResolution"].toLong());
                    tmpBuf = returnBuf((uint64_t)(xmpData_["Xmp.video.XResolution"].toLong() -
                                                  xmpData_["Xmp.video.XResolution"].toLong()/65536));
                    imageWidth.pData_[2] = tmpBuf.pData_[2];
                    tmpBuf = returnBuf((uint64_t)(xmpData_["Xmp.video.XResolution"].toLong() -
                                                  xmpData_["Xmp.video.XResolution"].toLong()/65536 -
                                                  xmpData_["Xmp.video.XResolution"].toLong()/ 16777216));
                    imageWidth.pData_[3] = tmpBuf.pData_[3];
                    io_->write(imageWidth.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case YResolution:
                if(xmpData_["Xmp.video.YResolution"].count() >0)
                {
                    DataBuf tmpBuf((unsigned long)5);
                    tmpBuf.pData_[3] ='\0';
                    DataBuf imageHeight = returnBuf((int64_t)xmpData_["Xmp.video.YResolution"].toLong());
                    tmpBuf = returnBuf((uint64_t)(xmpData_["Xmp.video.YResolution"].toLong() -
                                         xmpData_["Xmp.video.YResolution"].toLong()/65536));
                    imageHeight.pData_[2] = tmpBuf.pData_[2];
                    tmpBuf = returnBuf((uint64_t)(xmpData_["Xmp.video.YResolution"].toLong() -
                                                  xmpData_["Xmp.video.YResolution"].toLong()/65536 -
                                                  xmpData_["Xmp.video.YResolution"].toLong()/ 16777216));
                    imageHeight.pData_[3] = tmpBuf.pData_[3];
                    io_->write(imageHeight.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                size -= 3;
                break;
            case CompressorName:
                if(xmpData_["Xmp.video.Compressor"].count() >0)
                {
                    byte rawCompressor[32];
                    const std::string compressor = xmpData_["Xmp.video.Compressor"].toString();
                    for(int j=0;j<32; j++)
                    {
                        rawCompressor[j] = (byte)compressor[j];
                    }
                    io_->seek(4,BasicIo::cur);
                    io_->write(rawCompressor,32);
                }
                else
                {
                    io_->seek(36,BasicIo::cur);
                }
                size -= 32;
                break;
            default:
                io_->seek(4,BasicIo::cur);
                break;
            }
        }
        io_->read(buf.pData_, static_cast<long>(size % 4));
        xmpData_["Xmp.video.BitDepth"] = returnBufValue(buf, 1);
    }
} // QuickTimeVideo::imageDescDecoder

void QuickTimeVideo::multipleEntriesDecoder()
{
    DataBuf buf(4+1);
    io_->read(buf.pData_, 4);
    io_->read(buf.pData_, 4);
    uint64_t noOfEntries;

    noOfEntries = returnUnsignedBufValue(buf);

    for(unsigned long i = 1; i <= noOfEntries; i++)
        decodeBlock();
} // QuickTimeVideo::multipleEntriesDecoder

void QuickTimeVideo::videoHeaderDecoder(unsigned long size)
{
    DataBuf buf(3);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[2] = '\0';
    currentStream_ = Video;

    const RevTagDetails* rtd;
    const TagDetails* td;

    if(!m_modifyMetadata)
    {
        for (int i = 0; size/2 != 0 ; size -= 2, i++)
        {
            io_->read(buf.pData_, 2);

            switch(i)
            {
            case GraphicsMode:
                td = find(graphicsModetags, returnBufValue(buf,2));
                if(td)
                    xmpData_["Xmp.video.GraphicsMode"] = exvGettext(td->label_);
                break;
            case OpColor:
                xmpData_["Xmp.video.OpColor"] = returnBufValue(buf,2);
                break;
            default:
                break;
            }
        }
        io_->read(buf.pData_, size % 2);
    }
    else
    {
        RevTagDetails revTagDetails[(sizeof(graphicsModetags)/sizeof(graphicsModetags[0]))];
        reverseTagDetails(graphicsModetags,revTagDetails,((sizeof(graphicsModetags)/sizeof(graphicsModetags[0]))));

        for (int i = 0; size/2 != 0 ; size -= 2, i++)
        {
            switch(i)
            {
            case GraphicsMode:
                if(xmpData_["Xmp.video.GraphicsMode"].count() >0)
                {
                    byte rawGraphicsMode[2];
                    const std::string graphicsMode = xmpData_["Xmp.video.GraphicsMode"].toString();
                    char *graphicsModeData = (char *)malloc(xmpData_["Xmp.video.GraphicsMode"].size());
                    for(int j=0; j<xmpData_["Xmp.video.GraphicsMode"].size(); j++)
                    {
                        graphicsModeData[j] = graphicsMode[j];
                    }
                    rtd = find(revTagDetails,graphicsModeData);
                    short sGraphicsMode = (short)rtd->val_;
                    memcpy(rawGraphicsMode,&sGraphicsMode,2);
                    io_->write(rawGraphicsMode,2);
                }
                else
                {
                    io_->seek(2,BasicIo::cur);
                }
                break;
            case OpColor:
                if(xmpData_["Xmp.video.OpColor"].count() >0)
                {
                    DataBuf rawOpColor((unsigned long)3);
                    rawOpColor.pData_[2] = '\0';
                    const int64_t opColor = xmpData_["Xmp.video.OpColor"].toLong();
                    rawOpColor = returnBuf(opColor);
                    io_->write(rawOpColor.pData_,2);
                }
                break;
            default:
                io_->seek(2,BasicIo::cur);
                break;
            }
        }
        io_->read(buf.pData_, size % 2);
    }
} // QuickTimeVideo::videoHeaderDecoder

void QuickTimeVideo::handlerDecoder(unsigned long size)
{
    uint64_t cur_pos = io_->tell();
    DataBuf buf(100);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';

    const TagVocabulary* tv;

    if(!m_modifyMetadata)
    {
        for (int i = 0; i < 5 ; i++)
        {
            io_->read(buf.pData_, 4);

            switch(i)
            {
            case HandlerClass:
                tv = find(handlerClassTags, Exiv2::toString( buf.pData_));
                if(tv) {
                    if (currentStream_ == Video)
                        xmpData_["Xmp.video.HandlerClass"] = exvGettext(tv->label_);
                    else if (currentStream_ == Audio)
                        xmpData_["Xmp.audio.HandlerClass"] = exvGettext(tv->label_);
                }
                break;
            case HandlerType:
                tv = find(handlerTypeTags, Exiv2::toString( buf.pData_));
                if(tv) {
                    if (currentStream_ == Video)
                        xmpData_["Xmp.video.HandlerType"] = exvGettext(tv->label_);
                    else if (currentStream_ == Audio)
                        xmpData_["Xmp.audio.HandlerType"] = exvGettext(tv->label_);
                }
                break;
            case HandlerVendorID:
                tv = find(vendorIDTags, Exiv2::toString( buf.pData_));
                if(tv) {
                    if (currentStream_ == Video)
                        xmpData_["Xmp.video.HandlerVendorID"] = exvGettext(tv->label_);
                    else if (currentStream_ == Audio)
                        xmpData_["Xmp.audio.HandlerVendorID"] = exvGettext(tv->label_);
                }
                break;
            }
        }
        io_->seek(cur_pos + size, BasicIo::beg);
    }
    else
    {
        TagVocabulary revTagVocabulary[(sizeof(handlerClassTags)/sizeof(handlerClassTags[0]))];
        reverseTagVocabulary(handlerClassTags,revTagVocabulary,((sizeof(handlerClassTags)/sizeof(handlerClassTags[0]))));
        io_->seek(4,BasicIo::cur);

        for (int i = 0; i < 5 ; i++)
        {
            io_->read(buf.pData_, 4);

            switch(i)
            {
            case HandlerClass:
                if (currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.HandlerClass"].count() >0)
                    {
                        byte rawHandlerClass[4];
                        tv = find(revTagVocabulary, xmpData_["Xmp.video.HandlerClass"].toString());
                        const std::string handlerClass = tv->label_;
                        for(int j=0; j<4; j++)
                        {
                            rawHandlerClass[j] = handlerClass[j];
                        }
                        io_->write(rawHandlerClass,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if (currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.HandlerClass"].count() >0)
                    {
                        byte rawHandlerClass[4];
                        tv = find(revTagVocabulary, xmpData_["Xmp.audio.HandlerClass"].toString());
                        const std::string handlerClass = tv->label_;
                        for(int j=0; j<4; j++)
                        {
                            rawHandlerClass[j] = handlerClass[j];
                        }
                        io_->write(rawHandlerClass,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
            case HandlerType:
                if (currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.HandlerType"].count() >0)
                    {
                        byte rawHandlerType[4];
                        tv = find(revTagVocabulary, xmpData_["Xmp.video.HandlerType"].toString());
                        const std::string handlerType = tv->label_;
                        for(int j=0; j<4; j++)
                        {
                            rawHandlerType[j] = handlerType[j];
                        }
                        io_->write(rawHandlerType,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if (currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.HandlerType"].count() >0)
                    {
                        byte rawHandlerType[4];
                        tv = find(revTagVocabulary, xmpData_["Xmp.audio.HandlerType"].toString());
                        const std::string handlerType = tv->label_;
                        for(int j=0; j<4; j++)
                        {
                            rawHandlerType[j] = handlerType[j];
                        }
                        io_->write(rawHandlerType,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
            case HandlerVendorID:
                if (currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.HandlerVendorID"].count() >0)
                    {
                        byte rawHandlerVendorID[4];
                        tv = find(revTagVocabulary, xmpData_["Xmp.video.HandlerVendorID"].toString());
                        const std::string handlerVendorID = tv->label_;
                        for(int j=0; j<4; j++)
                        {
                            rawHandlerVendorID[j] = handlerVendorID[j];
                        }
                        io_->write(rawHandlerVendorID,4);
                    }
                }
                else if (currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.HandlerVendorID"].count() >0)
                    {
                        byte rawHandlerVendorID[4];
                        tv = find(revTagVocabulary, xmpData_["Xmp.audio.HandlerVendorID"].toString());
                        const std::string handlerVendorID = tv->label_;
                        for(int j=0; j<4; j++)
                        {
                            rawHandlerVendorID[j] = handlerVendorID[j];
                        }
                        io_->write(rawHandlerVendorID,4);
                    }
                }
                break;
            }
        }
        io_->seek(cur_pos + size, BasicIo::beg);
    }
} // QuickTimeVideo::handlerDecoder

void QuickTimeVideo::fileTypeDecoder(unsigned long size)
{
    DataBuf buf(5);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';
    Exiv2::Value::AutoPtr v = Exiv2::Value::create(Exiv2::xmpSeq);
    const TagVocabulary* td;

    if(!m_modifyMetadata)
    {
        for (int i = 0; size/4 != 0; size -=4, i++)
        {
            io_->read(buf.pData_, 4);
            td = find(qTimeFileType, Exiv2::toString( buf.pData_));
            switch(i)
            {
            case 0:
                if(td)
                    xmpData_["Xmp.video.MajorBrand"] = exvGettext(td->label_);
                break;
            case 1:
                xmpData_["Xmp.video.MinorVersion"] = returnBufValue(buf);
                break;
            default:
                if(td)
                    v->read(exvGettext(td->label_));
                else
                    v->read(Exiv2::toString(buf.pData_));
                break;
            }
            xmpData_.add(Exiv2::XmpKey("Xmp.video.CompatibleBrands"), v.get());
            io_->read(buf.pData_, size%4);
        }
    }
    else
    {
        TagVocabulary revTagVocabulary[(sizeof(qTimeFileType)/sizeof(qTimeFileType[0]))];
        reverseTagVocabulary(qTimeFileType,revTagVocabulary,((sizeof(qTimeFileType)/sizeof(qTimeFileType[0]))));
        DataBuf buf;
        for (int i = 0; size/4 != 0; size -=4, i++)
        {
            switch(i)
            {
            case 0:
                if(xmpData_["Xmp.video.MajorBrand"].count() > 0)
                {
                    byte rawMajorBrand[4];
                    td = find(revTagVocabulary, xmpData_["Xmp.video.MajorBrand"].toString());
                    const char *majorBrandVoc = td->voc_;
                    for(int j=0; j<4 ;j++)
                    {
                        rawMajorBrand[j] = majorBrandVoc[i];
                    }
                    io_->write(rawMajorBrand,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case 1:
                if(xmpData_["Xmp.video.MinorVersion"].count() >0)
                {
                    buf = returnBuf((int64_t)xmpData_["Xmp.video.MinorVersion"].toLong());
                    byte rawMinorVersion[4];
                    memcpy(rawMinorVersion,buf.pData_,4);
                    io_->write(rawMinorVersion,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            default:
                if(xmpData_["Xmp.video.CompatibleBrands"].count() >0)
                {
                    byte rawCompatibleBrand[4];
                    td = find(revTagVocabulary, xmpData_["Xmp.video.CompatibleBrands"].toString());
                    const char *compatibleBrandVoc = td->voc_;
                    if(td)
                    {
                        for(int j=0; j<4 ;j++)
                        {
                            rawCompatibleBrand[i]  = compatibleBrandVoc[i];
                        }
                    }
                    else
                    {
                        const std::string compatibleBrand = xmpData_["Xmp.video.CompatibleBrands"].toString();
                        for(int j=0; j<4; j++)
                        {
                            rawCompatibleBrand[i] = compatibleBrand[i];
                        }
                    }
                    io_->write(rawCompatibleBrand,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            }
            io_->seek(size%4,BasicIo::cur);
        }
    }
} // QuickTimeVideo::fileTypeDecoder

void QuickTimeVideo::mediaHeaderDecoder(unsigned long size) {
    DataBuf buf(5);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';
    int64_t time_scale = 1;

    if(!m_modifyMetadata)
    {
        for (int i = 0; size/4 != 0 ; size -=4, i++)
        {
            io_->read(buf.pData_, 4);

            switch(i)
            {
            case MediaHeaderVersion:
                if(currentStream_ == Video)
                    xmpData_["Xmp.video.MediaHeaderVersion"] = returnBufValue(buf,1);
                else if (currentStream_ == Audio)
                    xmpData_["Xmp.audio.MediaHeaderVersion"] = returnBufValue(buf,1);
                break;
            case MediaCreateDate:
                //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
                if(currentStream_ == Video)
                    xmpData_["Xmp.video.MediaCreateDate"] = returnUnsignedBufValue(buf);
                else if (currentStream_ == Audio)
                    xmpData_["Xmp.audio.MediaCreateDate"] = returnUnsignedBufValue(buf);
                break;
            case MediaModifyDate:
                //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
                if(currentStream_ == Video)
                    xmpData_["Xmp.video.MediaModifyDate"] = returnUnsignedBufValue(buf);
                else if (currentStream_ == Audio)
                    xmpData_["Xmp.audio.MediaModifyDate"] = returnUnsignedBufValue(buf);
                break;
            case MediaTimeScale:
                if(currentStream_ == Video)
                    xmpData_["Xmp.video.MediaTimeScale"] = returnBufValue(buf);
                else if (currentStream_ == Audio)
                    xmpData_["Xmp.audio.MediaTimeScale"] = returnBufValue(buf);
                time_scale = returnBufValue(buf);
                break;
            case MediaDuration:
                if(currentStream_ == Video)
                    xmpData_["Xmp.video.MediaDuration"] = returnBufValue(buf)/time_scale;
                else if (currentStream_ == Audio)
                    xmpData_["Xmp.audio.MediaDuration"] = returnBufValue(buf)/time_scale;
                break;
            case MediaLanguageCode:
                if(currentStream_ == Video)
                    xmpData_["Xmp.video.MediaLangCode"] = returnUnsignedBufValue(buf,2);
                else if (currentStream_ == Audio)
                    xmpData_["Xmp.audio.MediaLangCode"] = returnUnsignedBufValue(buf,2);
                break;

            default:
                break;
            }
        }
        io_->read(buf.pData_, size%4);
    }
    else
    {
        for (int i = 0; size/4 != 0 ; size -=4, i++)
        {
            io_->read(buf.pData_, 4);

            switch(i)
            {
            case MediaHeaderVersion:
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.MediaHeaderVersion"].count() > 0)
                    {
                        buf = returnBuf((int64_t)xmpData_["Xmp.video.MediaHeaderVersion"].toLong());
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if (currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.MediaHeaderVersion"].count() > 0)
                    {
                        buf = returnBuf((int64_t)xmpData_["Xmp.audio.MediaHeaderVersion"].toLong());
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
            case MediaCreateDate:
                //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.MediaCreateDate"].count() > 0)
                    {
                        buf = returnBuf((uint64_t)xmpData_["Xmp.video.MediaCreateDate"].toLong());
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if (currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.MediaCreateDate"].count() > 0)
                    {
                        buf = returnBuf((uint64_t)xmpData_["Xmp.audio.MediaCreateDate"].toLong());
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
            case MediaModifyDate:
                //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.MediaModifyDate"].count() > 0)
                    {
                        buf = returnBuf((uint64_t)xmpData_["Xmp.video.MediaModifyDate"].toLong());
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if (currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.MediaModifyDate"].count() > 0)
                    {
                        buf = returnBuf((uint64_t)xmpData_["Xmp.audio.MediaModifyDate"].toLong());
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
            case MediaTimeScale:
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.MediaTimeScale"].count() > 0)
                    {
                        buf = returnBuf((int64_t)xmpData_["Xmp.video.MediaTimeScale"].toLong());
                        io_->write(buf.pData_,4);
                        io_->seek(-4,BasicIo::cur);
                    }
                }
                else if (currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.MediaTimeScale"].count() > 0)
                    {
                        buf = returnBuf((int64_t)xmpData_["Xmp.audio.MediaTimeScale"].toLong());
                        io_->write(buf.pData_,4);
                        io_->seek(-4,BasicIo::cur);
                    }
                }
                break;
            case MediaDuration:
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.MediaDuration"].count() > 0)
                    {
                        io_->read(buf.pData_,4);
                        time_scale = returnBufValue(buf);
                        buf = returnBuf((int64_t)xmpData_["Xmp.video.MediaDuration"].toLong()*time_scale);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->read(buf.pData_,4);
                        time_scale = returnBufValue(buf);
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if (currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.MediaDuration"].count() > 0)
                    {
                        io_->read(buf.pData_,4);
                        time_scale = returnBufValue(buf);
                        buf = returnBuf((int64_t)xmpData_["Xmp.audio.MediaDuration"].toLong()*time_scale);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->read(buf.pData_,4);
                        time_scale = returnBufValue(buf);
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
            case MediaLanguageCode:
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.MediaLangCode"].count() > 0)
                    {
                        io_->read(buf.pData_,4);
                        time_scale = returnBufValue(buf);
                        buf = returnBuf((uint64_t)xmpData_["Xmp.video.MediaLangCode"].toLong()*time_scale);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if (currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.MediaLangCode"].count() > 0)
                    {
                        io_->read(buf.pData_,4);
                        time_scale = returnBufValue(buf);
                        buf = returnBuf((uint64_t)xmpData_["Xmp.audio.MediaLangCode"].toLong()*time_scale);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
            default:
                break;
            }
        }
        io_->read(buf.pData_, size%4);
    }
} // QuickTimeVideo::mediaHeaderDecoder

void QuickTimeVideo::trackHeaderDecoder(unsigned long size)
{
    DataBuf buf(5);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';
    int64_t temp = 0;

    if(!m_modifyMetadata)
    {
        for (int i = 0; size/4 != 0  ; size -=4, i++)
        {
            io_->read(buf.pData_, 4);

            switch(i)
            {
            case TrackHeaderVersion:
                if(currentStream_ == Video)
                    xmpData_["Xmp.video.TrackHeaderVersion"] = returnBufValue(buf,1);
                else if(currentStream_ == Audio)
                    xmpData_["Xmp.audio.TrackHeaderVersion"] = returnBufValue(buf,1);
                break;
            case TrackCreateDate:
                //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
                if(currentStream_ == Video)
                    xmpData_["Xmp.video.TrackCreateDate"] = returnUnsignedBufValue(buf);
                else if(currentStream_ == Audio)
                    xmpData_["Xmp.audio.TrackCreateDate"] = returnUnsignedBufValue(buf);
                break;
            case TrackModifyDate:
                //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
                if(currentStream_ == Video)
                    xmpData_["Xmp.video.TrackModifyDate"] = returnUnsignedBufValue(buf);
                else if(currentStream_ == Audio)
                    xmpData_["Xmp.audio.TrackModifyDate"] = returnUnsignedBufValue(buf);
                break;
            case TrackID:
                if(currentStream_ == Video)
                    xmpData_["Xmp.video.TrackID"] = returnBufValue(buf);
                else if(currentStream_ == Audio)
                    xmpData_["Xmp.audio.TrackID"] = returnBufValue(buf);
                break;
            case TrackDuration:
                if(currentStream_ == Video)
                    xmpData_["Xmp.video.TrackDuration"] = returnBufValue(buf)/timeScale_;
                else if(currentStream_ == Audio)
                    xmpData_["Xmp.audio.TrackDuration"] = returnBufValue(buf)/timeScale_;
                break;
            case TrackLayer:
                if(currentStream_ == Video)
                    xmpData_["Xmp.video.TrackLayer"] = returnBufValue(buf, 2);
                else if(currentStream_ == Audio)
                    xmpData_["Xmp.audio.TrackLayer"] = returnBufValue(buf, 2);
                break;
            case TrackVolume:
                if(currentStream_ == Video)
                    xmpData_["Xmp.video.TrackVolume"] = (returnBufValue(buf, 1) + (buf.pData_[2] * 0.1)) * 100;
                else if(currentStream_ == Audio)
                    xmpData_["Xmp.audio.TrackVolume"] = (returnBufValue(buf, 1) + (buf.pData_[2] * 0.1)) * 100;
                break;
            case ImageWidth:
                if(currentStream_ == Video) {
                    temp = returnBufValue(buf, 2) + static_cast<int64_t>((buf.pData_[2] * 256 + buf.pData_[3]) * 0.01);
                    xmpData_["Xmp.video.Width"] = temp;
                    width_ = temp;
                }
                break;
            case ImageHeight:
                if(currentStream_ == Video) {
                    temp = returnBufValue(buf, 2) + static_cast<int64_t>((buf.pData_[2] * 256 + buf.pData_[3]) * 0.01);
                    xmpData_["Xmp.video.Height"] = temp;
                    height_ = temp;
                }
                break;
            default:
                break;
            }
        }
        io_->read(buf.pData_, size%4);
    }
    else
    {
        for (int i = 0; size/4 != 0  ; size -=4, i++)
        {
            switch(i)
            {
            case TrackHeaderVersion:
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.TrackHeaderVersion"].count() >0)
                    {
                        int64_t trackHeaderVersion = xmpData_["Xmp.video.TrackHeaderVersion"].toLong();
                        buf = returnBuf(trackHeaderVersion);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if(currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.TrackHeaderVersion"].count() >0)
                    {
                        int64_t trackHeaderVersion = xmpData_["Xmp.audio.TrackHeaderVersion"].toLong();
                        buf = returnBuf(trackHeaderVersion);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
            case TrackCreateDate:
                //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.TrackCreateDate"].count() >0)
                    {
                        int64_t trackCreateDate = xmpData_["Xmp.video.TrackCreateDate"].toLong();
                        buf = returnBuf(trackCreateDate);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if(currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.TrackCreateDate"].count() >0)
                    {
                        int64_t trackCreateDate = xmpData_["Xmp.audio.TrackCreateDate"].toLong();
                        buf = returnBuf(trackCreateDate);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
            case TrackModifyDate:
                //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.TrackModifyDate"].count() >0)
                    {
                        int64_t trackModifyDate = xmpData_["Xmp.video.TrackModifyDate"].toLong();
                        buf = returnBuf(trackModifyDate);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if(currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.TrackModifyDate"].count() >0)
                    {
                        int64_t trackModifyDate = xmpData_["Xmp.audio.TrackModifyDate"].toLong();
                        buf = returnBuf(trackModifyDate);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
            case TrackID:
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.TrackID"].count() >0)
                    {
                        int64_t trackID = xmpData_["Xmp.video.TrackID"].toLong();
                        buf = returnBuf(trackID);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if(currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.TrackID"].count() >0)
                    {
                        int64_t trackID = xmpData_["Xmp.audio.TrackID"].toLong();
                        buf = returnBuf(trackID);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
            case TrackDuration:
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.TrackDuration"].count() >0)
                    {
                        int64_t trackDuration= xmpData_["Xmp.video.TrackDuration"].toLong();
                        buf = returnBuf(trackDuration);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if(currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.TrackDuration"].count() >0)
                    {
                        int64_t trackDuration= xmpData_["Xmp.audio.TrackDuration"].toLong();
                        buf = returnBuf(trackDuration);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
                //TODO:modify below 3 values before writing to file
            case TrackLayer:
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.TrackLayer"].count() >0)
                    {
                        int64_t trackLayer = xmpData_["Xmp.video.TrackLayer"].toLong();
                        buf = returnBuf(trackLayer);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if(currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.TrackLayer"].count() >0)
                    {
                        int64_t trackLayer = xmpData_["Xmp.audio.TrackLayer"].toLong();
                        buf = returnBuf(trackLayer);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
            case TrackVolume:
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.TrackVolume"].count() >0)
                    {
                        int64_t trackVolume = xmpData_["Xmp.video.TrackVolume"].toLong();
                        buf = returnBuf(trackVolume);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                else if(currentStream_ == Audio)
                {
                    if(xmpData_["Xmp.audio.TrackVolume"].count() >0)
                    {
                        int64_t trackVolume = xmpData_["Xmp.audio.TrackVolume"].toLong();
                        buf = returnBuf(trackVolume);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);
                    }
                }
                break;
            case ImageWidth:
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.Width"].count() >0)
                    {
                        int64_t width = xmpData_["Xmp.video.Width"].toLong();
                        buf = returnBuf(width);
                        io_->write(buf.pData_,4);
                    }
                    else
                    {
                        io_->seek(4,BasicIo::cur);

                    }
                }
                break;
            case ImageHeight:
                if(currentStream_ == Video)
                {
                    if(xmpData_["Xmp.video.Height"].count() >0)
                    {
                        int64_t Height = xmpData_["Xmp.video.Height"].toLong();
                        buf = returnBuf(Height);
                        io_->write(buf.pData_,4);
                    }
                }
                break;
            default:
                io_->seek(4,BasicIo::cur);
                break;
            }
        }
        io_->read(buf.pData_, size%4);
    }
} // QuickTimeVideo::trackHeaderDecoder

void QuickTimeVideo::movieHeaderDecoder(unsigned long size)
{
    DataBuf buf(5);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';

    if(!m_modifyMetadata)
    {
        for (int i = 0; size/4 != 0 ; size -=4, i++)
        {
            io_->read(buf.pData_, 4);
            switch(i)
            {
            case MovieHeaderVersion:
                xmpData_["Xmp.video.MovieHeaderVersion"] = returnBufValue(buf,1); break;
            case CreateDate:
                //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
                xmpData_["Xmp.video.DateUTC"] = returnUnsignedBufValue(buf); break;
            case ModifyDate:
                //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
                xmpData_["Xmp.video.ModificationDate"] = returnUnsignedBufValue(buf); break;
            case TimeScale:
                xmpData_["Xmp.video.TimeScale"] = returnBufValue(buf);
                timeScale_ = returnBufValue(buf); break;
            case Duration:
                xmpData_["Xmp.video.Duration"] = returnBufValue(buf) * 1000 / timeScale_; break;
            case PreferredRate:
                xmpData_["Xmp.video.PreferredRate"] = returnBufValue(buf, 2) + ((buf.pData_[2] * 256 + buf.pData_[3]) * 0.01); break;
            case PreferredVolume:
                xmpData_["Xmp.video.PreferredVolume"] = (returnBufValue(buf, 1) + (buf.pData_[2] * 0.1)) * 100; break;
            case PreviewTime:
                xmpData_["Xmp.video.PreviewTime"] = returnBufValue(buf); break;
            case PreviewDuration:
                xmpData_["Xmp.video.PreviewDuration"] = returnBufValue(buf); break;
            case PosterTime:
                xmpData_["Xmp.video.PosterTime"] = returnBufValue(buf); break;
            case SelectionTime:
                xmpData_["Xmp.video.SelectionTime"] = returnBufValue(buf); break;
            case SelectionDuration:
                xmpData_["Xmp.video.SelectionDuration"] = returnBufValue(buf); break;
            case CurrentTime:
                xmpData_["Xmp.video.CurrentTime"] = returnBufValue(buf); break;
            case NextTrackID:
                xmpData_["Xmp.video.NextTrackID"] = returnBufValue(buf); break;
            default:
                break;
            }
        }
        io_->read(buf.pData_, size%4);
    }
    else
    {
        for (int i = 0; size/4 != 0 ; size -=4, i++)
        {
            switch(i)
            {
            case MovieHeaderVersion:
                if(xmpData_["Xmp.video.MovieHeaderVersion"].count() > 0)
                {
                    int64_t movieHeaderVersion  = (int64_t)xmpData_["Xmp.video.MovieHeaderVersion"].toLong();
                    buf = returnBuf(movieHeaderVersion);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case CreateDate:
                if(xmpData_["Xmp.video.DateUTC"].count() > 0)
                {
                    uint64_t dateUTC  = (uint64_t)xmpData_["Xmp.video.DateUTC"].toLong();
                    buf = returnBuf(dateUTC);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case ModifyDate:
                if(xmpData_["Xmp.video.ModificationDate"].count() > 0)
                {
                    uint64_t modificationDate  = (uint64_t)xmpData_["Xmp.video.ModificationDate"].toLong();
                    buf = returnBuf(modificationDate);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case TimeScale:
                if(xmpData_["Xmp.video.TimeScale"].count() > 0)
                {
                    uint64_t timeScale  = (uint64_t)xmpData_["Xmp.video.TimeScale"].toLong();
                    buf = returnBuf(timeScale);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case Duration:
                if(xmpData_["Xmp.video.Duration"].count() > 0)
                {
                    int64_t duration  = (int64_t)xmpData_["Xmp.video.Duration"].toLong();
                    buf = returnBuf(duration);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case PreferredRate:
                if(xmpData_["Xmp.video.PreferredRate"].count() > 0)
                {
                    int64_t preferredRate  = (int64_t)xmpData_["Xmp.video.PreferredRate"].toLong();
                    buf = returnBuf(preferredRate);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case PreferredVolume:
                if(xmpData_["Xmp.video.PreferredVolume"].count() > 0)
                {
                    int64_t preferredVolume  = (int64_t)xmpData_["Xmp.video.PreferredVolume"].toLong();
                    buf = returnBuf(preferredVolume);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(48,BasicIo::cur);
                }
                break;
            case PreviewTime:
                if(xmpData_["Xmp.video.PreviewTime"].count() > 0)
                {
                    int64_t previewTime  = (int64_t)xmpData_["Xmp.video.PreviewTime"].toLong();
                    buf = returnBuf(previewTime);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case PreviewDuration:
                if(xmpData_["Xmp.video.PreviewDuration"].count() > 0)
                {
                    int64_t previewDuration  = (int64_t)xmpData_["Xmp.video.PreviewDuration"].toLong();
                    buf = returnBuf(previewDuration);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case PosterTime:
                if(xmpData_["Xmp.video.PosterTime"].count() > 0)
                {
                    int64_t posterTime  = (int64_t)xmpData_["Xmp.video.PosterTime"].toLong();
                    buf = returnBuf(posterTime);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case SelectionTime:
                if(xmpData_["Xmp.video.SelectionTime"].count() > 0)
                {
                    int64_t selectionTime  = (int64_t)xmpData_["Xmp.video.SelectionTime"].toLong();
                    buf = returnBuf(selectionTime);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case SelectionDuration:
                if(xmpData_["Xmp.video.SelectionDuration"].count() > 0)
                {
                    int64_t selectionDuration  = (int64_t)xmpData_["Xmp.video.SelectionDuration"].toLong();
                    buf = returnBuf(selectionDuration);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case CurrentTime:
                if(xmpData_["Xmp.video.CurrentTime"].count() > 0)
                {
                    int64_t currentTime  = (int64_t)xmpData_["Xmp.video.CurrentTime"].toLong();
                    buf = returnBuf(currentTime);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            case NextTrackID:
                if(xmpData_["Xmp.video.NextTrackID"].count() > 0)
                {
                    int64_t nextTrackID  = (int64_t)xmpData_["Xmp.video.NextTrackID"].toLong();
                    buf = returnBuf(nextTrackID);
                    io_->write(buf.pData_,4);
                }
                else
                {
                    io_->seek(4,BasicIo::cur);
                }
                break;
            default:
                io_->seek(4,BasicIo::cur);
                break;
            }
        }
        io_->seek(size%4,BasicIo::cur);
    }
}// QuickTimeVideo::movieHeaderDecoder

void QuickTimeVideo::aspectRatio()
{
    //TODO - Make a better unified method to handle all cases of Aspect Ratio

    double aspectRatio = (double)width_ / (double)height_;
    aspectRatio = floor(aspectRatio*10) / 10;
    xmpData_["Xmp.video.AspectRatio"] = aspectRatio;

    int aR = (int) ((aspectRatio*10.0)+0.1);

    switch  (aR) {
    case 13 : xmpData_["Xmp.video.AspectRatio"] = "4:3"		; break;
    case 17 : xmpData_["Xmp.video.AspectRatio"] = "16:9"	; break;
    case 10 : xmpData_["Xmp.video.AspectRatio"] = "1:1"		; break;
    case 16 : xmpData_["Xmp.video.AspectRatio"] = "16:10"	; break;
    case 22 : xmpData_["Xmp.video.AspectRatio"] = "2.21:1"  ; break;
    case 23 : xmpData_["Xmp.video.AspectRatio"] = "2.35:1"  ; break;
    case 12 : xmpData_["Xmp.video.AspectRatio"] = "5:4"     ; break;
    default : xmpData_["Xmp.video.AspectRatio"] = aspectRatio;break;
    }
} // QuickTimeVideo::aspectRatio


Image::AutoPtr newQTimeInstance(BasicIo::AutoPtr io, bool /*create*/) {
    Image::AutoPtr image(new QuickTimeVideo(io));
    if (!image->good()) {
        image.reset();
    }
    return image;
}

bool isQTimeType(BasicIo& iIo, bool advance) {
    const int32_t len = 4;
    byte buf[len];
    iIo.read(buf, len);
    iIo.read(buf, len);

    if (iIo.error() || iIo.eof()) {
        return false;
    }

    bool matched = isQuickTimeType(buf[0], buf[1], buf[2], buf[3]);
    if (!advance || !matched) {
        iIo.seek(static_cast<long>(0), BasicIo::beg);
    }

    return matched;
}

}                                       // namespace Exiv2

