//Riff Video - Detection And Display

#include "riffvideo.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include <iomanip>


namespace riffInternal {

bool compareString(Exiv2::DataBuf& buf ,const char* str2) {
    for(int i=0;i<4;i++)
        if(toupper(buf.pData_[i])!=str2[i])
            return false;
    return true;
}

unsigned long returnValueInDecimal(Exiv2::DataBuf& buf, int n=4) {
    long temp=0;
    for(int i=0;i<n;i++)
        temp=temp+ buf.pData_[i]*(pow(256,i));
    return temp;
}

long bufRead =0;

enum rifftags {
    container, filesize, filetype
} ;
enum streamTypeInfo {
    Audio,  MIDI, Text, Video
};
enum streamHeaderTags {
    codec = 1, sampleRate = 5, sampleCount = 8, quality = 10, sampleSize
};
enum bmptags {
    imageWidth, imageHeight, planes, bitDepth, compression, imageLength, pixelsPerMeterX, pixelsPerMeterY, numColors, numImportantColors
};
enum audioFormatTags {
    encoding, numberOfChannels, audioSampleRate, avgBytesPerSec = 4, bitsPerSample = 7
};
enum riffHdrlTags {
    IDIT, ISMP, LIST_odml, LIST_strl, avih
};
enum aviHeaderTags {
    frameRate, maxDataRate, frameCount = 4, streamCount = 6, imageWidth_h = 8, imageHeight_h
};

}

using namespace Exiv2::Internal;
using namespace riffInternal;

namespace Exiv2 {

RiffVideo::RiffVideo(BasicIo::AutoPtr io)
        : Image(ImageType::riff, mdNone, io) {
} // RiffVideo::RiffVideo

std::string RiffVideo::mimeType() const {
    return "video/riff";
}

void RiffVideo::writeMetadata() {
}

// Todo List for Audio Encoding Values
/*  extern const TagDetails audioEncodingValues[] =  {
       {    0x00001, "Microsoft PCM" }

  };
*/


void RiffVideo::readMetadata() {

    //Exiv2::XmpData xmpData;

    if (io_->open() != 0) throw Error(9, io_->path(), strError());

    // Ensure that this is the correct image type
    if (!isRiffType(*io_, false)) {
        if (io_->error() || io_->eof()) throw Error(14);
        throw Error(3, "RIFF");
    }

    IoCloser closer(*io_);
    clearMetadata();

    std::cerr <<std::setw(35)<<std::left<< "File Name"<<": " << io_->path()<<"\n";
    std::cerr <<std::setw(35)<<std::left<< "File Size"<<": " << (double)io_->size()/(double)1048576<<" MB"<< "\n";
    std::cerr <<std::setw(35)<<std::left<< "MIME Type"<<": " << mimeType()<<"\n";

    const long bufMinSize = 4;
    DataBuf buf(bufMinSize);
    buf.pData_[4] = '\0';

    for(long Counter = 0; Counter < 15000; Counter++) {
        std::memset(buf.pData_, 0x0, buf.size_);
        bufRead = io_->read(buf.pData_, bufMinSize);

        if(compareString(buf, "LIST") || compareString(buf, "JUNK")) {
            if(compareString(buf, "JUNK")) {
                junkHandler();
                std::cerr<<"\n";
                continue;
            }
            else
            listHandler();
        }

        switch(Counter) {
        case container:
            std::cerr <<std::setw(35)<<std::left<< "Container Type"<<": "<<buf.pData_<<"\n";
            break;
        case filetype:
            std::cerr <<std::setw(35)<<std::left<< "File Type"<<": "<<buf.pData_<<"\n";
            break;
        case filesize:
            long filesize_ = returnValueInDecimal(buf);
            std::cerr <<std::setw(35)<<std::left<< "File Size Indicated By Metadata"<<": "<<filesize_<<" Bytes\n";
            break;
        }

    }

    std::cout<<"\n\n";
}

void RiffVideo::listHandler() {
    const long bufMinSize = 4;
    DataBuf buf(bufMinSize);
    buf.pData_[4] = '\0';
    long chunkEndPosition;

    bufRead = io_->read(buf.pData_, bufMinSize);
    chunkEndPosition = returnValueInDecimal(buf);

    for (positionCounter_ = 0; positionCounter_ <= chunkEndPosition; ) {
        std::memset(buf.pData_, 0x0, buf.size_);
        bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;

        if(compareString(buf, "LIST") || compareString(buf, "JUNK")) {
            if(compareString(buf, "JUNK")) {
                junkHandler();
                std::cerr<<"\n";
                continue;
            }
            else
                listHandler();
        }
        else if(compareString(buf, "HDRL")) {
            std::memset(buf.pData_, 0x0, buf.size_);
            bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;

            if(compareString(buf, "AVIH"))
                aviHeaderTagsHandler();
        }
        else if(compareString(buf, "VIDS")) {
            std::cerr <<std::setw(35)<<std::left<< "Stream Type"<<": "<<"Video\n";
            streamHandler(Video);
        }
        else if(compareString(buf, "AUDS")) {
            std::cerr <<std::setw(35)<<std::left<< "Stream Type"<<": "<<"Audio\n";
            streamHandler(Audio);
        }
        else if(compareString(buf, "INFO")) {
            infoTagsHandler();
        }
        else if(compareString(buf, "MOVI")) {
            std::cerr <<std::setw(35)<<std::left<< "Metadata Has Reached Its End"<<": "<<buf.pData_<<"\n";
            return;
        }
    }
}

void RiffVideo::infoTagsHandler() {
    const long bufMinSize = 100;
    DataBuf buf(bufMinSize);
    buf.pData_[4] = '\0';
    long infoSize;

    bufRead = io_->read(buf.pData_, 4); positionCounter_ += 4;
    std::cerr <<std::setw(35)<<std::left<< "Info Name"<<": "<< buf.pData_<<"\n";
    bufRead = io_->read(buf.pData_, 4); positionCounter_ += 4;
    infoSize = returnValueInDecimal(buf);

    bufRead = io_->read(buf.pData_, infoSize); positionCounter_ += infoSize;
    std::cerr <<std::setw(35)<<std::left<< "Info Data"<<": "<< buf.pData_<<"\n";
}

void RiffVideo::junkHandler() {
    const long bufMinSize = 4;
    DataBuf buf(bufMinSize);
    buf.pData_[4] = '\0';
    long chunkEndPosition;


    bufRead = io_->read(buf.pData_, bufMinSize);
    chunkEndPosition = returnValueInDecimal(buf);

    std::cerr <<std::setw(35)<<std::left<< "Junk Data"<<": ";
    for (positionCounter_ = 0; positionCounter_ < chunkEndPosition; ){
        std::memset(buf.pData_, 0x0, buf.size_);
        if (chunkEndPosition - positionCounter_ < 4) {
            bufRead = io_->read(buf.pData_, 2); positionCounter_ += 2;
        }
        else {
            bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
        }
        std::cerr << buf.pData_;
    }
}

void RiffVideo::aviHeaderTagsHandler() {
    const long bufMinSize = 4;
    DataBuf buf(bufMinSize);
    buf.pData_[4] = '\0';
    long width = 0, height = 0, frame_count = 0;
    double frame_rate = 1;

    std::memset(buf.pData_, 0x0, buf.size_);
    bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;

    for(int i = 0; i <= 9; i++, positionCounter_ += 4) {
        std::memset(buf.pData_, 0x0, buf.size_);
        bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;

        switch(i) {
        case frameRate:
            std::cerr <<std::setw(35)<<std::left<< "Frame Rate"<<": "<<(double)1000000/(double)returnValueInDecimal(buf)<<" fps\n";
            frame_rate = (double)1000000/(double)returnValueInDecimal(buf);
            break;
        case (maxDataRate):
            std::cerr <<std::setw(35)<<std::left<< "Maximum Data Rate"<<": "<<(double)returnValueInDecimal(buf)/(double)1024<<" kB/s\n";
            break;
        case frameCount:
            std::cerr <<std::setw(35)<<std::left<< "Frame Count"<<": "<<returnValueInDecimal(buf)<<"\n";
            frame_count = returnValueInDecimal(buf);
            break;
        case streamCount:
            std::cerr <<std::setw(35)<<std::left<< "Stream Count"<<": "<<returnValueInDecimal(buf)<<"\n";
            break;
        case imageWidth_h:
            std::cerr <<std::setw(35)<<std::left<< "Image Width"<<": "<<returnValueInDecimal(buf)<<"\n";
            width = returnValueInDecimal(buf);
            break;
        case imageHeight_h:
            std::cerr <<std::setw(35)<<std::left<< "Image Height"<<": "<<returnValueInDecimal(buf)<<"\n";
            height = returnValueInDecimal(buf);
            break;
        }
    }
    std::cerr <<std::setw(35)<<std::left<< "Aspect Ratio"<<": ";
    displayAspectRatio(width, height);
    std::cerr <<"\n";

    std::cerr <<std::setw(35)<<std::left<< "Duration"<<": ";
    displayDuration(frame_rate, frame_count);
    std::cerr <<"\n";


}

void RiffVideo::streamHandler(int streamType) {
    const long bufMinSize = 4;
    DataBuf buf(bufMinSize);
    buf.pData_[4]='\0';
    long divisor = 1;

    for(int i=1; i<=25; i++,positionCounter_++) {
        std::memset(buf.pData_, 0x0, buf.size_);
        bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;

        switch(i) {
        case codec:
            if(streamType==Video)
                std::cerr <<std::setw(35)<<std::left<< "Video Codec"<<": "<<buf.pData_<<"\n";
            else if (streamType==Audio)
                std::cerr <<std::setw(35)<<std::left<< "Audio Codec"<<": "<<buf.pData_<<"\n";
            else
                std::cerr <<std::setw(35)<<std::left<< "Codec"<<": "<<buf.pData_<<"\n";
            break;
        case sampleRate:
            divisor=returnValueInDecimal(buf);
            break;
        case (sampleRate+1):
            if(streamType==Video)
                std::cerr <<std::setw(35)<<std::left<< "Video Frame Rate"<<": "<<returnSampleRate(buf,divisor)<<" fps\n";
            else if (streamType==Audio)
                std::cerr <<std::setw(35)<<std::left<< "Audio Sample Rate"<<": "<<returnSampleRate(buf,divisor)<<"\n";
            else
                std::cerr <<std::setw(35)<<std::left<< "Stream Sample Rate"<<": "<<returnSampleRate(buf,divisor)<<"\n";
            break;
        case sampleCount:
            if(streamType==Video)
                std::cerr <<std::setw(35)<<std::left<< "Video Frame Count"<<": "<<returnValueInDecimal(buf)<<"\n";
            else if (streamType==Audio)
                std::cerr <<std::setw(35)<<std::left<< "Audio Sample Count"<<": "<<returnValueInDecimal(buf)<<"\n";
            else
                std::cerr <<std::setw(35)<<std::left<< "Stream Sample Count"<<": "<<returnValueInDecimal(buf)<<"\n";
            break;
        case quality:
            if (streamType!=Audio)
                std::cerr <<std::setw(35)<<std::left<< "Quality"<<": "<<returnValueInDecimal(buf)<<"\n";
            break;
        case sampleSize:
            if (streamType!=Audio)
                std::cerr <<std::setw(35)<<std::left<< "Stream Sample Size"<<": "<<returnValueInDecimal(buf)<<"\n";
            break;
        }

        if(compareString(buf,"STRF")) {
            std::cerr <<std::setw(35)<<std::left<< "Stream Format"<<": "<<"\n";
            bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
            streamFormatHandler(streamType);
        }

        //Checks the end of Video Stream or Audio Stream and adds two bits to the buffer progress
        if(compareString(buf,"VIDE")|| compareString(buf,"AUDI")) {
            if(compareString(buf,"VIDE"))
                std::cerr <<std::setw(35)<<std::left<< "Stream Type"<<": "<<"Video Stream Ends\n";
            if(compareString(buf,"AUDI"))
                std::cerr <<std::setw(35)<<std::left<< "Stream Type"<<": "<<"Audio Stream Ends\n";
            bufRead = io_->read(buf.pData_, 2); positionCounter_ +=2;
            return;
        }
    }
}

void RiffVideo::streamFormatHandler(int streamType) {
    const long bufMinSize = 4;
    DataBuf buf(bufMinSize);
    buf.pData_[4] = '\0';

    if(streamType == Video) {
        bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;

        for(int i = 0; i <= 9; i++, positionCounter_++) {
            std::memset(buf.pData_, 0x0, buf.size_);

            switch(i) {
            case imageWidth:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                std::cerr <<std::setw(35)<<std::left<< "Image Width"<<": "<<returnValueInDecimal(buf)<<"\n";
                break;
            case imageHeight:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                std::cerr <<std::setw(35)<<std::left<< "Image Height"<<": "<<returnValueInDecimal(buf)<<"\n";
                break;
            case planes:
                bufRead = io_->read(buf.pData_, 2); positionCounter_ += 2;
                std::cerr <<std::setw(35)<<std::left<< "Planes"<<": "<<returnValueInDecimal(buf,2)<<"\n";
                break;
            case bitDepth:
                bufRead = io_->read(buf.pData_, 2); positionCounter_ += 2;
                std::cerr <<std::setw(35)<<std::left<< "Bit Depth"<<": "<<returnValueInDecimal(buf,2)<<"\n";
                break;
            case compression:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                std::cerr <<std::setw(35)<<std::left<< "Image Compression"<<": "<<buf.pData_<<"\n";
                break;
            case imageLength:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                std::cerr <<std::setw(35)<<std::left<< "Image Length"<<": "<<returnValueInDecimal(buf)<<"\n";
                break;
            case pixelsPerMeterX:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                std::cerr <<std::setw(35)<<std::left<< "Pixels Per MeterX"<<": "<<returnValueInDecimal(buf)<<"\n";
                break;
            case pixelsPerMeterY:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                std::cerr <<std::setw(35)<<std::left<< "Pixels Per Metery"<<": "<<returnValueInDecimal(buf)<<"\n";
                break;
            case numColors:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                if(returnValueInDecimal(buf) == 0)
                    std::cerr <<std::setw(35)<<std::left<< "Number Of Colours"<<": "<<"Unspecified\n";
                else
                    std::cerr <<std::setw(35)<<std::left<< "Number Of Colours"<<": "<<returnValueInDecimal(buf)<<"\n";
                break;
            case numImportantColors:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                if(returnValueInDecimal(buf) == 0)
                    std::cerr <<std::setw(35)<<std::left<< "Number Of Important Colours"<<": "<<"All\n";
                else
                    std::cerr <<std::setw(35)<<std::left<< "Number Of Important Colours"<<": "<<returnValueInDecimal(buf)<<"\n";
                break;
            }
        }
    }
    else if(streamType == Audio) {

        for(int i = 0; i <= 7; i++, positionCounter_++) {
            bufRead = io_->read(buf.pData_, 2); positionCounter_ += 2;

            switch(i) {
            case encoding:
                std::cerr <<std::setw(35)<<std::left<< "Encoding Format"<<": "<<returnValueInDecimal(buf,2)<<"\n"; //TODO - Decoding Audio Format
                break;
            case numberOfChannels:
                std::cerr <<std::setw(35)<<std::left<< "Number Of Channels"<<": "<<returnValueInDecimal(buf,2)<<"\n";
                break;
            case audioSampleRate:
                std::cerr <<std::setw(35)<<std::left<< "Audio Sample Rate"<<": "<<returnValueInDecimal(buf,2)<<"\n";
                break;
            case avgBytesPerSec:
                std::cerr <<std::setw(35)<<std::left<< "Average Bytes Per Second"<<": "<<returnValueInDecimal(buf,2)<<"\n";
                break;
            case bitsPerSample:
                std::cerr <<std::setw(35)<<std::left<< "Bits Per Sample"<<": "<<returnValueInDecimal(buf,2)<<"\n";
                bufRead = io_->read(buf.pData_, 2); positionCounter_ += 2;
                break;
            }
        }
    }
}

double RiffVideo::returnSampleRate(Exiv2::DataBuf& buf, long divisor) {
    return ((double)returnValueInDecimal(buf) / (double)divisor);
}

void RiffVideo::displayAspectRatio(long width, long height) {
    double aspectRatio = (double)width / (double)height;
    aspectRatio = floor(aspectRatio*10) / 10;
    if(aspectRatio == 1.3) std::cerr<<"4:3";
    else if(aspectRatio == 1.7) std::cerr<< "16:9";
    else if(aspectRatio == 1.0) std::cerr<< "1:1";
    else if(aspectRatio == 1.6) std::cerr<< "16:10";
    else if(aspectRatio == 2.2) std::cerr<< "2.21:1";
    else if(aspectRatio == 2.3) std::cerr<< "2.35:1";
    else if(aspectRatio == 1.2) std::cerr<< "5:4";
    else std::cerr<< "Undetermined";
}

void RiffVideo::displayDuration(double frame_rate, long frame_count) {
    if(frame_rate == 0)
        return;

    long hours = 0, minutes = 0, seconds = 0, duration = frame_count / frame_rate;
    hours = duration / 3600;
    duration %= 3600;
    minutes = duration /60;
    seconds = duration % 60;

    if (hours != 0) std::cerr <<hours <<"hr ";
    if (minutes != 0 || hours != 0) std::cerr <<minutes <<"mn ";
    std::cerr <<seconds <<"s";
}

Image::AutoPtr newRiffInstance(BasicIo::AutoPtr io, bool /*create*/) {
    Image::AutoPtr image(new RiffVideo(io));
    if (!image->good()) {
        image.reset();
    }
    return image;
}

bool isRiffType(BasicIo& iIo, bool advance) {
    const int32_t len = 2;
    const unsigned char RiffVideoId[4] = { 'R', 'I', 'F' ,'F'};
    byte buf[len];
    iIo.read(buf, len);
    if (iIo.error() || iIo.eof()) {
        return false;
    }
    bool matched = (memcmp(buf, RiffVideoId, len) == 0);
    if (!advance || !matched) {
        iIo.seek(-len, BasicIo::cur);
    }
    return matched;
}

}
