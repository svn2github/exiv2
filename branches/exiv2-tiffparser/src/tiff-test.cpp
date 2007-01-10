// ***************************************************************** -*- C++ -*-
// tiff-test.cpp, $Rev$
// TIFF writer tests.

#include "tiffimage.hpp"
#include "exif.hpp"
#include "error.hpp"

#include <iostream>
#include <iomanip>

using namespace Exiv2;

void print(const ExifData& exifData);

int main()
{
    BasicIo::AutoPtr io(new FileIo("image.tif"));
    TiffImage tiffImage(io, false);
    ExifData& exifData = tiffImage.exifData();
    exifData["Exif.Image.ImageWidth"] = uint32_t(42);
    exifData["Exif.Image.ImageLength"] = 24;
    exifData["Exif.Image.Model"] = "Model";
    exifData["Exif.Image.Make"] = "FujiFilm";
    exifData["Exif.Photo.0x0001"] = "Just for fun";
    exifData["Exif.Iop.RelatedImageFileFormat"] = "TIFF";
    print(exifData);
    tiffImage.writeMetadata();
    return 0;
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
