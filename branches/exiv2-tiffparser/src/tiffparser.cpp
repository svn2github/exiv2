// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006 Andreas Huggel <ahuggel@gmx.net>
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
  File:      tiffparser.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   15-Mar-06, ahu: created

 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "types.hpp"
#include "tiffparser.hpp"
#include "tiffcomposite.hpp"
#include "makernote2.hpp"
#include "tiffvisitor.hpp"
#include "tiffimage.hpp"
#include "error.hpp"

// + standard includes
#include <cassert>

/* --------------------------------------------------------------------------

   Todo:

   + Support all TIFF data types
   + Review boundary checking, is it better to check the offsets?
   + Define and implement consistent error handling for recursive hierarchy
   + Make TiffImage a template StandardImage, which can be parametrized with
     a parser and the necessary checking functions to cover all types of
     images which need to be loaded completely.
   + TiffComponent: should it have end() and setEnd() or pData and size?
   + Can NewTiffCompFct and TiffCompFactoryFct be combined?
   + Create function is repeated when actually only the table changes. Fix it.
   + Is it easier (for writing) to combine all creation tables into one?
   + CR2 Makernotes don't seem to have a next pointer but Canon Jpeg Makernotes
     do. What a mess. (That'll become an issue when it comes to writing to CR2)
   + Sony makernotes in RAW files do not seem to have header like those in Jpegs.
     And maybe no next pointer either.
   + Filtering of large unknown tags: Should be moved to writing/encoding code
     and done only if really needed (i.e., if writing to a Jpeg segment)
   + Make Tiff parser completely standalone, depending only on very low level
     stuff from exiv2

   in crwimage.* :

   + Fix CiffHeader according to TiffHeade2
   + Combine Error(15) and Error(33), add format argument %1
   + Search crwimage for todos, fix writeMetadata comment
   + Add Ciff components to TIFF component hierarchy
   + rename all Ciff stuff to Crw for easier reference - not needed when CIFF
     components are part of the TIFF hierarchy
   + rename loadStack to getPath for consistency

   -------------------------------------------------------------------------- */


// *****************************************************************************
// class member definitions
namespace Exiv2 {

    /*
      This table describes the standard TIFF layout (including non-standard
      Makernote structures) and determines the corresponding Exiv2 TIFF
      components. The key of the table consists of the first two attributes,
      (extended) tag and group. Tag is the TIFF tag or one of a few extended
      tags, group identifies the IFD or any other composite TIFF component.
      Each entry of the table defines for a particular tag and group
      combination, which create function is used, what the group and parent tag
      and group of the new component are.
     */
    const TiffStructure TiffCreator::tiffStructure_[] = {
        // ext. tag  group             child group       parent tag parent group      create function
        //---------  ----------------- ----------------- ---------- ----------------- -------------------
        // Root directory
        { Tag::root, Group::none,      Group::ifd0,      Tag::root, Group::none,      newTiffDirectory },

        // IFD0
        {    0x8769, Group::ifd0,      Group::exif,      Tag::root, Group::none,      newTiffSubIfd },
        {    0x8825, Group::ifd0,      Group::gps,       Tag::root, Group::none,      newTiffSubIfd },
        // SubIfd found in NEF images (up to 3 sub directories seen, groups sub0_0, sub0_1, sub0_2)
        {    0x014a, Group::ifd0,      Group::sub0_0,    Tag::root, Group::none,      newTiffSubIfd },
        { Tag::next, Group::ifd0,      Group::ifd1,      Tag::root, Group::none,      newTiffDirectory },
        {  Tag::all, Group::ifd0,      Group::ifd0,      Tag::root, Group::none,      newTiffEntry },

        // Subdir sub0_0
        { Tag::next, Group::sub0_0,    Group::ignr,      0x014a,    Group::ifd0,      newTiffDirectory },
        {  Tag::all, Group::sub0_0,    Group::sub0_0,    0x014a,    Group::ifd0,      newTiffEntry },

        // Subdir sub0_1
        { Tag::next, Group::sub0_1,    Group::ignr,      0x014a,    Group::ifd0,      newTiffDirectory },
        {  Tag::all, Group::sub0_1,    Group::sub0_1,    0x014a,    Group::ifd0,      newTiffEntry },

        // Subdir sub0_2
        { Tag::next, Group::sub0_2,    Group::ignr,      0x014a,    Group::ifd0,      newTiffDirectory },
        {  Tag::all, Group::sub0_2,    Group::sub0_2,    0x014a,    Group::ifd0,      newTiffEntry },

        // Exif subdir
        {    0xa005, Group::exif,      Group::iop,       0x8769,    Group::ifd0,      newTiffSubIfd },
        {    0x927c, Group::exif,      Group::mn,        0x8769,    Group::ifd0,      newTiffMnEntry },
        { Tag::next, Group::exif,      Group::ignr,      0x8769,    Group::ifd0,      newTiffDirectory },
        {  Tag::all, Group::exif,      Group::exif,      0x8769,    Group::ifd0,      newTiffEntry },

        // GPS subdir
        { Tag::next, Group::gps,       Group::ignr,      0x8825,    Group::ifd0,      newTiffDirectory },
        {  Tag::all, Group::gps,       Group::gps,       0x8825,    Group::ifd0,      newTiffEntry },

        // IOP subdir
        { Tag::next, Group::iop,       Group::ignr,      0xa005,    Group::exif,      newTiffDirectory },
        {  Tag::all, Group::iop,       Group::iop,       0xa005,    Group::exif,      newTiffEntry },

        // IFD1
        {    0x0111, Group::ifd1,      Group::ifd1,      Tag::next, Group::ifd0,      newTiffThumbData<0x0117, Group::ifd1> },
        {    0x0117, Group::ifd1,      Group::ifd1,      Tag::next, Group::ifd0,      newTiffThumbSize<0x0111, Group::ifd1> },
        {    0x0201, Group::ifd1,      Group::ifd1,      Tag::next, Group::ifd0,      newTiffThumbData<0x0202, Group::ifd1> },
        {    0x0202, Group::ifd1,      Group::ifd1,      Tag::next, Group::ifd0,      newTiffThumbSize<0x0201, Group::ifd1> },
        { Tag::next, Group::ifd1,      Group::ignr,      Tag::next, Group::ifd0,      newTiffDirectory },
        {  Tag::all, Group::ifd1,      Group::ifd1,      Tag::next, Group::ifd0,      newTiffEntry },

        // Olympus makernote - some Olympus cameras use Minolta structures
        // Todo: Adding such tags will not work (maybe result in a Minolta makernote), need separate groups
        {    0x0001, Group::olympmn,   Group::minocso,   0x927c,    Group::exif,      newTiffArrayEntry<unsignedLong> },
        {    0x0003, Group::olympmn,   Group::minocsn,   0x927c,    Group::exif,      newTiffArrayEntry<unsignedLong> },
        { Tag::next, Group::olympmn,   Group::ignr,      0x927c,    Group::exif,      newTiffDirectory },
        {  Tag::all, Group::olympmn,   Group::olympmn,   0x927c,    Group::exif,      newTiffEntry },

        // Fujifilm makernote
        { Tag::next, Group::fujimn,    Group::ignr,      0x927c,    Group::exif,      newTiffDirectory },
        {  Tag::all, Group::fujimn,    Group::fujimn,    0x927c,    Group::exif,      newTiffEntry },

        // Canon makernote
        {    0x0001, Group::canonmn,   Group::canoncs,   0x927c,    Group::exif,      newTiffArrayEntry<unsignedShort> },
        {    0x0004, Group::canonmn,   Group::canonsi,   0x927c,    Group::exif,      newTiffArrayEntry<unsignedShort> },
        {    0x0005, Group::canonmn,   Group::canonpa,   0x927c,    Group::exif,      newTiffArrayEntry<unsignedShort> },
        {    0x000f, Group::canonmn,   Group::canoncf,   0x927c,    Group::exif,      newTiffArrayEntry<unsignedShort> },
        {    0x0012, Group::canonmn,   Group::canonpi,   0x927c,    Group::exif,      newTiffArrayEntry<unsignedShort> },
        { Tag::next, Group::canonmn,   Group::ignr,      0x927c,    Group::exif,      newTiffDirectory },
        {  Tag::all, Group::canonmn,   Group::canonmn,   0x927c,    Group::exif,      newTiffEntry },

        // Canon makernote composite tags
        {  Tag::all, Group::canoncs,   Group::canoncs,   0x0001,    Group::canonmn,   newTiffArrayElement<unsignedShort> },
        {  Tag::all, Group::canonsi,   Group::canonsi,   0x0004,    Group::canonmn,   newTiffArrayElement<unsignedShort> },
        {  Tag::all, Group::canonpa,   Group::canonpa,   0x0005,    Group::canonmn,   newTiffArrayElement<unsignedShort> },
        {  Tag::all, Group::canoncf,   Group::canoncf,   0x000f,    Group::canonmn,   newTiffArrayElement<unsignedShort> },
        {  Tag::all, Group::canonpi,   Group::canonpi,   0x0012,    Group::canonmn,   newTiffArrayElement<unsignedShort> },

        // Nikon1 makernote
        { Tag::next, Group::nikon1mn,  Group::ignr,      0x927c,    Group::exif,      newTiffDirectory },
        {  Tag::all, Group::nikon1mn,  Group::nikon1mn,  0x927c,    Group::exif,      newTiffEntry },

        // Nikon2 makernote
        { Tag::next, Group::nikon2mn,  Group::ignr,      0x927c,    Group::exif,      newTiffDirectory },
        {  Tag::all, Group::nikon2mn,  Group::nikon2mn,  0x927c,    Group::exif,      newTiffEntry },

        // Nikon3 makernote
        { Tag::next, Group::nikon3mn,  Group::ignr,      0x927c,    Group::exif,      newTiffDirectory },
        {  Tag::all, Group::nikon3mn,  Group::nikon3mn,  0x927c,    Group::exif,      newTiffEntry },

        // Panasonic makernote
        { Tag::next, Group::panamn,    Group::ignr,      0x927c,    Group::exif,      newTiffDirectory },
        {  Tag::all, Group::panamn,    Group::panamn,    0x927c,    Group::exif,      newTiffEntry },

        // Sigma/Foveon makernote
        { Tag::next, Group::sigmamn,   Group::ignr,      0x927c,    Group::exif,      newTiffDirectory },
        {  Tag::all, Group::sigmamn,   Group::sigmamn,   0x927c,    Group::exif,      newTiffEntry },

        // Sony1 makernote
        { Tag::next, Group::sony1mn,   Group::ignr,      0x927c,    Group::exif,      newTiffDirectory },
        {  Tag::all, Group::sony1mn,   Group::sony1mn,   0x927c,    Group::exif,      newTiffEntry },

        // Sony2 makernote
        { Tag::next, Group::sony2mn,   Group::ignr,      0x927c,    Group::exif,      newTiffDirectory },
        {  Tag::all, Group::sony2mn,   Group::sony2mn,   0x927c,    Group::exif,      newTiffEntry },

        // Minolta makernote
        {    0x0001, Group::minoltamn, Group::minocso,   0x927c,    Group::exif,      newTiffArrayEntry<unsignedLong>  },
        {    0x0003, Group::minoltamn, Group::minocsn,   0x927c,    Group::exif,      newTiffArrayEntry<unsignedLong>  },
        {    0x0004, Group::minoltamn, Group::minocs7,   0x927c,    Group::exif,      newTiffArrayEntry<unsignedShort> },
        {    0x0114, Group::minoltamn, Group::minocs5,   0x927c,    Group::exif,      newTiffArrayEntry<unsignedShort> },
        { Tag::next, Group::minoltamn, Group::ignr,      0x927c,    Group::exif,      newTiffDirectory },
        {  Tag::all, Group::minoltamn, Group::minoltamn, 0x927c,    Group::exif,      newTiffEntry },

        // Minolta makernote composite tags
        {  Tag::all, Group::minocso,   Group::minocso,   0x0001,    Group::minoltamn, newTiffArrayElement<unsignedLong,  bigEndian> },
        {  Tag::all, Group::minocsn,   Group::minocsn,   0x0003,    Group::minoltamn, newTiffArrayElement<unsignedLong,  bigEndian> },
        {  Tag::all, Group::minocs7,   Group::minocs7,   0x0004,    Group::minoltamn, newTiffArrayElement<unsignedShort, bigEndian> },
        {  Tag::all, Group::minocs5,   Group::minocs5,   0x0114,    Group::minoltamn, newTiffArrayElement<unsignedShort, bigEndian> },

        // Tags which are not de/encoded
        { Tag::next, Group::ignr,      Group::ignr,      Tag::none, Group::none,      newTiffDirectory },
        {  Tag::all, Group::ignr,      Group::ignr,      Tag::none, Group::none,      newTiffEntry }
    };

    // TIFF mapping table for special decoding and encoding requirements
    const TiffMappingInfo TiffMapping::tiffMappingInfo_[] = {
        { "*",       Tag::all, Group::ignr,    0, 0 }, // Do not decode tags with group == Group::ignr
        { "OLYMPUS",   0x0100, Group::olympmn, &TiffDecoder::decodeOlympThumb, &TiffEncoder::encodeOlympThumb },
        { "*",         0x014a, Group::ifd0,    0, 0 }, // Todo: Controversial, causes problems with Exiftool
        { "*",       Tag::all, Group::sub0_0,  &TiffDecoder::decodeSubIfd, &TiffEncoder::encodeSubIfd },
        { "*",       Tag::all, Group::sub0_1,  &TiffDecoder::decodeSubIfd, &TiffEncoder::encodeSubIfd },
        { "*",         0x8649, Group::ifd0,    &TiffDecoder::decodeIrbIptc, &TiffEncoder::encodeIrbIptc },
        // Minolta makernote entries which need to be encoded in big endian byte order
        { "*",       Tag::all, Group::minocso, &TiffDecoder::decodeStdTiffEntry, &TiffEncoder::encodeBigEndianEntry },
        { "*",       Tag::all, Group::minocso, &TiffDecoder::decodeStdTiffEntry, &TiffEncoder::encodeBigEndianEntry },
        { "*",       Tag::all, Group::minocso, &TiffDecoder::decodeStdTiffEntry, &TiffEncoder::encodeBigEndianEntry },
        { "*",       Tag::all, Group::minocso, &TiffDecoder::decodeStdTiffEntry, &TiffEncoder::encodeBigEndianEntry }
    };

    const DecoderFct TiffMapping::findDecoder(const std::string& make,
                                                    uint32_t     extendedTag,
                                                    uint16_t     group)
    {
        DecoderFct decoderFct = &TiffDecoder::decodeStdTiffEntry;
        const TiffMappingInfo* td = find(tiffMappingInfo_,
                                         TiffMappingInfo::Key(make, extendedTag, group));
        if (td) {
            // This may set decoderFct to 0, meaning that the tag should not be decoded
            decoderFct = td->decoderFct_;
        }
        return decoderFct;
    }

    const EncoderFct TiffMapping::findEncoder(const std::string& make,
                                                    uint32_t     extendedTag,
                                                    uint16_t     group)
    {
        EncoderFct encoderFct = &TiffEncoder::encodeStdTiffEntry;
        const TiffMappingInfo* td = find(tiffMappingInfo_,
                                         TiffMappingInfo::Key(make, extendedTag, group));
        if (td) {
            // This may set decoderFct to 0, meaning that the tag should not be decoded
            encoderFct = td->encoderFct_;
        }
        return encoderFct;
    }

    TiffComponent::AutoPtr TiffCreator::create(uint32_t extendedTag,
                                               uint16_t group)
    {
        TiffComponent::AutoPtr tc(0);
        uint16_t tag = static_cast<uint16_t>(extendedTag & 0xffff);
        const TiffStructure* ts = find(tiffStructure_,
                                       TiffStructure::Key(extendedTag, group));
        if (ts && ts->newTiffCompFct_) {
            tc = ts->newTiffCompFct_(tag, ts);
        }
#ifdef DEBUG
        else {
            if (!ts) {
                std::cerr << "Warning: No TIFF structure entry found for "
            }
            else {
                std::cerr << "Warning: No TIFF component creator found for "
            }
            std::cerr << "extended tag 0x" << std::setw(4) << std::setfill('0')
                      << std::hex << std::right << extendedTag
                      << ", group " << tiffGroupName(group) << "\n";
        }
#endif
        return tc;
    } // TiffCreator::create

    void TiffCreator::getPath(TiffPath& tiffPath, uint32_t extendedTag, uint16_t group)
    {
        const TiffStructure* ts = 0;
        do {
            ts = find(tiffStructure_, TiffStructure::Key(extendedTag, group));
            assert(ts != 0);
            tiffPath.push(ts);
            extendedTag = ts->parentExtTag_;
            group = ts->parentGroup_;
        } while (!(ts->extendedTag_ == Tag::root && ts->group_ == Group::none));

    } // TiffCreator::getPath

    void TiffParser::decode(Image*             pImage,
                            const byte*        pData,
                            uint32_t           size,
                            TiffCompFactoryFct createFct,
                            FindDecoderFct     findDecoderFct)
    {
        TiffComponent::AutoPtr rootDir = parse(pData, size, createFct);
        if (0 != rootDir.get()) {
            TiffDecoder decoder(pImage, rootDir.get(), findDecoderFct);
            rootDir->accept(decoder);
        }

    } // TiffParser::decode


/*
  TODO

  STOP - THINK

  -what do decoder functions do exactly? (add to comments)
  -what encoder functionality is required to do the reverse?
  -try to be symmetrical
  -encode+add steps are together the equivalent of decode

     for each metadatum to be encoded
            map it to the correct tiff tag(s)
            encode data

 */

    void TiffParser::encode(Blob&              blob,
                            const byte*        pData,
                            uint32_t           size,
                            const Image*       pImage,
                            TiffCompFactoryFct createFct,
                            FindEncoderFct     findEncoderFct)
    {
        // Todo: What if there is no target image yet?
        assert(pImage != 0);

        TiffComponent::AutoPtr rootDir = parse(pData, size, createFct);
        if (0 == rootDir.get()) {
            rootDir = createFct(Tag::root, Group::none);
        }
        assert(rootDir.get());
        // Todo: Hack: Reads the header again to get byteorder. Image should 
        // have byteOrder() and possibly setByteOrder() methods
        TiffHeade2 tiffHeader;
        if (pData != 0 && !tiffHeader.read(pData, size)) {
            throw Error(3, "TIFF");
        }
        // Update existing TIFF components based on metadata entries
        TiffEncoder encoder(pImage,
                            rootDir.get(),
                            tiffHeader.byteOrder(),
                            findEncoderFct);
        rootDir->accept(encoder);
        // Add remaining entries from metadata to composite, if any
        encoder.add(rootDir.get(), createFct);
        if (encoder.dirty()) {

            // Todo: Remove debug output 
            std::cerr << "Intrusive writing\n";

            // Re-write binary representation from the composite tree
            tiffHeader.write(blob);
            rootDir->write(blob, tiffHeader.byteOrder(), tiffHeader.ifdOffset(), uint32_t(-1), uint32_t(-1));
        }
    } // TiffParser::encode

    TiffComponent::AutoPtr TiffParser::parse(const byte*        pData,
                                             uint32_t           size,
                                             TiffCompFactoryFct createFct)
    {
        if (pData == 0 || size == 0) return TiffComponent::AutoPtr(0);

        TiffHeade2 tiffHeader;
        if (!tiffHeader.read(pData, size) || tiffHeader.ifdOffset() >= size) {
            throw Error(3, "TIFF");
        }

        TiffComponent::AutoPtr rootDir = createFct(Tag::root, Group::none);
        if (0 != rootDir.get()) {
            rootDir->setStart(pData + tiffHeader.ifdOffset());

            TiffRwState::AutoPtr state(
                new TiffRwState(tiffHeader.byteOrder(), 0, createFct));
            TiffReader reader(pData, size, rootDir.get(), state);
            rootDir->accept(reader);
        }
        return rootDir;

    } // TiffParser::parse

}                                       // namespace Exiv2
