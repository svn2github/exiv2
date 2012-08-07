#ifndef RIFFVIDEO_HPP
#define RIFFVIDEO_HPP

#include "exif.hpp"
#include "image.hpp"
#include "tags_int.hpp"
#include <math.h>
#include <string>
#include <ctype.h>


namespace Exiv2 {

namespace ImageType {
    const int riff = 20; //!< Treating riff as an image type>
}

class EXIV2API RiffVideo:public Image
{
public:
    //! Copy constructor
    RiffVideo(const RiffVideo& rhs);
    //! Assignment operator
    RiffVideo& operator=(const RiffVideo& rhs);
    RiffVideo(BasicIo::AutoPtr io);

    void readMetadata();
    void writeMetadata();
    std::string mimeType() const;

protected:
    void streamHandler(long size);
    void streamFormatHandler(long size);
    void aviHeaderTagsHandler(long size);
    void decodeBlock();
    void tagDecoder(Exiv2::DataBuf& buf, unsigned long size);
    void junkHandler(long size);
    void listHandler(long size);
    void streamDataTagHandler(long size);
    void infoTagsHandler();
    void nikonTagsHandler();
    void odmlTagsHandler();
    void skipListData();
    void dateTimeOriginal(long size, int i = 0);
    const char* printAudioEncoding(long i);
    double returnSampleRate(Exiv2::DataBuf& buf, long divisor = 1);
    void displayAspectRatio(long width = 1,long height = 1);
    void displayDuration(double frame_rate, long frame_count);

private:
    long positionCounter_;
    bool continueTraversing_;
    int streamType_;
}; //RiffVideo End


EXIV2API Image::AutoPtr newRiffInstance(BasicIo::AutoPtr io, bool create);

//! Check if the file iIo is a Windows Riff Video.
EXIV2API bool isRiffType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2


#endif // RIFFVIDEO_HPP
