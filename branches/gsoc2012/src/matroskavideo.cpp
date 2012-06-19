//Riff Video - Detection And Display

#include "matroskavideo.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include <iomanip>


namespace Exiv2 {

MatroskaVideo::MatroskaVideo(BasicIo::AutoPtr io)
        : Image(ImageType::mkv, mdNone, io) {
} // MatroskaVideo::MatroskaVideo

std::string MatroskaVideo::mimeType() const {
    return "video/matroska";
}

void MatroskaVideo::writeMetadata() {
}


void MatroskaVideo::readMetadata() {
    if (io_->open() != 0) throw Error(9, io_->path(), strError());

    // Ensure that this is the correct image type
    if (!isMkvType(*io_, false)) {
        if (io_->error() || io_->eof()) throw Error(14);
        throw Error(3, "Matroska");
    }

    IoCloser closer(*io_);
    clearMetadata();

    std::cerr <<std::setw(35)<<std::left<< "File Name"<<": " << io_->path()<<"\n";
    std::cerr <<std::setw(35)<<std::left<< "File Size"<<": " << (double)io_->size()/(double)1048576<<" MB"<< "\n";
    std::cerr <<std::setw(35)<<std::left<< "MIME Type"<<": " << mimeType()<<"\n";

    const long bufMinSize = 8;
    DataBuf buf(bufMinSize);
    long bufRead = 0;
    std::memset(buf.pData_, 0x0, buf.size_);
    bufRead = io_->read(buf.pData_, bufMinSize);

    std::cerr <<std::setw(35)<<std::left<< "Doc Type"<<": " << buf.pData_<<"\n";

    decodeBlock();

}

void MatroskaVideo::decodeBlock() {
    const long bufMinSize = 8;
    DataBuf buf(bufMinSize);
    long s_ID = 0;
    long bufRead = 0;

    std::memset(buf.pData_, 0x0, buf.size_);
    bufRead = io_->read(buf.pData_, 8);
    std::cerr <<std::setw(35)<<std::left<< "buf"<<": " << buf.pData_<<"\n";


    std::cerr <<std::setw(35)<<std::left<< "s_ID"<<": " << s_ID<<"\n";
    std::memset(buf.pData_, 0x0, buf.size_);
    bufRead = io_->read(buf.pData_, 1);
    std::cerr <<std::setw(35)<<std::left<< "s_ID"<<": " << findBlockSize(buf) <<"\n";
    std::cerr <<std::setw(35)<<std::left<< "buf"<<": " << buf.pData_<<"\n";


}

int MatroskaVideo::findBlockSize(Exiv2::DataBuf& buf) {
    if (buf.pData_[0] & 128 )
        return 1;
    else if (buf.pData_[0] & 64 )
        return 2;
    else if (buf.pData_[0] & 32 )
        return 3;
    else if (buf.pData_[0] & 16 )
        return 4;
    else if (buf.pData_[0] & 8 )
        return 5;
    else if (buf.pData_[0] & 4 )
        return 6;
    else if (buf.pData_[0] & 2 )
        return 7;
    else
        return 0;
}

Image::AutoPtr newMkvInstance(BasicIo::AutoPtr io, bool /*create*/) {
    Image::AutoPtr image(new MatroskaVideo(io));
    if (!image->good()) {
        image.reset();
    }
    return image;
}

bool isMkvType(BasicIo& iIo, bool advance) {
    const int32_t len = 8;
    const unsigned char MatroskaVideoId[8] = { 'm', 'a', 't' ,'r' , 'o' , 's' , 'k' , 'a'};
    byte buf[len];
    iIo.read(buf, len);
    iIo.read(buf, len);
    if (iIo.error() || iIo.eof()) {
        return false;
    }
    bool matched = (memcmp(buf, MatroskaVideoId, len) == 0);
    if (!advance || !matched) {
        iIo.seek(-len, BasicIo::cur);
    }
    return matched;
}
}
