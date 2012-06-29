//QuickTime Video - Detection And Display

#include "quicktimevideo.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include <math.h>

namespace Exiv2 {
    namespace Internal {

    extern const TagVocabulary qTimeFileType[] =  {
        {   "3g2a", "3GPP2 Media (.3G2) compliant with 3GPP2 C.S0050-0 V1.0" },
        {   "3g2b", "3GPP2 Media (.3G2) compliant with 3GPP2 C.S0050-A V1.0.0" },
        {   "3g2c", "3GPP2 Media (.3G2) compliant with 3GPP2 C.S0050-B v1.0" },
        {   "3ge6", "3GPP (.3GP) Release 6 MBMS Extended Presentations" },
        {   "3ge7", "3GPP (.3GP) Release 7 MBMS Extended Presentations" },
        {   "3gg6", "3GPP Release 6 General Profile" },
        {   "3gp1", "3GPP Media (.3GP) Release 1 (probably non-existent)" },
        {   "3gp2", "3GPP Media (.3GP) Release 2 (probably non-existent)" },
        {   "3gp3", "3GPP Media (.3GP) Release 3 (probably non-existent)" },
        {   "3gp4", "3GPP Media (.3GP) Release 4" },
        {   "3gp5", "3GPP Media (.3GP) Release 5" },
        {   "3gp6", "3GPP Media (.3GP) Release 6 Streaming Servers" },
        {   "3gs7", "3GPP Media (.3GP) Release 7 Streaming Servers" },
        {   "CAEP", "Canon Digital Camera" },
        {   "CDes", "Convergent Design" },
        {   "F4A ", "Audio for Adobe Flash Player 9+ (.F4A)" },
        {   "F4B ", "Audio Book for Adobe Flash Player 9+ (.F4B)" },
        {   "F4P ", "Protected Video for Adobe Flash Player 9+ (.F4P)" },
        {   "F4V ", "Video for Adobe Flash Player 9+ (.F4V)" },
        {   "JP2 ", "JPEG 2000 Image (.JP2) [ISO 15444-1 ?]" },
        {   "JP20", "Unknown, from GPAC samples (prob non-existent)" },
        {   "KDDI", "3GPP2 EZmovie for KDDI 3G cellphones" },
        {   "M4A ", "Apple iTunes AAC-LC (.M4A) Audio" },
        {   "M4B ", "Apple iTunes AAC-LC (.M4B) Audio Book" },
        {   "M4P ", "Apple iTunes AAC-LC (.M4P) AES Protected Audio" },
        {   "M4V ", "Apple iTunes Video (.M4V) Video" },
        {   "M4VH", "Apple TV (.M4V)" },
        {   "M4VP", "Apple iPhone (.M4V)" },
        {   "MPPI", "Photo Player, MAF [ISO/IEC 23000-3]" },
        {   "MSNV", "MPEG-4 (.MP4) for SonyPSP" },
        {   "NDAS", "MP4 v2 [ISO 14496-14] Nero Digital AAC Audio" },
        {   "NDSC", "MPEG-4 (.MP4) Nero Cinema Profile" },
        {   "NDSH", "MPEG-4 (.MP4) Nero HDTV Profile" },
        {   "NDSM", "MPEG-4 (.MP4) Nero Mobile Profile" },
        {   "NDSP", "MPEG-4 (.MP4) Nero Portable Profile" },
        {   "NDSS", "MPEG-4 (.MP4) Nero Standard Profile" },
        {   "NDXC", "H.264/MPEG-4 AVC (.MP4) Nero Cinema Profile" },
        {   "NDXH", "H.264/MPEG-4 AVC (.MP4) Nero HDTV Profile" },
        {   "NDXM", "H.264/MPEG-4 AVC (.MP4) Nero Mobile Profile" },
        {   "NDXP", "H.264/MPEG-4 AVC (.MP4) Nero Portable Profile" },
        {   "NDXS", "H.264/MPEG-4 AVC (.MP4) Nero Standard Profile" },
        {   "ROSS", "Ross Video" },
        {   "avc1", "MP4 Base w/ AVC ext [ISO 14496-12:2005]" },
        {   "caqv", "Casio Digital Camera" },
        {   "da0a", "DMB MAF w/ MPEG Layer II aud, MOT slides, DLS, JPG/PNG/MNG images" },
        {   "da0b", "DMB MAF, extending DA0A, with 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "da1a", "DMB MAF audio with ER-BSAC audio, JPG/PNG/MNG images" },
        {   "da1b", "DMB MAF, extending da1a, with 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "da2a", "DMB MAF aud w/ HE-AAC v2 aud, MOT slides, DLS, JPG/PNG/MNG images" },
        {   "da2b", "DMB MAF, extending da2a, with 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "da3a", "DMB MAF aud with HE-AAC aud, JPG/PNG/MNG images" },
        {   "da3b", "DMB MAF, extending da3a w/ BIFS, 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "dmb1", "DMB MAF supporting all the components defined in the specification" },
        {   "dmpf", "Digital Media Project" },
        {   "drc1", "Dirac (wavelet compression), encapsulated in ISO base media (MP4)" },
        {   "dv1a", "DMB MAF vid w/ AVC vid, ER-BSAC aud, BIFS, JPG/PNG/MNG images, TS" },
        {   "dv1b", "DMB MAF, extending dv1a, with 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "dv2a", "DMB MAF vid w/ AVC vid, HE-AAC v2 aud, BIFS, JPG/PNG/MNG images, TS" },
        {   "dv2b", "DMB MAF, extending dv2a, with 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "dv3a", "DMB MAF vid w/ AVC vid, HE-AAC aud, BIFS, JPG/PNG/MNG images, TS" },
        {   "dv3b", "DMB MAF, extending dv3a, with 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "dvr1", "DVB (.DVB) over RTP" },
        {   "dvt1", "DVB (.DVB) over MPEG-2 Transport Stream" },
        {   "isc2", "ISMACryp 2.0 Encrypted File" },
        {   "iso2", "MP4 Base Media v2 [ISO 14496-12:2005]" },
        {   "isom", "MP4 Base Media v1 [IS0 14496-12:2003]" },
        {   "jpm ", "JPEG 2000 Compound Image (.JPM) [ISO 15444-6]" },
        {   "jpx ", "JPEG 2000 with extensions (.JPX) [ISO 15444-2]" },
        {   "mj2s", "Motion JPEG 2000 [ISO 15444-3] Simple Profile" },
        {   "mjp2", "Motion JPEG 2000 [ISO 15444-3] General Profile" },
        {   "mmp4", "MPEG-4/3GPP Mobile Profile (.MP4/3GP) (for NTT)" },
        {   "mp21", "MPEG-21 [ISO/IEC 21000-9]" },
        {   "mp41", "MP4 v1 [ISO 14496-1:ch13]" },
        {   "mp42", "MP4 v2 [ISO 14496-14]" },
        {   "mp71", "MP4 w/ MPEG-7 Metadata [per ISO 14496-12]" },
        {   "mqt ", "Sony / Mobile QuickTime (.MQV) US Patent 7,477,830 (Sony Corp)" },
        {   "odcf", "OMA DCF DRM Format 2.0 (OMA-TS-DRM-DCF-V2_0-20060303-A)" },
        {   "opf2", "OMA PDCF DRM Format 2.1 (OMA-TS-DRM-DCF-V2_1-20070724-C)" },
        {   "opx2", "OMA PDCF DRM + XBS extensions (OMA-TS-DRM_XBS-V1_0-20070529-C)" },
        {   "pana", "Panasonic Digital Camera" },
        {   "qt ", "Apple QuickTime (.MOV/QT)" },
        {   "sdv ", "SD Memory Card Video" },
        {   "ssc1", "Samsung stereoscopic, single stream" },
        {   "ssc2", "Samsung stereoscopic, dual stream" }
    };

    bool isQuickTimeType (char a, char b, char c, char d) {
        char qTimeTags[][5] = {
            "PICT", "free", "ftyp", "junk", "mdat",
            "moov", "pict", "pnot", "skip",  "uuid", "wide"
        };

        for(int i = 0; i <= 10; i++)
            if(a == qTimeTags[i][0] && b == qTimeTags[i][1] && c == qTimeTags[i][2] && d == qTimeTags[i][3])
                return true;

        return false;
    }

    }
}

using namespace Exiv2::Internal;

namespace Exiv2 {

QuickTimeVideo::QuickTimeVideo(BasicIo::AutoPtr io)
        : Image(ImageType::qtime, mdNone, io) {
} // QuickTimeVideo::QuickTimeVideo

std::string QuickTimeVideo::mimeType() const {
    return "video/quicktime";
}

void QuickTimeVideo::writeMetadata() {
}


void QuickTimeVideo::readMetadata() {

    if (io_->open() != 0) throw Error(9, io_->path(), strError());

    // Ensure that this is the correct image type
    if (!isQTimeType(*io_, false)) {
        if (io_->error() || io_->eof()) throw Error(14);
        throw Error(3, "QuickTime");
    }

    IoCloser closer(*io_);
    clearMetadata();

    for (int i =0 ;i <= 500; i++) {
//        std::cout<<std::setw(3)<<i<<": ";
        decodeBlock();
    }
}

void QuickTimeVideo::decodeBlock() {
    const long bufMinSize = 50;
    DataBuf buf(bufMinSize);
    DataBuf buf2(bufMinSize);
    unsigned long size = 0;

    std::memset(buf.pData_, 0x0, buf.size_);

    io_->read(buf.pData_, 4);
    size = Exiv2::getULong(buf.pData_, bigEndian);

/*    const TagDetails* td;
    td = find(matroskaTags , (returnTagValue(buf, buf2, s_ID)) );

    if (dataIgnoreList(td->val_))
        display = false;
    if (ignoreList(td->val_))
        readData = false;

    io_->read(buf.pData_, 1);
    s_Size = findBlockSize(buf);

    if(s_Size != 0)
        io_->read(buf2.pData_, s_Size - 1);

    size = returnTagValue(buf, buf2, s_Size);

    if (!display && readData)
        return;
*/
    io_->read(buf.pData_, size-4);
    std::cerr <<"("<<std::setw(5)<<std::right<<size<<"): ";
    std::cerr<<std::setw(20)<<std::left<<Exiv2::getULong(buf.pData_, bigEndian)<<": "<<buf.pData_<<"\n";

}

Image::AutoPtr newQTimeInstance(BasicIo::AutoPtr io, bool /*create*/) {
    Image::AutoPtr image(new QuickTimeVideo(io));
    if (!image->good()) {
        image.reset();
    }
    return image;
}

bool isQTimeType(BasicIo& iIo, bool advance) {
    const int32_t len = 4;
    byte buf[len];
    iIo.read(buf, len);
    iIo.read(buf, len);

    if (iIo.error() || iIo.eof()) {
        return false;
    }

    bool matched = isQuickTimeType(buf[0], buf[1], buf[2], buf[3]);
    if (!advance || !matched) {
        iIo.seek(0, BasicIo::beg);
    }

    return matched;
}
}
