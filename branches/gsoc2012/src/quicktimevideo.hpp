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
    void tagDecoder(Exiv2::DataBuf & buf, unsigned long size);

private:
    void fileTypeDecoder(unsigned long size);
    void mediaHeaderDecoder(unsigned long size);
    void videoHeaderDecoder(unsigned long size);
    void movieHeaderDecoder(unsigned long size);
    void trackHeaderDecoder(unsigned long size);
    void handlerDecoder(unsigned long size);
    void multipleEntriesDecoder();
    void sampleDesc(unsigned long size);
    void imageDescDecoder();
    void userDataDecoder(unsigned long size);
    void NikonTagsDecoder(unsigned long size);
    void audioDescDecoder();
    void timeToSampleDecoder();
    void setMediaStream();
    void discard(unsigned long size);

    uint64_t timeScale_;
    int currentStream_;
    bool continueTraversing_;

}; //QuickTimeVideo End

EXIV2API Image::AutoPtr newQTimeInstance(BasicIo::AutoPtr io, bool create);

//! Check if the file iIo is a Quick Time Video.
EXIV2API bool isQTimeType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif // QUICKTIMEVIDEO_HPP
