 
#ifndef MATROSKAVIDEO_HPP
#define MATROSKAVIDEO_HPP

#include "exif.hpp"
#include "image.hpp"
#include "tags_int.hpp"
#include <math.h>
#include <string>
#include <ctype.h>


namespace Exiv2 {

namespace ImageType {
    const int mkv = 21; //!< Treating mkv as an image type>
}

class EXIV2API MatroskaVideo:public Image
{
public:
    //! Copy constructor
    MatroskaVideo(const MatroskaVideo& rhs);
    //! Assignment operator
    MatroskaVideo& operator=(const MatroskaVideo& rhs);
    MatroskaVideo(BasicIo::AutoPtr io);

    void readMetadata();
    void writeMetadata();
    std::string mimeType() const;

protected:
    int findBlockSize(Exiv2::DataBuf& buf);
    void decodeBlock();

}; //MatroskaVideo End

EXIV2API Image::AutoPtr newMkvInstance(BasicIo::AutoPtr io, bool create);

//! Check if the file iIo is a Matroska Video.
EXIV2API bool isMkvType(BasicIo& iIo, bool advance);


}                                       // namespace Exiv2


#endif // MatroskaVideo_HPP
