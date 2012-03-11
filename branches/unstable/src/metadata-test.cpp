// ***************************************************************** -*- C++ -*-
/*
  Abstract : Metadata unit tests

  File     : metadata-test.cpp
  Version  : $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History  : 23-Jan-12, ahu: created

 */
// *****************************************************************************
// included header files
#include "exiv2.hpp"

#include <iostream>
#include <string>

using namespace Exiv2;

int testFailed(int tc)
{
    std::cout << "Testcase " << tc << " failed" << std::endl;
    return 1;
}

int main()
try {
    int tc = 0;
    int rc = 0;
    Metadata md;
    Metadata::const_iterator t;

    // begin(), end() for empty container
    Metadata::const_iterator b = md.begin();
    Metadata::const_iterator e = md.end();
    tc += 1; t = md.begin(mdExif); if (t != b) rc += testFailed(tc);
    tc += 1; t = md.begin(mdIptc); if (t != b) rc += testFailed(tc);
    tc += 1; t = md.begin(mdXmp);  if (t != b) rc += testFailed(tc);
    tc += 1; t = md.end(mdExif);   if (t != e) rc += testFailed(tc);
    tc += 1; t = md.end(mdIptc);   if (t != e) rc += testFailed(tc);
    tc += 1; t = md.end(mdXmp);    if (t != e) rc += testFailed(tc);

    // Add one XMP tag, repeat begin(), end() tests
    Metadata::const_iterator xi = md.add(Key1("Xmp.dc.source"));
    b = md.begin();
    e = md.end();
    tc += 1; t = md.begin(mdExif); if (t != b) rc += testFailed(tc);
    tc += 1; t = md.begin(mdIptc); if (t != b) rc += testFailed(tc);
    tc += 1; t = md.begin(mdXmp);  if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdExif);   if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdIptc);   if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdXmp);    if (t != e) rc += testFailed(tc);

    // Add one Exif tag, repeat begin(), end() tests
    Metadata::const_iterator ei = md.add(Key1("Exif.Image.ImageWidth"));
    b = md.begin();
    e = md.end();
    tc += 1; t = md.begin(mdExif); if (t != b) rc += testFailed(tc);
    tc += 1; t = md.begin(mdIptc); if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.begin(mdXmp);  if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdExif);   if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdIptc);   if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdXmp);    if (t != e) rc += testFailed(tc);

    // Add one IPTC tag, repeat begin(), end() tests
    Metadata::const_iterator ii = md.add(Key1("Iptc.Envelope.Destination"));
    b = md.begin();
    e = md.end();
    tc += 1; t = md.begin(mdExif); if (t != b || t != ei) rc += testFailed(tc);
    tc += 1; t = md.begin(mdIptc); if (t != ii) rc += testFailed(tc);
    tc += 1; t = md.begin(mdXmp);  if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdExif);   if (t != ii) rc += testFailed(tc);
    tc += 1; t = md.end(mdIptc);   if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdXmp);    if (t != e) rc += testFailed(tc);

    // Add another XMP tag, repeat begin(), end() tests
    Metadata::const_iterator x2i = md.add(Key1("Xmp.dc.target"));
    b = md.begin();
    e = md.end();
    tc += 1; t = md.begin(mdExif); if (t != b || t != ei) rc += testFailed(tc);
    tc += 1; t = md.begin(mdIptc); if (t != ii) rc += testFailed(tc);
    tc += 1; t = md.begin(mdXmp);  if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdExif);   if (t != ii) rc += testFailed(tc);
    tc += 1; t = md.end(mdIptc);   if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdXmp);    if (t != e) rc += testFailed(tc);

    // Add another IPTC tag, repeat begin(), end() tests
    Metadata::const_iterator i2i = md.add(Key1("Iptc.Envelope.FileFormat"));
    b = md.begin();
    e = md.end();
    tc += 1; t = md.begin(mdExif); if (t != b || t != ei) rc += testFailed(tc);
    tc += 1; t = md.begin(mdIptc); if (t != ii) rc += testFailed(tc);
    tc += 1; t = md.begin(mdXmp);  if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdExif);   if (t != ii) rc += testFailed(tc);
    tc += 1; t = md.end(mdIptc);   if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdXmp);    if (t != e) rc += testFailed(tc);

    // Add another Exif tag, repeat begin(), end() tests
    Metadata::const_iterator e2i = md.add(Key1("Exif.Image.ImageLength"));
    b = md.begin();
    e = md.end();
    tc += 1; t = md.begin(mdExif); if (t != b || t != ei) rc += testFailed(tc);
    tc += 1; t = md.begin(mdIptc); if (t != ii) rc += testFailed(tc);
    tc += 1; t = md.begin(mdXmp);  if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdExif);   if (t != ii) rc += testFailed(tc);
    tc += 1; t = md.end(mdIptc);   if (t != xi) rc += testFailed(tc);
    tc += 1; t = md.end(mdXmp);    if (t != e) rc += testFailed(tc);

    // Output tags by family
    tc += 1;
    std::ostringstream ose;
    for (t = md.begin(mdExif); t != md.end(mdExif); ++t) ose << t->key() << " ";
    if (ose.str() != "Exif.Image.ImageWidth Exif.Image.ImageLength ") rc += testFailed(tc);

    tc += 1;
    std::ostringstream osi;
    for (t = md.begin(mdIptc); t != md.end(mdIptc); ++t) osi << t->key() << " ";
    if (osi.str() != "Iptc.Envelope.Destination Iptc.Envelope.FileFormat ") rc += testFailed(tc);

    tc += 1;
    std::ostringstream osx;
    for (t = md.begin(mdXmp); t != md.end(mdXmp); ++t) osx << t->key() << " ";
    if (osx.str() != "Xmp.dc.source Xmp.dc.target ") rc += testFailed(tc);



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
