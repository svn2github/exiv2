// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2010 Andreas Huggel <ahuggel@gmx.net>
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
/*!
  @file    metadatum.hpp
  @brief   Provides class Tag1, a basic type for a metadatum.
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Brad Schick (brad)
           <a href="mailto:brad@robotbattle.com">brad@robotbattle.com</a>
  @date    09-Jan-04, ahu: created<BR>
           31-Jul-04, brad: isolated as a component<BR>
           23-Aug-04, ahu: added Key
           19-Jan-10, ahu: removed Key
 */
#ifndef METADATUM_HPP_
#define METADATUM_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "key.hpp"
#include "value.hpp"

// + standard includes
#include <string>
#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class ExifData;

// *****************************************************************************
// class definitions

    /*!
      @brief A tag is a metadatum of any metadata family and consists of a Key
             and a Value and methods to access the information contained in
             those.
     */
    class EXIV2API Tag1 {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor for new tags created by an application. The %Tag is
                 created from a \em key and \em value pair. %Tag copies the \em
                 key and clones the \em value if one is provided. Alternatively,
                 a program can create an 'empty' %Tag with only a key and set
                 the value using setValue().

          @param key %Metadata %Key.
          @param pValue Pointer to the value of the %Tag.
          @throw Error if the \em key cannot be parsed and converted.
         */
        explicit Tag1(const Key1& key, const Value* pValue =0);
        //! Copy constructor
        Tag1(const Tag1& rhs);
        //! Destructor
        ~Tag1();
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator
        Tag1& operator=(const Tag1& rhs);
        /*!
          @brief Assign \em value to the %Tag1. Calls setValue(const Value*).
         */
        Tag1& operator=(const Value& value);
        /*!
          @brief Assign a bool value \em value to the %Tag1. The resulting value is a string "True" or "False". 
         */
        Tag1& operator=(const bool& value);
        /*!
          @brief Assign \em value to the %Tag1. The type of the new Value
                 is set to UShortValue.
         */
        Tag1& operator=(const uint16_t& value);
        /*!
          @brief Assign \em value to the %Tag1. The type of the new Value
                 is set to ULongValue.
         */
        Tag1& operator=(const uint32_t& value);
        /*!
          @brief Assign \em value to the %Tag1. The type of the new Value
                 is set to URationalValue.
         */
        Tag1& operator=(const URational& value);
        /*!
          @brief Assign \em value to the %Tag1. The type of the new Value
                 is set to ShortValue.
         */
        Tag1& operator=(const int16_t& value);
        /*!
          @brief Assign \em value to the %Tag1. The type of the new Value
                 is set to LongValue.
         */
        Tag1& operator=(const int32_t& value);
        /*!
          @brief Assign \em value to the %Tag1. The type of the new Value
                 is set to RationalValue.
         */
        Tag1& operator=(const Rational& value);
        /*!
          @brief Assign \em value to the %Tag1. Calls operator=(const std::string&).
         */
        Tag1& operator=(const char* value);
        /*!
          @brief Assign \em value to the %Tag1. Calls operator=(const std::string&).
         */
        Tag1& operator=(const std::string& value);
        /*!
          @brief Assign a \em value of any type with an output operator to the %Tag1.
                 Calls operator=(const std::string&).
         */
        template<typename T>
        Tag1& operator=(const T& value);
        /*!
          @brief Set the value. This method copies (clones) the value pointed
                 to by pValue.
         */
        void setValue(const Value* pValue);
        /*!
          @brief Set the value to the string buf.
                 Uses Value::read(const std::string& buf). If the metadatum does
                 not have a value yet, then one is created. 
                 Return 0 if the value was read successfully.
         */
        int setValue(const std::string& buf);
        /*!
          @brief Set the data area of the value by copying (cloning) the buffer
                 pointed to by \em buf.

          Values may have a data area, which can contain additional
          information besides the actual value. This method is used to set such
          a data area. The data area is used for certain Exif tags.

          @param buf Pointer to the source data area
          @param len Size of the data area
          @return Return -1 if the %Tag1 does not have a value or the value has
                  no data area, else 0.
         */
        int setDataArea(const byte* buf, long len);
        //@}

        //! @name Accessors
        //@{
        //! See Key1::key()
        std::string key() const                 { return key_.key(); }
        //! See Key1::familyName()
        const char* familyName() const          { return key_.familyName(); }
        //! See Key1::family()
        MetadataId family() const               { return key_.family(); }
        //! See Key1::groupName()
        std::string groupName() const           { return key_.groupName(); }
        //! See Key1::group()
        int group() const                       { return key_.group(); }
        //! See Key1::tagName()
        std::string tagName() const             { return key_.tagName(); }
        //! See Key1::tag()
        uint16_t tag() const                    { return key_.tag(); }
        //! See Key1::tagLabel()
        std::string tagLabel() const            { return key_.tagLabel(); }
        //! See Key1::tagDesc()
        std::string tagDesc() const             { return key_.tagDesc(); }
        //! See Key1::defaultTypeId()
        TypeId defaultTypeId() const            { return key_.defaultTypeId(); }
        //! See Key1::idx()
        int idx() const                         { return key_.idx(); }

        //! Return invalidTypeId if value is not set, else Value::typeId()
        TypeId typeId() const                   { return pValue_ == 0 ? invalidTypeId : pValue_->typeId(); }
        //! See TypeInfo::typeName()
        const char* typeName() const            { return TypeInfo::typeName(typeId()); }
        //! See TypeInfo::typeSize()
        long typeSize() const                   { return TypeInfo::typeSize(typeId()); }
        //! Return 0 if value is not set, else Value::count()
        long count() const                      { return pValue_ == 0 ? 0 : pValue_->count(); }
        //! Return 0 if value is not set, else Value::size()
        long size() const                       { return pValue_ == 0 ? 0 : pValue_->size(); }
        //! Return empty string if value is not set, else Value::toString()
        std::string toString() const            { return pValue_ == 0 ? "" : pValue_->toString(); }
        //! Return empty string if value is not set, else Value::toString(long n)
        std::string toString(long n) const      { return pValue_ == 0 ? "" : pValue_->toString(n); }
        //! Return 0 if value is not set, else Value::toLong()
        long toLong(long n =0) const            { return pValue_ == 0 ? 0 : pValue_->toLong(n); }
        //! Return 0.0 if value is not set, else Value::toFloat()
        float toFloat(long n =0) const          { return pValue_ == 0 ? 0.0 : pValue_->toFloat(n); }
        //! Return 0/0 if value is not set, else Value::toRational()
        Rational toRational(long n =0) const    { return pValue_ == 0 ? Rational(0, 0) : pValue_->toRational(n); }
        /*!
          @brief Write value to a data buffer and return the number
                 of bytes written.

          The user must ensure that the buffer has enough memory. Otherwise
          the call results in undefined behaviour.

          @param buf Data buffer to write to.
          @param byteOrder Applicable byte order (little or big endian).
          @return Number of characters written.
        */
        long copy(byte* buf, ByteOrder byteOrder) const { return pValue_ == 0 ? 0 : pValue_->copy(buf, byteOrder); }

        /*!
          @brief Write the interpreted value to a string.

          Implemented in terms of write(), see there.
         */
        std::string print(const ExifData* pMetadata =0) const;
        /*!
          @brief Write the interpreted value to an output stream, return
                 the stream.

          The method takes an optional pointer to a metadata container.
          Pretty-print functions may use that to refer to other metadata as it
          is sometimes not sufficient to know only the value of the metadatum
          that should be interpreted. Thus, it is advisable to always call this
          method with a pointer to the metadata container if possible.

          This functionality is currently only implemented for Exif tags.
          The pointer is ignored when used to write IPTC datasets or XMP
          properties.

          Without the optional metadata pointer, you do not usually have to use
          this function; it is used for the implementation of the output
          operator for %Tag, operator<<(std::ostream& os, const Tag1& md).

          See also print(), which prints the interpreted value to a string.
         */
        std::ostream& write(std::ostream& os, const ExifData* pMetadata =0) const;
        /*!
          @brief Return an auto-pointer to a copy (clone) of the value. The
                 caller owns this copy and the auto-poiner ensures that it will
                 be deleted.

          This method is provided for users who need full control over the
          value. A caller may, e.g., downcast the pointer to the appropriate
          subclass of Value to make use of the interface of the subclass to set
          or modify its contents.

          @return An auto-pointer containing a pointer to a copy (clone) of the
                  value, 0 if the value is not set.
         */
        Value::AutoPtr getValue() const         { return pValue_ == 0 ? Value::AutoPtr(0) : pValue_->clone(); }
        /*!
          @brief Return a constant reference to the value.

          This method is provided for convenient and versatile output of the
          value which can (to some extent) be formatted through standard stream
          manipulators.  Do not attempt to write to the value through this
          reference. An Error is thrown if the value is not set; as an
          alternative to catching it, one can use count() to check if there
          is any data before calling this method.

          @return A constant pointer to the value.
          @throw Error if the value is not set.
         */
        const Value& value() const;
        //! Return the size of the data area.
        long sizeDataArea() const               { return pValue_ == 0 ? 0 : pValue_->sizeDataArea(); }
        /*!
          @brief Return a copy of the data area of the value. The caller owns
                 this copy and %DataBuf ensures that it will be deleted.

          Values may have a data area, which can contain additional
          information besides the actual value. This method is used to access
          such a data area. The data area is used for certain Exif tags.

          @return A %DataBuf containing a copy of the data area or an empty
                  %DataBuf if the value does not have a data area assigned or the
                  value is not set.
         */
        DataBuf dataArea() const                { return pValue_ == 0 ? DataBuf(0, 0) : pValue_->dataArea(); }
        //@}

    private:
        /*!
          @brief Set the value to \em value. A ValueType\<T\> value is created
                 and set to \em value. If the metadatum already has a value, it
                 is replaced.

          This is a helper function, called from Tag1 members. It is meant to
          be used with T = (u)int16_t, (u)int32_t or (U)Rational.
        */
        template<typename T>
            void setValueHelper(const T& value)
        {
            std::auto_ptr<Exiv2::ValueType<T> > v
                = std::auto_ptr<Exiv2::ValueType<T> >(new Exiv2::ValueType<T>);
            v->value_.push_back(value);
            delete pValue_;
            pValue_ = v.release();
        }

    private:
        // DATA
        Key1   key_;                            //!< Key
        Value* pValue_;                         //!< Value

    }; // class Tag1

    /*!
      @brief Output operator for Tag types, writing the interpreted
             tag value.
     */
    inline std::ostream& operator<<(std::ostream& os, const Tag1& md)
    {
        return md.write(os);
    }

    inline Tag1& Tag1::operator=(const char* value)
    {
        return Tag1::operator=(std::string(value));
    }

    inline Tag1& Tag1::operator=(const bool& value)
    {
        return operator=(value ? "True" : "False");
    }

    template<typename T>
    Tag1& Tag1::operator=(const T& value)
    {
        setValue(Exiv2::toString(value));
        return *this;
    }

// Todo: The following class should be internal ----------------------

    //! Unary predicate that matches an Xmpdatum by key
    class FindTag1ByKey {
    public:
        //! Constructor, initializes the object with key
        FindTag1ByKey(const Exiv2::Key1& key)
            : key_(key.key()) {}
        /*!
          @brief Returns \c true if \em tag has the same key as that of this object.
        */
        bool operator()(const Exiv2::Tag1& tag) const
            { return key_ == tag.key(); }

    private:
        std::string key_;

    }; // class FindTag1ByKey

// -------------------------------------------------------------------

    /*!
      @brief Abstract base class defining the interface to access information
             related to one metadata tag.
     */
    class EXIV2API Metadatum {
    public:
        //! @name Creators
        //@{
        //! Default Constructor
        Metadatum();
        //! Copy constructor
        Metadatum(const Metadatum& rhs);
        //! Destructor
        virtual ~Metadatum();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Set the value. This method copies (clones) the value pointed
                 to by pValue.
         */
        virtual void setValue(const Value* pValue) =0;
        /*!
          @brief Set the value to the string buf.
                 Uses Value::read(const std::string& buf). If the metadatum does
                 not have a value yet, then one is created. See subclasses for
                 more details. Return 0 if the value was read successfully.
         */
        virtual int setValue(const std::string& buf) =0;
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Write the interpreted value to a string.

          Implemented in terms of write(), see there.
         */
        std::string print(const ExifData* pMetadata =0) const;
        /*!
          @brief Write value to a data buffer and return the number
                 of bytes written.

          The user must ensure that the buffer has enough memory. Otherwise
          the call results in undefined behaviour.

          @param buf Data buffer to write to.
          @param byteOrder Applicable byte order (little or big endian).
          @return Number of characters written.
        */
        virtual long copy(byte* buf, ByteOrder byteOrder) const =0;
        /*!
          @brief Write the interpreted value to an output stream, return
                 the stream.

          The method takes an optional pointer to a metadata container.
          Pretty-print functions may use that to refer to other metadata as it
          is sometimes not sufficient to know only the value of the metadatum
          that should be interpreted. Thus, it is advisable to always call this
          method with a pointer to the metadata container if possible.

          This functionality is currently only implemented for Exif tags.
          The pointer is ignored when used to write IPTC datasets or XMP
          properties.

          Without the optional metadata pointer, you do not usually have to use
          this function; it is used for the implementation of the output
          operator for %Metadatum,
          operator<<(std::ostream &os, const Metadatum &md).

          See also print(), which prints the interpreted value to a string.
         */
        virtual std::ostream& write(
                  std::ostream& os,
            const ExifData*     pMetadata =0
        ) const =0;
        /*!
          @brief Return the key of the metadatum. The key is of the form
                 'familyName.groupName.tagName'. Note however that the key
                 is not necessarily unique, e.g., an ExifData object may
                 contain multiple metadata with the same key.
         */
        virtual std::string key() const =0;
        //! Return the name of the metadata family (which is also the first part of the key)
        virtual const char* familyName() const =0;
        //! Return the name of the metadata group (which is also the second part of the key)
        virtual std::string groupName() const =0;
        //! Return the name of the tag (which is also the third part of the key)
        virtual std::string tagName() const =0;
        //! Return a label for the tag 	
        virtual std::string tagLabel() const =0;
        //! Return the tag
        virtual uint16_t tag() const =0;
        //! Return the type id of the value
        virtual TypeId typeId() const =0;
        //! Return the name of the type
        virtual const char* typeName() const =0;
        //! Return the size in bytes of one component of this type
        virtual long typeSize() const =0;
        //! Return the number of components in the value
        virtual long count() const =0;
        //! Return the size of the value in bytes
        virtual long size() const =0;
        //! Return the value as a string.
        virtual std::string toString() const =0;
        /*!
          @brief Return the <EM>n</EM>-th component of the value converted to
                 a string. The behaviour of the method is undefined if there
                 is no <EM>n</EM>-th component.
         */
        virtual std::string toString(long n) const =0;
        /*!
          @brief Return the <EM>n</EM>-th component of the value converted to long.
                 The return value is -1 if the value is not set and the behaviour
                 of the method is undefined if there is no <EM>n</EM>-th component.
         */
        virtual long toLong(long n =0) const =0;
        /*!
          @brief Return the <EM>n</EM>-th component of the value converted to float.
                 The return value is -1 if the value is not set and the behaviour
                 of the method is undefined if there is no <EM>n</EM>-th component.
         */
        virtual float toFloat(long n =0) const =0;
        /*!
          @brief Return the <EM>n</EM>-th component of the value converted to Rational.
                 The return value is -1/1 if the value is not set and the behaviour
                 of the method is undefined if there is no <EM>n</EM>-th component.
         */
        virtual Rational toRational(long n =0) const =0;
        /*!
          @brief Return an auto-pointer to a copy (clone) of the value. The
                 caller owns this copy and the auto-poiner ensures that it will
                 be deleted.

          This method is provided for users who need full control over the
          value. A caller may, e.g., downcast the pointer to the appropriate
          subclass of Value to make use of the interface of the subclass to set
          or modify its contents.

          @return An auto-pointer containing a pointer to a copy (clone) of the
                  value, 0 if the value is not set.
         */
        virtual Value::AutoPtr getValue() const =0;
        /*!
          @brief Return a constant reference to the value.

          This method is provided mostly for convenient and versatile output of
          the value which can (to some extent) be formatted through standard
          stream manipulators.  Do not attempt to write to the value through
          this reference. An Error is thrown if the value is not set; as an
          alternative to catching it, one can use count() to check if there
          is any data before calling this method.

          @return A constant reference to the value.
          @throw Error if the value is not set.
         */
        virtual const Value& value() const =0;
        //@}

    protected:
        //! @name Manipulators
        //@{
        /*!
          @brief Assignment operator. Protected so that it can only be used
                 by subclasses but not directly.
         */
        Metadatum& operator=(const Metadatum& rhs);
        //@}

    }; // class Metadatum

    /*!
      @brief Output operator for Metadatum types, writing the interpreted
             tag value.
     */
    inline std::ostream& operator<<(std::ostream& os, const Metadatum& md)
    {
        return md.write(os);
    }

    /*!
      @brief Compare two tags by their tag number. Return true if the tag number of
             lhs is less than that of rhs.
     */
    EXIV2API bool cmpTag1ByTag(const Tag1& lhs, const Tag1& rhs);
    /*!
      @brief Compare two tags by key. Return true if the key of
             lhs is less than that of rhs.
     */
    EXIV2API bool cmpTag1ByKey(const Tag1& lhs, const Tag1& rhs);

}                                       // namespace Exiv2

#endif                                  // #ifndef METADATUM_HPP_
