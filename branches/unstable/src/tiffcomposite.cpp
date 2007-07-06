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
  File:      tiffcomposite.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   11-Apr-06, ahu: created
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

#include "tiffcomposite.hpp"
#include "tiffvisitor.hpp"
#include "makernote2.hpp"
#include "value.hpp"
#include "error.hpp"

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! Structure for group and group name info
    struct TiffGroupInfo {
        //! Comparison operator for group (id)
        bool operator==(const uint16_t& group) const;
        //! Comparison operator for group name
        bool operator==(const std::string& groupName) const;

        uint16_t group_;   //!< group
        const char* name_; //!< group name
    };

    // Todo: This mapping table probably belongs somewhere else - move it
    //! List of groups and their names
    extern const TiffGroupInfo tiffGroupInfo[] = {
        {   1, "Image"        },
        {   2, "Thumbnail"    },
        {   3, "Photo"        },
        {   4, "GPSInfo"      },
        {   5, "Iop"          },
        {   6, "ImageSubIfd0" },
        {   7, "ImageSubIfd1" },
        {   8, "ImageSubIfd2" },
        {   9, "ImageSubIfd3" },
        { 257, "Olympus"      },
        { 258, "Fujifilm"     },
        { 259, "Canon"        },
        { 260, "CanonCs"      },
        { 261, "CanonSi"      },
        { 262, "CanonCf"      },
        // 263 not needed (nikonmn)
        { 264, "Nikon1"       },
        { 265, "Nikon2"       },
        { 266, "Nikon3"       },
        { 267, "Panasonic"    },
        { 268, "Sigma"        },
        // 269 not needed (sonymn)
        { 270, "Sony"         },
        { 271, "Sony"         },
        { 272, "Minolta"      },
        { 273, "MinoltaCsOld" },
        { 274, "MinoltaCsNew" },
        { 275, "MinoltaCs5D"  },
        { 276, "MinoltaCs7D"  },
        { 277, "CanonPi"      },
        { 278, "CanonPa"      }
    };

    bool TiffGroupInfo::operator==(const uint16_t& group) const
    {
        return group_ == group;
    }

    bool TiffGroupInfo::operator==(const std::string& groupName) const
    {
        std::string name(name_);
        return name == groupName;
    }

    const char* tiffGroupName(uint16_t group)
    {
        const TiffGroupInfo* gi = find(tiffGroupInfo, group);
        if (!gi) return "Unknown";
        return gi->name_;
    }

    uint16_t tiffGroupId(const std::string& groupName)
    {
        const TiffGroupInfo* gi = find(tiffGroupInfo, groupName);
        if (!gi) return 0;
        return gi->group_;
    }

    bool TiffStructure::operator==(const TiffStructure::Key& key) const
    {
        return    (Tag::all == extendedTag_ || key.e_ == extendedTag_)
               && key.g_ == group_;
    }

    bool TiffMappingInfo::operator==(const TiffMappingInfo::Key& key) const
    {
        std::string make(make_);
        return    ("*" == make || make == key.m_.substr(0, make.length()))
               && (Tag::all == extendedTag_ || key.e_ == extendedTag_)
               && key.g_ == group_;
    }

    TiffSubIfd::TiffSubIfd(uint16_t tag, uint16_t group, uint16_t newGroup)
        : TiffEntryBase(tag, group, unsignedLong), newGroup_(newGroup)
    {
    }

    TiffMnEntry::TiffMnEntry(uint16_t tag, uint16_t group, uint16_t mnGroup)
        : TiffEntryBase(tag, group, undefined), mnGroup_(mnGroup), mn_(0)
    {
    }

    TiffArrayEntry::TiffArrayEntry(uint16_t tag,
                                   uint16_t group,
                                   uint16_t elGroup,
                                   TypeId   elTypeId,
                                   bool     addSizeElement)
        : TiffEntryBase(tag, group, elTypeId),
          elSize_(static_cast<uint16_t>(TypeInfo::typeSize(elTypeId))),
          elGroup_(elGroup),
          addSizeElement_(addSizeElement)
    {
    }

    TiffDirectory::~TiffDirectory()
    {
        for (Components::iterator i = components_.begin(); i != components_.end(); ++i) {
            delete *i;
        }
        delete pNext_;
    } // TiffDirectory::~TiffDirectory

    TiffSubIfd::~TiffSubIfd()
    {
        for (Ifds::iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            delete *i;
        }
    } // TiffSubIfd::~TiffSubIfd

    TiffEntryBase::~TiffEntryBase()
    {
        if (isMalloced_) {
            delete[] pData_;
        }
        delete pValue_;
    } // TiffEntryBase::~TiffEntryBase

    TiffMnEntry::~TiffMnEntry()
    {
        delete mn_;
    } // TiffMnEntry::~TiffMnEntry

    TiffArrayEntry::~TiffArrayEntry()
    {
        for (Components::iterator i = elements_.begin(); i != elements_.end(); ++i) {
            delete *i;
        }
    } // TiffArrayEntry::~TiffArrayEntry

    void TiffEntryBase::allocData(uint32_t len)
    {
        if (isMalloced_) {
            delete[] pData_;
        }
        pData_ = new byte[len];
        size_ = len;
        isMalloced_ = true;
    } // TiffEntryBase::allocData

    void TiffDataEntry::setDataArea(const Value* pSize,
                                    const byte*  pData,
                                    uint32_t     sizeData,
                                    uint32_t     baseOffset)
    {
        assert(pSize);
        assert(pValue());

        long size = 0;
        for (long i = 0; i < pSize->count(); ++i) {
            size += pSize->toLong(i);
        }
        long offset = pValue()->toLong(0);
        // Todo: Remove limitation of JPEG writer: strips must be contiguous
        // Until then we check: last offset + last size - first offset == size?
        if (  pValue()->toLong(pValue()->count()-1)
            + pSize->toLong(pSize->count()-1)
            - offset != size) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: "
                      << "Directory " << tiffGroupName(group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << tag()
                      << " Data area is not contiguous, ignoring it.\n";
#endif
            return;
        }
        if (baseOffset + offset + size > sizeData) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: "
                      << "Directory " << tiffGroupName(group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << tag()
                      << " Data area exceeds data buffer, ignoring it.\n";
#endif
            return;
        }
        pDataArea_ = const_cast<byte*>(pData) + baseOffset + offset;
        sizeDataArea_ = size;
        const_cast<Value*>(pValue())->setDataArea(pDataArea_, sizeDataArea_);
    } // TiffDataEntry::setDataArea

    TiffComponent* TiffComponent::addPath(uint16_t tag, TiffPath& tiffPath)
    {
        return doAddPath(tag, tiffPath);
    } // TiffComponent::addPath

    TiffComponent* TiffDirectory::doAddPath(uint16_t tag, TiffPath& tiffPath)
    {
        tiffPath.pop();
        assert(!tiffPath.empty());
        const TiffStructure* ts = tiffPath.top();
        assert(ts != 0);
        TiffComponent* tc = 0;
        // To allow duplicate entries, we only check if the new component already
        // exists if there is still at least one composite tag on the stack
        // Todo: Find a generic way to require subIFDs to be unique tags
        if (tiffPath.size() > 1 || ts->newTiffCompFct_ == newTiffSubIfd) {
            if (ts->extendedTag_ == Tag::next) {
                tc = pNext_;
            }
            else {
                for (Components::iterator i = components_.begin(); i != components_.end(); ++i) {
                    if ((*i)->tag() == ts->tag() && (*i)->group() == ts->group_) {
                        tc = *i;
                        break;
                    }
                }
            }
        }
        if (tc == 0) {
            assert(ts->newTiffCompFct_ != 0);
            uint16_t tg = tiffPath.size() == 1 ? tag : ts->tag();
            TiffComponent::AutoPtr atc(ts->newTiffCompFct_(tg, ts));
            if (ts->extendedTag_ == Tag::next) {
                tc = this->addNext(atc);
            }
            else {
                tc = this->addChild(atc);
            }
        }
        return tc->addPath(tag, tiffPath);
    } // TiffDirectory::doAddPath

    TiffComponent* TiffSubIfd::doAddPath(uint16_t tag, TiffPath& tiffPath)
    {
        const TiffStructure* ts1 = tiffPath.top();
        assert(ts1 != 0);
        tiffPath.pop();
        if (tiffPath.empty()) {
            // If the last element in the path is the sub-IFD tag itself we're done
            return this;
        }
        const TiffStructure* ts2 = tiffPath.top();
        assert(ts2 != 0);
        tiffPath.push(ts1);
        uint16_t dt = ts1->tag();
        TiffComponent* tc = 0;
        for (Ifds::iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            if ((*i)->group() == ts2->group_) {
                tc = *i;
                break;
            }
        }
        if (tc == 0) {
            TiffComponent::AutoPtr atc(new TiffDirectory(dt, ts2->group_));
            tc = this->addChild(atc);
            this->setCount(this->count() + 1);
        }
        return tc->addPath(tag, tiffPath);
    } // TiffSubIfd::doAddPath

    TiffComponent* TiffMnEntry::doAddPath(uint16_t tag, TiffPath& tiffPath)
    {
        const TiffStructure* ts1 = tiffPath.top();
        assert(ts1 != 0);
        tiffPath.pop();
        assert(!tiffPath.empty());
        const TiffStructure* ts2 = tiffPath.top();
        assert(ts2 != 0);
        tiffPath.push(ts1);
        if (mn_ == 0) {
            mnGroup_ = ts2->group_;
            mn_ = TiffMnCreator::create(ts1->tag(), ts1->group_, mnGroup_);
            assert(mn_);
        }
        return mn_->addPath(tag, tiffPath);
    } // TiffMnEntry::doAddPath

    TiffComponent* TiffArrayEntry::doAddPath(uint16_t tag, TiffPath& tiffPath)
    {
        tiffPath.pop();
        assert(!tiffPath.empty());
        const TiffStructure* ts = tiffPath.top();
        assert(ts != 0);
        TiffComponent* tc = 0;
        // To allow duplicate entries, we only check if the new component already
        // exists if there is still at least one composite tag on the stack
        if (tiffPath.size() > 1) {
            for (Components::iterator i = elements_.begin(); i != elements_.end(); ++i) {
                if ((*i)->tag() == ts->tag() && (*i)->group() == ts->group_) {
                    tc = *i;
                    break;
                }
            }
        }
        if (tc == 0) {
            assert(ts->newTiffCompFct_ != 0);
            uint16_t tg = tiffPath.size() == 1 ? tag : ts->tag();
            TiffComponent::AutoPtr atc(ts->newTiffCompFct_(tg, ts));
            assert(ts->extendedTag_ != Tag::next);
            tc = this->addChild(atc);
            this->setCount(this->count() + 1);
        }
        return tc->addPath(tag, tiffPath);
    } // TiffArrayEntry::doAddPath

    TiffComponent* TiffComponent::addChild(TiffComponent::AutoPtr tiffComponent)
    {
        return doAddChild(tiffComponent);
    } // TiffComponent::addChild

    TiffComponent* TiffDirectory::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = tiffComponent.release();
        components_.push_back(tc);
        return tc;
    } // TiffDirectory::doAddChild

    TiffComponent* TiffSubIfd::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        TiffDirectory* d = dynamic_cast<TiffDirectory*>(tiffComponent.release());
        assert(d);
        ifds_.push_back(d);
        return d;
    } // TiffSubIfd::doAddChild

    TiffComponent* TiffMnEntry::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = 0;
        if (mn_) {
            tc =  mn_->addChild(tiffComponent);
        }
        return tc;
    } // TiffMnEntry::doAddChild

    TiffComponent* TiffArrayEntry::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = tiffComponent.release();
        elements_.push_back(tc);
        return tc;
    } // TiffArrayEntry::doAddChild

    TiffComponent* TiffComponent::addNext(TiffComponent::AutoPtr tiffComponent)
    {
        return doAddNext(tiffComponent);
    } // TiffComponent::addNext

    TiffComponent* TiffDirectory::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = 0;
        if (hasNext_) {
            tc = tiffComponent.release();
            pNext_ = tc;
        }
        return tc;
    } // TiffDirectory::doAddNext

    TiffComponent* TiffMnEntry::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        TiffComponent* tc = 0;
        if (mn_) {
            tc = mn_->addNext(tiffComponent);
        }
        return tc;
    } // TiffMnEntry::doAddNext

    void TiffComponent::accept(TiffVisitor& visitor)
    {
        if (visitor.go(TiffVisitor::geTraverse)) doAccept(visitor); // one for NVI :)
    } // TiffComponent::accept

    void TiffEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitEntry(this);
    } // TiffEntry::doAccept

    void TiffDataEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitDataEntry(this);
    } // TiffDataEntry::doAccept

    void TiffSizeEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitSizeEntry(this);
    } // TiffSizeEntry::doAccept

    void TiffDirectory::doAccept(TiffVisitor& visitor)
    {
        visitor.visitDirectory(this);
        for (Components::const_iterator i = components_.begin();
             visitor.go(TiffVisitor::geTraverse) && i != components_.end(); ++i) {
            (*i)->accept(visitor);
        }
        if (visitor.go(TiffVisitor::geTraverse)) visitor.visitDirectoryNext(this);
        if (pNext_) pNext_->accept(visitor);
        if (visitor.go(TiffVisitor::geTraverse)) visitor.visitDirectoryEnd(this);
    } // TiffDirectory::doAccept

    void TiffSubIfd::doAccept(TiffVisitor& visitor)
    {
        visitor.visitSubIfd(this);
        for (Ifds::iterator i = ifds_.begin();
             visitor.go(TiffVisitor::geTraverse) && i != ifds_.end(); ++i) {
            (*i)->accept(visitor);
        }
    } // TiffSubIfd::doAccept

    void TiffMnEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitMnEntry(this);
        if (mn_) mn_->accept(visitor);
        if (!visitor.go(TiffVisitor::geKnownMakernote)) {
            delete mn_;
            mn_ = 0;
        }

    } // TiffMnEntry::doAccept

    void TiffArrayEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitArrayEntry(this);
        for (Components::const_iterator i = elements_.begin();
             visitor.go(TiffVisitor::geTraverse) && i != elements_.end(); ++i) {
            (*i)->accept(visitor);
        }
    } // TiffArrayEntry::doAccept

    void TiffArrayElement::doAccept(TiffVisitor& visitor)
    {
        visitor.visitArrayElement(this);
    } // TiffArrayElement::doAccept

    uint32_t TiffEntryBase::count() const
    {
        return doCount();
    }

    uint32_t TiffEntryBase::doCount() const
    {
        return count_;
    }

    uint32_t TiffMnEntry::doCount() const
    {
        assert(typeId() == undefined);
        return size();
    }

    uint32_t TiffArrayEntry::doCount() const
    {
        if (elements_.empty()) return 0;

        uint16_t maxTag = 0;
        for (Components::const_iterator i = elements_.begin(); i != elements_.end(); ++i) {
            if ((*i)->tag() > maxTag) maxTag = (*i)->tag();
        }
        return maxTag + 1;
    }

    uint32_t TiffComponent::write(Blob&     blob,
                                  ByteOrder byteOrder,
                                  int32_t   offset,
                                  uint32_t  valueIdx,
                                  uint32_t  dataIdx)
    {
        return doWrite(blob, byteOrder, offset, valueIdx, dataIdx);
    } // TiffComponent::write

    uint32_t TiffDirectory::doWrite(Blob&     blob,
                                    ByteOrder byteOrder,
                                    int32_t   offset,
                                    uint32_t  valueIdx,
                                    uint32_t  dataIdx)
    {
        // TIFF standard requires IFD entries to be sorted in ascending order by tag
        std::sort(components_.begin(), components_.end(), cmpTagLt);

        // Size of all directory entries, without values and additional data
        const uint32_t sizeDir = 2 + 12 * components_.size() + (hasNext_ ? 4 : 0);

        // Size of IFD values
        uint32_t sizeValue = 0;
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            uint32_t sv = (*i)->size();
            if (sv > 4) sizeValue += sv;
        }

        uint32_t idx = 0;                   // Current IFD index / bytes written
        valueIdx = sizeDir;                 // Offset to the current IFD value
        dataIdx  = sizeDir + sizeValue;     // Offset to the entry's data area

        // Write the number of directory entries
        if (components_.size() > 0xffff) {
            throw Error(38, tiffGroupName(group()));
        }
        byte buf[4];
        us2Data(buf, static_cast<uint16_t>(components_.size()), byteOrder);
        append(blob, buf, 2);
        idx += 2;

        // Write directory entries - may contain pointers to the value or data
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            idx += writeDirEntry(blob, byteOrder, offset, *i, valueIdx, dataIdx);
            uint32_t sv = (*i)->size();
            if (sv > 4) valueIdx += sv;
            dataIdx += (*i)->sizeData();
        }
        if (hasNext_) {
            // Add the offset to the next IFD
            memset(buf, 0x0, 4);
            if (pNext_) {
                l2Data(buf, offset + dataIdx, byteOrder);
            }
            append(blob, buf, 4);
            idx += 4;
        }
        assert(idx == sizeDir);

        // Write IFD values - may contain pointers to additional data
        valueIdx = sizeDir;
        dataIdx = sizeDir + sizeValue;
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            uint32_t sv = (*i)->size();
            if (sv > 4) {
                idx += (*i)->write(blob, byteOrder, offset, valueIdx, dataIdx);
                valueIdx += sv;
            }
            dataIdx += (*i)->sizeData();
        }
        assert(idx == sizeDir + sizeValue);

        // Write data - may contain offsets too (eg sub-IFD)
        dataIdx = sizeDir + sizeValue;
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            idx += (*i)->writeData(blob, byteOrder, offset, dataIdx);
            dataIdx += (*i)->sizeData();
        }

        if (pNext_) {
            idx += pNext_->write(blob, byteOrder, offset + idx, uint32_t(-1), uint32_t(-1));
        }

        // Todo: Another loop over the components to write data that
        // goes at the end of everything else?

        return idx;
    } // TiffDirectory::doWrite

    uint32_t TiffDirectory::writeDirEntry(Blob&          blob,
                                          ByteOrder      byteOrder,
                                          int32_t        offset,
                                          TiffComponent* pTiffComponent,
                                          uint32_t       valueIdx,
                                          uint32_t       dataIdx) const
    {
        assert(pTiffComponent);
        TiffEntryBase* pTiffEntry = dynamic_cast<TiffEntryBase*>(pTiffComponent);
        assert(pTiffEntry);
        byte buf[8];
        us2Data(buf,     pTiffEntry->tag(),    byteOrder);
        us2Data(buf + 2, pTiffEntry->typeId(), byteOrder);
        ul2Data(buf + 4, pTiffEntry->count(),  byteOrder);
        append(blob, buf, 8);
        if (pTiffEntry->size() > 4) {
            pTiffEntry->setOffset(offset + static_cast<int32_t>(valueIdx));
            l2Data(buf, pTiffEntry->offset(), byteOrder);
            append(blob, buf, 4);
        }
        else {
            const uint32_t len = pTiffEntry->write(blob,
                                                   byteOrder,
                                                   offset,
                                                   valueIdx,
                                                   dataIdx);
            assert(len <= 4);
            if (len < 4) {
                memset(buf, 0x0, 4);
                append(blob, buf, 4 - len);
            }
        }
        return 12;
    } // TiffDirectory::writeDirEntry

    uint32_t TiffEntryBase::doWrite(Blob&     blob,
                                    ByteOrder byteOrder,
                                    int32_t   /*offset*/,
                                    uint32_t  /*valueIdx*/,
                                    uint32_t  /*dataIdx*/)
    {
        if (!pValue_) return 0;

        DataBuf buf(pValue_->size());
        pValue_->copy(buf.pData_, byteOrder);
        append(blob, buf.pData_, buf.size_);
        return buf.size_;
    } // TiffEntryBase::doWrite

    uint32_t TiffEntryBase::writeOffset(byte*     buf,
                                        int32_t   offset,
                                        TypeId    type,
                                        ByteOrder byteOrder)
    {
        uint32_t rc = 0;
        switch(type) {
        case unsignedShort:
        case signedShort:
            if (static_cast<uint32_t>(offset) > 0xffff) throw Error(26);
            rc = s2Data(buf, static_cast<int16_t>(offset), byteOrder);
            break;
        case unsignedLong:
        case signedLong:
            rc = l2Data(buf, static_cast<int32_t>(offset), byteOrder);
            break;
        default:
            throw Error(27);
            break;
        }
        return rc;
    } // TiffEntryBase::writeOffset

    uint32_t TiffDataEntry::doWrite(Blob&     blob,
                                    ByteOrder byteOrder,
                                    int32_t   offset,
                                    uint32_t  /*valueIdx*/,
                                    uint32_t  dataIdx)
    {
        if (!pValue()) return 0;

        DataBuf buf(pValue()->size());
        uint32_t idx = 0;
        const long prevOffset = pValue()->toLong(0);
        for (uint32_t i = 0; i < count(); ++i) {
            const long newDataIdx =   pValue()->toLong(i) - prevOffset
                                    + static_cast<long>(dataIdx);
            idx += writeOffset(buf.pData_ + idx,
                               offset + newDataIdx,
                               typeId(),
                               byteOrder);
        }
        append(blob, buf.pData_, buf.size_);
        return buf.size_;
    } // TiffDataEntry::doWrite

    uint32_t TiffSubIfd::doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  /*valueIdx*/,
                                 uint32_t  dataIdx)
    {
        DataBuf buf(ifds_.size() * 4);
        uint32_t idx = 0;
        for (Ifds::const_iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            idx += writeOffset(buf.pData_ + idx, offset + dataIdx, typeId(), byteOrder);
            dataIdx += (*i)->size();
        }
        append(blob, buf.pData_, buf.size_);
        return buf.size_;
    } // TiffSubIfd::doWrite

    uint32_t TiffMnEntry::doWrite(Blob&     blob,
                                  ByteOrder byteOrder,
                                  int32_t   offset,
                                  uint32_t  valueIdx,
                                  uint32_t  /*dataIdx*/)
    {
        if (!mn_) return 0;
        return mn_->write(blob, byteOrder, offset + valueIdx, uint32_t(-1), uint32_t(-1));
    } // TiffMnEntry::doWrite

    uint32_t TiffArrayEntry::doWrite(Blob&     blob,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  valueIdx,
                                     uint32_t  dataIdx)
    {
        std::sort(elements_.begin(), elements_.end(), cmpTagLt);

        uint32_t idx = 0;
        int32_t nextTag = 0;

        // Some array entries need to have the size in the first element
        if (addSizeElement_ && !elements_.empty()) {
            byte buf[4];
            switch (elSize_) {
            case 2:
                idx += us2Data(buf, size(), byteOrder);
                break;
            case 4:
                idx += ul2Data(buf, size(), byteOrder);
                break;
            }
            append(blob, buf, elSize_);
            nextTag = 1;
        }

        // Tags must be sorted in ascending order
        for (Components::const_iterator i = elements_.begin(); i != elements_.end(); ++i) {
            // Skip the manufactured tag, if it exists
            if (addSizeElement_ && (*i)->tag() == 0x0000) continue;
            // Fill gaps. Repeated tags will cause an exception
            int32_t gap = ((*i)->tag() - nextTag) * elSize_;
            if (gap < 0) throw Error(39, (*i)->tag());
            if (gap > 0) {
                blob.insert(blob.end(), gap, 0);
                idx += gap;
            }
            idx += (*i)->write(blob, byteOrder, offset + idx, valueIdx, dataIdx);
            nextTag = (*i)->tag() + 1;
        }
        return idx;
    } // TiffArrayEntry::doWrite

    uint32_t TiffArrayElement::doWrite(Blob&     blob,
                                       ByteOrder byteOrder,
                                       int32_t   /*offset*/,
                                       uint32_t  /*valueIdx*/,
                                       uint32_t  /*dataIdx*/)
    {
        Value const* pv = pValue();
        if (!pv || pv->count() == 0) return 0;
        if (!(pv->count() == 1 && pv->typeId() == elTypeId_)) {
            throw Error(40, tag());
        }
        DataBuf buf(pv->size());
        if (elByteOrder_ != invalidByteOrder) byteOrder = elByteOrder_;
        pv->copy(buf.pData_, byteOrder);
        append(blob, buf.pData_, buf.size_);
        return buf.size_;
    } // TiffArrayElement::doWrite

    uint32_t TiffComponent::writeData(Blob&     blob,
                                      ByteOrder byteOrder,
                                      int32_t   offset,
                                      uint32_t  dataIdx) const
    {
        return doWriteData(blob, byteOrder, offset, dataIdx);
    } // TiffComponent::writeData

    uint32_t TiffDirectory::doWriteData(Blob&     /*blob*/,
                                        ByteOrder /*byteOrder*/,
                                        int32_t   /*offset*/,
                                        uint32_t  /*dataIdx*/) const
    {
        assert(false);          // Overwrite to do something more useful
        return 0;
    } // TiffDirectory::doWriteData

    uint32_t TiffEntryBase::doWriteData(Blob&     /*blob*/,
                                        ByteOrder /*byteOrder*/,
                                        int32_t   /*offset*/,
                                        uint32_t  /*dataIdx*/) const
    {
        return 0;
    } // TiffEntryBase::doWriteData

    uint32_t TiffDataEntry::doWriteData(Blob&     blob,
                                        ByteOrder /*byteOrder*/,
                                        int32_t   /*offset*/,
                                        uint32_t  /*dataIdx*/) const
    {
        if (!pValue()) return 0;

        DataBuf buf = pValue()->dataArea();
        append(blob, buf.pData_, buf.size_);
        return buf.size_;
    } // TiffDataEntry::doWriteData

    uint32_t TiffSubIfd::doWriteData(Blob&     blob,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx) const
    {
        uint32_t len = 0;
        for (Ifds::const_iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            len  += (*i)->write(blob, byteOrder, offset + dataIdx + len, uint32_t(-1), uint32_t(-1));
        }
        return len;
    } // TiffSubIfd::doWriteData

    uint32_t TiffComponent::size() const
    {
        return doSize();
    } // TiffComponent::size

    uint32_t TiffDirectory::doSize() const
    {
        // Size of the directory, without values and additional data
        uint32_t len = 2 + 12 * components_.size() + (hasNext_ ? 4 : 0);
        // Size of IFD values and data
        for (Components::const_iterator i = components_.begin(); i != components_.end(); ++i) {
            uint32_t sv = (*i)->size();
            if (sv > 4) len += sv;
            len += (*i)->sizeData();
        }
        // Size of next-IFD, if any
        if (pNext_) {
            len += pNext_->size();
        }
        return len;
    } // TiffDirectory::doSize

    uint32_t TiffEntryBase::doSize() const
    {
        return size_;
    } // TiffEntryBase::doSize

    uint32_t TiffSubIfd::doSize() const
    {
        return ifds_.size() * 4;
    } // TiffSubIfd::doSize

    uint32_t TiffMnEntry::doSize() const
    {
        if (!mn_) return 0;
        return mn_->size();
    } // TiffMnEntry::doSize

    uint32_t TiffArrayEntry::doSize() const
    {
        return count() * elSize_;
    } // TiffArrayEntry::doSize

    uint32_t TiffComponent::sizeData() const
    {
        return doSizeData();
    } // TiffComponent::sizeData

    uint32_t TiffDirectory::doSizeData() const
    {
        assert(false);
        return 0;
    } // TiffDirectory::doSizeData

    uint32_t TiffEntryBase::doSizeData() const
    {
        return 0;
    } // TiffEntryBase::doSizeData

    uint32_t TiffDataEntry::doSizeData() const
    {
        if (!pValue()) return 0;
        return pValue()->sizeDataArea();
    } // TiffDataEntry::doSizeData

    uint32_t TiffSubIfd::doSizeData() const
    {
        uint32_t len = 0;
        for (Ifds::const_iterator i = ifds_.begin(); i != ifds_.end(); ++i) {
            len += (*i)->size();
        }
        return len;
    } // TiffSubIfd::doSizeData

    // *************************************************************************
    // free functions

    bool cmpTagLt(TiffComponent const* lhs, TiffComponent const* rhs)
    {
        assert(lhs != 0);
        assert(rhs != 0);
        return lhs->tag() < rhs->tag();
    }

    TiffComponent::AutoPtr newTiffDirectory(uint16_t tag,
                                            const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffDirectory(tag, ts->newGroup_));
    }

    TiffComponent::AutoPtr newTiffEntry(uint16_t tag,
                                        const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffEntry(tag, ts->newGroup_));
    }

    TiffComponent::AutoPtr newTiffSubIfd(uint16_t tag,
                                         const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffSubIfd(tag,
                                                     ts->group_,
                                                     ts->newGroup_));
    }

    TiffComponent::AutoPtr newTiffMnEntry(uint16_t tag,
                                          const TiffStructure* ts)
    {
        assert(ts);
        return TiffComponent::AutoPtr(new TiffMnEntry(tag,
                                                      ts->group_,
                                                      ts->newGroup_));
    }

}                                       // namespace Exiv2