#ifndef ASFVIDEO_HPP
#define ASFVIDEO_HPP

#include "exif.hpp"
#include "image.hpp"
#include "tags_int.hpp"
using namespace Exiv2::Internal;

namespace Exiv2 {

namespace ImageType {
    const int asf = 24; //!< Treating asf as an image type>
}

class EXIV2API AsfVideo:public Image
{
public:
    //! Copy constructor
    AsfVideo(const AsfVideo& rhs);
    //! Assignment operator
    AsfVideo& operator=(const AsfVideo& rhs);
    AsfVideo(BasicIo::AutoPtr io);

    void readMetadata();
    void writeMetadata();
    std::string mimeType() const;

protected:
    void decodeBlock();
    void tagDecoder(const TagVocabulary* tv, uint64_t size);
    void fileProperties();
    void streamProperties();
    void codecList();
    void metadataHandler(int meta = 1);
    void headerExtension(uint64_t size);
private:
    bool continueTraversing_;
    uint64_t localPosition_;
}; //AsfVideo End


EXIV2API Image::AutoPtr newAsfInstance(BasicIo::AutoPtr io, bool create);

//! Check if the file iIo is a Windows Asf Video.
EXIV2API bool isAsfType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2


#endif // ASFVIDEO_HPP
