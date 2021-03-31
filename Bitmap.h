/*!
 * @file Bitmap.h
 * @brief A library for creating and manipulating a Bitmap image file.
 *
 * @discussion This Bitmap file opening/manipulating library currently only supports:
 *             BM Header Types, BITMAPINFOHEADER DIBs with 24-bit Color
 *
 * @author Sargis Yonan (sargis@yonan.org)
 * @date 28 March 2021
 *
 * @note 100-line Ruler
 */

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace Pixel
{
    /*!
     * @struct BGR24
     * @brief Definition of a 24-bit pixel.
     * 
     * @field b The blue color for the pixel.
     * @field g The green color for the pixel.
     * @field r The red color for the pixel.
     */
    #pragma pack(push)  /* push current alignment to stack */
    #pragma pack(1)     /* set alignment to 4 byte boundary */
    struct BGR24
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
    };
    #pragma pack(pop)

    /*!
     * @struct BGR32
     * @brief Definition of a 32-bit pixel.
     * 
     * @field b     The blue color for the pixel.
     * @field g     The green color for the pixel.
     * @field r     The red color for the pixel.
     * @field alpha The alpha transparency value for the pixel.
     */
    #pragma pack(push)  /* push current alignment to stack */
    #pragma pack(4)     /* set alignment to 4 byte boundary */
    struct BGR32
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t alpha;
    };
    #pragma pack(pop)
}; /* namespace Pixel */

enum BMPError : int32_t
{
    BMP_SUCCESS,
    
    BMP_OOM             = 0xE001,
    BMP_FILEERROR       = 0xE002,
    BMP_OOB             = 0xE003,
    BMP_NOTINIT         = 0xE004,
    BMP_INVALID_HDR     = 0xE005,
    BMP_INVALID_DIB     = 0xE006,
    BMP_UNSUPPORTED_FMT = 0xE007,
    BMP_ALREADY_INIT    = 0xE008,
    BMP_BAD_INPUT       = 0xE009,
};

/*!
 * @constant DEFAULT_DPI
 * @brief The default DPI value to use when writing Bitmap images.
 */
#define DEFAULT_DPI (72U)

/*!
 * @class Bitmap
 * @brief Object for loading, manipulating, and writing Bitmap images.
 * 
 * @discussion This implementation supports BITMAPINFOHEADER DIB headers only.
 *
 * @tparam Pixel The type of the pixel to use. See Pixel namespace for pixel type definitions.
 */
template <typename Pixel>
class Bitmap
{
        /*!
     * @enum BitmapHeader
     * @brief Possible Bitmap header formats
     *
     * @const BM Windows 3.1x, 95, NT, ... etc.
     * @const BA OS/2 struct bitmap array
     * @const CI OS/2 struct color icon
     * @const CP OS/2 const color pointer
     * @const IC OS/2 struct icon
     * @const PT OS/2 pointer
     */
    enum HeaderType : uint16_t
    {
        BM = 0x4D42, /* 'B'=0x42, 'M'=0x4D */
        BA = 0x4142, /* 'B'=0x42, 'A'=0x41 */
        CI = 0x4943, /* 'C'=0x43, 'I'=0x49 */
        CP = 0x5043, /* 'C'=0x43, 'P'=0x50 */
        IC = 0x4349, /* 'I'=0x49, 'C'=0x43 */
        PT = 0x5450, /* 'P'=0x50, 'P'=0x54 */
    };

    /*!
     * @struct FileHeader
     * @brief Bitmap header metadata definition
     *
     * @field header_type The bitmap header type. See @p HeaderType for possible types.
     * @field size        The size of BMP file in bytes.
     * @field _rsvd1      Reserved; actual value depends on the application that creates the image.
     * @field _rsvd2      Reserved; actual value depends on the application that creates the image.
     * @field offset      The offset, i.e. starting address, of the byte where the bitmap image data 
     *                    (pixel array) can be found.
     *
     * @note @p _rsvd1 and @p _rsvd2 should be zeroed if not used.
     *
     * @discussion The total size of the FileHeader is:
     *             14B = 2 (Header) + 4 (Size) + 4 (Reserved) + 4 (Address Offset)
     *             Due to possible compiler optimizations, we will enforce 2B alignment 
     *             to enforce the size.
     */
    #pragma pack(push)  /* push current alignment to stack */
    #pragma pack(2)     /* set alignment to 2 byte boundary */
    struct FileHeader
    {
        HeaderType header_type;
        uint32_t   size;
        uint8_t    _rsvd1[2];
        uint8_t    _rsvd2[2];
        uint32_t   offset;
    };
    #pragma pack(pop)   /* restore original alignment from stack */

    /*!
     * @enum DIBHeaderType
     * @brief Device-Independent Bitmap header types
     *
     * @discussion The type is represented by the number of bytes the DIB section contains.
     * 
     * @const BITMAPCOREHEADER    Windows 2.0 or later. Also OS21XBITMAPHEADER (OS/2 1.x).
     * @const OS22XBITMAPHEADER64 OS/2 BITMAPCOREHEADER2. Adds halftoning. Adds RLE and 
     *                            Huffman 1D compression.
     * @const OS22XBITMAPHEADER16 This variant of the previous header contains only the first 
     *                            16 bytes and the remaining bytes are assumed to be zero values.
     * @const BITMAPINFOHEADER    Windows NT, 3.1x or later. Adds 16 bpp and 32 bpp formats. 
     *                            Adds RLE compression.
     * @const BITMAPV2INFOHEADER  Undocumented - Adobe Photoshop. Adds RGB bit masks.
     * @const BITMAPV3INFOHEADER  Undocumented - Adobe Photoshop. Adds alpha channel bit mask.
     * @const BITMAPV4HEADER      Windows NT 4.0, 95 or later. Adds color space type and gamma 
     *                            correction.
     * @const BITMAPV5HEADER      Windows NT 5.0, 98 or later - The GIMP. Adds ICC color profiles.
     */
    enum DIBHeaderType : uint32_t
    {
        BITMAPCOREHEADER    = 12,
        OS22XBITMAPHEADER64 = 64,
        OS22XBITMAPHEADER16 = 16,
        BITMAPINFOHEADER    = 40,
        BITMAPV2INFOHEADER  = 52,
        BITMAPV3INFOHEADER  = 56,
        BITMAPV4HEADER      = 108,
        BITMAPV5HEADER      = 124,
    };

    /*!
     * @enum BitsPerPixel
     * @brief BitmapInfoHeader type bits per pixel types.
     * 
     * @const Monochrome 1 - Monochrome palette -  1 color.
     * @const Palletized4 - 2^4 = 16 colors.
     * @const Palletized8 - 2^8 = 256 colors.
     * @const RGB16 - 2^16 = 64K colors.
     * @const RGB24 - 2^24 = 16M colors.
     * @const RGB32 - 2^32 = 4 Billion colors.
     */
    enum BitsPerPixel : uint16_t 
    {
        Monochrome  = 1,
        Palletized4 = 4,
        Palletized8 = 8,
        RGB16       = 16,
        RGB24       = 24,
        RGB32       = 32,
    };

    /*!
     * @enum InfoHeaderCompression
     * @brief BitmapInfoHeader type compression types.
     *
     * @const BI_RGB            None; most common.
     * @const BI_RLE8 RLE       8-bit/pixel Can be used only with 8-bit/pixel bitmaps.
     * @const BI_RLE4 RLE       4-bit/pixel Can be used only with 4-bit/pixel bitmaps.
     * @const BI_BITFIELDS      OS22XBITMAPHEADER: Huffman 1D   
     *                          BITMAPV2INFOHEADER: RGB bit field masks, BITMAPV3INFOHEADER+: RGBA.
     * @const BI_JPEG           OS22XBITMAPHEADER: RLE-24 - BITMAPV4INFOHEADER+: JPEG image for 
     *                          printing[14].
     * @const BI_PNG            BITMAPV4INFOHEADER+: PNG image for printing[14].
     * @const BI_ALPHABITFIELDS RGBA bit field masks only Windows CE 5.0 with .NET 4.0 or later.
     * @const BI_CMYK           None; only Windows Metafile CMYK[4].
     * @const BI_CMYKRLE8       RLE-8 - only Windows Metafile CMYK.
     * @const BI_CMYKRLE4       RLE-4 - only Windows Metafile CMYK.
     * 
     */
    enum Compression : uint32_t
    {
        BI_RGB            = 0,
        BI_RLE8           = 1,
        BI_RLE4           = 2,
        BI_BITFIELDS      = 3,
        BI_JPEG           = 4,
        BI_PNG            = 5,
        BI_ALPHABITFIELDS = 6,
        BI_CMYK           = 11,
        BI_CMYKRLE8       = 12,
        BI_CMYKRLE4       = 13,
    };

    /*!
     * @struct BitmapInfoHeader
     * @brief Definition for the BITMAPINFOHEADER DIB header type
     *
     * @field size         The size of this header, in bytes (40).
     * @field width        The bitmap width in pixels (signed integer).
     * @field height       The bitmap height in pixels (signed integer).
     * @field color_planes The number of color planes (must be 1).
     * @field bpp          The number of bits per pixel, which is the color depth of the image. 
     *                     Typical values are 1, 4, 8, 16, 24 and 32. See @p BitsPerPixel.
     * @field compression  The compression method being used. See @p InfoHeaderCompression.
     * @field raw_size     The image size. This is the size of the raw bitmap data. 
     *                     A dummy 0 can be given for BI_RGB bitmaps.
     * @field hres         The horizontal resolution of the image. (pixel/metre, signed integer).
     * @field vres         The vertical resolution of the image. (pixel/metre, signed integer).
     * @field n_colors     The number of colors in the color palette, or 0 to default to 2n.
     * @field icolors      The number of important colors used, or 0 when every color is important; 
     *                     generally ignored.
     *
     * @discussion The total size of BitmapInfoHeader is:
     *             40B --> % 4 == 0. Align to 4B alignment.
     */
    #pragma pack(push)  /* push current alignment to stack */
    #pragma pack(4)     /* set alignment to 4 byte boundary */
    struct BitmapInfoHeader {
        uint32_t     size;
        int32_t      width;
        int32_t      height;
        uint16_t     color_planes;
        BitsPerPixel bbp;
        Compression  compression;
        uint32_t     raw_size;
        int32_t      hres;
        int32_t      vres;
        uint32_t     n_colors;
        uint32_t     icolors;
    };
    #pragma pack(pop)

    /*!
     * @enum BitmapInfoHeaderOffset
     * @brief Offset locations in the BitmapInfoHeader type DIB structure.
     *
     * @note These offset locations are relative to the start of the file, not the DIB structure.
     *
     * @const SizeAddress offset.           
     * @const WidthAddress offset.          
     * @const HeightAddress offset.         
     * @const ColorPlanesAddress offset.    
     * @const BBPAddress offset.            
     * @const CompressionAddress offset.    
     * @const RawSizeAddress offset.        
     * @const HorizontalResAddress offset.  
     * @const VerticalResAddress offset.    
     * @const NumColorsAddress offset.      
     * @const ImportantColorsAddress offset.
     * @const PixelDataStartAddress offset.
     */
    enum BitmapInfoHeaderOffset : uint32_t
    {
        SizeAddress            = 0x000E,
        WidthAddress           = 0x0012,
        HeightAddress          = 0x0016,
        ColorPlanesAddress     = 0x001A,
        BBPAddress             = 0x001C,
        CompressionAddress     = 0x001E,
        RawSizeAddress         = 0x0022,
        HorizontalResAddress   = 0x0026,
        VerticalResAddress     = 0x002A,
        NumColorsAddress       = 0x002E,
        ImportantColorsAddress = 0x0032,
        PixelDataStartAddress  = 0x0036,
    };

    /*!
     * @struct ColorTable
     * @brief Present only if BitsPerPixel <= 8 - importance of colors.
     *
     * @field red_intensity
     * @field green_intensity
     * @field blue_intensity
     * @field rsvd Not used.
     */
    struct ColorTable 
    {
        uint8_t red_intensity;
        uint8_t green_intensity;
        uint8_t blue_intensity;
        uint8_t rsvd;
    };

public:
    /*! 
     * Constructor
     * @param[in] _dpi Dots per pixel in inches per metre
     */
    Bitmap(const uint32_t _dpi = DEFAULT_DPI) : loaded(false), dpi(_dpi) {}
    
    /*! 
     * Destructor
     */
    ~Bitmap();

    /*!
     * @function load
     * @brief Load a Bitmap file into program memory. 
     * 
     * @discussion The loaded file is the object file of this instance.
     *
     * @param[in] filename The name of th file to load.
     *
     * @return BMP_SUCCESS upon sucess, else relevant @p BMPError status.
     */
    BMPError load(const char * const filename);

    /*!
     * @function create
     * @brief Create a new blank Bitmap image.
     *
     * @param[in] filename The name of the file to load.
     *
     * @return BMP_SUCCESS upon sucess, else relevant @p BMPError status.
     */
    BMPError create(const uint64_t width, const uint64_t height);

    /*!
     * @function write
     * @brief Write the currently loaded file to the filesystem.
     *
     * @param[in] filename The name of the file to write to.
     *
     * @return BMP_SUCCESS upon sucess, else relevant @p BMPError status.
     */
    BMPError write(const char * const filename);

    /*!
     * @function width
     * @brief Get the width of the currently loaded image.
     *
     * @return The image width. BMP_NOTINIT if not loaded.
     */
    int32_t width() const;

    /*!
     * @function height
     * @brief Get the height of the currently loaded image.
     *
     * @return The image height. BMP_NOTINIT if not loaded.
     */
    int32_t height() const;
    
    /*!
     * @function get
     * @brief Get the pixel value at a given coordinate.
     *
     * @param[in]  row   The row coordinate to pull from.
     * @param[in]  col   The column coordinate to pull from.
     * @param[out] pixel The pixel at @p row @p col.
     *
     * @return BMP_SUCCESS upon sucess, else relevant @p BMPError status.
     */
    BMPError get(const uint32_t row, const uint32_t col, Pixel &pixel) const;

    /*!
     * @function set
     * @brief Set the pixel value at a given coordinate.
     *
     * @param[in] row   The row coordinate to set.
     * @param[in] col   The column coordinate to set.
     * @param[in] pixel The pixel at @p row @p col.
     *
     * @return BMP_SUCCESS upon sucess, else relevant @p BMPError status.
     */
    BMPError set(const uint64_t row, const uint64_t col, Pixel &pixel);

    /*!
     * @function WriteHeaderRsvd
     * @brief Write to the reserved metadata field of the Bitmap file header.
     *
     * @param[in] data The 4B of data to write.
     *
     * @return BMP_SUCCESS upon sucess, else relevant @p BMPError status.
     */
    BMPError WriteHeaderRsvd(const uint8_t data[4]);
    
    /*!
     * @function ReadHeaderRsvd
     * @brief Read the reserved metadata field of the Bitmap file header.
     *
     * @param[out] data The 4B of data read from the reserved bytes.
     *
     * @return BMP_SUCCESS upon sucess, else relevant @p BMPError status.
     */
    BMPError ReadHeaderRsvd(uint8_t data[4]) const;

protected:
    /*!
     * @inline pixel_index
     * @brief Get the 1-Dimensional array offset of @p pixel_array from 2-Dimensional coordinates.
     *
     * @param[in] row The row coordinate.
     * @param[in] col The column coordinate.
     *
     * @return The 2D --> 1D pixel map.
     */
    inline uint64_t pixel_index(const int32_t row, const int32_t col) const 
    { 
        return row + (col * dib.width); 
    }

    /*!
     * @inline pixel_max
     * @brief Get the max number of the pixel data array.
     *
     * @return The number of pixels.
     */
    inline uint64_t pixel_max() const 
    { 
        return dib.width * dib.height; 
    }
    
    /*!
     * @var pixel_array
     * @brief A pointer to the internal pixel data of the Bitmap image.
     */
    Pixel *pixel_array;

    /*!
     * @var loaded
     * @brief A flag to indicate that a Bitmap image has been loaded by this instance.
     */
    bool loaded;

private:
    /*!
     * @var dib
     * @brief The Device Independent header for this Bitmap image.
     */
    BitmapInfoHeader dib;
    
    /*!
     * @var file_header
     * @brief The file header for this Bitmap image.
     */
    FileHeader file_header;

    /*!
     * @var dpi
     * @brief Dots per inch setting for this instance.
     */
    uint32_t dpi;
}; /* class Bitmap<> */

/* Templated Base Class Definitions */

template <typename Pixel>
Bitmap<Pixel>::~Bitmap()
{
    if (pixel_array)
    {
        delete[] pixel_array;
        pixel_array = nullptr;
    }

    memset(&dib, 0, sizeof(dib));
    memset(&file_header, 0, sizeof(file_header));

    loaded = false;
}

template <typename Pixel>
BMPError Bitmap<Pixel>::load(const char * const filename)
{
    if (loaded)
    {
        return BMP_ALREADY_INIT;
    }

    FILE *fptr = fopen(filename, "rb");
    if (!fptr)
    {
        return BMP_FILEERROR;
    }
    fread(&file_header, sizeof(file_header), 1, fptr);

    if (file_header.header_type != Bitmap::HeaderType::BM)
    {
        return BMP_INVALID_HDR;
    }

    fread((void*)&dib, DIBHeaderType::BITMAPINFOHEADER, 1, fptr);
    if (dib.size != DIBHeaderType::BITMAPINFOHEADER)
    {
        return BMP_UNSUPPORTED_FMT;
    }

    if (dib.bbp != 8 * sizeof(Pixel))
    {
        return BMP_UNSUPPORTED_FMT;
    }

    if (dib.compression != Bitmap::Compression::BI_RGB)
    {
        return BMP_UNSUPPORTED_FMT;
    }

    if (dib.color_planes != 1)
    {
        return BMP_INVALID_DIB;
    }

    // Assuming dib.vres == dib.hres
    dpi = dib.hres;

    pixel_array = new Pixel[dib.width * dib.height];
    if (!pixel_array)
    {
        return BMP_OOM;
    }
    
    fread((void*)pixel_array, dib.width * dib.height * sizeof(Pixel), 1, fptr);
    loaded = true;
    fclose(fptr);

    return BMP_SUCCESS;
}

template <typename Pixel>
BMPError Bitmap<Pixel>::create(const uint64_t width, const uint64_t height)
{
    if (loaded)
    {
        return BMP_ALREADY_INIT;
    }

    file_header.header_type = Bitmap::HeaderType::BM;
    
    dib.raw_size = (width * height) * sizeof(Pixel);
    
    file_header.offset = DIBHeaderType::BITMAPINFOHEADER + sizeof(FileHeader);
    file_header.size = file_header.offset + dib.raw_size;

    dib.size = DIBHeaderType::BITMAPINFOHEADER;
    dib.height = height;
    dib.width = width;
    dib.color_planes = 1;
    dib.bbp = static_cast<Bitmap::BitsPerPixel>(8 * sizeof(Pixel));
    dib.compression = Bitmap::Compression::BI_RGB;
    dib.n_colors = 0;
    dib.icolors = 0;

    // DPI × 39.3701 inches per metre
    dib.vres = static_cast<uint32_t>(dpi * 39.3701);
    dib.hres = dib.vres;

    pixel_array = new Pixel[width * height];
    if (!pixel_array)
    {
        return BMP_OOM;
    }

    memset(pixel_array, 0, width * height * sizeof(Pixel));

    loaded = true;
    return BMP_SUCCESS;
}

template <typename Pixel>
BMPError Bitmap<Pixel>::write(const char * const filename)
{
    if (!loaded)
    {
        return BMP_NOTINIT;
    }

    FILE *fptr = fopen(filename, "wb+");
    if (!fptr)
    {
        return BMP_FILEERROR;
    }

    fwrite((void*)&file_header, sizeof(FileHeader), 1, fptr);
    fwrite((void*)&dib, DIBHeaderType::BITMAPINFOHEADER, 1, fptr);
    fwrite((void*)pixel_array, dib.width * dib.height * sizeof(Pixel), 1, fptr);
    
    // 4B alignment
    size_t padding = file_header.size % sizeof(uint32_t);
    if (padding > 0)
    {
        const uint8_t zero = 0;
        for (uint8_t pad = 0; pad < 4 - padding; pad++)
        {
            fwrite((void*)&zero, 1, 1, fptr);    
        }        
    }

    fclose(fptr);

    return BMP_SUCCESS;
}

template <typename Pixel>
BMPError Bitmap<Pixel>::get(const uint32_t row, const uint32_t col, Pixel &pixel) const
{
    if (!pixel_array || !loaded)
    {
        return BMP_NOTINIT;
    }

    const uint64_t idx = pixel_index(row, col);
    if (idx >= pixel_max())
    {
        return BMP_OOB;
    }

    memcpy(&pixel, &pixel_array[idx], sizeof(pixel));

    return BMP_SUCCESS;
}

template <typename Pixel>
BMPError Bitmap<Pixel>::set(const uint64_t row, const uint64_t col, Pixel &pixel)
{
    if (!pixel_array || !loaded)
    {
        return BMP_NOTINIT;
    }

    const uint64_t idx = pixel_index(row, col);
    if (idx >= pixel_max())
    {
        return BMP_OOB;
    }

    memcpy(&pixel_array[idx], &pixel, sizeof(pixel));

    return BMP_SUCCESS;
}

template <typename Pixel>
int32_t Bitmap<Pixel>::width() const
{
    if (loaded)
    {
        return dib.width;
    }

    return BMP_NOTINIT;
}

template <typename Pixel>
int32_t Bitmap<Pixel>::height() const
{
    if (loaded)
    {
        return dib.width;
    }

    return BMP_NOTINIT;
}

template <typename Pixel>
BMPError Bitmap<Pixel>::WriteHeaderRsvd(const uint8_t data[4])
{
    if (!loaded)
    {
        return BMP_NOTINIT;
    }

    if (!data)
    {
        return BMP_BAD_INPUT;
    }

    file_header._rsvd1[0] = data[0];
    file_header._rsvd1[1] = data[1];
    file_header._rsvd2[0] = data[2];
    file_header._rsvd2[1] = data[3];

    return BMP_SUCCESS;
}

template <typename Pixel>
BMPError Bitmap<Pixel>::ReadHeaderRsvd(uint8_t data[4]) const
{
    if (!loaded)
    {
        return BMP_NOTINIT;
    }

    if (!data)
    {
        return BMP_BAD_INPUT;
    }

    data[0] = file_header._rsvd1[0];
    data[1] = file_header._rsvd1[1];
    data[2] = file_header._rsvd2[0];
    data[3] = file_header._rsvd2[1];

    return BMP_SUCCESS;
}
