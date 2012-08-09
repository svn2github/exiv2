#ifndef ASFVIDEO_HPP
#define ASFVIDEO_HPP

#include "exif.hpp"
#include "image.hpp"
#include "tags_int.hpp"
//#include <math.h>
//#include <string>
//#include <ctype.h>


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
private:
    bool continueTraversing_;
}; //AsfVideo End


EXIV2API Image::AutoPtr newAsfInstance(BasicIo::AutoPtr io, bool create);

//! Check if the file iIo is a Windows Asf Video.
EXIV2API bool isAsfType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2


#endif // ASFVIDEO_HPP
