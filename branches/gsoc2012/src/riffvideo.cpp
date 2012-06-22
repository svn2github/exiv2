//Riff Video - Detection And Display

#include "riffvideo.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "types.hpp"

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



    if (io_->open() != 0) throw Error(9, io_->path(), strError());

    // Ensure that this is the correct image type
    if (!isRiffType(*io_, false)) {
        if (io_->error() || io_->eof()) throw Error(14);
        throw Error(3, "RIFF");
    }

    IoCloser closer(*io_);
    clearMetadata();

    xmpData_["Xmp.video.fileSize"] = (double)io_->size()/(double)1048576;
    xmpData_["Xmp.video.fileName"] = io_->path();
    xmpData_["Xmp.video.mimeType"] = mimeType();

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
            xmpData_["Xmp.video.container"] = buf.pData_;
            break;
        case filetype:
            xmpData_["Xmp.video.fileType"] = buf.pData_;
            break;
        case filesize:
            //long filesize_ = returnValueInDecimal(buf); (For Debug)
            //std::cerr <<std::setw(35)<<std::left<< "File Size Indicated By Metadata"<<": "<<filesize_<<" Bytes\n";
            break;
        }

    }

    std::cout<<"\n\n";

    // Output XMP properties
    for (Exiv2::XmpData::const_iterator md = xmpData_.begin(); md != xmpData_.end(); ++md) {
        std::cout << std::setfill(' ') << std::left
                  << std::setw(44)
                  << md->key() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << md->typeName() << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << md->count() << "  "
                  << std::dec << md->value()
                  << std::endl;
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
            streamHandler(Video);
        }
        else if(compareString(buf, "AUDS")) {
            streamHandler(Audio);
        }
        else if(compareString(buf, "INFO")) {
            infoTagsHandler();
        }
        else if(compareString(buf, "MOVI")) {
            //std::cerr <<std::setw(35)<<std::left<< "Metadata Has Reached Its End"<<": "<<buf.pData_<<"\n"; (For Debug)
            return;
        }
    }
}

void RiffVideo::infoTagsHandler() { //Todo Decoding Info Tags
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
    Exiv2::Value::AutoPtr v = Exiv2::Value::create(Exiv2::xmpSeq);


    bufRead = io_->read(buf.pData_, bufMinSize);
    chunkEndPosition = returnValueInDecimal(buf);

    //std::cerr <<std::setw(35)<<std::left<< "Junk Data"<<": ";   (For Debug)
    for (positionCounter_ = 0; positionCounter_ < chunkEndPosition; ){
        std::memset(buf.pData_, 0x0, buf.size_);
        if (chunkEndPosition - positionCounter_ < 4) {
            bufRead = io_->read(buf.pData_, 2); positionCounter_ += 2;
        }
        else {
            bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
        }
        v->read(Exiv2::toString( buf.pData_));
    }
    xmpData_.add(Exiv2::XmpKey("Xmp.video.junk"), v.get());
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
            xmpData_["Xmp.video.microSecPerFrame"] = returnValueInDecimal(buf);
            frame_rate = (double)1000000/(double)returnValueInDecimal(buf);
            break;
        case (maxDataRate):
            xmpData_["Xmp.video.maxDataRate"] = (double)returnValueInDecimal(buf)/(double)1024;
            break;
        case frameCount:
            xmpData_["Xmp.video.frameCount"] = returnValueInDecimal(buf);
            frame_count = returnValueInDecimal(buf);
            break;
        case streamCount:
            xmpData_["Xmp.video.streamCount"] = returnValueInDecimal(buf);
            break;
        case imageWidth_h:
            width = returnValueInDecimal(buf);
            xmpData_["Xmp.video.width"] = width;
            break;
        case imageHeight_h:
            height = returnValueInDecimal(buf);
            xmpData_["Xmp.video.height"] = height;
            break;
        }
    }

    displayAspectRatio(width, height);

    Exiv2::XmpTextValue tv(Exiv2::toString(width));
        xmpData_.add(Exiv2::XmpKey("Xmp.video.frameSize/stDim:w"), &tv);
        tv.read(Exiv2::toString(height));
        xmpData_.add(Exiv2::XmpKey("Xmp.video.frameSize/stDim:h"), &tv);
        tv.read("pixels");
        xmpData_.add(Exiv2::XmpKey("Xmp.video.frameSize/stDim:unit"), &tv);

    displayDuration(frame_rate, frame_count);
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
            if(streamType == Video)
                xmpData_["Xmp.video.videoCodec"] = buf.pData_;
            else if (streamType == Audio)
                xmpData_["Xmp.video.audioCodec"] = buf.pData_;
            else
                xmpData_["Xmp.video.codec"] = buf.pData_;
            break;
        case sampleRate:
            divisor=returnValueInDecimal(buf);
            break;
        case (sampleRate+1):
            if(streamType == Video)
                xmpData_["Xmp.video.frameRate"] = returnSampleRate(buf,divisor);
            else if (streamType == Audio)
                xmpData_["Xmp.video.audioSampleRate"] = returnSampleRate(buf,divisor);
            else
                 xmpData_["Xmp.video.streamSampleRate"] = returnSampleRate(buf,divisor);
            break;
        case sampleCount:
            if(streamType == Video)
                xmpData_["Xmp.video.frameCount"] = returnValueInDecimal(buf);
            else if (streamType == Audio)
                xmpData_["Xmp.video.audioSampleCount"] = returnValueInDecimal(buf);
            else
                xmpData_["Xmp.video.streamSampleCount"] = returnValueInDecimal(buf);
            break;
        case quality:
            if(streamType == Video)
                xmpData_["Xmp.video.videoQuality"] = returnValueInDecimal(buf);
            else if(streamType != Audio)
                xmpData_["Xmp.video.streamQuality"] = returnValueInDecimal(buf);
            break;
        case sampleSize:
            if(streamType == Video)
                xmpData_["Xmp.video.videoSampleSize"] = returnValueInDecimal(buf);
            else if(streamType != Audio)
                xmpData_["Xmp.video.streamSampleSize"] = returnValueInDecimal(buf);
            break;
        }

        if(compareString(buf,"STRF")) {
            //std::cerr <<std::setw(35)<<std::left<< "Stream Format"<<": "<<"\n";  (For Debug)
            bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
            streamFormatHandler(streamType);
        }

        //Checks the end of Video Stream or Audio Stream and adds two bits to the buffer progress
        if(compareString(buf,"VIDE")|| compareString(buf,"AUDI")) {
            //if(compareString(buf,"VIDE"))
            //    std::cerr <<std::setw(35)<<std::left<< "Stream Type"<<": "<<"Video Stream Ends\n";
            //if(compareString(buf,"AUDI"))
            //    std::cerr <<std::setw(35)<<std::left<< "Stream Type"<<": "<<"Audio Stream Ends\n";  (For Debug)
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
            case imageWidth: //Will be used in case of debugging
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                //std::cerr <<std::setw(35)<<std::left<< "Image Width"<<": "<<returnValueInDecimal(buf)<<"\n";
                break;
            case imageHeight: //Will be used in case of debugging
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                //std::cerr <<std::setw(35)<<std::left<< "Image Height"<<": "<<returnValueInDecimal(buf)<<"\n";
                break;
            case planes:
                bufRead = io_->read(buf.pData_, 2); positionCounter_ += 2;
                xmpData_["Xmp.video.planes"] = returnValueInDecimal(buf,2);
                break;
            case bitDepth:
                bufRead = io_->read(buf.pData_, 2); positionCounter_ += 2;
                xmpData_["Xmp.video.pixelDepth"] = returnValueInDecimal(buf,2);
                break;
            case compression:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                xmpData_["Xmp.video.compressor"] = buf.pData_;
                break;
            case imageLength:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                xmpData_["Xmp.video.imageLength"] = returnValueInDecimal(buf);
                break;
            case pixelsPerMeterX:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                xmpData_["Xmp.video.pixelPerMeterX"] = returnValueInDecimal(buf);
                break;
            case pixelsPerMeterY:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                xmpData_["Xmp.video.pixelPerMeterY"] = returnValueInDecimal(buf);
                break;
            case numColors:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                if(returnValueInDecimal(buf) == 0)
                    xmpData_["Xmp.video.numOfColours"] = "Unspecified";
                else
                    xmpData_["Xmp.video.numOfColours"] = returnValueInDecimal(buf);
                break;
            case numImportantColors:
                bufRead = io_->read(buf.pData_, bufMinSize); positionCounter_ += 4;
                if(returnValueInDecimal(buf) == 0)
                    xmpData_["Xmp.video.numOfImpColours"] = "All";
                else
                    xmpData_["Xmp.video.numIfImpColours"] = returnValueInDecimal(buf);
                break;
            }
        }
    }
    else if(streamType == Audio) {
        int c = 0;

        for(int i = 0; i <= 7; i++, positionCounter_++) {
            bufRead = io_->read(buf.pData_, 2); positionCounter_ += 2;

            switch(i) {
            case encoding:
                xmpData_["Xmp.video.audioCompressor"] = returnValueInDecimal(buf,2);
                break;
            case numberOfChannels:
                c = returnValueInDecimal(buf,2);
                if(c == 1) xmpData_["Xmp.video.audioChannelType"] = "Mono";
                else if(c == 2) xmpData_["Xmp.video.audioChannelType"] = "Stereo";
                else if(c == 5) xmpData_["Xmp.video.audioChannelType"] = "5.1 Surround Sound";
                else if(c == 7) xmpData_["Xmp.video.audioChannelType"] = "7.1 Surround Sound";
                else xmpData_["Xmp.video.audioChannelType"] = "Mono";
                break;
            case audioSampleRate:
                xmpData_["Xmp.video.audioSampleRate"] = returnValueInDecimal(buf,2);
                break;
            case avgBytesPerSec:
                xmpData_["Xmp.video.audioSampleType"] = returnValueInDecimal(buf,2);
                break;
            case bitsPerSample:
                xmpData_["Xmp.video.bitsPerSample"] = returnValueInDecimal(buf,2);
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
    if(aspectRatio == 1.3) xmpData_["Xmp.video.aspectRatio"] = "4:3";
    else if(aspectRatio == 1.7) xmpData_["Xmp.video.aspectRatio"] = "16:9";
    else if(aspectRatio == 1.0) xmpData_["Xmp.video.aspectRatio"] = "1:1";
    else if(aspectRatio == 1.6) xmpData_["Xmp.video.aspectRatio"] = "16:10";
    else if(aspectRatio == 2.2) xmpData_["Xmp.video.aspectRatio"] = "2.21:1";
    else if(aspectRatio == 2.3) xmpData_["Xmp.video.aspectRatio"] = "2.35:1";
    else if(aspectRatio == 1.2) xmpData_["Xmp.video.aspectRatio"] = "5:4";
    else xmpData_["Xmp.video.aspectRatio"] = aspectRatio;
}

void RiffVideo::displayDuration(double frame_rate, long frame_count) {
    if(frame_rate == 0)
        return;

    long hours = 0, minutes = 0, seconds = 0, duration = frame_count / frame_rate;
    xmpData_["Xmp.video.fileDataRate"] = (double)io_->size()/(double)(1048576*duration);
    Exiv2::XmpTextValue tv("a");

    hours = duration / 3600;
    duration %= 3600;
    minutes = duration /60;
    seconds = duration % 60;
    xmpData_["Xmp.video.duration/stDim:totalSec"] = duration;

    if (hours != 0) {
        tv.read(Exiv2::toString(hours));
        xmpData_.add(Exiv2::XmpKey("Xmp.video.duration/stDim:hr"), &tv);
    }
    if (minutes != 0 || hours != 0) {
        tv.read(Exiv2::toString(minutes));
        xmpData_.add(Exiv2::XmpKey("Xmp.video.duration/stDim:min"), &tv);
    }

    tv.read(Exiv2::toString(seconds));
    xmpData_.add(Exiv2::XmpKey("Xmp.video.duration/stDim:sec"), &tv);
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
