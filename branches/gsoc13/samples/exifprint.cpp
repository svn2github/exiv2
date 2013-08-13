// ***************************************************************** -*- C++ -*-
// exifprint.cpp, $Rev$
// Sample program to print the Exif metadata of an image

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>
#include <cstring>

int main(int argc, char* const argv[])
try {

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " file {--nocurl | --curl} {-struc}\n";
        return 1;
    }

    bool useCurlFromExiv2TestApps = true;
    bool printStructure = false;
    for ( int a = 1 ; a < argc ; a++ ) {
        std::string arg(argv[a]);
        if (arg == "--nocurl")      useCurlFromExiv2TestApps = false;
        else if (arg == "--curl")   useCurlFromExiv2TestApps = true;
        else if (arg == "-struc")   printStructure = true;
    }

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[1], useCurlFromExiv2TestApps);
    assert(image.get() != 0);

    // print the structure of image file.
    if (printStructure) {
        std::cout << "STRUTURE OF FILE:" << std::endl;
        image->printStructure();
        std::cout << "-----------------" << std::endl;
    }

    image->readMetadata();

    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty()) {
        std::string error(argv[1]);
        error += ": No Exif data found in the file";
        throw Exiv2::Error(1, error);
    }
    Exiv2::ExifData::const_iterator end = exifData.end();
    for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
        const char* tn = i->typeName();
        std::cout << std::setw(44) << std::setfill(' ') << std::left
                  << i->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << i->tag() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << (tn ? tn : "Unknown") << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << i->count() << "  "
                  << std::dec << i->value()
                  << "\n";
    }

    return 0;
}
//catch (std::exception& e) {
//catch (Exiv2::AnyError& e) {
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e.what() << "'\n";
    return -1;
}
