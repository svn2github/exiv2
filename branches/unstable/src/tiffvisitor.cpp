// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006-2007 Andreas Huggel <ahuggel@gmx.net>
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
  File:      tiffvisitor.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   11-Apr-06, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// Remove debug
#define DEBUG

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "tiffcomposite.hpp"  // Do not change the order of these 2 includes,
#include "tiffvisitor.hpp"    // see bug #487
#include "tiffparser.hpp"
#include "makernote2.hpp"
#include "exif.hpp"
#include "iptc.hpp"
#include "value.hpp"
#include "image.hpp"
#include "jpgimage.hpp"
#include "i18n.h"             // NLS support.

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    TiffVisitor::TiffVisitor()
    {
        for (int i = 0; i < events_; ++i) {
            go_[i] = true;
        }
    }

    void TiffVisitor::setGo(GoEvent event, bool go)
    {
        assert(event >= 0 && event < events_);
        go_[event] = go;
    }

    bool TiffVisitor::go(GoEvent event) const
    {
        assert(event >= 0 && event < events_);
        return go_[event]; 
    }

    void TiffFinder::init(uint16_t tag, uint16_t group)
    {
        tag_ = tag;
        group_ = group;
        tiffComponent_ = 0;
    }

    void TiffFinder::findObject(TiffComponent* object)
    {
        if (object->tag() == tag_ && object->group() == group_) {
            tiffComponent_ = object;
            setGo(geTraverse, false);
        }
    }

    void TiffFinder::visitEntry(TiffEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitDataEntry(TiffDataEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitSizeEntry(TiffSizeEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitDirectory(TiffDirectory* object)
    {
        findObject(object);
    }

    void TiffFinder::visitSubIfd(TiffSubIfd* object)
    {
        findObject(object);
    }

    void TiffFinder::visitMnEntry(TiffMnEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitIfdMakernote(TiffIfdMakernote* object)
    {
        findObject(object);
    }

    void TiffFinder::visitArrayEntry(TiffArrayEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitArrayElement(TiffArrayElement* object)
    {
        findObject(object);
    }

    TiffDecoder::TiffDecoder(Image* pImage,
                             TiffComponent* const pRoot,
                             FindDecoderFct findDecoderFct)
        : pImage_(pImage),
          pRoot_(pRoot),
          findDecoderFct_(findDecoderFct),
          decodedIptc_(false)
    {
        assert(pImage_ != 0);
        assert(pRoot != 0);

        // Find camera make
        TiffFinder finder(0x010f, Group::ifd0);
        pRoot_->accept(finder);
        TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
        if (te && te->pValue()) {
            make_ = te->pValue()->toString();
        }
    }

    void TiffDecoder::visitEntry(TiffEntry* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitDataEntry(TiffDataEntry* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitSizeEntry(TiffSizeEntry* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitDirectory(TiffDirectory* /*object*/)
    {
        // Nothing to do
    }

    void TiffDecoder::visitSubIfd(TiffSubIfd* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitMnEntry(TiffMnEntry* object)
    {
        if (!object->mn_) decodeTiffEntry(object);
    }

    void TiffDecoder::visitIfdMakernote(TiffIfdMakernote* /*object*/)
    {
        // Nothing to do
    }

    void TiffDecoder::decodeOlympThumb(const TiffEntryBase* object)
    {
        const DataValue* v = dynamic_cast<const DataValue*>(object->pValue());
        if (v != 0) {
            ExifData& exifData = pImage_->exifData();
            exifData["Exif.Thumbnail.Compression"] = uint16_t(6);
            DataBuf buf(v->size());
            v->copy(buf.pData_);
            Exifdatum& ed = exifData["Exif.Thumbnail.JPEGInterchangeFormat"];
            ed = uint32_t(0);
            ed.setDataArea(buf.pData_, buf.size_);
            exifData["Exif.Thumbnail.JPEGInterchangeFormatLength"] = uint32_t(buf.size_);
        }
    }

    void TiffDecoder::getObjData(byte const*& pData,
                                 long& size,
                                 uint16_t tag,
                                 uint16_t group,
                                 const TiffEntryBase* object)
    {
        if (object && object->tag() == tag && object->group() == group) {
            pData = object->pData();
            size = object->size();
            return;
        }
        TiffFinder finder(tag, group);
        pRoot_->accept(finder);
        TiffEntryBase const* te = dynamic_cast<TiffEntryBase*>(finder.result());
        if (te) {
            pData = te->pData();
            size = te->size();
            return;
        }
    }

    void TiffDecoder::decodeXmp(const TiffEntryBase* object)
    {
        // add Exif tag anyway
        decodeStdTiffEntry(object);

        if (object->pData()) {
            // Todo: decode rather than just dump the contents
            pImage_->xmpPacket().assign(
                std::string(reinterpret_cast<const char*>(
                                object->pData()), object->size()));
        }
    } // TiffMetadataDecoder::decodeXmp

    void TiffDecoder::decodeIptc(const TiffEntryBase* object)
    {
        // add Exif tag anyway
        decodeStdTiffEntry(object);

        // All tags are read at this point, so the first time we come here,
        // find the relevant IPTC tag and decode IPTC if found
        if (decodedIptc_) {
            return;
        }
        decodedIptc_ = true;
        // 1st choice: IPTCNAA
        byte const* pData = 0;
        long size = 0;
        getObjData(pData, size, 0x83bb, Group::ifd0, object);
        if (pData) {
            if (0 == pImage_->iptcData().load(pData, size)) {
                return;
            }
#ifndef SUPPRESS_WARNINGS
            else {
                std::cerr << "Warning: Failed to decode IPTC block found in "
                          << "Directory Image, entry 0x83bb\n";
            }
#endif
        }

        // 2nd choice if no IPTCNAA record found or failed to decode it:
        // ImageResources
        pData = 0;
        size = 0;
        getObjData(pData, size, 0x8649, Group::ifd0, object);
        if (pData) {
            byte const* record = 0;
            uint32_t sizeHdr = 0;
            uint32_t sizeData = 0;
            if (0 != Photoshop::locateIptcIrb(pData, size,
                                              &record, &sizeHdr, &sizeData)) {
                return;
            }
            if (0 == pImage_->iptcData().load(record + sizeHdr, sizeData)) {
                return;
            }
#ifndef SUPPRESS_WARNINGS
            else {
                std::cerr << "Warning: Failed to decode IPTC block found in "
                          << "Directory Image, entry 0x8649\n";
            }
#endif
        }
    } // TiffMetadataDecoder::decodeIptc

    void TiffDecoder::decodeSubIfd(const TiffEntryBase* object)
    {
        assert(object);

        // Only applicable if ifd0 NewSubfileType is Thumbnail/Preview image
        GroupType::const_iterator i = groupType_.find(Group::ifd0);
        if (i == groupType_.end() || (i->second & 1) == 0) return;

        // Only applicable if subIFD NewSubfileType is Primary image
        i = groupType_.find(object->group());
        if (i == groupType_.end() || (i->second & 1) == 1) return;

        // Todo: ExifKey should have an appropriate c'tor, it should not be
        //       necessary to use groupName here
        ExifKey key(object->tag(), tiffGroupName(Group::ifd0));
        setExifTag(key, object->pValue());

    }

    void TiffDecoder::decodeTiffEntry(const TiffEntryBase* object)
    {
        assert(object != 0);

        // Remember NewSubfileType
        if (object->tag() == 0x00fe && object->pValue()) {
            groupType_[object->group()] = object->pValue()->toLong();
        }

        const DecoderFct decoderFct = findDecoderFct_(make_,
                                                      object->tag(),
                                                      object->group());
        // skip decoding if decoderFct == 0
        if (decoderFct) {
            EXV_CALL_MEMBER_FN(*this, decoderFct)(object);
        }
    } // TiffDecoder::decodeTiffEntry

    void TiffDecoder::decodeStdTiffEntry(const TiffEntryBase* object)
    {
        assert(object !=0);
        assert(pImage_ != 0);
        // "Normal" tag has low priority: only decode if it doesn't exist yet.
        // Todo: This also filters duplicates (common in some makernotes)
        // Todo: ExifKey should have an appropriate c'tor, it should not be
        //       necessary to use groupName here
        ExifKey key(object->tag(), tiffGroupName(object->group()));
        ExifData::iterator pos = pImage_->exifData().findKey(key);
        if (pos == pImage_->exifData().end()) {
            pImage_->exifData().add(key, object->pValue());
        }
    } // TiffDecoder::decodeTiffEntry

    void TiffDecoder::setExifTag(const ExifKey& key, const Value* pValue)
    {
        assert(pImage_ != 0);
        ExifData::iterator pos = pImage_->exifData().findKey(key);
        if (pos != pImage_->exifData().end()) pImage_->exifData().erase(pos);
        pImage_->exifData().add(key, pValue);

    } // TiffDecoder::setExifTag

    void TiffDecoder::visitArrayEntry(TiffArrayEntry* /*object*/)
    {
        // Nothing to do
    }

    void TiffDecoder::visitArrayElement(TiffArrayElement* object)
    {
        decodeTiffEntry(object);
    }

    TiffEncoder::TiffEncoder(const Image*   pImage,
                             TiffComponent* pRoot,
                             ByteOrder      byteOrder,
                             FindEncoderFct findEncoderFct)
        : pImage_(pImage),
          exifData_(pImage->exifData()),
          del_(true),
          pRoot_(pRoot),
          byteOrder_(byteOrder),
          origByteOrder_(byteOrder),
          findEncoderFct_(findEncoderFct),
          dirty_(false)
    {
        assert(pRoot != 0);

        // Find camera make
        ExifKey key("Exif.Image.Make");
        ExifData::const_iterator pos = exifData_.findKey(key);
        if (pos != exifData_.end()) {
            make_ = pos->toString();
        }
        if (make_.empty() && pRoot_) {
            TiffFinder finder(0x010f, Group::ifd0);
            pRoot_->accept(finder);
            TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
            if (te && te->pValue()) {
                make_ = te->pValue()->toString();
            }
        }
    }

    void TiffEncoder::visitEntry(TiffEntry* object)
    {
        encodeTiffEntry(object);
    }

    void TiffEncoder::visitDataEntry(TiffDataEntry* object)
    {
        encodeTiffEntry(object);

        if (!dirty_) {
            assert(object);
            assert(object->pValue());
            if (  object->sizeDataArea_
                < static_cast<uint32_t>(object->pValue()->sizeDataArea())) {
#ifdef DEBUG
                ExifKey key(object->tag(), tiffGroupName(object->group()));
                std::cerr << "DATAAREA GREW     " << key << "\n";
#endif
                dirty_ = true;
            }
            else {
                // Write the new dataarea, fill with 0x0
#ifdef DEBUG
                ExifKey key(object->tag(), tiffGroupName(object->group()));
                std::cerr << "Writing data area for " << key << "\n";
#endif
                DataBuf buf = object->pValue()->dataArea();
                memcpy(object->pDataArea_, buf.pData_, buf.size_);
                if (object->sizeDataArea_ - buf.size_ > 0) {
                    memset(object->pDataArea_ + buf.size_,
                           0x0, object->sizeDataArea_ - buf.size_);
                }
            }
        }
    }

    void TiffEncoder::visitSizeEntry(TiffSizeEntry* object)
    {
        encodeTiffEntry(object);
    }

    void TiffEncoder::visitDirectory(TiffDirectory* /*object*/)
    {
        // Nothing to do
    }

    void TiffEncoder::visitDirectoryNext(TiffDirectory* object)
    {
        // Update type and count in IFD entries, in case they changed
        assert(object != 0);

        byte* p = object->start() + 2;
        for (TiffDirectory::Components::iterator i = object->components_.begin();
             i != object->components_.end(); ++i) {
            p += updateDirEntry(p, byteOrder(), *i);
        }
    }

    uint32_t TiffEncoder::updateDirEntry(byte* buf,
                                         ByteOrder byteOrder,
                                         TiffComponent* pTiffComponent) const
    {
        assert(buf);
        assert(pTiffComponent);
        TiffEntryBase* pTiffEntry = dynamic_cast<TiffEntryBase*>(pTiffComponent);
        assert(pTiffEntry);
        us2Data(buf + 2, pTiffEntry->typeId(), byteOrder);
        ul2Data(buf + 4, pTiffEntry->count(),  byteOrder);
        // Move data to offset field, if it fits and is not yet there.
        if (pTiffEntry->size() <= 4 && buf + 8 != pTiffEntry->pData()) {

            // Todo: Remove debug output
            std::cerr << "Copying data for tag " << pTiffEntry->tag()
                      << " to offset area.\n";

            memset(buf + 8, 0x0, 4);
            memcpy(buf + 8, pTiffEntry->pData(), pTiffEntry->size());
            memset(const_cast<byte*>(pTiffEntry->pData()), 0x0, pTiffEntry->size());
        }
        return 12;
    }

    void TiffEncoder::visitSubIfd(TiffSubIfd* object)
    {
        encodeTiffEntry(object);
    }

    void TiffEncoder::visitMnEntry(TiffMnEntry* object)
    {
        if (!object->mn_) encodeTiffEntry(object);
    }

    void TiffEncoder::visitIfdMakernote(TiffIfdMakernote* object)
    {
        assert(object != 0);

        // Modify encoder for Makernote peculiarities, byte order
        if (object->byteOrder() != invalidByteOrder) {
            byteOrder_ = object->byteOrder();
        }
    } // TiffEncoder::visitIfdMakernote

    void TiffEncoder::visitIfdMakernoteEnd(TiffIfdMakernote* /*object*/)
    {
        // Reset byte order back to that from the c'tor
        byteOrder_ = origByteOrder_;

    } // TiffEncoder::visitIfdMakernoteEnd

    void TiffEncoder::visitArrayEntry(TiffArrayEntry* /*object*/)
    {
        // Nothing to do
    }

    void TiffEncoder::visitArrayElement(TiffArrayElement* object)
    {
        encodeTiffEntry(object);
    }

    void TiffEncoder::encodeTiffEntry(TiffEntryBase* object)
    {
        assert(object != 0);

        const EncoderFct encoderFct = findEncoderFct_(make_,
                                                      object->tag(),
                                                      object->group());
        // skip encoding if encoderFct == 0
        if (encoderFct) {
            EXV_CALL_MEMBER_FN(*this, encoderFct)(object);
        }
    } // TiffEncoder::encodeTiffEntry

    void TiffEncoder::encodeStdTiffEntry(TiffEntryBase* object)
    {
        assert(object !=0);

        ExifKey key(object->tag(), tiffGroupName(object->group()));
        ExifData::iterator pos = exifData_.findKey(key);
        if (pos == exifData_.end()) { // metadatum not found
#ifdef DEBUG
            std::cerr << "DELETING          " << key << "\n";
#endif
            object->isDeleted_ = true;
            dirty_ = true;
        }
        else { // found metadatum corresponding to object
#ifdef DEBUG
            DataBuf buf(object->size_);
            memcpy(buf.pData_, object->pData_, object->size_);
#endif
            bool overwrite = true;
            uint32_t newSize = pos->size();
            if (newSize > object->size_) {
                object->allocData(newSize);
                dirty_ = true;
#ifdef DEBUG
                std::cerr << "ALLOCATING        " << key << " newSize = " << newSize << " ";
#endif
            }
            else if (pos->sizeDataArea() == 0) {
                memset(object->pData_, 0x0, object->size_);
#ifdef DEBUG
                std::cerr << "OVERWRITING       " << key;
#endif
            }
	    else {
                // Hack: Don't change the entry's value if there is a data area.
                // This ensures that the original offsets are not overwritten
                // during non-intrusive writing.
                // On the other hand, it is thus not possible to change the offsets
                // of an entry with a data area in non-intrusive mode. This can be
                // considered a bug.
                // Todo: Fix me!
	        overwrite = false;
#ifdef DEBUG
	        std::cerr << "NOT OVERWRITING   " << key
                          << " \tdata area size = "
                          << std::dec << pos->sizeDataArea();
#endif
            }
	    if (overwrite) {
                object->type_ = pos->typeId();
                object->count_ = pos->count();
                // offset will be calculated later
                object->size_ = pos->copy(object->pData_, byteOrder());
                assert(object->size_ == newSize);
#ifdef DEBUG
                if (0 != memcmp(object->pData_, buf.pData_, buf.size_)) {
                    std::cerr << "\t\t\t NOT MATCHING";
                }
#endif
            } // overwrite
#ifdef DEBUG
            std::cerr << "\n";
#endif
            object->pValue_ = pos->getValue().release();
            if (del_) exifData_.erase(pos);
        }
    } // TiffEncoder::encodeStdTiffEntry

    void TiffEncoder::encodeOlympThumb(TiffEntryBase* object)
    {
        // Todo
    }

    void TiffEncoder::encodeSubIfd(TiffEntryBase* object)
    {
        // Todo
    }

    void TiffEncoder::encodeIptc(TiffEntryBase* object)
    {
        // Todo
    }

    void TiffEncoder::encodeXmp(TiffEntryBase* object)
    {
        // Todo
    }

    void TiffEncoder::encodeBigEndianEntry(TiffEntryBase* object)
    {
        byteOrder_ = bigEndian;
        encodeStdTiffEntry(object);
        byteOrder_ = origByteOrder_;
    }

    void TiffEncoder::add(TiffComponent*     pRootDir,
                          TiffCompFactoryFct createFct)
    {
        assert(pRootDir != 0);
        // Ensure that the exifData_ entries are not deleted, to be able to
        // iterate over all remaining entries.
        del_ = false;

        // Todo: What if an image format has a comment??
        //       i.e., how to take care of non-Exif metadata?

        for (ExifData::const_iterator i = exifData_.begin();
             i != exifData_.end(); ++i) {

            // Assumption is that the corresponding TIFF entry doesn't exist

            // Todo: This takes tag and group straight from the exif datum.
            // There is a need for a simple mapping and a provision for quite
            // sophisticated logic to determine the mapped tag and group to
            // handle complex cases (eg, NEF sub-IFDs)

            // Todo: getPath depends on the Creator class, not the createFct
            //       how to get it through to here???

            TiffPath tiffPath;
            TiffCreator::getPath(tiffPath, i->tag(), tiffGroupId(i->groupName()));
            TiffComponent* tc = pRootDir->addPath(i->tag(), tiffPath);
            TiffEntryBase* object = dynamic_cast<TiffEntryBase*>(tc);
#ifdef DEBUG
            if (object == 0) {
                std::cerr << "Warning: addPath() didn't add an entry for "
                          << tiffGroupId(i->groupName())
                          << " tag 0x" << std::setw(4) << std::setfill('0')
                          << i->tag() << "\n";
            }
#endif
            if (object != 0) {
                encodeTiffEntry(object);
            }
        }
    } // TiffEncoder::add

    const std::string TiffPrinter::indent_("   ");

    void TiffPrinter::incIndent()
    {
        prefix_ += indent_;
    } // TiffPrinter::incIndent

    void TiffPrinter::decIndent()
    {
        if (prefix_.length() >= indent_.length()) {
            prefix_.erase(prefix_.length() - indent_.length(), indent_.length());
        }
    } // TiffPrinter::decIndent

    void TiffPrinter::visitEntry(TiffEntry* object)
    {
        printTiffEntry(object, prefix());
    } // TiffPrinter::visitEntry

    void TiffPrinter::visitDataEntry(TiffDataEntry* object)
    {
        printTiffEntry(object, prefix());
        if (object->pValue()) {
            os_ << prefix() << _("Data area") << " "
                << object->pValue()->sizeDataArea()
                << " " << _("bytes.\n");
        }
    } // TiffPrinter::visitEntry

    void TiffPrinter::visitSizeEntry(TiffSizeEntry* object)
    {
        printTiffEntry(object, prefix());
    }

    void TiffPrinter::visitDirectory(TiffDirectory* object)
    {
        assert(object != 0);
        os_ << prefix() << tiffGroupName(object->group())
            << " " << _("directory with") << " "
            // cast to make MSVC happy
            << std::dec << static_cast<unsigned int>(object->components_.size());
        if (object->components_.size() == 1) os_ << " " << _("entry:\n");
        else os_ << " " << _("entries:\n");
        incIndent();

    } // TiffPrinter::visitDirectory

    void TiffPrinter::visitDirectoryNext(TiffDirectory* object)
    {
        decIndent();
        if (object->hasNext()) {
            if (object->pNext_) os_ << prefix() << _("Next directory:\n");
            else os_ << prefix() << _("No next directory\n");
        }
    } // TiffPrinter::visitDirectoryNext

    void TiffPrinter::visitDirectoryEnd(TiffDirectory* /*object*/)
    {
        // Nothing to do
    } // TiffPrinter::visitDirectoryEnd

    void TiffPrinter::visitSubIfd(TiffSubIfd* object)
    {
        os_ << prefix() << _("Sub-IFD") << " ";
        printTiffEntry(object);
    } // TiffPrinter::visitSubIfd

    void TiffPrinter::visitMnEntry(TiffMnEntry* object)
    {
        if (!object->mn_) printTiffEntry(object, prefix());
        else os_ << prefix() << _("Makernote") << " ";
    } // TiffPrinter::visitMnEntry

    void TiffPrinter::visitIfdMakernote(TiffIfdMakernote* /*object*/)
    {
        // Nothing to do
    } // TiffPrinter::visitIfdMakernote

    void TiffPrinter::printTiffEntry(TiffEntryBase* object,
                                     const std::string& px) const
    {
        assert(object != 0);

        os_ << px << tiffGroupName(object->group())
            << " " << _("tag") << " 0x" << std::setw(4) << std::setfill('0')
            << std::hex << std::right << object->tag()
            << ", " << _("type") << " " << TypeInfo::typeName(object->typeId())
            << ", " << std::dec << object->count() << " "<< _("component");
        if (object->count() > 1) os_ << "s";
        os_ << " in " << object->size() << " " << _("bytes");
        if (object->size() > 4) os_ << ", " << _("offset") << " " << object->offset();
        os_ << "\n";
        const Value* vp = object->pValue();
        if (vp && vp->count() < 100) os_ << prefix() << *vp;
        else os_ << prefix() << "...";
        os_ << "\n";

    } // TiffPrinter::printTiffEntry

    void TiffPrinter::visitArrayEntry(TiffArrayEntry* object)
    {
        os_ << prefix() << _("Array Entry") << " " << tiffGroupName(object->group())
            << " " << _("tag") << " 0x" << std::setw(4) << std::setfill('0')
            << std::hex << std::right << object->tag() << " " << _("with")
            << " " << std::dec << object->count() << " ";
        if (object->count() > 1) os_ << _("elements");
        else os_ << _("element");
        os_ << "\n";
    } // TiffPrinter::visitArrayEntry

    void TiffPrinter::visitArrayElement(TiffArrayElement* object)
    {
        printTiffEntry(object, prefix());
    } // TiffPrinter::visitArrayElement

    TiffReader::TiffReader(const byte*    pData,
                           uint32_t       size,
                           TiffComponent* pRoot,
                           TiffRwState::AutoPtr state)
        : pData_(pData),
          size_(size),
          pLast_(pData + size),
          pRoot_(pRoot),
          pState_(state.release()),
          pOrigState_(pState_)
    {
        assert(pData_);
        assert(size_ > 0);

    } // TiffReader::TiffReader

    TiffReader::~TiffReader()
    {
        if (pOrigState_ != pState_) delete pOrigState_;
        delete pState_;
    }

    void TiffReader::resetState() {
        if (pOrigState_ != pState_) delete pState_;
        pState_ = pOrigState_;
    }

    void TiffReader::changeState(TiffRwState::AutoPtr state)
    {
        if (state.get() != 0) {
            if (pOrigState_ != pState_) delete pState_;
            // 0 for create function indicates 'no change'
            if (state->createFct_ == 0) state->createFct_ = pState_->createFct_;
            // invalidByteOrder indicates 'no change'
            if (state->byteOrder_ == invalidByteOrder) state->byteOrder_ = pState_->byteOrder_;
            pState_ = state.release();
        }
    }

    ByteOrder TiffReader::byteOrder() const
    {
        assert(pState_);
        return pState_->byteOrder_;
    }

    uint32_t TiffReader::baseOffset() const
    {
        assert(pState_);
        return pState_->baseOffset_;
    }

    TiffComponent::AutoPtr TiffReader::create(uint32_t extendedTag,
                                              uint16_t group) const
    {
        assert(pState_);
        assert(pState_->createFct_);
        return pState_->createFct_(extendedTag, group);
    }

    void TiffReader::visitEntry(TiffEntry* object)
    {
        readTiffEntry(object);
    }

    void TiffReader::visitDataEntry(TiffDataEntry* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        TiffFinder finder(object->szTag(), object->szGroup());
        pRoot_->accept(finder);
        TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
        if (te && te->pValue()) {
            object->setDataArea(te->pValue(), pData_, size_, baseOffset());
        }
    }

    void TiffReader::visitSizeEntry(TiffSizeEntry* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        TiffFinder finder(object->dtTag(), object->dtGroup());
        pRoot_->accept(finder);
        TiffDataEntry* te = dynamic_cast<TiffDataEntry*>(finder.result());
        if (te && te->pValue()) {
            te->setDataArea(object->pValue(), pData_, size_, baseOffset());
        }
    }

    void TiffReader::visitDirectory(TiffDirectory* object)
    {
        assert(object != 0);

        const byte* p = object->start();
        assert(p >= pData_);

        if (p + 2 > pLast_) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: "
                      << "Directory " << tiffGroupName(object->group())
                      << ": IFD exceeds data buffer, cannot read entry count.\n";
#endif
            return;
        }
        const uint16_t n = getUShort(p, byteOrder());
        p += 2;
        // Sanity check with an "unreasonably" large number
        if (n > 256) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: "
                      << "Directory " << tiffGroupName(object->group()) << " with "
                      << n << " entries considered invalid; not read.\n";
#endif
            return;
        }
        for (uint16_t i = 0; i < n; ++i) {
            if (p + 12 > pLast_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Directory " << tiffGroupName(object->group())
                          << ": IFD entry " << i
                          << " lies outside of the data buffer.\n";
#endif
                return;
            }
            uint16_t tag = getUShort(p, byteOrder());
            TiffComponent::AutoPtr tc = create(tag, object->group());
            assert(tc.get());
            tc->setStart(p);
            object->addChild(tc);
            p += 12;
        }

        if (object->hasNext()) {
            if (p + 4 > pLast_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Directory " << tiffGroupName(object->group())
                          << ": IFD exceeds data buffer, cannot read next pointer.\n";
#endif
                return;
            }
            TiffComponent::AutoPtr tc(0);
            uint32_t next = getLong(p, byteOrder());
            if (next) {
                tc = create(Tag::next, object->group());
#ifndef SUPPRESS_WARNINGS
                if (tc.get() == 0) {
                    std::cerr << "Warning: "
                              << "Directory " << tiffGroupName(object->group())
                              << " has an unhandled next pointer.\n";
                }
#endif
            }
            if (tc.get()) {
                if (baseOffset() + next > size_) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Error: "
                              << "Directory " << tiffGroupName(object->group())
                              << ": Next pointer is out of bounds.\n";
#endif
                    return;
                }
                tc->setStart(pData_ + baseOffset() + next);
                object->addNext(tc);
            }
        } // object->hasNext()

    } // TiffReader::visitDirectory

    void TiffReader::visitSubIfd(TiffSubIfd* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        if ((object->typeId() == unsignedLong || object->typeId() == signedLong)
             && object->count() >= 1) {
            for (uint32_t i = 0; i < object->count(); ++i) {
                int32_t offset = getLong(object->pData() + 4*i, byteOrder());
                if (   baseOffset() + offset > size_
                    || static_cast<int32_t>(baseOffset()) + offset < 0) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Error: "
                              << "Directory " << tiffGroupName(object->group())
                              << ", entry 0x" << std::setw(4)
                              << std::setfill('0') << std::hex << object->tag()
                              << " Sub-IFD pointer " << i
                              << " is out of bounds; ignoring it.\n";
#endif
                    return;
                }
                // If there are multiple dirs, group is incremented for each
                TiffComponent::AutoPtr td(new TiffDirectory(object->tag(),
                                                            object->newGroup_ + i));
                td->setStart(pData_ + baseOffset() + offset);
                object->addChild(td);
            }
        }
#ifndef SUPPRESS_WARNINGS
        else {
            std::cerr << "Warning: "
                      << "Directory " << tiffGroupName(object->group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << object->tag()
                      << " doesn't look like a sub-IFD.\n";
        }
#endif

    } // TiffReader::visitSubIfd

    void TiffReader::visitMnEntry(TiffMnEntry* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        // Find camera make
        TiffFinder finder(0x010f, Group::ifd0);
        pRoot_->accept(finder);
        TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
        std::string make;
        if (te && te->pValue()) {
            make = te->pValue()->toString();
            // create concrete makernote, based on make and makernote contents
            object->mn_ = TiffMnCreator::create(object->tag(),
                                                object->mnGroup_,
                                                make,
                                                object->pData_,
                                                object->size_,
                                                byteOrder());
        }
        if (object->mn_) object->mn_->setStart(object->pData());

    } // TiffReader::visitMnEntry

    void TiffReader::visitIfdMakernote(TiffIfdMakernote* object)
    {
        assert(object != 0);

        if (!object->readHeader(object->start(),
                                static_cast<uint32_t>(pLast_ - object->start()),
                                byteOrder())) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Failed to read "
                      << tiffGroupName(object->ifd_.group())
                      << " IFD Makernote header.\n";
#ifdef DEBUG
            if (static_cast<uint32_t>(pLast_ - object->start()) >= 16) {
                hexdump(std::cerr, object->start(), 16);
            }
#endif // DEBUG
#endif // SUPPRESS_WARNINGS
            setGo(geKnownMakernote, false);
            return;
        }
        // Modify reader for Makernote peculiarities, byte order and offset
        TiffRwState::AutoPtr state(
            new TiffRwState(object->byteOrder(),
                            object->baseOffset(static_cast<uint32_t>(object->start() - pData_))));
        changeState(state);
        object->ifd_.setStart(object->start() + object->ifdOffset());

    } // TiffReader::visitIfdMakernote

    void TiffReader::visitIfdMakernoteEnd(TiffIfdMakernote* /*object*/)
    {
        // Reset state (byte order, create function, offset) back to that
        // for the image
        resetState();
    } // TiffReader::visitIfdMakernoteEnd

    void TiffReader::readTiffEntry(TiffEntryBase* object)
    {
        assert(object != 0);

        byte* p = object->start();
        assert(p >= pData_);

        if (p + 12 > pLast_) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Entry in directory " << tiffGroupName(object->group())
                      << "requests access to memory beyond the data buffer. "
                      << "Skipping entry.\n";
#endif
            return;
        }
        // Component already has tag
        p += 2;
        object->type_ = getUShort(p, byteOrder());
        long typeSize = TypeInfo::typeSize(object->typeId());
        if (0 == typeSize) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Directory " << tiffGroupName(object->group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << object->tag()
                      << " has an invalid type:\n"
                      << "Type = " << std::dec << object->type_
                      << "; skipping entry.\n";
#endif
            return;
        }
        p += 2;
        object->count_ = getULong(p, byteOrder());
        if (object->count_ >= 0x10000000) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Directory " << tiffGroupName(object->group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << object->tag()
                      << " has invalid size "
                      << std::dec << object->count_ << "*" << typeSize 
                      << "; skipping entry.\n";
#endif
            return;
        }
        p += 4;
        object->size_ = typeSize * object->count_;
        object->offset_ = getLong(p, byteOrder());
        object->pData_ = p;
        if (object->size_ > 4) {
            if (baseOffset() + object->offset_ >= size_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: Offset of "
                          << "directory " << tiffGroupName(object->group())
                          << ", entry 0x" << std::setw(4)
                          << std::setfill('0') << std::hex << object->tag()
                          << " is out of bounds:\n"
                          << "Offset = 0x" << std::setw(8)
                          << std::setfill('0') << std::hex << object->offset_
                          << "; truncating the entry\n";
#endif
                object->size_ = 0;
                object->count_ = 0;
                object->offset_ = 0;
                return;
            }
            object->pData_ = const_cast<byte*>(pData_) + baseOffset() + object->offset_;
            if (object->size_ > static_cast<uint32_t>(pLast_ - object->pData_)) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Warning: Upper boundary of data for "
                          << "directory " << tiffGroupName(object->group())
                          << ", entry 0x" << std::setw(4)
                          << std::setfill('0') << std::hex << object->tag()
                          << " is out of bounds:\n"
                          << "Offset = 0x" << std::setw(8)
                          << std::setfill('0') << std::hex << object->offset_
                          << ", size = " << std::dec << object->size_
                          << ", exceeds buffer size by "
                          // cast to make MSVC happy
                          << static_cast<uint32_t>(object->pData_ + object->size_ - pLast_)
                          << " Bytes; adjusting the size\n";
#endif
                object->size_ = static_cast<uint32_t>(pLast_ - object->pData_ + 1);
                // todo: adjust count_, make size_ a multiple of typeSize
            }
        }
        // On the fly type conversion for Exif.Photo.UserComment
        // Todo: This should be somewhere else, maybe in a Value factory
        // which takes a Key and Type
        TypeId t = TypeId(object->typeId());
        if (   object->tag()    == 0x9286
            && object->group()  == Group::exif
            && object->typeId() == undefined) {
            t = comment;
        }
        Value::AutoPtr v = Value::create(t);
        if (v.get()) {
            v->read(object->pData_, object->size_, byteOrder());
            object->pValue_ = v.release();
        }

    } // TiffReader::readTiffEntry

    void TiffReader::visitArrayEntry(TiffArrayEntry* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        // Todo: size here is that of the data area
        uint16_t s = static_cast<uint16_t>(object->size_ / object->elSize());
        for (uint16_t i = 0; i < s; ++i) {
            uint16_t tag = i;
            TiffComponent::AutoPtr tc = create(tag, object->elGroup());
            assert(tc.get());
            tc->setStart(object->pData() + i * object->elSize());
            object->addChild(tc);
        }

    } // TiffReader::visitArrayEntry

    void TiffReader::visitArrayElement(TiffArrayElement* object)
    {
        assert(object != 0);

        byte* p = object->start();
        assert(p >= pData_);

        if (p + 2 > pLast_) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Array element in group "
                      << tiffGroupName(object->group())
                      << "requests access to memory beyond the data buffer. "
                      << "Skipping element.\n";
#endif
            return;
        }
        object->type_ = object->elTypeId();
        object->count_ = 1;
        object->size_ = TypeInfo::typeSize(object->typeId()) * object->count();
        object->offset_ = 0;
        object->pData_ = p;
        Value::AutoPtr v = Value::create(object->typeId());
        if (v.get()) {
            ByteOrder b =
                object->elByteOrder() == invalidByteOrder ?
                byteOrder() : object->elByteOrder();
            v->read(object->pData(), object->size(), b);
            object->pValue_ = v.release();
        }

    } // TiffReader::visitArrayElement

}                                       // namespace Exiv2