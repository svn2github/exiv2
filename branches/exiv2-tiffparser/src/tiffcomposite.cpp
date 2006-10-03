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

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>

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
        : TiffEntryBase(tag, group), newGroup_(newGroup) 
    {
        this->setTypeId(unsignedLong);
    }

    TiffArrayEntry::TiffArrayEntry(uint16_t tag, 
                                   uint16_t group,
                                   uint16_t elGroup,
                                   TypeId elTypeId)
        : TiffEntryBase(tag, group),
          elSize_(static_cast<uint16_t>(TypeInfo::typeSize(elTypeId))),
          elGroup_(elGroup) 
    {
        this->setTypeId(elTypeId);
    }

    TiffDirectory::~TiffDirectory()
    {
        Components::iterator b = components_.begin();
        Components::iterator e = components_.end();
        for (Components::iterator i = b; i != e; ++i) {
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
        Components::iterator b = elements_.begin();
        Components::iterator e = elements_.end();
        for (Components::iterator i = b; i != e; ++i) {
            delete *i;
        }
    } // TiffArrayEntry::~TiffArrayEntry

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
        if (tiffPath.size() > 1) {
            if (ts->extendedTag_ == Tag::next) {
                tc = pNext_;
            }
            else {
                for (Components::iterator i = components_.begin(); 
                     i != components_.end(); ++i) {
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
        assert(!tiffPath.empty());
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
        if (visitor.go()) doAccept(visitor);    // one for NVI :)
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
        Components::const_iterator b = components_.begin();
        Components::const_iterator e = components_.end();
        for (Components::const_iterator i = b; visitor.go() && i != e; ++i) {
            (*i)->accept(visitor);
        }
        if (visitor.go()) visitor.visitDirectoryNext(this);
        if (pNext_) pNext_->accept(visitor);
        if (visitor.go()) visitor.visitDirectoryEnd(this);

    } // TiffDirectory::doAccept

    void TiffSubIfd::doAccept(TiffVisitor& visitor)
    {
        visitor.visitSubIfd(this);
        for (Ifds::iterator i = ifds_.begin(); visitor.go() && i != ifds_.end(); ++i) {
            (*i)->accept(visitor);
        }
    } // TiffSubIfd::doAccept

    void TiffMnEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitMnEntry(this);
        if (mn_) mn_->accept(visitor);
        if (!visitor.go()) {
            delete mn_;
            mn_ = 0;
            visitor.setGo(true);
        }
    } // TiffMnEntry::doAccept

    void TiffArrayEntry::doAccept(TiffVisitor& visitor)
    {
        visitor.visitArrayEntry(this);
        Components::const_iterator b = elements_.begin();
        Components::const_iterator e = elements_.end();
        for (Components::const_iterator i = b; visitor.go() && i != e; ++i) {
            (*i)->accept(visitor);
        }
    } // TiffArrayEntry::doAccept

    void TiffArrayElement::doAccept(TiffVisitor& visitor)
    {
        visitor.visitArrayElement(this);
    } // TiffArrayElement::doAccept

    // *************************************************************************
    // free functions

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
