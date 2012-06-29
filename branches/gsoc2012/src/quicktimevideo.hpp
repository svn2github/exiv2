#ifndef QUICKTIMEVIDEO_HPP
#define QUICKTIMEVIDEO_HPP

#include "exif.hpp"
#include "image.hpp"
#include "tags_int.hpp"

namespace Exiv2 {

namespace ImageType {
    const int qtime = 22; //!< Treating mkv as an image type>
}

class EXIV2API QuickTimeVideo:public Image
{
public:
    //! Copy constructor
    QuickTimeVideo(const QuickTimeVideo& rhs);
    //! Assignment operator
    QuickTimeVideo& operator=(const QuickTimeVideo& rhs);
    QuickTimeVideo(BasicIo::AutoPtr io);

    void readMetadata();
    void writeMetadata();
    std::string mimeType() const;

protected:
    void decodeBlock();

}; //QuickTimeVideo End

EXIV2API Image::AutoPtr newQTimeInstance(BasicIo::AutoPtr io, bool create);

//! Check if the file iIo is a Quick Time Video.
EXIV2API bool isQTimeType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif // QUICKTIMEVIDEO_HPP
