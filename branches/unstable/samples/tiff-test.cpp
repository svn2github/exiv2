// ***************************************************************** -*- C++ -*-
// tiff-test.cpp, $Rev$
// First and very simple TIFF write test.

#include <exiv2/tiffimage.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/error.hpp>

#include <string>
#include <iostream>
#include <iomanip>

using namespace Exiv2;

void print(const ExifData& exifData);

int main(int argc, char* const argv[])
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    TiffImage tiffImage(BasicIo::AutoPtr(new FileIo(argv[1])), false);
    tiffImage.readMetadata();

    std::cout << "MIME type:  " << tiffImage.mimeType() << "\n";
    std::cout << "Image size: " << tiffImage.pixelWidth() << " x " << tiffImage.pixelHeight() << "\n";

    ExifData& exifData = tiffImage.exifData();
    std::cout << "Before\n";
    print(exifData);
    std::cout << "======\n";

    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";

    std::cout << "After\n";
    print(exifData);
    tiffImage.writeMetadata();

    return 0;
}
catch (const AnyError& e) {
    std::cout << e << "\n";
}

void print(const ExifData& exifData)
{
    if (exifData.empty()) {
        std::string error("No Exif data found in the file");
        throw Exiv2::Error(1, error);
    }
    Exiv2::ExifData::const_iterator end = exifData.end();
    for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
        std::cout << std::setw(44) << std::setfill(' ') << std::left
                  << i->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << i->tag() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << i->typeName() << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << i->count() << "  "
                  << std::dec << i->value()
                  << "\n";
    }
}
