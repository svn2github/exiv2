 
#ifndef MATROSKAVIDEO_HPP
#define MATROSKAVIDEO_HPP

#include "exif.hpp"
#include "image.hpp"
#include "tags_int.hpp"

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
    /*!
      @brief Function used to calulate Size of a block.
             This information is only stored in one byte.
             The size of the block is calculated by counting
             the number of leading zeros in the binary code of the byte.
             Size = (No. of Leading Zeros + 1) bytes
      @param Exiv2 DataBuffer, which stores the information
      @return Returns an integer value
    */
    int findBlockSize(Exiv2::DataBuf& buf);

    /*!
      @brief Function checks for a valid Tag, and takes the required action.
             Calls contentManagement() or skips to next Tag, if required
    */
    void decodeBlock();

    /*!
      @brief Function used to interpret the Tag information,
             and save it in the respective XMP container
      @param TagDetails pointer (points to current Tag), Exiv2 DataBuffer (stores the Tag information),
             unsignedLong size (contains the Size of DataBuf)
    */
    void contentManagement(const Exiv2::Internal::TagDetails* td, Exiv2::DataBuf & buf, unsigned long size);

private:
    //! Variable to check the End Of Metadata Traversing
    bool continueTraversing_;

}; //MatroskaVideo End

EXIV2API Image::AutoPtr newMkvInstance(BasicIo::AutoPtr io, bool create);

//! Check if the file iIo is a Matroska Video.
EXIV2API bool isMkvType(BasicIo& iIo, bool advance);


}                                       // namespace Exiv2


#endif // MatroskaVideo_HPP
