// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2012 Andreas Huggel <ahuggel@gmx.net>
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
  File:      riffvideo.cpp
  Version:   $Rev$
  Author(s): Abhinav Badola for GSoC 2012 (AB) <mail.abu.to@gmail.com>
             Mahesh Hegde for GSoC 2013 mailto:maheshmhegade at gmail dot com <maheshmhegade@gmail.com>
  History:   18-Jun-12, AB: created
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "riffvideo.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "types.hpp"
#include "tiffimage_int.hpp"
// + standard includes
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    /*!
      @brief Dummy TIFF header structure.
     */
    class DummyTiffHeader : public TiffHeaderBase {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        DummyTiffHeader(ByteOrder byteOrder);
        //! Destructor
        ~DummyTiffHeader();
        //@}

        //! @name Manipulators
        //@{
        //! Dummy read function. Does nothing and returns true.
        bool read(const byte* pData, uint32_t size);
        //@}

    }; // class TiffHeader

    DummyTiffHeader::DummyTiffHeader(ByteOrder byteOrder)
        : TiffHeaderBase(42, 0, byteOrder, 0)
    {
    }

    DummyTiffHeader::~DummyTiffHeader()
    {
    }

    bool DummyTiffHeader::read(const byte* /*pData*/, uint32_t /*size*/)
    {
        return true;
    }

    extern const TagVocabulary infoTags[] =  {
        {   "AGES", "Xmp.video.Rated" },
        {   "CMNT", "Xmp.video.Comment" },
        {   "CODE", "Xmp.video.EncodedBy" },
        {   "COMM", "Xmp.video.Comment" },
        {   "DIRC", "Xmp.video.Director" },
        {   "DISP", "Xmp.audio.SchemeTitle" },
        {   "DTIM", "Xmp.video.DateTimeOriginal" },
        {   "GENR", "Xmp.video.Genre" },
        {   "IARL", "Xmp.video.ArchivalLocation" },
        {   "IART", "Xmp.video.Artist" },
        {   "IAS1", "Xmp.video.Edit1" },
        {   "IAS2", "Xmp.video.Edit2" },
        {   "IAS3", "Xmp.video.Edit3" },
        {   "IAS4", "Xmp.video.Edit4" },
        {   "IAS5", "Xmp.video.Edit5" },
        {   "IAS6", "Xmp.video.Edit6" },
        {   "IAS7", "Xmp.video.Edit7" },
        {   "IAS8", "Xmp.video.Edit8" },
        {   "IAS9", "Xmp.video.Edit9" },
        {   "IBSU", "Xmp.video.BaseURL" },
        {   "ICAS", "Xmp.audio.DefaultStream" },
        {   "ICDS", "Xmp.video.CostumeDesigner" },
        {   "ICMS", "Xmp.video.Commissioned" },
        {   "ICMT", "Xmp.video.Comment" },
        {   "ICNM", "Xmp.video.Cinematographer" },
        {   "ICNT", "Xmp.video.Country" },
        {   "ICOP", "Xmp.video.Copyright" },
        {   "ICRD", "Xmp.video.DateTimeDigitized" },
        {   "ICRP", "Xmp.video.Cropped" },
        {   "IDIM", "Xmp.video.Dimensions	" },
        {   "IDPI", "Xmp.video.DotsPerInch" },
        {   "IDST", "Xmp.video.DistributedBy" },
        {   "IEDT", "Xmp.video.EditedBy" },
        {   "IENC", "Xmp.video.EncodedBy" },
        {   "IENG", "Xmp.video.Engineer" },
        {   "IGNR", "Xmp.video.Genre" },
        {   "IKEY", "Xmp.video.PerformerKeywords" },
        {   "ILGT", "Xmp.video.Lightness" },
        {   "ILGU", "Xmp.video.LogoURL" },
        {   "ILIU", "Xmp.video.LogoIconURL" },
        {   "ILNG", "Xmp.video.Language" },
        {   "IMBI", "Xmp.video.InfoBannerImage" },
        {   "IMBU", "Xmp.video.InfoBannerURL" },
        {   "IMED", "Xmp.video.Medium" },
        {   "IMIT", "Xmp.video.InfoText" },
        {   "IMIU", "Xmp.video.InfoURL" },
        {   "IMUS", "Xmp.video.MusicBy" },
        {   "INAM", "Xmp.video.Title" },
        {   "IPDS", "Xmp.video.ProductionDesigner" },
        {   "IPLT", "Xmp.video.NumOfColors" },
        {   "IPRD", "Xmp.video.Product" },
        {   "IPRO", "Xmp.video.ProducedBy" },
        {   "IRIP", "Xmp.video.RippedBy" },
        {   "IRTD", "Xmp.video.Rating" },
        {   "ISBJ", "Xmp.video.Subject" },
        {   "ISFT", "Xmp.video.Software" },
        {   "ISGN", "Xmp.video.SecondaryGenre" },
        {   "ISHP", "Xmp.video.Sharpness" },
        {   "ISRC", "Xmp.video.Source" },
        {   "ISRF", "Xmp.video.SourceForm" },
        {   "ISTD", "Xmp.video.ProductionStudio" },
        {   "ISTR", "Xmp.video.Starring" },
        {   "ITCH", "Xmp.video.Technician" },
        {   "IWMU", "Xmp.video.WatermarkURL" },
        {   "IWRI", "Xmp.video.WrittenBy" },
        {   "LANG", "Xmp.video.Language" },
        {   "LOCA", "Xmp.video.LocationInfo" },
        {   "PRT1", "Xmp.video.Part" },
        {   "PRT2", "Xmp.video.NumOfParts" },
        {   "RATE", "Xmp.video.Rate" },
        {   "STAR", "Xmp.video.Starring" },
        {   "STAT", "Xmp.video.Statistics" },
        {   "TAPE", "Xmp.video.TapeName" },
        {   "TCDO", "Xmp.video.EndTimecode" },
        {   "TCOD", "Xmp.video.StartTimecode" },
        {   "TITL", "Xmp.video.Title" },
        {   "TLEN", "Xmp.video.Length" },
        {   "TORG", "Xmp.video.Organization" },
        {   "TRCK", "Xmp.video.TrackNumber" },
        {   "TURL", "Xmp.video.URL" },
        {   "TVER", "Xmp.video.SoftwareVersion" },
        {   "VMAJ", "Xmp.video.VegasVersionMajor" },
        {   "VMIN", "Xmp.video.VegasVersionMinor" },
        {   "YEAR", "Xmp.video.Year" }
    };

    extern const TagDetails audioEncodingValues[] =  {
        {   0x1, "Microsoft PCM" },
        {   0x2, "Microsoft ADPCM" },
        {   0x3, "Microsoft IEEE float" },
        {   0x4, "Compaq VSELP" },
        {   0x5, "IBM CVSD" },
        {   0x6, "Microsoft a-Law" },
        {   0x7, "Microsoft u-Law" },
        {   0x8, "Microsoft DTS" },
        {   0x9, "DRM" },
        {   0xa, "WMA 9 Speech" },
        {   0xb, "Microsoft Windows Media RT Voice" },
        {   0x10, "OKI-ADPCM" },
        {   0x11, "Intel IMA/DVI-ADPCM" },
        {   0x12, "Videologic Mediaspace ADPCM" },
        {   0x13, "Sierra ADPCM" },
        {   0x14, "Antex G.723 ADPCM" },
        {   0x15, "DSP Solutions DIGISTD" },
        {   0x16, "DSP Solutions DIGIFIX" },
        {   0x17, "Dialoic OKI ADPCM" },
        {   0x18, "Media Vision ADPCM" },
        {   0x19, "HP CU" },
        {   0x1a, "HP Dynamic Voice" },
        {   0x20, "Yamaha ADPCM" },
        {   0x21, "SONARC Speech Compression" },
        {   0x22, "DSP Group True Speech" },
        {   0x23, "Echo Speech Corp." },
        {   0x24, "Virtual Music Audiofile AF36" },
        {   0x25, "Audio Processing Tech." },
        {   0x26, "Virtual Music Audiofile AF10" },
        {   0x27, "Aculab Prosody 1612" },
        {   0x28, "Merging Tech. LRC" },
        {   0x30, "Dolby AC2" },
        {   0x31, "Microsoft GSM610" },
        {   0x32, "MSN Audio" },
        {   0x33, "Antex ADPCME" },
        {   0x34, "Control Resources VQLPC" },
        {   0x35, "DSP Solutions DIGIREAL" },
        {   0x36, "DSP Solutions DIGIADPCM" },
        {   0x37, "Control Resources CR10" },
        {   0x38, "Natural MicroSystems VBX ADPCM" },
        {   0x39, "Crystal Semiconductor IMA ADPCM" },
        {   0x3a, "Echo Speech ECHOSC3" },
        {   0x3b, "Rockwell ADPCM" },
        {   0x3c, "Rockwell DIGITALK" },
        {   0x3d, "Xebec Multimedia" },
        {   0x40, "Antex G.721 ADPCM" },
        {   0x41, "Antex G.728 CELP" },
        {   0x42, "Microsoft MSG723" },
        {   0x43, "IBM AVC ADPCM" },
        {   0x45, "ITU-T G.726" },
        {   0x50, "Microsoft MPEG" },
        {   0x51, "RT23 or PAC" },
        {   0x52, "InSoft RT24" },
        {   0x53, "InSoft PAC" },
        {   0x55, "MP3" },
        {   0x59, "Cirrus" },
        {   0x60, "Cirrus Logic" },
        {   0x61, "ESS Tech. PCM" },
        {   0x62, "Voxware Inc." },
        {   0x63, "Canopus ATRAC" },
        {   0x64, "APICOM G.726 ADPCM" },
        {   0x65, "APICOM G.722 ADPCM" },
        {   0x66, "Microsoft DSAT" },
        {   0x67, "Micorsoft DSAT DISPLAY" },
        {   0x69, "Voxware Byte Aligned" },
        {   0x70, "Voxware AC8" },
        {   0x71, "Voxware AC10" },
        {   0x72, "Voxware AC16" },
        {   0x73, "Voxware AC20" },
        {   0x74, "Voxware MetaVoice" },
        {   0x75, "Voxware MetaSound" },
        {   0x76, "Voxware RT29HW" },
        {   0x77, "Voxware VR12" },
        {   0x78, "Voxware VR18" },
        {   0x79, "Voxware TQ40" },
        {   0x7a, "Voxware SC3" },
        {   0x7b, "Voxware SC3" },
        {   0x80, "Soundsoft" },
        {   0x81, "Voxware TQ60" },
        {   0x82, "Microsoft MSRT24" },
        {   0x83, "AT&T G.729A" },
        {   0x84, "Motion Pixels MVI MV12" },
        {   0x85, "DataFusion G.726" },
        {   0x86, "DataFusion GSM610" },
        {   0x88, "Iterated Systems Audio" },
        {   0x89, "Onlive" },
        {   0x8a, "Multitude, Inc. FT SX20" },
        {   0x8b, "Infocom ITS A/S G.721 ADPCM" },
        {   0x8c, "Convedia G729" },
        {   0x8d, "Not specified congruency, Inc." },
        {   0x91, "Siemens SBC24" },
        {   0x92, "Sonic Foundry Dolby AC3 APDIF" },
        {   0x93, "MediaSonic G.723" },
        {   0x94, "Aculab Prosody 8kbps" },
        {   0x97, "ZyXEL ADPCM" },
        {   0x98, "Philips LPCBB" },
        {   0x99, "Studer Professional Audio Packed" },
        {   0xa0, "Malden PhonyTalk" },
        {   0xa1, "Racal Recorder GSM" },
        {   0xa2, "Racal Recorder G720.a" },
        {   0xa3, "Racal G723.1" },
        {   0xa4, "Racal Tetra ACELP" },
        {   0xb0, "NEC AAC NEC Corporation" },
        {   0xff, "AAC" },
        {   0x100, "Rhetorex ADPCM" },
        {   0x101, "IBM u-Law" },
        {   0x102, "IBM a-Law" },
        {   0x103, "IBM ADPCM" },
        {   0x111, "Vivo G.723" },
        {   0x112, "Vivo Siren" },
        {   0x120, "Philips Speech Processing CELP" },
        {   0x121, "Philips Speech Processing GRUNDIG" },
        {   0x123, "Digital G.723" },
        {   0x125, "Sanyo LD ADPCM" },
        {   0x130, "Sipro Lab ACEPLNET" },
        {   0x131, "Sipro Lab ACELP4800" },
        {   0x132, "Sipro Lab ACELP8V3" },
        {   0x133, "Sipro Lab G.729" },
        {   0x134, "Sipro Lab G.729A" },
        {   0x135, "Sipro Lab Kelvin" },
        {   0x136, "VoiceAge AMR" },
        {   0x140, "Dictaphone G.726 ADPCM" },
        {   0x150, "Qualcomm PureVoice" },
        {   0x151, "Qualcomm HalfRate" },
        {   0x155, "Ring Zero Systems TUBGSM" },
        {   0x160, "Microsoft Audio1" },
        {   0x161, "Windows Media Audio V2 V7 V8 V9 / DivX audio (WMA) / Alex AC3 Audio" },
        {   0x162, "Windows Media Audio Professional V9" },
        {   0x163, "Windows Media Audio Lossless V9" },
        {   0x164, "WMA Pro over S/PDIF" },
        {   0x170, "UNISYS NAP ADPCM" },
        {   0x171, "UNISYS NAP ULAW" },
        {   0x172, "UNISYS NAP ALAW" },
        {   0x173, "UNISYS NAP 16K" },
        {   0x174, "MM SYCOM ACM SYC008 SyCom Technologies" },
        {   0x175, "MM SYCOM ACM SYC701 G726L SyCom Technologies" },
        {   0x176, "MM SYCOM ACM SYC701 CELP54 SyCom Technologies" },
        {   0x177, "MM SYCOM ACM SYC701 CELP68 SyCom Technologies" },
        {   0x178, "Knowledge Adventure ADPCM" },
        {   0x180, "Fraunhofer IIS MPEG2AAC" },
        {   0x190, "Digital Theater Systems DTS DS" },
        {   0x200, "Creative Labs ADPCM" },
        {   0x202, "Creative Labs FASTSPEECH8" },
        {   0x203, "Creative Labs FASTSPEECH10" },
        {   0x210, "UHER ADPCM" },
        {   0x215, "Ulead DV ACM" },
        {   0x216, "Ulead DV ACM" },
        {   0x220, "Quarterdeck Corp." },
        {   0x230, "I-Link VC" },
        {   0x240, "Aureal Semiconductor Raw Sport" },
        {   0x241, "ESST AC3" },
        {   0x250, "Interactive Products HSX" },
        {   0x251, "Interactive Products RPELP" },
        {   0x260, "Consistent CS2" },
        {   0x270, "Sony SCX" },
        {   0x271, "Sony SCY" },
        {   0x272, "Sony ATRAC3" },
        {   0x273, "Sony SPC" },
        {   0x280, "TELUM Telum Inc." },
        {   0x281, "TELUMIA Telum Inc." },
        {   0x285, "Norcom Voice Systems ADPCM" },
        {   0x300, "Fujitsu FM TOWNS SND" },
        {   0x301, "Fujitsu (not specified)" },
        {   0x302, "Fujitsu (not specified)" },
        {   0x303, "Fujitsu (not specified)" },
        {   0x304, "Fujitsu (not specified)" },
        {   0x305, "Fujitsu (not specified)" },
        {   0x306, "Fujitsu (not specified)" },
        {   0x307, "Fujitsu (not specified)" },
        {   0x308, "Fujitsu (not specified)" },
        {   0x350, "Micronas Semiconductors, Inc. Development" },
        {   0x351, "Micronas Semiconductors, Inc. CELP833" },
        {   0x400, "Brooktree Digital" },
        {   0x401, "Intel Music Coder (IMC)" },
        {   0x402, "Ligos Indeo Audio" },
        {   0x450, "QDesign Music" },
        {   0x500, "On2 VP7 On2 Technologies" },
        {   0x501, "On2 VP6 On2 Technologies" },
        {   0x680, "AT&T VME VMPCM" },
        {   0x681, "AT&T TCP" },
        {   0x700, "YMPEG Alpha (dummy for MPEG-2 compressor)" },
        {   0x8ae, "ClearJump LiteWave (lossless)" },
        {   0x1000, "Olivetti GSM" },
        {   0x1001, "Olivetti ADPCM" },
        {   0x1002, "Olivetti CELP" },
        {   0x1003, "Olivetti SBC" },
        {   0x1004, "Olivetti OPR" },
        {   0x1100, "Lernout & Hauspie" },
        {   0x1101, "Lernout & Hauspie CELP codec" },
        {   0x1102, "Lernout & Hauspie SBC codec" },
        {   0x1103, "Lernout & Hauspie SBC codec" },
        {   0x1104, "Lernout & Hauspie SBC codec" },
        {   0x1400, "Norris Comm. Inc." },
        {   0x1401, "ISIAudio" },
        {   0x1500, "AT&T Soundspace Music Compression" },
        {   0x181c, "VoxWare RT24 speech codec" },
        {   0x181e, "Lucent elemedia AX24000P Music codec" },
        {   0x1971, "Sonic Foundry LOSSLESS" },
        {   0x1979, "Innings Telecom Inc. ADPCM" },
        {   0x1c07, "Lucent SX8300P speech codec" },
        {   0x1c0c, "Lucent SX5363S G.723 compliant codec" },
        {   0x1f03, "CUseeMe DigiTalk (ex-Rocwell)" },
        {   0x1fc4, "NCT Soft ALF2CD ACM" },
        {   0x2000, "FAST Multimedia DVM" },
        {   0x2001, "Dolby DTS (Digital Theater System)" },
        {   0x2002, "RealAudio 1 / 2 14.4" },
        {   0x2003, "RealAudio 1 / 2 28.8" },
        {   0x2004, "RealAudio G2 / 8 Cook (low bitrate)" },
        {   0x2005, "RealAudio 3 / 4 / 5 Music (DNET)" },
        {   0x2006, "RealAudio 10 AAC (RAAC)" },
        {   0x2007, "RealAudio 10 AAC+ (RACP)" },
        {   0x2500, "Reserved range to 0x2600 Microsoft" },
        {   0x3313, "makeAVIS (ffvfw fake AVI sound from AviSynth scripts)" },
        {   0x4143, "Divio MPEG-4 AAC audio" },
        {   0x4201, "Nokia adaptive multirate" },
        {   0x4243, "Divio G726 Divio, Inc." },
        {   0x434c, "LEAD Speech" },
        {   0x564c, "LEAD Vorbis" },
        {   0x5756, "WavPack Audio" },
        {   0x674f, "Ogg Vorbis (mode 1)" },
        {   0x6750, "Ogg Vorbis (mode 2)" },
        {   0x6751, "Ogg Vorbis (mode 3)" },
        {   0x676f, "Ogg Vorbis (mode 1+)" },
        {   0x6770, "Ogg Vorbis (mode 2+)" },
        {   0x6771, "Ogg Vorbis (mode 3+)" },
        {   0x7000, "3COM NBX 3Com Corporation" },
        {   0x706d, "FAAD AAC" },
        {   0x7a21, "GSM-AMR (CBR, no SID)" },
        {   0x7a22, "GSM-AMR (VBR, including SID)" },
        {   0xa100, "Comverse Infosys Ltd. G723 1" },
        {   0xa101, "Comverse Infosys Ltd. AVQSBC" },
        {   0xa102, "Comverse Infosys Ltd. OLDSBC" },
        {   0xa103, "Symbol Technologies G729A" },
        {   0xa104, "VoiceAge AMR WB VoiceAge Corporation" },
        {   0xa105, "Ingenient Technologies Inc. G726" },
        {   0xa106, "ISO/MPEG-4 advanced audio Coding" },
        {   0xa107, "Encore Software Ltd G726" },
        {   0xa109, "Speex ACM Codec xiph.org" },
        {   0xdfac, "DebugMode SonicFoundry Vegas FrameServer ACM Codec" },
        {   0xe708, "Unknown -" },
        {   0xf1ac, "Free Lossless Audio Codec FLAC" },
        {   0xfffe, "Extensible" },
        {   0xffff, "Development" }
    };

    extern const TagDetails nikonAVITags[] =  {
        {   0x0003, "Xmp.video.Make" },
        {   0x0004, "Xmp.video.Model" },
        {   0x0005, "Xmp.video.Software" },
        {   0x0006, "Xmp.video.Equipment" },
        {   0x0007, "Xmp.video.Orientation" },
        {   0x0008, "Xmp.video.ExposureTime" },
        {   0x0009, "Xmp.video.FNumber" },
        {   0x000a, "Xmp.video.ExposureCompensation" },
        {   0x000b, "Xmp.video.MaxApertureValue" },
        {   0x000c, "Xmp.video.MeteringMode" },
        {   0x000f, "Xmp.video.FocalLength" },
        {   0x0010, "Xmp.video.XResolution" },
        {   0x0011, "Xmp.video.YResolution" },
        {   0x0012, "Xmp.video.ResolutionUnit" },
        {   0x0013, "Xmp.video.DateTimeOriginal" },
        {   0x0014, "Xmp.video.DateTimeDigitized" },
        {   0x0016, "Xmp.video.duration" },
        {   0x0018, "Xmp.video.FocusMode" },
        {   0x001b, "Xmp.video.DigitalZoomRatio" },
        {   0x001d, "Xmp.video.ColorMode" },
        {   0x001e, "Xmp.video.Sharpness" },
        {   0x001f, "Xmp.video.WhiteBalance" },
        {   0x0020, "Xmp.video.ColorNoiseReduction" }
    };

    /*
    extern const TagDetails orientation[] =  {
        {   1, "Horizontal (normal)" },
        {   2, "Mirror horizontal" },
        {   3, "Rotate 180" },
        {   4, "Mirror vertical" },
        {   5, "Mirror horizontal and rotate 270 CW" },
        {   6, "Rotate 90 CW" },
        {   7, "Mirror horizontal and rotate 90 CW" },
        {   8, "Rotate 270 CW" }
    };
     */
    extern const TagDetails meteringMode[] =  {
        {   0, "Unknown" },
        {   1, "Average" },
        {   2, "Center-weighted average" },
        {   3, "Spot" },
        {   4, "Multi-spot" },
        {   5, "Multi-segment" },
        {   6, "Partial" },
        { 255, "Other" }
    };

    extern const TagDetails resolutionUnit[] =  {
        {   1, "None" },
        {   2, "inches" },
        {   3, "cm" }
    };

    /*!
      @brief Function used to check equality of a Tags with a
          particular string (ignores case while comparing).
      @param buf Data buffer that will contain Tag to compare
      @param str char* Pointer to string
      @return Returns true if the buffer value is equal to string.
     */
    bool equalsRiffTag(Exiv2::DataBuf& buf ,const char* str) {
        for(int i = 0; i < 4; i++ )
            if(toupper(buf.pData_[i]) != str[i])
                return false;
        return true;
    }

    bool simpleBytesComparision(Exiv2::DataBuf& buf ,const char* str,int size)
    {
        for(int i = 0; i < size; i++ )
            if(toupper(buf.pData_[i]) != str[i])
                return false;
        return true;
    }

    bool equalsRiffTag(Exiv2::DataBuf& buf,const char arr[][5],int arraysize)
    {
        for (int i=0; i< arraysize; i++)
        {
            bool matched = true;
            for(int j = 0; j < 4; j++ )
            {
                if(toupper(buf.pData_[j]) != arr[i][j])
                {
                    matched = false;
                    break;
                }
            }
            if(matched)
            {
                return true;
            }
        }
    }

    enum streamTypeInfo
    {
        Audio = 1, MIDI, Text, Video
    };
    enum streamHeaderTags
    {
        codec = 1, sampleRate = 5, sampleCount = 8, quality = 10, sampleSize
    };
    enum bmptags
    {
        imageWidth, imageHeight, planes, bitDepth, compression, imageLength, pixelsPerMeterX, pixelsPerMeterY, numColors, numImportantColors
    };
    enum audioFormatTags
    {
        encoding, numberOfChannels, audioSampleRate, avgBytesPerSec = 4, bitsPerSample = 7
    };
    enum aviHeaderTags
    {
        frameRate, maxDataRate, frameCount = 4,  initialFrames ,streamCount, suggestedBufferSize ,imageWidth_h, imageHeight_h
    };
    }
}                                      // namespace Internal, Exiv2

namespace Exiv2 {
using namespace Exiv2::Internal;

RiffVideo::RiffVideo(BasicIo::AutoPtr io)
    : Image(ImageType::riff, mdNone, io)
{
    m_modifyMetadata = false;
    m_decodeMetaData = true;
} // RiffVideo::RiffVideo

std::string RiffVideo::mimeType() const
{
    return "video/riff";
}

std::vector<long> RiffVideo::findChunkPositions(const char* chunkId)
{
    DataBuf chkId((unsigned long)5);
    chkId.pData_[4] = '\0';

    std::vector<long> positions;
    for (int i=0;i<m_riffFileSkeleton.m_headerChunks.size();i++)
    {
        for (int j=0;j<m_riffFileSkeleton.m_headerChunks[i]->m_primitiveChunks.size();j++)
        {
            io_->seek(m_riffFileSkeleton.m_headerChunks[i]->m_primitiveChunks[j]->m_chunkLocation,BasicIo::beg);
            io_->read(chkId.pData_,4);
            if(equalsRiffTag(chkId,chunkId))
            {
                positions.push_back((long)io_->tell());
            }
        }
    }
    return positions;
}

std::vector<long> RiffVideo::findHeaderPositions(const char* headerId)
{
    DataBuf hdrId((unsigned long)5);
    hdrId.pData_[4] = '\0';

    std::vector<long> positions;
    for (int i=0;i<m_riffFileSkeleton.m_headerChunks.size();i++)
    {
        io_->seek((m_riffFileSkeleton.m_headerChunks[i]->m_headerLocation + 4),BasicIo::beg);
        io_->read(hdrId.pData_,4);
        if(equalsRiffTag(hdrId,headerId))
        {
            positions.push_back((long)io_->tell() - 8);
        }

    }
    return positions;
}

void RiffVideo::writeMetadata()
{
    if (io_->open() != 0)
    {
        throw Error(9, io_->path(), strError());
    }
    IoCloser closer(*io_);

    doWriteMetadata();

    io_->close();

} // RiffVideo::writeMetadata

void RiffVideo::doWriteMetadata()
{
    //pre-write checks to make sure that we are writing to provided Riff file only
    if (!io_->isopen())
    {
        throw Error(20);
    }
    if (!isRiffType(*io_, false))
    {
        if (io_->error() || io_->eof()) throw Error(14);
        throw Error(3, "RIFF");
    }

    unsigned long bufMinSize = 4;

    //some initial variable declarations and initialization before traversing through chunks
    DataBuf chkId(bufMinSize+1);
    DataBuf chkSize(bufMinSize+1);

    chkId.pData_[4] = '\0';
    chkSize.pData_[4] = '\0';

    //skip "RIFF" "chkSize" "AVI"
    io_->read(chkId.pData_, bufMinSize);
    io_->read(chkSize.pData_, bufMinSize);
    io_->read(chkId.pData_, bufMinSize);

    m_decodeMetaData = false;
    m_modifyMetadata = true;
    tagDecoder();
    //write metadata in order

    //find and move to avih position
    long dummyLong = 0;

    std::vector<long> avihChunkPositions = findChunkPositions("AVIH");
    for (int i=0; i<avihChunkPositions.size(); i++)
    {
        io_->seek(avihChunkPositions[i]+4,BasicIo::beg);
        aviHeaderTagsHandler(dummyLong);
    }

    //find and move to strh position
    std::vector<long> strhChunkPositions = findChunkPositions("STRH");
    for(int i=0; i< strhChunkPositions.size(); i++)
    {
        io_->seek(strhChunkPositions[i]+4,BasicIo::beg);
        setStreamType();
        streamHandler(dummyLong);
    }

    std::vector<long> strfChunkPositions = findChunkPositions("STRF");
    for(int i=0; i<strfChunkPositions.size(); i++)
    {
        io_->seek(strhChunkPositions[i]+4,BasicIo::beg);
        setStreamType();
        io_->seek(strfChunkPositions[i]+4,BasicIo::beg);
        streamFormatHandler(dummyLong);
    }

    std::vector<long> fmtChunkPositions = findChunkPositions("FMT ");
    for(int i=0; i< fmtChunkPositions.size(); i++)
    {
        streamType_ = Audio;
        io_->seek(fmtChunkPositions[i]+4,BasicIo::beg);
        streamHandler(dummyLong);
    }

    std::vector<long> strnChunkPositions = findChunkPositions("STRN");
    for(int i=0;i<strnChunkPositions.size(); i++)
    {
        io_->seek(strnChunkPositions[i]+4,BasicIo::beg);
        dateTimeOriginal(dummyLong);
    }

    std::vector<long> iditChunkPositions = findHeaderPositions("IDIT");
    for(int i=0; i<iditChunkPositions.size(); i++)
    {
        io_->seek(iditChunkPositions[i]+8,BasicIo::beg);
        dateTimeOriginal(dummyLong);
    }

    std::vector<long> odmlHeaderPositions = findHeaderPositions("ODML");
    for(int i=0;i<odmlHeaderPositions.size(); i++)
    {
        io_->seek(odmlHeaderPositions[i],BasicIo::beg);
        odmlTagsHandler();
    }

    std::vector<long> ncdtHeaderPositions = findHeaderPositions("NCDT");
    for(int i=0;i<ncdtHeaderPositions.size(); i++)
    {
        io_->seek(ncdtHeaderPositions[i],BasicIo::beg);
        nikonTagsHandler();
    }

    std::vector<long> infoHeaderPositions = findHeaderPositions("INFO");
    for(int i=0;i<infoHeaderPositions.size(); i++)
    {
        io_->seek(infoHeaderPositions[i],BasicIo::beg);
        infoTagsHandler();
    }
    //TODO :implement write functionality for header chunks and exif and iptc metadata

}// RiffVideo::doWriteMetadata

void RiffVideo::readMetadata()
{
    if (io_->open() != 0) throw Error(9, io_->path(), strError());

    // Ensure that this is the correct image type
    if (!isRiffType(*io_, false))
    {
        if (io_->error() || io_->eof()) throw Error(14);
        throw Error(3, "RIFF");
    }

    clearMetadata();
    IoCloser closer(*io_);

    xmpData_["Xmp.video.FileSize"] = (double)io_->size()/(double)1048576;
    xmpData_["Xmp.video.FileName"] = io_->path();
    xmpData_["Xmp.video.MimeType"] = mimeType();

    DataBuf chkId((const long)5);
    DataBuf chkSize((const long)5);
    DataBuf chkHeader((const long)5);

    chkId.pData_[4] = '\0';

    io_->read(chkId.pData_, 4);
    xmpData_["Xmp.video.Container"] = chkId.pData_;

    //skip file size
    io_->read(chkSize.pData_, 4);
    io_->read(chkHeader.pData_, 4);
    xmpData_["Xmp.video.FileType"] = chkHeader.pData_;

    m_decodeMetaData = true;
    decodeBlock();
} // RiffVideo::readMetadata

void RiffVideo::decodeBlock()
{
    DataBuf chkId((const long)5);
    chkId.pData_[4] = '\0' ;

    io_->read(chkId.pData_, 4);

    if(io_->eof())
    {
        return;
    }
    else if(equalsRiffTag(chkId, "LIST") || equalsRiffTag(chkId, "JUNK"))
    {
        io_->seek(-4,BasicIo::cur);
        tagDecoder();
    }
} // RiffVideo::decodeBlock

void RiffVideo::tagDecoder()
{
    DataBuf chkMainId((const long)5);
    chkMainId.pData_[4] = '\0';
    io_->read(chkMainId.pData_, 4);

    if (equalsRiffTag(chkMainId, "LIST"))
    {
        DataBuf listSize((const long)5);
        DataBuf chkHeader((const long)5);

        listSize.pData_[4] = '\0';
        chkHeader.pData_[4] = '\0';

        io_->read(listSize.pData_, 4);
        unsigned long listsize = Exiv2::getULong(listSize.pData_, littleEndian);
        io_->read(chkHeader.pData_, 4);

        ListChunk *tmpList = new ListChunk();
        tmpList->m_listLocation = io_->tell() - 8;
        tmpList->m_listSize = listsize;

        m_riffFileSkeleton.m_lists.push_back(tmpList);

        HeaderChunk *tmpHeaderChunk = new HeaderChunk();
        memcpy((byte *)tmpHeaderChunk->m_headerId,(const byte*)chkHeader.pData_,5);

        unsigned long listend = io_->tell() + listsize - 4 ;

        IoPosition position = PremitiveChunk;
        while( (position == PremitiveChunk) && (io_->tell() < listend) )
        {
            DataBuf chkId((const long)5);
            DataBuf chkSize((const long)5);
            chkId.pData_[4] = '\0';
            chkSize.pData_[4] = '\0';

            io_->read(chkId.pData_, 4);
            io_->read(chkSize.pData_, 4);

            unsigned long size = Exiv2::getULong(chkSize.pData_, littleEndian);

            const char allPrimitiveFlags[][5]={"JUNK","AVIH","FMT ","STRH","STRF","STRN","STRD"};
            const char allHeaderFlags[][5]={"INFO","NCDT","ODML"};

            if(equalsRiffTag(chkId,allPrimitiveFlags,(int)(sizeof(allPrimitiveFlags)/5)))
            {
                PrimitiveChunk *tmpPremitiveChunk = new PrimitiveChunk();
                memcpy((byte*)tmpPremitiveChunk->m_chunkId,(const byte*)chkId.pData_,5);
                tmpPremitiveChunk->m_chunkLocation = io_->tell() - 8;
                tmpPremitiveChunk->m_chunkSize = size;
                tmpHeaderChunk->m_primitiveChunks.push_back(tmpPremitiveChunk);
            }

            if(equalsRiffTag(chkHeader,allHeaderFlags,(int)(sizeof(allHeaderFlags)/5)))
            {
                tmpHeaderChunk->m_headerLocation = io_->tell() - 16;
                tmpHeaderChunk->m_headerSize = listsize ;
                position = RiffVideo::TraversingChunk;
            }
            //to handle AVI file formats
            if(!m_decodeMetaData && equalsRiffTag(chkId,allPrimitiveFlags,(int)(sizeof(allPrimitiveFlags)/5)))
            {
                io_->seek(size,BasicIo::cur);
                m_riffFileSkeleton.m_headerChunks.push_back(tmpHeaderChunk);
            }

            else if(!m_decodeMetaData && equalsRiffTag(chkHeader,allHeaderFlags,(int)(sizeof(allHeaderFlags)/5)))
            {
                io_->seek((listsize - 12),BasicIo::cur);
                m_riffFileSkeleton.m_headerChunks.push_back(tmpHeaderChunk);
            }

            else if(equalsRiffTag(chkId, "JUNK"))
            {
                junkHandler(size);
            }
            //primitive chunks note:compare chkId
            else if(equalsRiffTag(chkId, "AVIH"))
            {
                aviHeaderTagsHandler(size);
            }
            else if(equalsRiffTag(chkId, "STRH"))
            {
                //since strh and strf exist sequential stream type can be set once
                setStreamType();
                streamHandler(size);
            }
            else if(equalsRiffTag(chkId,"STRF") || equalsRiffTag(chkId, "FMT "))
            {
                if(equalsRiffTag(chkId,"FMT "))
                    streamType_ = Audio;
                streamFormatHandler(size);
            }
            else if(equalsRiffTag(chkId, "STRN"))
            {
                dateTimeOriginal(size, 1);
            }
            else if(equalsRiffTag(chkId, "STRD"))
            {
                streamDataTagHandler(size);
            }
            //add more else if to support more primitive chunks below if any
            //TODO:add indx support

            //recursive call to decode LIST by breaking out of the loop
            else if(equalsRiffTag(chkId, "LIST"))
            {
                position = RiffVideo::TraversingChunk;
                io_->seek(-8,BasicIo::cur);
            }

            //header chunks note:compare chkHeader
            else if(equalsRiffTag(chkHeader, "INFO"))
            {
                io_->seek(-16,BasicIo::cur);
                infoTagsHandler();
            }
            else if(equalsRiffTag(chkHeader, "NCDT"))
            {
                io_->seek(-16,BasicIo::cur);
                nikonTagsHandler();
            }
            else if(equalsRiffTag(chkHeader, "ODML"))
            {
                io_->seek(-16,BasicIo::cur);
                odmlTagsHandler();
            }
            //only way to exit from readMetadata()
            else if(equalsRiffTag(chkHeader, "MOVI") || equalsRiffTag(chkMainId, "DATA"))
            {
                return;
            }
            //Add read functionality for custom chunk headers or user created tags here with more (else if) before else.

            //skip block with unknown chunk ID (primitive chunk)
            else if(io_->tell() < listend)
            {
                position = RiffVideo::TraversingChunk;
                io_->seek((listend -  (io_->tell()+4)) ,BasicIo::cur);
            }

            //skip chunk with unknown headerId
            else
            {
                io_->seek(io_->tell()+size, BasicIo::cur);
            }

            if(equalsRiffTag(chkId,allPrimitiveFlags,(int)(sizeof(allPrimitiveFlags)/5)) && (size % 2 != 0))
            {
                io_->seek(1,BasicIo::cur);
            }

            if(equalsRiffTag(chkId,allHeaderFlags,(int)(sizeof(allHeaderFlags)/5)) && (listsize % 2 != 0))
            {
                io_->seek(1,BasicIo::cur);
            }
        }
    }

    //AVIX can have JUNK chunk directly inside RIFF chunk
    else if(equalsRiffTag(chkMainId, "JUNK"))
    {
        DataBuf junkSize((const long)5);
        junkSize.pData_[4] = '\0';
        io_->read(junkSize.pData_, 4);
        unsigned long size = Exiv2::getULong(junkSize.pData_, littleEndian);
        if(!m_decodeMetaData)
        {
            ListChunk *tmpList = new ListChunk();
            tmpList->m_listLocation = io_->tell() - 8;
            tmpList->m_listSize = size;
            m_riffFileSkeleton.m_lists.push_back(tmpList);
        }
        else
        {
            junkHandler(size);
        }
    }
    else if(equalsRiffTag(chkMainId, "IDIT"))
    {
        DataBuf dataSize((const long)5);
        dataSize.pData_[4] = '\0';
        io_->read(dataSize.pData_, 4);
        unsigned long size = Exiv2::getULong(dataSize.pData_, littleEndian);
        if(!m_decodeMetaData)
        {
            HeaderChunk *tmpHeaderChunk = new HeaderChunk();
            tmpHeaderChunk->m_headerLocation = io_->tell() - 12;
            tmpHeaderChunk->m_headerSize = size;
            memcpy((byte *)tmpHeaderChunk->m_headerId,(const byte*)chkMainId.pData_,5);
            m_riffFileSkeleton.m_headerChunks.push_back(tmpHeaderChunk);
        }
        else
        {
            dateTimeOriginal(size);
        }
    }

    tagDecoder();
} // RiffVideo::tagDecoder

void RiffVideo::streamDataTagHandler(long size)
{
    const long bufMinSize = 20000;
    DataBuf buf(bufMinSize);
    buf.pData_[4] = '\0';
    uint64_t cur_pos = io_->tell();

    io_->read(buf.pData_, 8);

    if(equalsRiffTag(buf, "AVIF"))
    {

        if (size - 4 < 0)
        {
#ifndef SUPPRESS_WARNINGS
            EXV_ERROR   << " Exif Tags found in this RIFF file are not of valid size ."
                        << " Entries considered invalid. Not Processed.\n";
#endif
        }
        else
        {
            io_->read(buf.pData_, size - 4);

            IptcData iptcData;
            XmpData  xmpData;
            DummyTiffHeader tiffHeader(littleEndian);
            TiffParserWorker::decode(exifData_,
                                     iptcData,
                                     xmpData,
                                     buf.pData_,
                                     buf.size_,
                                     Tag::root,
                                     TiffMapping::findDecoder,
                                     &tiffHeader);

#ifndef SUPPRESS_WARNINGS
            if (!iptcData.empty())
            {
                EXV_WARNING << "Ignoring IPTC information encoded in the Exif data.\n";
            }
            if (!xmpData.empty())
            {
                EXV_WARNING << "Ignoring XMP information encoded in the Exif data.\n";
            }
#endif
        }
    }
    // TODO decode CasioData and ZORA Tag
    io_->seek(cur_pos + size, BasicIo::beg);

} // RiffVideo::streamDataTagHandler

void RiffVideo::dateTimeOriginal(long size, int i)
{
    if(!m_modifyMetadata)
    {
        uint64_t cur_pos = io_->tell();
        io_->seek(-4,BasicIo::cur);
        DataBuf dataLength(2);
        dataLength.pData_[1] = '\0';
        io_->read(dataLength.pData_,1);
        unsigned long bufMinSize = (unsigned long)Exiv2::getShort(dataLength.pData_, littleEndian);
        DataBuf buf((unsigned long)bufMinSize);
        io_->seek(3,BasicIo::cur);
        io_->read(buf.pData_, bufMinSize);
        if(!i)
            xmpData_["Xmp.video.DateUT"] = buf.pData_;
        else
            xmpData_["Xmp.video.StreamName"] = buf.pData_;
        io_->seek(cur_pos + size, BasicIo::beg);
    }
    else
    {
        if(xmpData_["Xmp.video.DateUT"].count() > 0)
        {
            DataBuf dataLength(2);
            dataLength.pData_[1] = '\0';
            io_->read(dataLength.pData_,1);
            io_->seek(3,BasicIo::cur);
            unsigned long bufMinSize = (unsigned long)Exiv2::getShort(dataLength.pData_, littleEndian);
            byte rawDateTimeOriginal[bufMinSize];
            const std::string dateAndTime = xmpData_["Xmp.video.DateUT"].toString();
            for(int i=0; i<(int)bufMinSize; i++)
            {
                rawDateTimeOriginal[i] = dateAndTime[i];
            }
            io_->write(rawDateTimeOriginal,bufMinSize);
        }

        if(xmpData_["Xmp.video.StreamName"].count() >0)
        {
            DataBuf dataLength(2);
            dataLength.pData_[1] = '\0';
            io_->read(dataLength.pData_,1);
            io_->seek(3,BasicIo::cur);
            unsigned long bufMinSize = (unsigned long)Exiv2::getShort(dataLength.pData_, littleEndian);
            byte rawStreamName[bufMinSize];
            const std::string streamName = xmpData_["Xmp.video.StreamName"].toString();
            for(int i=0; i<(int)bufMinSize; i++)
            {
                rawStreamName[i] = streamName[i];
            }
            io_->write(rawStreamName,4);
        }
    }
} // RiffVideo::dateTimeOriginal

void RiffVideo::odmlTagsHandler()
{
    const long bufMinSize = 4;
    DataBuf buf(bufMinSize + 1);
    buf.pData_[4] = '\0';
    io_->read(buf.pData_,4);

    if(!m_modifyMetadata)
    {
        unsigned long size = Exiv2::getULong(buf.pData_, littleEndian);
        unsigned long size2 = size;

        uint64_t cur_pos = io_->tell();
        io_->read(buf.pData_, 4); size -= 4;

        while(size > 0)
        {
            io_->read(buf.pData_, 4); size -= 4;
            if(equalsRiffTag(buf,"DMLH"))
            {
                io_->read(buf.pData_, 4); size -= 4;
                io_->read(buf.pData_, 4); size -= 4;
                xmpData_["Xmp.video.TotalFrameCount"] = Exiv2::getULong(buf.pData_, littleEndian);
            }
        }
        io_->seek(cur_pos + size2, BasicIo::beg);
    }
    else
    {
        if(xmpData_["Xmp.video.TotalFrameCount"].count() > 0)
        {
            unsigned long tmpSize = Exiv2::getULong(buf.pData_, littleEndian);
            while(tmpSize > 0)
            {
                io_->read(buf.pData_,4); tmpSize -= 4;
                if(equalsRiffTag(buf,"DMLH"))
                {
                    io_->seek(4,BasicIo::cur);
                    byte rawFrameCount[4];
                    long frameCnt =  xmpData_["Xmp.video.TotalFrameCount"].toLong();
                    memcpy(rawFrameCount,&frameCnt,4);
                    io_->write(rawFrameCount,4);
                    return;
                }
            }
        }
    }
} // RiffVideo::odmlTagsHandler

void RiffVideo::skipListData()
{
    const long bufMinSize = 4;
    DataBuf buf(bufMinSize+1);
    buf.pData_[4] = '\0';
    io_->seek(-12, BasicIo::cur);
    io_->read(buf.pData_, 4);
    unsigned long size = Exiv2::getULong(buf.pData_, littleEndian);

    uint64_t cur_pos = io_->tell();
    io_->seek(cur_pos + size, BasicIo::beg);
} // RiffVideo::skipListData

void RiffVideo::nikonTagsHandler()
{
    const long bufMinSize = 100;
    DataBuf buf(bufMinSize), buf2(4+1);
    buf.pData_[4] = '\0';
    io_->read(buf.pData_, 4);

    long internal_size = 0, tagID = 0, dataSize = 0, tempSize, size = Exiv2::getULong(buf.pData_, littleEndian);
    tempSize = size; char str[9] = " . . . ";
    uint64_t internal_pos, cur_pos; internal_pos = cur_pos = io_->tell();
    const TagDetails* td;
    double denominator = 1;
    io_->read(buf.pData_, 4); tempSize -= 4;

    while((long)tempSize > 0)
    {
        std::memset(buf.pData_, 0x0, buf.size_);
        io_->read(buf.pData_, 4);
        io_->read(buf2.pData_, 4);
        int temp = internal_size = Exiv2::getULong(buf2.pData_, littleEndian);
        internal_pos = io_->tell(); tempSize -= (internal_size + 8);

        if(equalsRiffTag(buf, "NCVR"))
        {
            while((long)temp > 3)
            {
                std::memset(buf.pData_, 0x0, buf.size_);
                io_->read(buf.pData_, 2);
                tagID = Exiv2::getULong(buf.pData_, littleEndian);
                io_->read(buf.pData_, 2);
                dataSize = Exiv2::getULong(buf.pData_, littleEndian);
                temp -= (4 + dataSize);

                if(tagID == 0x0001)
                {
                    if (dataSize <= 0)
                    {
#ifndef SUPPRESS_WARNINGS
                        EXV_ERROR   << " Makernotes found in this RIFF file are not of valid size ."
                                    << " Entries considered invalid. Not Processed.\n";
#endif
                    }
                    else if(!m_modifyMetadata)
                    {
                        io_->read(buf.pData_, dataSize);
                        xmpData_["Xmp.video.MakerNoteType"] = buf.pData_;
                    }
                    else if(xmpData_["Xmp.video.MakerNoteType"].count() > 0)
                    {
                        byte rawMakerNoteType[dataSize];
                        std::string makerNoteType = xmpData_["Xmp.video.MakerNoteType"].toString();
                        for(int i=0; i<dataSize; i++)
                        {
                            rawMakerNoteType[i] = makerNoteType[i];
                        }
                        io_->write(rawMakerNoteType,dataSize);
                    }
                    else
                    {
                        io_->seek(dataSize,BasicIo::cur);
                    }
                }
                else if (tagID == 0x0002)
                {
                    if (dataSize <= 0)
                    {
                    }
                    else if(!m_modifyMetadata)
                    {
                        while(dataSize)
                        {
                            std::memset(buf.pData_, 0x0, buf.size_); io_->read(buf.pData_, 1);
                            str[(4 - dataSize) * 2] = (char)(Exiv2::getULong(buf.pData_, littleEndian) + 48);
                            --dataSize;
                        }
                        xmpData_["Xmp.video.MakerNoteVersion"] = str;
                    }
                    else if(xmpData_["Xmp.video.MakerNoteVersion"].count() > 0)
                    {
                        long originalDataSize = dataSize;
                        std::string makerNoteVersion = xmpData_["Xmp.video.MakerNoteVersion"].toString();
                        byte rawMakerNoteVersion[originalDataSize];
                        int tmpCounter = 0;
                        while(dataSize)
                        {
                            rawMakerNoteVersion[tmpCounter] = (byte) (makerNoteVersion[(4 - dataSize) * 2] - 48);
                            --dataSize;tmpCounter++;
                        }
                        io_->write(rawMakerNoteVersion,originalDataSize);
                    }
                    else
                    {
                        io_->seek(dataSize,BasicIo::cur);
                    }
                }
            }
        }
        else if(equalsRiffTag(buf, "NCTG"))
        {
            while((long)temp > 3)
            {
                std::memset(buf.pData_, 0x0, buf.size_);
                io_->read(buf.pData_, 2);
                tagID = Exiv2::getULong(buf.pData_, littleEndian);
                io_->read(buf.pData_, 2);
                dataSize = Exiv2::getULong(buf.pData_, littleEndian);
                temp -= (4 + dataSize);
                td = find(nikonAVITags , tagID);

                if (dataSize <= 0)
                {
                }
                else
                {
                    switch (tagID)
                    {
                    case 0x0003: case 0x0004: case 0x0005: case 0x0006:
                    case 0x0013: case 0x0014: case 0x0018: case 0x001d:
                    case 0x001e: case 0x001f: case 0x0020:
                        if(!m_modifyMetadata)
                        {
                            io_->read(buf.pData_, dataSize);
                            xmpData_[exvGettext(td->label_)] = buf.pData_;
                        }
                        else if(xmpData_[exvGettext(td->label_)].count() > 0)
                        {
                            byte rawnikonData[dataSize];
                            std::string nikonData = xmpData_[exvGettext(td->label_)].toString();
                            for(int i=0; i<dataSize; i++)
                            {
                                rawnikonData[i] = nikonData[i];
                            }
                            io_->write(rawnikonData,dataSize);
                        }
                        else
                        {
                            io_->seek(dataSize,BasicIo::cur);
                        }
                        break;
                    case 0x0007: case 0x0010: case 0x0011: case 0x000c:
                    case 0x0012:
                        if(!m_modifyMetadata)
                        {
                            io_->read(buf.pData_, dataSize);
                            xmpData_[exvGettext(td->label_)] = Exiv2::getULong(buf.pData_, littleEndian);
                        }
                        else if(xmpData_[exvGettext(td->label_)].count() > 0)
                        {
                            byte rawnikonData[dataSize];
                            const unsigned long nikonData = xmpData_[exvGettext(td->label_)].toLong();
                            memcpy(rawnikonData,&nikonData,dataSize);
                            io_->write(rawnikonData,dataSize);
                        }
                        else
                        {
                            io_->seek(dataSize,BasicIo::cur);
                        }
                        break;
                    case 0x0008: case 0x0009: case 0x000a: case 0x000b:
                    case 0x000f: case 0x001b: case 0x0016:
                        buf2.pData_[0] = buf.pData_[4]; buf2.pData_[1] = buf.pData_[5];
                        buf2.pData_[2] = buf.pData_[6]; buf2.pData_[3] = buf.pData_[7];
                        denominator = (double)Exiv2::getLong(buf2.pData_, littleEndian);

                        if(!m_modifyMetadata)
                        {
                            io_->read(buf.pData_, dataSize);
                            if (denominator != 0)
                                xmpData_[exvGettext(td->label_)] = (double)Exiv2::getLong(buf.pData_, littleEndian) / denominator;
                            else
                                xmpData_[exvGettext(td->label_)] = 0;
                        }
                        else if(xmpData_[exvGettext(td->label_)].count() > 0)
                        {
                            byte rawnikonData[dataSize];
                            const long nikonData = (long)((double)xmpData_[exvGettext(td->label_)].toLong() * (double)denominator);
                            memcpy(rawnikonData,&nikonData,dataSize);
                            io_->write(rawnikonData,dataSize);
                        }
                        else
                        {
                            io_->seek(dataSize,BasicIo::cur);
                        }
                        break;

                    default:
                        io_->read(buf.pData_, dataSize);
                        break;
                    }
                }
            }
        }

        else if(equalsRiffTag(buf, "NCTH"))
        {//TODO Nikon Thumbnail Image
        }

        else if(equalsRiffTag(buf, "NCVW"))
        {//TODO Nikon Preview Image
        }

        io_->seek(internal_pos + internal_size, BasicIo::beg);
    }

    if (size ==0)
    {
        io_->seek(cur_pos + 4, BasicIo::beg);
    }
    else
    {
        io_->seek(cur_pos + size, BasicIo::beg);
    }
} // RiffVideo::nikonTagsHandler

void RiffVideo::infoTagsHandler()
{
    const long bufMinSize = 4;
    DataBuf buf(bufMinSize + 1);
    buf.pData_[4] = '\0';
    io_->read(buf.pData_, 4);

    long infoSize, size = Exiv2::getULong(buf.pData_, littleEndian);
    long size_external = size;
    const TagVocabulary* tv;

    if(!m_modifyMetadata)
    {
        uint64_t cur_pos = io_->tell();
        io_->read(buf.pData_, 4); size -= 4;

        while(size > 3)
        {
            io_->read(buf.pData_, 4); size -= 4;
            if(!Exiv2::getULong(buf.pData_, littleEndian))
                break;
            tv = find(infoTags , Exiv2::toString( buf.pData_));
            io_->read(buf.pData_, 4); size -= 4;
            infoSize = Exiv2::getULong(buf.pData_, littleEndian);

            if(infoSize >= 0)
            {
                DataBuf tmpBuf(infoSize +1);
                tmpBuf.pData_[infoSize] = '\0';
                size -= infoSize;
                io_->read(tmpBuf.pData_, infoSize);
                if(tv)
                {
                    xmpData_[exvGettext(tv->label_)] = tmpBuf.pData_;
                }
            }
        }
        io_->seek(cur_pos + size_external, BasicIo::beg);
    }
    else
    {
        io_->read(buf.pData_, 4); size -= 4;
        while(size > 3)
        {
            io_->read(buf.pData_, 4); size -= 4;
            if(!Exiv2::getULong(buf.pData_, littleEndian))
                break;
            tv = find(infoTags , Exiv2::toString( buf.pData_));
            io_->read(buf.pData_, 4); size -= 4;
            infoSize = Exiv2::getULong(buf.pData_, littleEndian);

            if(infoSize >= 0)
            {
                DataBuf tmpBuf(infoSize +1);
                tmpBuf.pData_[infoSize] = '\0';
                size -= infoSize;

                if(tv)
                {
                    if(xmpData_[exvGettext(tv->label_)].count() >0)
                    {
                        byte rawInfoData[infoSize];
                        const std::string infoData = xmpData_[exvGettext(tv->label_)].toString();
                        for(int i=0; i<infoSize ;i++)
                        {
                            rawInfoData[i] = infoData[i];
                        }
                        io_->write(rawInfoData,infoSize);
                    }
                }
            }
        }
    }
} // RiffVideo::infoTagsHandler

void RiffVideo::junkHandler(long size)
{
    const long bufMinSize = size;

    if (size < 0)
    {
#ifndef SUPPRESS_WARNINGS
        EXV_ERROR   << " Junk Data found in this RIFF file are not of valid size ."
                    << " Entries considered invalid. Not Processed.\n";
#endif
        io_->seek(io_->tell() + 4, BasicIo::beg);
    }

    else
    {
        DataBuf buf(bufMinSize+1), buf2(4+1);
        std::memset(buf.pData_, 0x0, buf.size_);
        buf2.pData_[4] = '\0';
        uint64_t cur_pos = io_->tell();

        io_->read(buf.pData_, 4);
        //! Pentax Metadata and Tags
        if(equalsRiffTag(buf, "PENT"))
        {

            io_->seek(cur_pos + 18, BasicIo::beg);
            io_->read(buf.pData_, 26);
            xmpData_["Xmp.video.Make"] = buf.pData_;

            io_->read(buf.pData_, 50);
            xmpData_["Xmp.video.Model"] = buf.pData_;

            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, 8);
            buf2.pData_[0] = buf.pData_[4]; buf2.pData_[1] = buf.pData_[5];
            buf2.pData_[2] = buf.pData_[6]; buf2.pData_[3] = buf.pData_[7];
            xmpData_["Xmp.video.FNumber"] = (double)Exiv2::getLong(buf.pData_, littleEndian) / (double)Exiv2::getLong(buf2.pData_, littleEndian);;

            io_->seek(cur_pos + 131, BasicIo::beg);
            io_->read(buf.pData_, 26);
            xmpData_["Xmp.video.DateTimeOriginal"] = buf.pData_;

            io_->read(buf.pData_, 26);
            xmpData_["Xmp.video.DateTimeDigitized"] = buf.pData_;

            io_->seek(cur_pos + 299, BasicIo::beg);
            std::memset(buf.pData_, 0x0, buf.size_);

            io_->read(buf.pData_, 2);
            Exiv2::XmpTextValue tv(Exiv2::toString(Exiv2::getLong(buf.pData_, littleEndian)));
            xmpData_.add(Exiv2::XmpKey("Xmp.xmp.Thumbnails/xmpGImg:width"), &tv);

            io_->read(buf.pData_, 2);
            tv.read(Exiv2::toString(Exiv2::getLong(buf.pData_, littleEndian)));
            xmpData_.add(Exiv2::XmpKey("Xmp.xmp.Thumbnails/xmpGImg:height"), &tv);

            io_->read(buf.pData_, 4);

            /* TODO - Storing the image Thumbnail in Base64 Format


            uint64_t length = Exiv2::getLong(buf.pData_, littleEndian);
            io_->read(buf.pData_, length);

            char *rawStr = Exiv2::toString(buf.pData_);
            char *encodedStr;

            SXMPUtils::EncodeToBase64(rawStr, encodedStr);

            tv.read(Exiv2::toString(encodedStr));
            xmpData_.add(Exiv2::XmpKey("Xmp.xmp.Thumbnails/xmpGImg:image"), &tv);
        */
        }
        else
        {
            io_->seek(cur_pos, BasicIo::beg);
            io_->read(buf.pData_, size);
            xmpData_["Xmp.video.Junk"] = buf.pData_;
        }

        io_->seek(cur_pos + size, BasicIo::beg);
    }
} // RiffVideo::junkHandler

void RiffVideo::aviHeaderTagsHandler(long size)
{
    if(!m_modifyMetadata)
    {
        const long bufMinSize = 4;
        DataBuf buf(bufMinSize+1);
        buf.pData_[4] = '\0';
        long width = 0, height = 0, frame_count = 0;
        double frame_rate = 1;

        uint64_t cur_pos = io_->tell();

        for(int i = 0; i <= 9; i++) {
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, bufMinSize);

            switch(i)
            {
            case frameRate:
                xmpData_["Xmp.video.MicroSecPerFrame"] = Exiv2::getULong(buf.pData_, littleEndian);
                frame_rate = (double)1000000/(double)Exiv2::getULong(buf.pData_, littleEndian);
                break;
            case (maxDataRate):
                xmpData_["Xmp.video.MaxDataRate"] = (double)Exiv2::getULong(buf.pData_, littleEndian)/(double)1024;
                break;
            case frameCount:
                xmpData_["Xmp.video.FrameCount"] = Exiv2::getULong(buf.pData_, littleEndian);
                frame_count = Exiv2::getULong(buf.pData_, littleEndian);
                break;
            case initialFrames:
                xmpData_["Xmp.video.InitialFrames"] = Exiv2::getULong(buf.pData_, littleEndian);
                break;
            case streamCount:
                xmpData_["Xmp.video.StreamCount"] = Exiv2::getULong(buf.pData_, littleEndian);
                break;
            case suggestedBufferSize:
                xmpData_["Xmp.video.SuggestedBufferSize"] = Exiv2::getULong(buf.pData_, littleEndian);
                break;
            case imageWidth_h:
                width = Exiv2::getULong(buf.pData_, littleEndian);
                xmpData_["Xmp.video.Width"] = width;
                break;
            case imageHeight_h:
                height = Exiv2::getULong(buf.pData_, littleEndian);
                xmpData_["Xmp.video.Height"] = height;
                break;
            }
        }

        fillAspectRatio(width, height);
        fillDuration(frame_rate, frame_count);

        io_->seek(cur_pos + size, BasicIo::beg);
    }
    else//overwrite metadata from current xmpData_ container
    {
        if(xmpData_["Xmp.video.MicroSecPerFrame"].count() > 0 )
        {
            byte rawMicroSecondsperFrame[4];
            long microSecondsperFrame = xmpData_["Xmp.video.MicroSecPerFrame"].toLong();
            memcpy(rawMicroSecondsperFrame,&microSecondsperFrame,4);
            io_->write(rawMicroSecondsperFrame,4);
        }
        else
        {
            io_->seek(4,BasicIo::cur);
        }

        if(xmpData_["Xmp.video.MaxDataRate"].count() > 0 )
        {
            byte rawMaxDatarate[4];
            long maxDataRate = xmpData_["Xmp.video.MaxDataRate"].toLong()*1024;
            memcpy(rawMaxDatarate,&maxDataRate,4);
            io_->write(rawMaxDatarate,4);
            io_->seek(8,BasicIo::cur);
        }
        else
        {
            io_->seek(12,BasicIo::cur);
        }

        if(xmpData_["Xmp.video.FrameCount"].count() > 0)
        {
            byte rawFrameCount[4];
            long frameCount = xmpData_["Xmp.video.FrameCount"].toLong();
            memcpy(rawFrameCount,&frameCount,4);
            io_->write(rawFrameCount,4);
        }
        else
        {
            io_->seek(4,BasicIo::cur);
        }

        if(xmpData_["Xmp.video.InitialFrames"].count() > 0)
        {
            byte rawInitialFrames[4];
            long initFrames = xmpData_["Xmp.video.InitialFrames"].toLong();
            memcpy(rawInitialFrames,&initFrames,4);
            io_->write(rawInitialFrames,4);
        }
        else
        {
            io_->seek(4,BasicIo::cur);
        }

        if(xmpData_["Xmp.video.StreamCount"].count() > 0)
        {
            byte rawStreamCount[4];
            long streamCount = xmpData_["Xmp.video.StreamCount"].toLong();
            memcpy(rawStreamCount,&streamCount,4);
            io_->write(rawStreamCount,4);
        }
        else
        {
            io_->seek(4,BasicIo::cur);
        }

        if(xmpData_["Xmp.video.SuggestedBufferSize"].count() >0)
        {
            byte rawSuggestedBufferSize[4];
            long sugBufferSize = xmpData_["Xmp.video.SuggestedBufferSize"].toLong();
            memcpy(rawSuggestedBufferSize,&sugBufferSize,4);
            io_->write(rawSuggestedBufferSize,4);
        }
        else
        {
            io_->seek(4,BasicIo::cur);
        }

        if(xmpData_["Xmp.video.Width"].count() > 0)
        {
            byte rawImageWidth[4];
            long imageWidth = xmpData_["Xmp.video.Width"].toLong();
            memcpy(rawImageWidth,&imageWidth,4);
            io_->write(rawImageWidth,4);
        }
        else
        {
            io_->seek(4,BasicIo::cur);
        }

        if(xmpData_["Xmp.video.Height"].count() > 0)
        {

            byte rawImageHeight[4];
            long imageHeight = xmpData_["Xmp.video.Height"].toLong();
            memcpy(rawImageHeight,&imageHeight,4);
            io_->write(rawImageHeight,4);
        }
    }
} // RiffVideo::aviHeaderTagsHandler

void RiffVideo::setStreamType()
{
    DataBuf chkId((unsigned long)5);
    io_->read(chkId.pData_,(unsigned long)4);
    if(equalsRiffTag(chkId, "VIDS"))
        streamType_ = Video;
    else if (equalsRiffTag(chkId, "AUDS"))
        streamType_ = Audio;
}
void RiffVideo::streamHandler(long size)
{
    if (!m_modifyMetadata)
    {
        unsigned long bufMinSize = 4;
        DataBuf buf(bufMinSize+1);
        buf.pData_[4] =  '\0';
        long divisor = 1;
        uint64_t cur_pos = io_->tell() - 4;

        for(int i=1; i<=25; i++)
        {
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, bufMinSize);

            switch(i)
            {
            case codec:
                if(streamType_ == Video)
                    xmpData_["Xmp.video.Codec"] = buf.pData_;
                else if (streamType_ == Audio)
                    xmpData_["Xmp.audio.Codec"] = buf.pData_;
                else
                    xmpData_["Xmp.video.Codec"] = buf.pData_;
                break;
            case sampleRate:
                divisor=Exiv2::getULong(buf.pData_, littleEndian);
                break;
            case (sampleRate+1):
                if(streamType_ == Video)
                    xmpData_["Xmp.video.FrameRate"] = returnSampleRate(buf,divisor);
                else if (streamType_ == Audio)
                    xmpData_["Xmp.audio.SampleRate"] = returnSampleRate(buf,divisor);
                else
                    xmpData_["Xmp.video.StreamSampleRate"] = returnSampleRate(buf,divisor);
                break;
            case sampleCount:
                if(streamType_ == Video)
                    xmpData_["Xmp.video.FrameCount"] = Exiv2::getULong(buf.pData_, littleEndian);
                else if (streamType_ == Audio)
                    xmpData_["Xmp.audio.SampleCount"] = Exiv2::getULong(buf.pData_, littleEndian);
                else
                    xmpData_["Xmp.video.StreamSampleCount"] = Exiv2::getULong(buf.pData_, littleEndian);
                break;
            case quality:
                if(streamType_ == Video)
                    xmpData_["Xmp.video.VideoQuality"] = Exiv2::getULong(buf.pData_, littleEndian);
                else if(streamType_ != Audio)
                    xmpData_["Xmp.video.StreamQuality"] = Exiv2::getULong(buf.pData_, littleEndian);
                break;
            case sampleSize:
                if(streamType_ == Video)
                    xmpData_["Xmp.video.VideoSampleSize"] = Exiv2::getULong(buf.pData_, littleEndian);
                else if(streamType_ != Audio)
                    xmpData_["Xmp.video.StreamSampleSize"] = Exiv2::getULong(buf.pData_, littleEndian);
                break;
            }

        }
        io_->seek(cur_pos + size, BasicIo::beg);
    }

    else
    {
        DataBuf chkId((unsigned long)5);
        chkId.pData_[4] = '\0';
        const long bufMinSize = 4;

        long multiplier;
        if(streamType_ == Video)
        {
            if(xmpData_["Xmp.video.Codec"].count() > 0)
            {
                byte rawVideoCodec[4];
                const std::string codec = xmpData_["Xmp.video.Codec"].toString();
                for(int i=0;i<4;i++)
                {
                    rawVideoCodec[i] = (byte)codec[i];
                }
                io_->write(rawVideoCodec,4);
                io_->seek(12,BasicIo::cur);
            }
            else
            {
                io_->seek(16,BasicIo::cur);
            }

            io_->read(chkId.pData_,bufMinSize);
            multiplier = Exiv2::getULong(chkId.pData_, littleEndian);

            if(xmpData_["Xmp.video.FrameRate"].count() > 0)
            {
                byte rawFrameRate[4];
                long frameRate = (long)xmpData_["Xmp.video.FrameRate"].toLong()*multiplier;
                memcpy(rawFrameRate,&frameRate,4);
                io_->write(rawFrameRate,4);
                io_->seek(4,BasicIo::cur);
            }
            else
            {
                io_->seek(8,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.FrameCount"].count() > 0)
            {
                byte rawFrameCount[4];
                long frameCount = xmpData_["Xmp.video.FrameCount"].toLong();
                memcpy(rawFrameCount,&frameCount,4);
                io_->write(rawFrameCount,4);
                io_->seek(4,BasicIo::cur);
            }
            else
            {
                io_->seek(8,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.VideoQuality"].count() > 0)
            {
                byte rawVideoQuality[4];
                long quality = xmpData_["Xmp.video.VideoQuality"].toLong();
                memcpy(rawVideoQuality,&quality,4);
                io_->write(rawVideoQuality,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.VideoSampleSize"].count() > 0)
            {
                byte rawVideoSampleSize[4];
                long sampleSize = xmpData_["Xmp.video.VideoSampleSize"].toLong();
                memcpy(rawVideoSampleSize,&sampleSize,4);
                io_->write(rawVideoSampleSize,4);
            }
        }
        else if(streamType_ == Audio)
        {
            if(xmpData_["Xmp.audio.Codec"].count() > 0)
            {
                byte rawAudioCodec[4];
                const std::string audioCodec = xmpData_["Xmp.audio.Codec"].toString();
                for(int i=0;i<4;i++)
                {
                    rawAudioCodec[i] = (byte)audioCodec[i];
                }
                io_->write(rawAudioCodec,4);
                io_->seek(12,BasicIo::cur);
            }
            else
            {
                io_->seek(16,BasicIo::cur);
            }

            io_->read(chkId.pData_,bufMinSize);
            multiplier = Exiv2::getULong(chkId.pData_, littleEndian);

            if(xmpData_["Xmp.audio.SampleRate"].count() > 0)
            {
                byte rawSampleRate[4];
                long sampleRate = (long)xmpData_["Xmp.audio.SampleRate"].toLong()*multiplier;
                memcpy(rawSampleRate,&sampleRate,4);
                io_->write(rawSampleRate,4);
                io_->seek(4,BasicIo::cur);
            }
            else
            {
                io_->seek(8,BasicIo::cur);
            }

            if(xmpData_["Xmp.audio.SampleCount"].count() > 0)
            {
                byte rawSampleCount[4];
                long sampleCount = xmpData_["Xmp.audio.SampleCount"].toLong();
                memcpy(rawSampleCount,&sampleCount,4);
                io_->write(rawSampleCount,4);
            }
        }
        else
        {
            if(xmpData_["Xmp.video.Codec"].count() > 0)
            {
                byte rawVideoCodec[4];
                long codec = xmpData_["Xmp.video.Codec"].toLong();
                memcpy(rawVideoCodec,&codec,4);
                io_->write(rawVideoCodec,4);
                io_->seek(12,BasicIo::cur);
            }
            else
            {
                io_->seek(16,BasicIo::cur);
            }

            io_->read(chkId.pData_,bufMinSize);
            multiplier = Exiv2::getULong(chkId.pData_, littleEndian);

            if(xmpData_["Xmp.video.StreamSampleRate"].count() > 0)
            {
                byte rawStreamSampleRate[4];
                long streamSampleRate = (long)xmpData_["Xmp.video.StreamSampleRate"].toLong()*multiplier;
                memcpy(rawStreamSampleRate,&streamSampleRate,4);
                io_->write(rawStreamSampleRate,4);
                io_->seek(4,BasicIo::cur);
            }
            else
            {
                io_->seek(8,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.StreamSampleCount"].count() > 0)
            {
                byte rawStreamSampleCount[4];
                long streamSampleCount = xmpData_["Xmp.video.StreamSampleCount"].toLong();
                memcpy(rawStreamSampleCount,&streamSampleCount,4);
                io_->write(rawStreamSampleCount,4);
                io_->seek(4,BasicIo::cur);
            }
            else
            {
                io_->seek(8,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.StreamQuality"].count() > 0)
            {
                byte rawStreamQuality[4];
                long streamQuality = xmpData_["Xmp.video.StreamQuality"].toLong();
                memcpy(rawStreamQuality,&streamQuality,4);
                io_->write(rawStreamQuality,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.StreamSampleSize"].count() > 0)
            {
                byte rawSampleSize[4];
                long streamSampleSize = xmpData_["Xmp.video.StreamSampleSize"].toLong();
                memcpy(rawSampleSize,&streamSampleSize,4);
                io_->write(rawSampleSize,4);
            }
        }
    }
} // RiffVideo::streamHandler

void RiffVideo::streamFormatHandler(long size)
{
    if (!m_modifyMetadata)
    {
        const long bufMinSize = 4;
        DataBuf buf(bufMinSize+1);
        buf.pData_[4] = '\0';
        uint64_t cur_pos = io_->tell();

        if(streamType_ == Video)
        {
            io_->read(buf.pData_, bufMinSize);

            for(int i = 0; i <= 9; i++)
            {
                std::memset(buf.pData_, 0x0, buf.size_);

                switch(i)
                {
                case imageWidth: //Will be used in case of debugging
                    io_->read(buf.pData_, bufMinSize); break;
                case imageHeight: //Will be used in case of debugging
                    io_->read(buf.pData_, bufMinSize); break;
                case planes:
                    io_->read(buf.pData_, 2);
                    xmpData_["Xmp.video.Planes"] = Exiv2::getUShort(buf.pData_, littleEndian); break;
                case bitDepth:
                    io_->read(buf.pData_, 2);
                    xmpData_["Xmp.video.PixelDepth"] = Exiv2::getUShort(buf.pData_, littleEndian); break;
                case compression:
                    io_->read(buf.pData_, bufMinSize);
                    xmpData_["Xmp.video.Compressor"] = buf.pData_; break;
                case imageLength:
                    io_->read(buf.pData_, bufMinSize);
                    xmpData_["Xmp.video.ImageLength"] = Exiv2::getULong(buf.pData_, littleEndian); break;
                case pixelsPerMeterX:
                    io_->read(buf.pData_, bufMinSize);
                    xmpData_["Xmp.video.PixelPerMeterX"] = Exiv2::getULong(buf.pData_, littleEndian); break;
                case pixelsPerMeterY:
                    io_->read(buf.pData_, bufMinSize);
                    xmpData_["Xmp.video.PixelPerMeterY"] = Exiv2::getULong(buf.pData_, littleEndian); break;
                case numColors:
                    io_->read(buf.pData_, bufMinSize);
                    if(Exiv2::getULong(buf.pData_, littleEndian) == 0)
                    {
                        xmpData_["Xmp.video.NumOfColours"] = "Unspecified";
                    }
                    else
                    {
                        xmpData_["Xmp.video.NumOfColours"] = Exiv2::getULong(buf.pData_, littleEndian);
                    }
                    break;
                case numImportantColors:
                    io_->read(buf.pData_, bufMinSize);
                    if(Exiv2::getULong(buf.pData_, littleEndian))
                    {
                        xmpData_["Xmp.video.NumIfImpColours"] = Exiv2::getULong(buf.pData_, littleEndian);
                    }
                    else
                    {
                        xmpData_["Xmp.video.NumOfImpColours"] = "All";
                    }
                    break;
                }
            }
        }
        else if(streamType_ == Audio)
        {
            int c = 0;
            const TagDetails* td;
            for(int i = 0; i <= 7; i++)
            {
                io_->read(buf.pData_, 2);

                switch(i)
                {
                case encoding:
                    td = find(audioEncodingValues , Exiv2::getUShort(buf.pData_, littleEndian));
                    if(td)
                    {
                        xmpData_["Xmp.audio.Compressor"] = exvGettext(td->label_);
                    }
                    else
                    {
                        xmpData_["Xmp.audio.Compressor"] = Exiv2::getUShort(buf.pData_, littleEndian);
                    }
                    break;
                case numberOfChannels:
                    c = Exiv2::getUShort(buf.pData_, littleEndian);
                    if(c == 1) xmpData_["Xmp.audio.ChannelType"] = "Mono";
                    else if(c == 2) xmpData_["Xmp.audio.ChannelType"] = "Stereo";
                    else if(c == 5) xmpData_["Xmp.audio.ChannelType"] = "5.1 Surround Sound";
                    else if(c == 7) xmpData_["Xmp.audio.ChannelType"] = "7.1 Surround Sound";
                    else xmpData_["Xmp.audio.ChannelType"] = "Mono";
                    break;
                case audioSampleRate:
                    xmpData_["Xmp.audio.SampleRate"] = Exiv2::getUShort(buf.pData_, littleEndian);
                    break;
                case avgBytesPerSec:
                    xmpData_["Xmp.audio.SampleType"] = Exiv2::getUShort(buf.pData_, littleEndian);
                    break;
                case bitsPerSample:
                    xmpData_["Xmp.audio.BitsPerSample"] = Exiv2::getUShort(buf.pData_,littleEndian);
                    io_->read(buf.pData_, 2);
                    break;
                }
            }
        }
        io_->seek(cur_pos + size, BasicIo::beg);
    }
    else
    {
        if(streamType_ == Video)
        {
            DataBuf chkId((unsigned long) 5);
            chkId.pData_[4] = '\0';
            io_->read(chkId.pData_,4);

            if(xmpData_["Xmp.video.Width"].count() > 0)
            {
                byte rawVideoWidth[4];
                const long width = xmpData_["Xmp.video.Width"].toLong();
                memcpy(rawVideoWidth,&width,4);
                io_->write(rawVideoWidth,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.Height"].count() > 0)
            {
                byte rawVideoHeight[4];
                const long height = xmpData_["Xmp.video.Height"].toLong();
                memcpy(rawVideoHeight,&height,4);
                io_->write(rawVideoHeight,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.Planes"].count() > 0)
            {
                byte rawVideoPlanes[2];
                const ushort videoPlanes= (ushort)xmpData_["Xmp.video.Planes"].toLong();
                memcpy(rawVideoPlanes,&videoPlanes,2);
                io_->write(rawVideoPlanes,2);
            }
            else
            {
                io_->seek(2,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.PixelDepth"].count() > 0)
            {
                byte rawVideoPixelDepth[2];
                const ushort pixelDepth = (ushort)xmpData_["Xmp.video.PixelDepth"].toLong();
                memcpy(rawVideoPixelDepth,&pixelDepth,2);
                io_->write(rawVideoPixelDepth,2);
            }
            else
            {
                io_->seek(2,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.Compressor"].count() > 0)
            {
                byte rawVideoCompressor[4];
                const std::string compressor = xmpData_["Xmp.video.Compressor"].toString();
                for(int i=0;i<4;i++)
                {
                    rawVideoCompressor[i] = (byte)compressor[i];
                }
                io_->write(rawVideoCompressor,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.ImageLength"].count() > 0)
            {
                byte rawVideoImageLenght[4];
                const long imageLength = xmpData_["Xmp.video.ImageLength"].toLong();
                memcpy(rawVideoImageLenght,&imageLength,4);
                io_->write(rawVideoImageLenght,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.PixelPerMeterX"].count() > 0)
            {
                byte rawVideoPixelPerMeterX[4];
                const long pixelPerMeterX = xmpData_["Xmp.video.PixelPerMeterX"].toLong();
                memcpy(rawVideoPixelPerMeterX,&pixelPerMeterX,4);
                io_->write(rawVideoPixelPerMeterX,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.PixelPerMeterY"].count() > 0)
            {
                byte rawVideoPixelPerMeterY[4];
                const long pixelPerMeterY = xmpData_["Xmp.video.PixelPerMeterY"].toLong();
                memcpy(rawVideoPixelPerMeterY,&pixelPerMeterY,4);
                io_->write(rawVideoPixelPerMeterY,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.NumOfColours"].count() > 0)
            {
                byte rawVideoNumOfColours[4];
                const long numOfColours = xmpData_["Xmp.video.NumOfColours"].toLong();
                memcpy(rawVideoNumOfColours,&numOfColours,4);
                io_->write(rawVideoNumOfColours,4);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }

            if(xmpData_["Xmp.video.NumOfImpColours"].count() > 0)
            {
                byte rawVideoNumIfImpColours[4];
                if(xmpData_["Xmp.video.NumOfImpColours"].toString() == "All")
                {
                    const long numImportantColours = (long) 0;
                    memcpy(rawVideoNumIfImpColours,&numImportantColours,4);
                    io_->write(rawVideoNumIfImpColours,4);
                }
                else
                {
                    const long numImportantColours = xmpData_["Xmp.video.NumOfImpColours"].toLong();
                    memcpy(rawVideoNumIfImpColours,&numImportantColours,4);
                    io_->write(rawVideoNumIfImpColours,4);
                }
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }
        }
        else if(streamType_ == Audio)
        {
            //TODO :audio compressor
            if(0)//xmpData_["Xmp.audio.Compressor"].count() > 0)
            {
                int sizeComp = xmpData_["Xmp.audio.Compressor"].size();
                DataBuf audCompressor(sizeComp);
                const std::string audioCompressor = xmpData_["Xmp.audio.Compressor"].toString();
                for(int i=0; i<=sizeComp;i++)
                {
                    audCompressor.pData_[i] = audioCompressor[i];
                }
            }
            else
            {
                io_->seek(2,BasicIo::cur);
            }

            if(xmpData_["Xmp.audio.ChannelType"].count() > 0)
            {
                const int sizeChan = xmpData_["Xmp.audio.ChannelType"].size();
                DataBuf tmpBuf(sizeChan+1);
                tmpBuf.pData_[sizeChan+1] = '\0';
                const std::string audioChannealType = xmpData_["Xmp.audio.ChannelType"].toString();
                for(int i=0; i<=sizeChan;i++)
                {
                    tmpBuf.pData_[i] = (byte)audioChannealType[i];
                }
                byte rawChannelType[2];
                ushort channelType;
                if(simpleBytesComparision(tmpBuf,"MONO",sizeChan))
                    channelType = 1;
                else if(simpleBytesComparision(tmpBuf,"STEREO",sizeChan))
                {
                    channelType = 2;
                }
                else if(simpleBytesComparision(tmpBuf,"5.1 SURROUND SOUND",sizeChan))
                    channelType = 5;
                else if(simpleBytesComparision(tmpBuf,"7.1 SURROUND SOUND",sizeChan))
                    channelType = 7;
                memcpy(rawChannelType,&channelType,2);
                io_->write(rawChannelType,2);
            }
            else
            {
                io_->seek(2,BasicIo::cur);
            }

            if(xmpData_["Xmp.audio.SampleRate"].count() > 0)
            {
                byte rawSampleRate[2];
                const ushort sampleRate = (ushort)xmpData_["Xmp.audio.SampleRate"].toLong();
                memcpy(rawSampleRate,&sampleRate,2);
                io_->write(rawSampleRate,2);
                io_->seek(2,BasicIo::cur);
            }
            else
            {
                io_->seek(4,BasicIo::cur);
            }
            if(xmpData_["Xmp.audio.SampleType"].count() > 0)
            {
                byte rawSampleType[2];
                const ushort sampleType = (ushort)xmpData_["Xmp.audio.SampleType"].toLong();
                memcpy(rawSampleType,&sampleType,2);
                io_->write(rawSampleType,2);
                io_->seek(4,BasicIo::cur);
            }
            else
            {
                io_->seek(6,BasicIo::cur);
            }
            if(xmpData_["Xmp.audio.BitsPerSample"].count() > 0)
            {
                byte rawBitsPerSample[2];
                const ushort bitsPerSample= (ushort)xmpData_["Xmp.audio.BitsPerSample"].toLong();
                memcpy(rawBitsPerSample,&bitsPerSample,2);
                io_->write(rawBitsPerSample,2);
            }
        }
    }
} // RiffVideo::streamFormatHandler

double RiffVideo::returnSampleRate(Exiv2::DataBuf& buf, long divisor)
{
    return ((double)Exiv2::getULong(buf.pData_, littleEndian) / (double)divisor);
} // RiffVideo::returnSampleRate

const char* RiffVideo::printAudioEncoding(uint64_t i)
{
    const TagDetails* td;
    td = find(audioEncodingValues , i);
    if(td)
        return exvGettext(td->label_);

    return "Undefined";
} // RiffVideo::printAudioEncoding

void RiffVideo::fillAspectRatio(long width, long height)
{
    double aspectRatio = (double)width / (double)height;
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
} // RiffVideo::fillAspectRatio

void RiffVideo::fillDuration(double frame_rate, long frame_count)
{
    if(frame_rate == 0)
        return;

    uint64_t duration = static_cast<uint64_t>((double)frame_count * (double)1000 / (double)frame_rate);
    xmpData_["Xmp.video.FileDataRate"] = (double)io_->size()/(double)(1048576*duration);
    xmpData_["Xmp.video.Duration"] = duration; //Duration in number of seconds
} // RiffVideo::fillDuration

Image::AutoPtr newRiffInstance(BasicIo::AutoPtr io, bool /*create*/)
{
    Image::AutoPtr image(new RiffVideo(io));
    if (!image->good()) {
        image.reset();
    }
    return image;
}

bool isRiffType(BasicIo& iIo, bool advance)
{
    const int32_t len = 2;
    const unsigned char RiffVideoId[4] = { 'R', 'I', 'F' ,'F'};
    byte buf[len];
    iIo.read(buf, len);
    if (iIo.error() || iIo.eof()) {
        return false;
    }
    bool matched = (memcmp(buf, RiffVideoId, len) == 0);
    if (!advance || !matched) {
        iIo.seek(-len, BasicIo::cur);
    }
    return matched;
}

}                                       // namespace Exiv2
