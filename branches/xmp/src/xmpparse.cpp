// ***************************************************************** -*- C++ -*-
// xmpparse.cpp, $Rev$
// Read an XMP packet from a file, parse it and print all (known) properties.

#include "basicio.hpp"
#include "xmp.hpp"
#include "error.hpp"

#include <string>
#include <iostream>
#include <iomanip>

int main(int argc, char* const argv[])
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    Exiv2::DataBuf buf = Exiv2::readFile(argv[1]);
    Exiv2::XmpData xmpData;
    if (0 != xmpData.load(buf.pData_, buf.size_)) {
        std::string error(argv[1]);
        error += ": Failed to parse file contents (XMP packet)";
        throw Exiv2::Error(1, error);
    }
    if (xmpData.empty()) {
        std::string error(argv[1]);
        error += ": No XMP properties found in the XMP packet";
        throw Exiv2::Error(1, error);
    }
    for (Exiv2::XmpData::const_iterator md = xmpData.begin(); 
         md != xmpData.end(); ++md) {
        std::cout << std::setfill(' ') << std::left
                  << std::setw(44)
                  << md->key() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << md->typeName() << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << md->count() << "  "
                  << std::dec << md->value()
                  << std::endl;
    }
    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
