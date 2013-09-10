// ***************************************************************** -*- C++ -*-
// exifprint.cpp, $Rev$
// Sample program to print the Exif metadata of an image

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>
#include <cstring>
#include <vector>
#include <string>

int main(int argc, char* const argv[])
try {
    int result = 0 ;

    // parse command-line
    bool useCurlFromExiv2TestApps = true;
    bool printStructure           = false;
    std::vector<std::string> files;
        
    for ( int a = 1 ; a < argc ; a++ ) {
        char* arg  = argv[a];
        bool  bOpt = *arg == '-';
        // skip past the -'s on the argument
        while ( *arg == '-' ) arg++;

        if ( bOpt ) {
            std::string opt(arg);
            if      (opt == "nocurl") useCurlFromExiv2TestApps = false;
            else if (opt == "curl"  ) useCurlFromExiv2TestApps = true;
            else if (opt == "struc" ) printStructure = true;
        } else {
            files.push_back(arg);
        }
    }
    
    if ( !files.size() ) {
        std::cout << "Usage: " << argv[0] << " [[ {--nocurl | --curl} {--struc}]+] file ]+\n";
        result = 1;
    }
    
    // process files in sequences
    for ( size_t f = 0 ; f < files.size() ; f++ ) {
        std::string&          file  = files[f];
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file, useCurlFromExiv2TestApps);
        assert(image.get() != 0);

        if (printStructure) {
            // print the structure of image file.
            image->printStructure();
        } else {
            // print the metadata
            image->readMetadata();

            Exiv2::ExifData &exifData = image->exifData();
            if (exifData.empty()) {
                std::string error = file + ": No Exif data found in the file";
                std::cerr << error << std::endl;
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
        }
    }

    return result;
}
//catch (std::exception& e) {
//catch (Exiv2::AnyError& e) {
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e.what() << "'\n";
    return -1;
}
