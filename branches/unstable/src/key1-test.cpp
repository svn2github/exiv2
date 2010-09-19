// ***************************************************************** -*- C++ -*-
/*
  Abstract : Key unit tests

  File     : key-test.cpp
  Version  : $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History  : 24-Aug-04, ahu: created

 */
// *****************************************************************************
// included header files
#include "exiv2.hpp"
#include "key.hpp"

#include <iostream>
#include <string>

using namespace Exiv2;

int main()
try {
    int tc = 0;
    int rc = 0;

    std::string key("Xmp.dc.source");
    Key1 ek(key);

    // operator<<
    tc += 1;
    std::ostringstream os;
    os << ek;
    if (os.str() != key) {
        std::cout << "Testcase 1 failed (operator<<)" << std::endl;
        rc += 1;
    }
    // familyName
    tc += 1;
    if (std::string(ek.familyName()) != "Xmp") {
        std::cout << "Testcase 2 failed (familyName)" << std::endl;
        rc += 1;
    }
    // groupName
    tc += 1;
    if (ek.groupName() != "dc") {
        std::cout << "Testcase 3 failed (groupName)" << std::endl;
        rc += 1;
    }
    // tagName
    tc += 1;
    if (ek.tagName() != "source") {
        std::cout << "Testcase 4 failed (tagName)" << std::endl;
        rc += 1;
    }

    // -----

    // Copy constructor
    ek.setIdx(33);
    Key1 ek2(ek);

    tc += 1;
    if (ek2.idx() != 33 || ek2.family() != ek.family()) {
        std::cout << "Testcase 5 failed (operator<<)" << std::endl;
        rc += 1;
    }

    // operator<<
    tc += 1;
    std::ostringstream os2;
    os2 << ek2;
    if (os2.str() != key) {
        std::cout << "Testcase 6 failed (operator<<)" << std::endl;
        rc += 1;
    }
    // familyName
    tc += 1;
    if (std::string(ek2.familyName()) != "Xmp") {
        std::cout << "Testcase 7 failed (familyName)" << std::endl;
        rc += 1;
    }
    // groupName
    tc += 1;
    if (ek2.groupName() != "dc") {
        std::cout << "Testcase 8 failed (groupName)" << std::endl;
        rc += 1;
    }
    // tagName
    tc += 1;
    if (ek2.tagName() != "source") {
        std::cout << "Testcase 9 failed (tagName)" << std::endl;
        rc += 1;
    }

    // -----

    // Assignment of a key with a different family
    key = "Exif.Image.Make";
    ek = Key1(key);

    // operator<<
    tc += 1;
    std::ostringstream os3;
    os3 << ek;
    if (os3.str() != key) {
        std::cout << "Testcase 10 failed (operator<<)" << std::endl;
        rc += 1;
    }
    // familyName
    tc += 1;
    if (std::string(ek.familyName()) != "Exif") {
        std::cout << "Testcase 11 failed (familyName)" << std::endl;
        rc += 1;
    }
    // groupName
    tc += 1;
    if (ek.groupName() != "Image") {
        std::cout << "Testcase 12 failed (groupName)" << std::endl;
        rc += 1;
    }
    // tagName
    tc += 1;
    if (ek.tagName() != "Make") {
        std::cout << "Testcase 13 failed (tagName)" << std::endl;
        rc += 1;
    }

    // -----

    // Assignment of same-family key
    ek = Key1("Exif.Image.0x0110");
    tc += 1;
    if (ek.key() != "Exif.Image.Model") {
        std::cout << "Testcase 14 failed (converted key), key = " << ek.key() << std::endl;
        rc += 1;
    }
    tc += 1;
    if (ek.tagName() != "Model") {
        std::cout << "Testcase 15 failed (converted tagName)" << std::endl;
        rc += 1;
    }

    // -----

    Key1 ek5(0x0007, "Nikon3");
    tc += 1;
    if (ek5.key() != "Exif.Nikon3.Focus") {
        std::cout << "Testcase 16 failed (converted key)" << std::endl;
        rc += 1;
    }
    tc += 1;
    if (ek5.tagName() != "Focus") {
        std::cout << "Testcase 17 failed (converted tagName)" << std::endl;
        rc += 1;
    }

    // -----

    Key1 ik1("Iptc.Envelope.0x0005");
    tc += 1;
    if (ik1.key() != "Iptc.Envelope.Destination") {
        std::cout << "Testcase 18 failed (converted Iptc key)" << std::endl;
        rc += 1;
    }
    tc += 1;
    if (ik1.tagName() != "Destination") {
        std::cout << "Testcase 19 failed (converted tagName)" << std::endl;
        rc += 1;
    }
    tc += 1;
    if (ik1.groupName() != "Envelope") {
        std::cout << "Testcase 20 failed (converted recordName)" << std::endl;
        rc += 1;
    }

    // -----

    Key1 ik2(0xabcd, 0x1234);
    tc += 1;
    if (ik2.key() != "Iptc.0x1234.0xabcd") {
        std::cout << "Testcase 21 failed (unknown Iptc key)" << std::endl;
        rc += 1;
    }
    tc += 1;
    if (ik2.tagName() != "0xabcd") {
        std::cout << "Testcase 22 failed (converted tagName)" << std::endl;
        rc += 1;
    }
    tc += 1;
    if (ik2.groupName() != "0x1234") {
        std::cout << "Testcase 23 failed (converted recordName)" << std::endl;
        rc += 1;
    }

    // -----

    if (rc == 0) {
        std::cout << "All " << tc << " testcases passed." << std::endl;
    }
    else {
        std::cout << rc << " of " << tc << " testcases failed." << std::endl;
    }
    return 0;
}
catch (const Exiv2::AnyError& e) {
    std::cout << "Caught exception " << e << "\n";
    return 1;
}
