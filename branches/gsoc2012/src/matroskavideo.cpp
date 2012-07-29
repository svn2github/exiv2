//Matroska Video - Detection And Display

#include "matroskavideo.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include <math.h>

namespace Exiv2 {
    namespace Internal {

    //! Function used to ignore Tags, basically Tags which contain other tags inside them, since they are not necessary as metadata information
    //! Returns true, if Tag is found in the ignoreList[]
    bool dataIgnoreList (unsigned long tagValue) {
        unsigned long ignoreList[] = {
            0x0000, 0x000e, 0x000f, 0x0020, 0x0026, 0x002e, 0x0036,
            0x0037, 0x003b, 0x005b, 0x0060, 0x0061, 0x0068, 0x05b9,
            0x0dbb, 0x1034, 0x1035, 0x1854, 0x21a7, 0x2240, 0x23c0,
            0x2624, 0x27c8, 0x2911, 0x2924, 0x2944, 0x2d80, 0x3373,
            0x35a1, 0x3e5b, 0x3e7b,
            0x14d9b74, 0x254c367, 0x549a966, 0x654ae6b, 0x8538067,
            0x941a469, 0xa45dfa3, 0xb538667, 0xc53bb6b, 0xf43b675

        };

        for(int i = 0; i <= 40; i++)
            if(tagValue == ignoreList[i])
                return true;

        return false;
    }

    //! Function used to ignore Tags and values stored in them, since they are not necessary as metadata information
    //! Returns true, if Tag is found in the ignoreList[]
    bool ignoreList (unsigned long tagValue) {
        unsigned long ignoreList[] = {
            0x0023, 0x0021, 0x0033, 0x0071, 0x0077, 0x006c, 0x0067, 0x007b, 0x02f2, 0x02f3,
            0x1031, 0x1032, 0x13ab, 0x13ac, 0x15ee, 0x23a2, 0x23c6, 0x2e67, 0x33a4, 0x33c5,
            0x3446, 0x2de7, 0x2df8, 0x3314f, 0x43a770
        };

        for(int i = 0; i <= 24; i++)
            if(tagValue == ignoreList[i])
                return true;

        return false;
    }

    extern const TagDetails matroskaTags[] =  {
        {      0x0000, "ChapterDisplay" },
        {      0x0003, "TrackType" },
        {      0x0005, "ChapterString" },
        {      0x0006, "VideoCodecID/AudioCodecID/CodecID" },
        {      0x0008, "TrackDefault" },
        {      0x0009, "ChapterTrackNumber" },
        {      0x000e, "Slices" },
        {      0x000f, "ChapterTrack" },
        {      0x0011, "ChapterTimeStart" },
        {      0x0012, "ChapterTimeEnd" },
        {      0x0016, "CueRefTime" },
        {      0x0017, "CueRefCluster" },
        {      0x0018, "ChapterFlagHidden" },
        {      0x001a, "VideoScanType" },
        {      0x001b, "BlockDuration" },
        {      0x001c, "TrackLacing" },
        {      0x001f, "AudioChannels" },
        {      0x0020, "BlockGroup" },
        {      0x0021, "Block" },
        {      0x0022, "BlockVirtual" },
        {      0x0023, "SimpleBlock" },
        {      0x0024, "CodecState" },
        {      0x0025, "BlockAdditional" },
        {      0x0026, "BlockMore" },
        {      0x0027, "Position" },
        {      0x002a, "CodecDecodeAll" },
        {      0x002b, "PrevSize" },
        {      0x002e, "TrackEntry" },
        {      0x002f, "EncryptedBlock" },
        {      0x0030, "ImageWidth" },
        {      0x0033, "CueTime" },
        {      0x0035, "AudioSampleRate" },
        {      0x0036, "ChapterAtom" },
        {      0x0037, "CueTrackPositions" },
        {      0x0039, "TrackUsed" },
        {      0x003a, "ImageHeight" },
        {      0x003b, "CuePoint" },
        {      0x003f, "CRC-32" },
        {      0x004b, "BlockAdditionalID" },
        {      0x004c, "LaceNumber" },
        {      0x004d, "FrameNumber" },
        {      0x004e, "Delay" },
        {      0x004f, "ClusterDuration" },
        {      0x0057, "TrackNumber" },
        {      0x005b, "CueReference" },
        {      0x0060, "Video" },
        {      0x0061, "Audio" },
        {      0x0067, "Timecode" },
        {      0x0068, "TimeSlice" },
        {      0x006a, "CueCodecState" },
        {      0x006b, "CueRefCodecState" },
        {      0x006c, "Void" },
        {      0x006e, "BlockAddID" },
        {      0x0071, "CueClusterPosition" },
        {      0x0077, "CueTrack" },
        {      0x007a, "ReferencePriority" },
        {      0x007b, "ReferenceBlock" },
        {      0x007d, "ReferenceVirtual" },
        {      0x0254, "ContentCompressionAlgorithm" },
        {      0x0255, "ContentCompressionSettings" },
        {      0x0282, "DocType" },
        {      0x0285, "DocTypeReadVersion" },
        {      0x0286, "EBMLVersion" },
        {      0x0287, "DocTypeVersion" },
        {      0x02f2, "EBMLMaxIDLength" },
        {      0x02f3, "EBMLMaxSizeLength" },
        {      0x02f7, "EBMLReadVersion" },
        {      0x037c, "ChapterLanguage" },
        {      0x037e, "ChapterCountry" },
        {      0x0444, "SegmentFamily" },
        {      0x0461, "DateTimeOriginal" },
        {      0x047a, "TagLanguage" },
        {      0x0484, "TagDefault" },
        {      0x0485, "TagBinary" },
        {      0x0487, "TagString" },
        {      0x0489, "Duration" },
        {      0x050d, "ChapterProcessPrivate" },
        {      0x0598, "ChapterFlagEnabled" },
        {      0x05a3, "TagName" },
        {      0x05b9, "EditionEntry" },
        {      0x05bc, "EditionUID" },
        {      0x05bd, "EditionFlagHidden" },
        {      0x05db, "EditionFlagDefault" },
        {      0x05dd, "EditionFlagOrdered" },
        {      0x065c, "AttachedFileData" },
        {      0x0660, "AttachedFileMIMEType" },
        {      0x066e, "AttachedFileName" },
        {      0x0675, "AttachedFileReferral" },
        {      0x067e, "AttachedFileDescription" },
        {      0x06ae, "AttachedFileUID" },
        {      0x07e1, "ContentEncryptionAlgorithm" },
        {      0x07e2, "ContentEncryptionKeyID" },
        {      0x07e3, "ContentSignature" },
        {      0x07e4, "ContentSignatureKeyID" },
        {      0x07e5, "ContentSignatureAlgorithm" },
        {      0x07e6, "ContentSignatureHashAlgorithm" },
        {      0x0d80, "MuxingApp" },
        {      0x0dbb, "Seek" },
        {      0x1031, "ContentEncodingOrder" },
        {      0x1032, "ContentEncodingScope" },
        {      0x1033, "ContentEncodingType" },
        {      0x1034, "ContentCompression" },
        {      0x1035, "ContentEncryption" },
        {      0x135f, "CueRefNumber" },
        {      0x136e, "TrackName" },
        {      0x1378, "CueBlockNumber" },
        {      0x137f, "TrackOffset" },
        {      0x13ab, "SeekID" },
        {      0x13ac, "SeekPosition" },
        {      0x13b8, "Stereo3DMode" },
        {      0x14aa, "CropBottom" },
        {      0x14b0, "DisplayWidth" },
        {      0x14b2, "DisplayUnit" },
        {      0x14b3, "AspectRatioType" },
        {      0x14ba, "DisplayHeight" },
        {      0x14bb, "CropTop" },
        {      0x14cc, "CropLeft" },
        {      0x14dd, "CropRight" },
        {      0x15aa, "TrackForced" },
        {      0x15ee, "MaxBlockAdditionID" },
        {      0x1741, "WritingApp" },
        {      0x1854, "SilentTracks" },
        {      0x18d7, "SilentTrackNumber" },
        {      0x21a7, "AttachedFile" },
        {      0x2240, "ContentEncoding" },
        {      0x2264, "AudioBitsPerSample" },
        {      0x23a2, "CodecPrivate" },
        {      0x23c0, "Targets" },
        {      0x23c3, "ChapterPhysicalEquivalent" },
        {      0x23c4, "TagChapterUID" },
        {      0x23c5, "TagTrackUID" },
        {      0x23c6, "TagAttachmentUID" },
        {      0x23c9, "TagEditionUID" },
        {      0x23ca, "TargetType" },
        {      0x2532, "SignedElement" },
        {      0x2624, "TrackTranslate" },
        {      0x26a5, "TrackTranslateTrackID" },
        {      0x26bf, "TrackTranslateCodec" },
        {      0x26fc, "TrackTranslateEditionUID" },
        {      0x27c8, "SimpleTag" },
        {      0x28ca, "TargetTypeValue" },
        {      0x2911, "ChapterProcessCommand" },
        {      0x2922, "ChapterProcessTime" },
        {      0x2924, "ChapterTranslate" },
        {      0x2933, "ChapterProcessData" },
        {      0x2944, "ChapterProcess" },
        {      0x2955, "ChapterProcessCodecID" },
        {      0x29a5, "ChapterTranslateID" },
        {      0x29bf, "ChapterTranslateCodec" },
        {      0x29fc, "ChapterTranslateEditionUID" },
        {      0x2d80, "ContentEncodings" },
        {      0x2de7, "MinCache" },
        {      0x2df8, "MaxCache" },
        {      0x2e67, "ChapterSegmentUID" },
        {      0x2ebc, "ChapterSegmentEditionUID" },
        {      0x2fab, "TrackOverlay" },
        {      0x3373, "Tag" },
        {      0x3384, "SegmentFileName" },
        {      0x33a4, "SegmentUID" },
        {      0x33c4, "ChapterUID" },
        {      0x33c5, "TrackUID" },
        {      0x3446, "TrackAttachmentUID" },
        {      0x35a1, "BlockAdditions" },
        {      0x38b5, "OutputAudioSampleRate" },
        {      0x3ba9, "Title" },
        {      0x3d7b, "ChannelPositions" },
        {      0x3e5b, "SignatureElements" },
        {      0x3e7b, "SignatureElementList" },
        {      0x3e8a, "SignatureAlgo" },
        {      0x3e9a, "SignatureHash" },
        {      0x3ea5, "SignaturePublicKey" },
        {      0x3eb5, "Signature" },
        {     0x2b59c, "TrackLanguage" },
        {     0x3314f, "TrackTimecodeScale" },
        {     0x383e3, "FrameRate" },
        {     0x3e383, "VideoFrameRate/DefaultDuration" },
        {     0x58688, "VideoCodecName/AudioCodecName/CodecName" },
        {     0x6b240, "CodecDownloadURL" },
        {     0xad7b1, "TimecodeScale" },
        {     0xeb524, "ColorSpace" },
        {     0xfb523, "Gamma" },
        {    0x1a9697, "CodecSettings" },
        {    0x1b4040, "CodecInfoURL" },
        {    0x1c83ab, "PrevFileName" },
        {    0x1cb923, "PrevUID" },
        {    0x1e83bb, "NextFileName" },
        {    0x1eb923, "NextUID" },
        {    0x43a770, "Chapters" },
        {   0x14d9b74, "SeekHead" },
        {   0x254c367, "Tags" },
        {   0x549a966, "Info" },
        {   0x654ae6b, "Tracks" },
        {   0x8538067, "SegmentHeader" },
        {   0x941a469, "Attachments" },
        {   0xa45dfa3, "EBMLHeader" },
        {   0xb538667, "SignatureSlot" },
        {   0xc53bb6b, "Cues" },
        {   0xf43b675, "Cluster" },
    };

    extern const TagDetails matroskaTrackType[] =  {
        {   0x1,  "Video" },
        {   0x2,  "Audio" },
        {   0x3,  "Complex" },
        {   0x10, "Logo" },
        {   0x11, "Subtitle" },
        {   0x12, "Buttons" },
        {   0x20, "Control" }
    };

    extern const TagDetails compressionAlgorithm[] =  {
        {   0, "zlib " },
        {   1, "bzlib" },
        {   2, "lzo1x" },
        {   3, "Header Stripping" }
    };

    extern const TagDetails audioChannels[] =  {
        {   1, "Mono" },
        {   2, "Stereo" },
        {   5, "5.1 Surround Sound" },
        {   7, "7.1 Surround Sound" }
    };

    extern const TagDetails encryptionAlgorithm[] =  {
        {   0, "Not Encrypted" },
        {   1, "DES" },
        {   2, "3DES" },
        {   3, "Twofish" },
        {   4, "Blowfish" },
        {   5, "AES" }
    };

    extern const TagDetails encodingType[] =  {
        {   0, "Compression" },
        {   1, "Encryption" }
    };

    extern const TagDetails videoScanType[] =  {
        {   0, "Progressive" },
        {   1, "Interlaced" }
    };

    unsigned long returnTagValue(Exiv2::DataBuf& buf, Exiv2::DataBuf& buf2, int n ) {
        long temp = 0;
        long reg1 = 0;
        reg1 = (buf.pData_[0] & (int)(pow(2,8-n)-1));

        for(int i = n-2; i >= 0; i--) {
            temp = temp + buf2.pData_[i]*(pow(256,n-i-2));
        }
        temp += reg1 * pow(256,n-1);
        return temp;
    }

    int64_t returnValue(Exiv2::DataBuf& buf, int n ) {
        int64_t temp = 0;

        for(int i = n-1; i >= 0; i--) {
            temp = temp + buf.pData_[i]*(pow(256,n-i-1));
        }
        return temp;
    }

    }
}

using namespace Exiv2::Internal;


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

    xmpData_["Xmp.video.fileName"] = io_->path();
    xmpData_["Xmp.video.fileSize"] = (double)io_->size()/(double)1048576;
    xmpData_["Xmp.video.mimeType"] = mimeType();
//    int i=0;

    while (continueTraversing_) {
//        std::cout<<std::setw(3)<<i<<": ";i++;
        decodeBlock();
    }

}

void MatroskaVideo::decodeBlock() {
    const long bufMinSize = 200;
    DataBuf buf(bufMinSize);
    DataBuf buf2(bufMinSize);
    unsigned long s_ID = 0;
    unsigned long s_Size = 0;
    unsigned long size = 0;
    bool display = true;
    bool readData = true;

    std::memset(buf.pData_, 0x0, buf.size_);
    io_->read(buf.pData_, 1);

    if(io_->eof()) {
        continueTraversing_ = false;
        return;
    }

    s_ID = findBlockSize(buf);
    if(s_ID != 0)
        io_->read(buf2.pData_, s_ID - 1);

    const TagDetails* td;
    td = find(matroskaTags , (returnTagValue(buf, buf2, s_ID)) );

    if(td->val_ == 0xc53bb6b || td->val_ == 0xf43b675) {
        continueTraversing_ = false;
        return;
    }
//    std::cerr<<std::setw(35)<<std::left<<td->label_<<":";
    if (dataIgnoreList(td->val_))
        display = false;
    if (ignoreList(td->val_))
        readData = false;

    io_->read(buf.pData_, 1);
    s_Size = findBlockSize(buf);

    if(s_Size != 0)
        io_->read(buf2.pData_, s_Size - 1);

    size = returnTagValue(buf, buf2, s_Size);
//    std::cerr<<std::setw(7)<<size;

    if (!display && readData)
        return;

    if (!readData) {
        io_->seek(size, BasicIo::cur);
        return;
    }
//    std::cerr<<"check\n";

    io_->read(buf.pData_, size);
    contentManagement(td, buf, size);

}

void MatroskaVideo::contentManagement(const TagDetails* td, Exiv2::DataBuf& buf, unsigned long size) {

    int64_t duration_in_sec = 0;
    static double time_code_scale = 1;
    static long stream = 0, track_count = 0;
    char str[4] = "No";

    const TagDetails* internal_td;

    switch(td->val_) {

    case 0x0282:
        xmpData_["Xmp.video.docType"] = buf.pData_;
        break;

    case 0x0287:
        xmpData_["Xmp.video.docTypeVersion"] = returnValue(buf, size);
        break;

    case 0x285:
        xmpData_["Xmp.video.docTypeReadVersion"] = returnValue(buf, size);
        break;

    case 0xad7b1:
        time_code_scale = (double)returnValue(buf, size)/(double)1000000000;
        xmpData_["Xmp.video.timecodeScale"] = time_code_scale;
        break;

    case 0x0d80:
        xmpData_["Xmp.video.muxingApp"] = buf.pData_;
        break;

    case 0x1741:
        xmpData_["Xmp.video.writingApp"] = buf.pData_;
        break;

    case 0x0489:
        duration_in_sec = Exiv2::getFloat(buf.pData_, bigEndian) * time_code_scale;
        xmpData_["Xmp.video.duration"] = duration_in_sec;

    case 0x0461:
        duration_in_sec = returnValue(buf, size)/1000000000;
        xmpData_["Xmp.video.dateUTC"] = duration_in_sec; //Date Time Original - measured in seconds relatively to Jan 01, 2001, 0:00:00 GMT+0h

    case 0x3ba9:
        xmpData_["Xmp.video.title"] = buf.pData_;
        break;

    case 0x0057:
        track_count++;
        xmpData_["Xmp.video.totalStream"] = track_count;
        break;

    case 0x0003:
        internal_td = find(matroskaTrackType ,returnValue(buf, size));
        stream = internal_td->val_;
        break;

    case 0x0039:
        if (returnValue(buf, size))
            strcpy(str,"Yes");

        switch(stream) {
        case 1:
            xmpData_["Xmp.video.enabled"] = str; break;
        case 2:
            xmpData_["Xmp.audio.enabled"] = str; break;
        case 17:
            xmpData_["Xmp.video.subTEnabled"] = str; break;
        default: break;
        }
        break;

    case 0x0008:
        if (returnValue(buf, size))
            strcpy(str,"Yes");

        switch(stream) {
        case 1:
            xmpData_["Xmp.video.defaultOn"] = str; break;
        case 2:
            xmpData_["Xmp.audio.defaultOn"] = str; break;
        case 17:
            xmpData_["Xmp.video.subTDefaultOn"] = str; break;
        default: break;
        }
        break;

    case 0x15aa:
        if (returnValue(buf, size))
            strcpy(str,"Yes");

        switch(stream) {
        case 1:
            xmpData_["Xmp.video.trackForced"] = str; break;
        case 2:
            xmpData_["Xmp.audio.trackForced"] = str; break;
        case 17:
            xmpData_["Xmp.video.subTTrackForced"] = str; break;
        default: break;
        }
        break;

    case 0x001c:
        if (returnValue(buf, size))
            strcpy(str,"Yes");
        switch(stream) {
        case 1:
            xmpData_["Xmp.video.trackLacing"] = str; break;
        case 2:
            xmpData_["Xmp.audio.trackLacing"] = str; break;
        case 17:
            xmpData_["Xmp.video.subTTrackLacing"] = str; break;
        default: break;
        }
        break;

    case 0x0006:
        switch(stream) {
        case 1:
            xmpData_["Xmp.video.codec"] = buf.pData_; break;
        case 2:
            xmpData_["Xmp.audio.codec"] = buf.pData_; break;
        case 17:
            xmpData_["Xmp.video.subTCodec"] = buf.pData_; break;
        default: break;
        }
        break;

    case 0x002a:
        if (returnValue(buf, size))
            strcpy(str,"Yes");
        switch(stream) {
        case 1:
            xmpData_["Xmp.video.trackLacing"] = str; break;
        case 2:
            xmpData_["Xmp.audio.trackLacing"] = str; break;
        case 17:
            xmpData_["Xmp.video.subTTrackLacing"] = str; break;
        default: break;
        }
        break;

    case 0x2b59c:
        switch(stream) {
        case 1:
            xmpData_["Xmp.video.trackLang"] = buf.pData_; break;
        case 2:
            xmpData_["Xmp.audio.trackLang"] = buf.pData_; break;
        case 17:
            xmpData_["Xmp.video.subTLang"] = buf.pData_; break;
        }
        break;

    case 0x14b0:
        xmpData_["Xmp.video.width"] = returnValue(buf, size);
        break;

    case 0x14ba:
        xmpData_["Xmp.video.height"] = returnValue(buf, size);
        break;

    case 0x0030:
        xmpData_["Xmp.video.width"] = returnValue(buf, size);
        break;

    case 0x003a:
        xmpData_["Xmp.video.height"] = returnValue(buf, size);
        break;

    case 0x001a:
        internal_td = find(videoScanType ,returnValue(buf, size));
        xmpData_["Xmp.video.videoScanTpye"] = exvGettext(internal_td->label_);
        break;

    case 0x23a2:
        switch(stream) {
        case 1:
            xmpData_["Xmp.video.videoCodecInfo"] = buf.pData_;
            break;
        case 2:
            xmpData_["Xmp.audio.codecInfo"] = buf.pData_;
            break;
        case 17:
            xmpData_["Xmp.video.subTCodecInfo"] = buf.pData_;
            break;
        }
        break;

    case 0x3e383:
        switch(stream) {
        case 1:
            if(returnValue(buf, size))
                xmpData_["Xmp.video.frameRate"] = (double)1000000000/(double)returnValue(buf, size);
            else
                xmpData_["Xmp.video.frameRate"] = "Variable Bit Rate";
            break;
        case 2:
            if(returnValue(buf, size))
                xmpData_["Xmp.audio.defaultDuration"] = returnValue(buf, size)/1000;
            else
                xmpData_["Xmp.audio.defaultDuration"] = "Variable Bit Rate";
            break;
        default:
            xmpData_["Xmp.video.streamSampleRate"] = (double)1000000000/(double)returnValue(buf, size);
            break;
        }
        break;

    case 0x0035:
        xmpData_["Xmp.audio.sampleRate"] = Exiv2::getFloat(buf.pData_, bigEndian) ;
        break;

    case 0x38b5:
        xmpData_["Xmp.audio.outputSampleRate"] = Exiv2::getFloat(buf.pData_, bigEndian) ;
        break;

    case 0x001f:
        internal_td = find(audioChannels ,returnValue(buf, size));
        xmpData_["Xmp.audio.channelType"] = exvGettext(internal_td->label_);
        break;

    case 0x066e:
        xmpData_["Xmp.video.attachFileName"] = buf.pData_;
        break;

    case 0x0660:
        xmpData_["Xmp.video.attachFileMIME"] = buf.pData_;
        break;

    case 0x065c:
        xmpData_["Xmp.video.attachFileData"] = buf.pData_;
        break;

    case 0x0254:
        internal_td = find(compressionAlgorithm ,returnValue(buf, size));
        xmpData_["Xmp.video.contentCompressAlgo"] = exvGettext(internal_td->label_);
        break;

    case 0x07e1:
        internal_td = find(encryptionAlgorithm ,returnValue(buf, size));
        xmpData_["Xmp.video.contentEncryptAlgo"] = exvGettext(internal_td->label_);
        break;

    case 0x1033:
        internal_td = find(encodingType ,returnValue(buf, size));
        xmpData_["Xmp.video.contentEncodingType"] = exvGettext(internal_td->label_);
        break;

    case 0x067e:
        xmpData_["Xmp.video.attachFileDesc"] = buf.pData_;
        break;

    case 0x06ae:
        xmpData_["Xmp.video.attachFileUID"] = returnValue(buf, size);
        break;

    case 0x0286:
        xmpData_["Xmp.video.EBMLVersion"] = returnValue(buf, size);
        break;

    case 0x02f7:
        xmpData_["Xmp.video.EBMLReadVersion"] = returnValue(buf, size);
        break;

    case 0x2264:
        xmpData_["Xmp.audio.bitsPerSample"] = returnValue(buf, size);
        break;

    default:
        std::cerr <<std::setw(35)<<std::left<< exvGettext(td->label_)<<": ";
        std::cerr <<"("<<std::setw(5)<<std::right<<size<<"): ";
        std::cerr<<std::setw(20)<<std::left<< std::hex<<returnValue(buf, size)<<": "<<buf.pData_<<"\n";
        break;
    }
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
    else if (buf.pData_[0] & 1 )
        return 8;
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

    bool result = true;
    byte tmpBuf[4];
    iIo.read(tmpBuf, 4);

    if (iIo.error() || iIo.eof()) return false;

    if (0x1a != tmpBuf[0] || 0x45 != tmpBuf[1] || 0xdf != tmpBuf[2] || 0xa3 != tmpBuf[3] ) {
        result = false;
    }

    if (!advance || !result ) iIo.seek(0, BasicIo::beg);
    return result;
}
}
