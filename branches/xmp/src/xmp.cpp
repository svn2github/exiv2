// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2007 Andreas Huggel <ahuggel@gmx.net>
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
  File:      xmp.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   13-July-07, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "xmp.hpp"
#include "types.hpp"
#include "error.hpp"
#include "value.hpp"
#include "properties.hpp"

// + standard includes
#include <iostream>
#include <algorithm>
#include <cassert>
#include <string>

// Adobe XMP Toolkit
#ifdef EXV_HAVE_XMP_TOOLKIT
# define TXMP_STRING_TYPE std::string
# include <XMP.hpp>
# include <XMP.incl_cpp>
#endif // EXV_HAVE_XMP_TOOLKIT

// *****************************************************************************
namespace {
    //! Unary predicate that matches an Xmpdatum by key
    class FindXmpdatum {
    public:
        //! Constructor, initializes the object with key
        FindXmpdatum(const Exiv2::XmpKey& key)
            : key_(key.key()) {}
        /*!
          @brief Returns true if prefix and property of the argument
                 Xmpdatum are equal to that of the object.
        */
        bool operator()(const Exiv2::Xmpdatum& xmpdatum) const
            { return key_ == xmpdatum.key(); }

    private:
        std::string key_;

    }; // class FindXmpdatum

#ifdef EXV_HAVE_XMP_TOOLKIT
    //! Convert XMP Toolkit struct option bit to Value::XmpStruct
    Exiv2::XmpValue::XmpStruct xmpStruct(const XMP_OptionBits& opt);

    //! Convert Value::XmpStruct to XMP Toolkit array option bits
    XMP_OptionBits xmpOptionBits(Exiv2::XmpValue::XmpStruct xs);

    //! Convert XMP Toolkit array option bits to Value::XmpArrayType
    Exiv2::XmpValue::XmpArrayType xmpArrayType(const XMP_OptionBits& opt);

    //! Convert Value::XmpArrayType to XMP Toolkit array option bits
    XMP_OptionBits xmpOptionBits(Exiv2::XmpValue::XmpArrayType xat);

#define DEBUG
# ifdef DEBUG
    //! Print information about a parsed XMP node
    void printNode(const std::string& schemaNs,
                   const std::string& propPath,
                   const std::string& propValue,
                   const XMP_OptionBits& opt);
# endif // DEBUG
#endif // EXV_HAVE_XMP_TOOLKIT

    //! Make an XMP key from a schema namespace and property path
    Exiv2::XmpKey::AutoPtr makeXmpKey(const std::string& schemaNs,
                                      const std::string& propPath);
}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! @cond IGNORE

    //! Internal Pimpl structure with private members and data of class Xmpdatum.
    struct Xmpdatum::Impl {
        Impl(const XmpKey& key, const Value* pValue);  //!< Constructor
        Impl(const Impl& rhs);                         //!< Copy constructor
        Impl& operator=(const Impl& rhs);              //!< Assignment

        // DATA
        XmpKey::AutoPtr key_;                          //!< Key
        Value::AutoPtr  value_;                        //!< Value
    };
    //! @endcond

    Xmpdatum::Impl::Impl(const XmpKey& key, const Value* pValue)
        : key_(key.clone())
    {
        if (pValue) value_ = pValue->clone();
    }

    Xmpdatum::Impl::Impl(const Impl& rhs)
    {
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy
    }

    Xmpdatum::Impl::Impl& Xmpdatum::Impl::operator=(const Impl& rhs)
    {
        if (this == &rhs) return *this;
        key_.reset();
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy
        value_.reset();
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy
        return *this;
    }

    Xmpdatum::Xmpdatum(const XmpKey& key, const Value* pValue)
        : p_(new Impl(key, pValue))
    {
    }

    Xmpdatum::Xmpdatum(const Xmpdatum& rhs)
        : Metadatum(rhs), p_(new Impl(*rhs.p_))
    {
    }

    Xmpdatum& Xmpdatum::operator=(const Xmpdatum& rhs)
    {
        if (this == &rhs) return *this;
        Metadatum::operator=(rhs);
        *p_ = *rhs.p_;
        return *this;
    }

    Xmpdatum::~Xmpdatum()
    {
        delete p_;
    }

    std::string Xmpdatum::key() const
    {
        return p_->key_.get() == 0 ? "" : p_->key_->key(); 
    }

    std::string Xmpdatum::groupName() const
    {
        return p_->key_.get() == 0 ? "" : p_->key_->groupName();
    }

    std::string Xmpdatum::tagName() const
    {
        return p_->key_.get() == 0 ? "" : p_->key_->tagName();
    }

    std::string Xmpdatum::tagLabel() const
    {
        return p_->key_.get() == 0 ? "" : p_->key_->tagLabel();
    }

    uint16_t Xmpdatum::tag() const
    {
        return p_->key_.get() == 0 ? 0 : p_->key_->tag();
    }

    TypeId Xmpdatum::typeId() const
    {
        return p_->value_.get() == 0 ? invalidTypeId : p_->value_->typeId();
    }

    const char* Xmpdatum::typeName() const
    {
        return TypeInfo::typeName(typeId());
    }

    long Xmpdatum::count() const
    {
        return p_->value_.get() == 0 ? 0 : p_->value_->count();
    }

    long Xmpdatum::size() const
    {
        return p_->value_.get() == 0 ? 0 : p_->value_->size();
    }

    std::string Xmpdatum::toString() const
    {
        return p_->value_.get() == 0 ? "" : p_->value_->toString();
    }

    std::string Xmpdatum::toString(long n) const
    {
        return p_->value_.get() == 0 ? "" : p_->value_->toString(n);
    }

    long Xmpdatum::toLong(long n) const
    {
        return p_->value_.get() == 0 ? -1 : p_->value_->toLong(n);
    }

    float Xmpdatum::toFloat(long n) const
    {
        return p_->value_.get() == 0 ? -1 : p_->value_->toFloat(n); 
    }

    Rational Xmpdatum::toRational(long n) const
    {
        return p_->value_.get() == 0 ? Rational(-1, 1) : p_->value_->toRational(n);
    }

    Value::AutoPtr Xmpdatum::getValue() const
    {
        return p_->value_.get() == 0 ? Value::AutoPtr(0) : p_->value_->clone(); 
    }

    const Value& Xmpdatum::value() const
    {
        if (p_->value_.get() == 0) throw Error(8);
        return *p_->value_;
    }

    long Xmpdatum::copy(byte* /*buf*/, ByteOrder /*byteOrder*/) const
    {
        throw Error(34, "Xmpdatum::copy");
        return 0;
    }

    Xmpdatum& Xmpdatum::operator=(const uint16_t& value)
    {
        UShortValue::AutoPtr v(new UShortValue);
        v->value_.push_back(value);
        p_->value_ = v;
        return *this;
    }

    Xmpdatum& Xmpdatum::operator=(const std::string& value)
    {
        setValue(value);
        return *this;
    }

    Xmpdatum& Xmpdatum::operator=(const Value& value)
    {
        setValue(&value);
        return *this;
    }

    void Xmpdatum::setValue(const Value* pValue)
    {
        p_->value_.reset();
        if (pValue) p_->value_ = pValue->clone();
    }

    void Xmpdatum::setValue(const std::string& value)
    {
        // Todo: What's the correct default? Adjust doc
        if (p_->value_.get() == 0) {
            assert(0 != p_->key_.get());
            TypeId type = XmpProperties::propertyType(*p_->key_.get());
            p_->value_ = Value::create(type);
        }
        p_->value_->read(value);
    }

    Xmpdatum& XmpData::operator[](const std::string& key)
    {
        XmpKey xmpKey(key);
        iterator pos = findKey(xmpKey);
        if (pos == end()) {
            add(Xmpdatum(xmpKey));
            pos = findKey(xmpKey);
        }
        return *pos;
    }

    int XmpData::add(const XmpKey& key, Value* value)
    {
        return add(Xmpdatum(key, value));
    }

    int XmpData::add(const Xmpdatum& xmpDatum)
    {
        xmpMetadata_.push_back(xmpDatum);
        return 0;
    }

    XmpData::const_iterator XmpData::findKey(const XmpKey& key) const
    {
        return std::find_if(xmpMetadata_.begin(), xmpMetadata_.end(),
                            FindXmpdatum(key));
    }

    XmpData::iterator XmpData::findKey(const XmpKey& key)
    {
        return std::find_if(xmpMetadata_.begin(), xmpMetadata_.end(),
                            FindXmpdatum(key));
    }

    void XmpData::clear()
    {
        xmpMetadata_.clear();
    }

    void XmpData::sortByKey()
    {
        std::sort(xmpMetadata_.begin(), xmpMetadata_.end(), cmpMetadataByKey);
    }

    XmpData::const_iterator XmpData::begin() const 
    {
        return xmpMetadata_.begin(); 
    }

    XmpData::const_iterator XmpData::end() const
    {
        return xmpMetadata_.end(); 
    }

    bool XmpData::empty() const 
    {
        return count() == 0;
    }

    long XmpData::count() const
    {
        return static_cast<long>(xmpMetadata_.size());
    }

    XmpData::iterator XmpData::begin()
    {
        return xmpMetadata_.begin();
    }

    XmpData::iterator XmpData::end()
    {
        return xmpMetadata_.end();
    }

    XmpData::iterator XmpData::erase(XmpData::iterator pos)
    {
        return xmpMetadata_.erase(pos);
    }

    bool XmpParser::initialized_ = false;

    bool XmpParser::initialize()
    {
        if (!initialized_) {
            initialized_ = SXMPMeta::Initialize();
        }
        return initialized_;
    }

    void XmpParser::terminate()
    {
        if (initialized_) {
            SXMPMeta::Terminate();
            initialized_ = false;
        }
    }

#ifdef EXV_HAVE_XMP_TOOLKIT
    int XmpParser::decode(      XmpData&     xmpData,
                          const std::string& xmpPacket)
    { try {
        xmpData.clear();

        if (!initialize()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "XMP Toolkit initialization failed.\n";
#endif
            return 2;
        }

        SXMPMeta meta(xmpPacket.data(), xmpPacket.size());
        SXMPIterator iter(meta);
        std::string schemaNs, propPath, propValue;
        XMP_OptionBits opt;
        while (iter.Next(&schemaNs, &propPath, &propValue, &opt)) {
#ifdef DEBUG
            printNode(schemaNs, propPath, propValue, opt);
#endif
            if (XMP_PropIsAlias(opt)) {
                // Todo: What should we do with these? Skip for now
                continue;
            }
            if (XMP_NodeIsSchema(opt)) {
                // Register unknown namespaces with Exiv2 
                // (Namespaces are automatically registered with the XMP Toolkit)
                if (XmpProperties::prefix(schemaNs).empty()) {
                    std::string prefix;
                    bool ret = meta.GetNamespacePrefix(schemaNs.c_str(), &prefix);
                    if (!ret) throw Exiv2::Error(45, schemaNs);
                    prefix = prefix.substr(0, prefix.size() - 1);
                    XmpProperties::registerNs(schemaNs, prefix);
                }
                continue;
            }
            XmpKey::AutoPtr key = makeXmpKey(schemaNs, propPath);
            if (XMP_ArrayIsAltText(opt)) {
                // Read Lang Alt property
                LangAltValue::AutoPtr val(new LangAltValue);
                XMP_Index count = meta.CountArrayItems(schemaNs.c_str(), propPath.c_str());
                while (count-- > 0) {
                    // Get the text
                    bool haveNext = iter.Next(&schemaNs, &propPath, &propValue, &opt);
#ifdef DEBUG
                    printNode(schemaNs, propPath, propValue, opt);
#endif
                    if (   !haveNext
                        || !XMP_PropIsSimple(opt)
                        || !XMP_PropHasLang(opt)) {
                        throw Error(41, propPath, opt);
                    }
                    const std::string text = propValue;
                    // Get the language qualifier
                    haveNext = iter.Next(&schemaNs, &propPath, &propValue, &opt);
#ifdef DEBUG
                    printNode(schemaNs, propPath, propValue, opt);
#endif
                    if (   !haveNext
                        || !XMP_PropIsSimple(opt)
                        || !XMP_PropIsQualifier(opt)
                        || propPath.substr(propPath.size() - 8, 8) != "xml:lang") {
                        throw Error(42, propPath, opt);
                    }
                    val->value_[propValue] = text;
                }
                xmpData.add(*key.get(), val.get());
                continue;
            }
            if (    XMP_PropIsArray(opt)
                && !XMP_PropHasQualifiers(opt)
                && !XMP_ArrayIsAltText(opt)) {
                // Check if all elements are simple
                bool simpleArray = true;
                SXMPIterator aIter(meta, schemaNs.c_str(), propPath.c_str());
                std::string aSchemaNs, aPropPath, aPropValue;
                XMP_OptionBits aOpt;
                while (aIter.Next(&aSchemaNs, &aPropPath, &aPropValue, &aOpt)) {
                    if (propPath == aPropPath) continue;
                    if (   !XMP_PropIsSimple(aOpt)
                        ||  XMP_PropHasQualifiers(aOpt)
                        ||  XMP_PropIsQualifier(aOpt)
                        ||  XMP_NodeIsSchema(aOpt)
                        ||  XMP_PropIsAlias(aOpt)) {
                        simpleArray = false;

                        std::cerr << "NOT SO SIMPLE ==> " << propPath << ", " << aPropPath << "\n";

                        break;
                    }
                }
                if (simpleArray) {
                    // Read the array into an XmpArrayValue
                    XmpArrayValue::AutoPtr val(new XmpArrayValue);
                    XMP_Index count = meta.CountArrayItems(schemaNs.c_str(), propPath.c_str());
                    val->setXmpArrayType(xmpArrayType(opt));
                    while (count-- > 0) {
                        iter.Next(&schemaNs, &propPath, &propValue, &opt);
#ifdef DEBUG
                        printNode(schemaNs, propPath, propValue, opt);
#endif
                        val->read(propValue);
                    }
                    xmpData.add(*key.get(), val.get());
                    continue;
                }
            }
            XmpTextValue::AutoPtr val(new XmpTextValue);
            if (   XMP_PropIsStruct(opt)
                || XMP_PropIsArray(opt)) {
                // Create a metadatum with only XMP options
                val->setXmpArrayType(xmpArrayType(opt));
                val->setXmpStruct(xmpStruct(opt));
                xmpData.add(*key.get(), val.get());
                continue;
            }
            if (   XMP_PropIsSimple(opt)
                || XMP_PropIsQualifier(opt)) {
                val->read(propValue);
                xmpData.add(*key.get(), val.get());
                continue;
            }
            // Don't let any node go by unnoticed
            throw Error(39, key->key(), opt);
        } // iterate through all XMP nodes

        return 0;
    }
    catch (const XMP_Error& e) {
#ifndef SUPPRESS_WARNINGS
        std::cerr << Error(40, e.GetID(), e.GetErrMsg()) << "\n";
#endif
        xmpData.clear();
        return 3;
    }} // XmpParser::decode
#else
    int XmpParser::decode(      XmpData&     /*xmpData*/,
                          const std::string& xmpPacket)
    {
        if (!xmpPacket.empty()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: XMP toolkit support not compiled in.\n";
#endif
        }
        return 1;
    } // XmpParser::decode
#endif // !EXV_HAVE_XMP_TOOLKIT

#ifdef EXV_HAVE_XMP_TOOLKIT
    int XmpParser::encode(      std::string& xmpPacket,
                           const XmpData&     xmpData)
    { try {
        xmpPacket.clear();

        if (!initialize()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "XMP Toolkit initialization failed.\n";
#endif
            return 2;
        }

        SXMPMeta meta;

        for (XmpData::const_iterator i = xmpData.begin(); i != xmpData.end(); ++i) {

            std::string ns = XmpProperties::ns(i->groupName());

            // Todo: Make sure the namespace is registered with XMP-SDK

            // Todo: What about structure namespaces?

            XMP_OptionBits options = 0;

            if (i->typeId() == langAlt) {
                // Encode Lang Alt property
                const LangAltValue* la = dynamic_cast<const LangAltValue*>(&i->value());
                if (la == 0) throw Error(43, i->key());
                int idx = 1;
                for (LangAltValue::ValueType::const_iterator k = la->value_.begin();
                     k != la->value_.end(); ++k) {
                    meta.AppendArrayItem(ns.c_str(), i->tagName().c_str(), kXMP_PropArrayIsAltText, k->second.c_str());
                    const std::string item = i->tagName() + "[" + toString(idx++) + "]";
                    meta.SetQualifier(ns.c_str(), item.c_str(), kXMP_NS_XML, "lang", k->first.c_str());
                }
                continue;
            }
            // Todo: Xmpdatum should have an XmpValue, not a Value
            const XmpValue* val = dynamic_cast<const XmpValue*>(&i->value());
            assert(val);
            options =   xmpOptionBits(val->xmpArrayType())
                      | xmpOptionBits(val->xmpStruct());
            if (i->typeId() == xmpArray) {
                meta.SetProperty(ns.c_str(), i->tagName().c_str(), 0, options);
                for (int idx = 0; idx < i->count(); ++idx) {
                    const std::string item = i->tagName() + "[" + toString(idx + 1) + "]";

                    //-ahu
                    std::cerr << "Element " << item << " = " << i->toString(idx) << "\n";

                    meta.SetProperty(ns.c_str(), item.c_str(), i->toString(idx).c_str());
                }
            }
            if (i->typeId() == xmpText) {
                if (i->count() == 0) {
                    meta.SetProperty(ns.c_str(), i->tagName().c_str(), 0, options);
                }
                else {
                    meta.SetProperty(ns.c_str(), i->tagName().c_str(), i->toString(0).c_str(), options);
                }
            }
        }
        meta.SerializeToBuffer(&xmpPacket, kXMP_UseCompactFormat);

        return 0;
    }
    catch (const XMP_Error& e) {
#ifndef SUPPRESS_WARNINGS
        std::cerr << Error(40, e.GetID(), e.GetErrMsg()) << "\n";
#endif
        xmpPacket.clear();
        return 3;
    }} // XmpParser::decode
#else
    void XmpParser::encode(      std::string& /*xmpPacket*/,
                           const XmpData&     xmpData)
    {
        if (!xmpData.empty()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: XMP toolkit support not compiled in.\n";
#endif
        return 1;
        }
    } // XmpParser::encode
#endif // !EXV_HAVE_XMP_TOOLKIT

    // *************************************************************************
    // free functions
    std::ostream& operator<<(std::ostream& os, const Xmpdatum& md)
    {
        return os << md.value();
    }

}                                       // namespace Exiv2

// *****************************************************************************
// local definitions
namespace {

#ifdef EXV_HAVE_XMP_TOOLKIT
    Exiv2::XmpValue::XmpStruct xmpStruct(const XMP_OptionBits& opt)
    {
        Exiv2::XmpValue::XmpStruct var(Exiv2::XmpValue::xsNone);
        if (XMP_PropIsStruct(opt)) {
            var = Exiv2::XmpValue::xsStruct;
        }
        return var;
    }

    XMP_OptionBits xmpOptionBits(Exiv2::XmpValue::XmpStruct xs)
    {
        XMP_OptionBits var(0);
        switch (xs) {
        case Exiv2::XmpValue::xsNone:
            break;
        case Exiv2::XmpValue::xsStruct:
            XMP_SetOption(var, kXMP_PropValueIsStruct);
            break;
        }
        return var;
    }

    Exiv2::XmpValue::XmpArrayType xmpArrayType(const XMP_OptionBits& opt)
    {
        Exiv2::XmpValue::XmpArrayType var(Exiv2::XmpValue::xaNone);
        if (XMP_PropIsArray(opt)) {
            if (XMP_ArrayIsAlternate(opt))      var = Exiv2::XmpValue::xaAlt;
            else if (XMP_ArrayIsOrdered(opt))   var = Exiv2::XmpValue::xaSeq;
            else if (XMP_ArrayIsUnordered(opt)) var = Exiv2::XmpValue::xaBag;
        }
        return var;
    }

    XMP_OptionBits xmpOptionBits(Exiv2::XmpValue::XmpArrayType xat)
    {
        XMP_OptionBits var(0);
        switch (xat) {
        case Exiv2::XmpValue::xaNone:
            break;
        case Exiv2::XmpValue::xaAlt:
            XMP_SetOption(var, kXMP_PropValueIsArray);
            XMP_SetOption(var, kXMP_PropArrayIsAlternate);
            break;
        case Exiv2::XmpValue::xaSeq:
            XMP_SetOption(var, kXMP_PropValueIsArray);
            XMP_SetOption(var, kXMP_PropArrayIsOrdered);
            break;
        case Exiv2::XmpValue::xaBag:
            XMP_SetOption(var, kXMP_PropValueIsArray);
            break;
        }
        return var;
    }

# ifdef DEBUG
    void printNode(const std::string& schemaNs,
                   const std::string& propPath,
                   const std::string& propValue,
                   const XMP_OptionBits& opt)
    {
        static bool first = true;
        if (first) {
            first = false;
            std::cout << "ashisaas\n"
                      << "lcqqtrti\n";
        }
        enum { alia=0, sche, hasq, isqu, stru, arra, lang, simp, len };
        std::string opts(len, '.');
        if (XMP_PropIsAlias(opt))       opts[alia] = 'X';
        if (XMP_NodeIsSchema(opt))      opts[sche] = 'X';
        if (XMP_PropHasQualifiers(opt)) opts[hasq] = 'X';
        if (XMP_PropIsQualifier(opt))   opts[isqu] = 'X';
        if (XMP_PropIsStruct(opt))      opts[stru] = 'X';
        if (XMP_PropIsArray(opt))       opts[arra] = 'X';
        if (XMP_ArrayIsAltText(opt))    opts[lang] = 'X';
        if (XMP_PropIsSimple(opt))      opts[simp] = 'X';

        std::cout << opts << " ";
        if (opts[sche] == 'X') {
            std::cout << "ns=" << schemaNs;
        }
        else {
            std::cout << propPath << " = " << propValue;
        }
        std::cout << std::endl;
    }
# endif // DEBUG
#endif // EXV_HAVE_XMP_TOOLKIT

    Exiv2::XmpKey::AutoPtr makeXmpKey(const std::string& schemaNs,
                                      const std::string& propPath)
    {
        std::string property;
        std::string::size_type idx = propPath.find(':');
        if (idx == std::string::npos) {
            throw Exiv2::Error(44, propPath, schemaNs);
        }
        // Don't worry about out_of_range, XMP parser takes care of this
        property = propPath.substr(idx + 1);
        std::string prefix = Exiv2::XmpProperties::prefix(schemaNs);
        if (prefix.empty()) {
            throw Exiv2::Error(47, propPath, schemaNs);
        }
        return Exiv2::XmpKey::AutoPtr(new Exiv2::XmpKey(prefix, property));
    } // makeXmpKey

}
