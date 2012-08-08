//Asf Video - Detection And Display

#include "asfvideo.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "types.hpp"

namespace Exiv2 {
    namespace Internal {

    bool isASFType (byte buf[]) {

        if(buf[0]  == 0x30   && buf[1]  == 0x26   && buf[2]  == 0xb2   && buf[3]  == 0x75 &&
           buf[4]  == 0x8e   && buf[5]  == 0x66   && buf[6]  == 0xcf   && buf[7]  == 0x11 &&
           buf[8]  == 0xa6   && buf[9]  == 0xd9   && buf[10] == 0x00   && buf[11] == 0xaa &&
           buf[12] == 0x00   && buf[13] == 0x62   && buf[14] == 0xce   && buf[15] == 0x6c )
                return true;

        return false;
    }

    }
}

using namespace Exiv2::Internal;


namespace Exiv2 {

AsfVideo::AsfVideo(BasicIo::AutoPtr io)
        : Image(ImageType::asf, mdNone, io) {
} // AsfVideo::AsfVideo

std::string AsfVideo::mimeType() const {
    return "video/asf";
}

void AsfVideo::writeMetadata() {
}

void AsfVideo::readMetadata() {



    if (io_->open() != 0) throw Error(9, io_->path(), strError());

    // Ensure that this is the correct image type
    if (!isAsfType(*io_, false)) {
        if (io_->error() || io_->eof()) throw Error(14);
        throw Error(3, "ASF");
    }

    IoCloser closer(*io_);
    clearMetadata();
//    continueTraversing_ = true;

    xmpData_["Xmp.video.fileSize"] = (double)io_->size()/(double)1048576;
    xmpData_["Xmp.video.fileName"] = io_->path();
    xmpData_["Xmp.video.mimeType"] = mimeType();

//    const long bufMinSize = 4;
//    DataBuf buf(bufMinSize);
//    buf.pData_[4] = '\0';
//    io_->read(buf.pData_, bufMinSize);
//    xmpData_["Xmp.video.container"] = buf.pData_;
//    io_->read(buf.pData_, bufMinSize);
//    io_->read(buf.pData_, bufMinSize);
//    xmpData_["Xmp.video.fileType"] = buf.pData_;

//    while (continueTraversing_) {
//        decodeBlock();
//    }
}

Image::AutoPtr newAsfInstance(BasicIo::AutoPtr io, bool /*create*/) {
    Image::AutoPtr image(new AsfVideo(io));
    if (!image->good()) {
        image.reset();
    }
    return image;
}

bool isAsfType(BasicIo& iIo, bool advance) {
    const int32_t len = 16;

    byte buf[len];
    iIo.read(buf, len);

    if (iIo.error() || iIo.eof()) {
        return false;
    }

    bool matched = isASFType(buf);
    if (!advance || !matched) {
        iIo.seek(0, BasicIo::beg);
    }

    return matched;

}

}
