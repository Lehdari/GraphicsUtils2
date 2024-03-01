//
// Project: GraphicsUtils2
// File: ImageConversion.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include "Macros.hpp"
#include "MathTypes.hpp"
#include "MathUtils.hpp"

#include <immintrin.h>


#define GU2_IMAGE_FORMAT_CONVERSION_TO_RGBA_MATRIX(IMAGE_FORMAT, ...)       \
    ToRGBAMatrix<ImageFormat::IMAGE_FORMAT>                                 \
    ImageFormatConversionParams<ImageFormat::IMAGE_FORMAT>::toRGBAMatrix =  \
    initializeMatrix<ToRGBAMatrix<ImageFormat::IMAGE_FORMAT>>(__VA_ARGS__);

#define GU2_IMAGE_FORMAT_CONVERSION_FROM_RGBA_MATRIX(IMAGE_FORMAT, ...)         \
    FromRGBAMatrix<ImageFormat::IMAGE_FORMAT>                                   \
    ImageFormatConversionParams<ImageFormat::IMAGE_FORMAT>::fromRGBAMatrix =    \
    initializeMatrix<FromRGBAMatrix<ImageFormat::IMAGE_FORMAT>>(__VA_ARGS__);


// X macro for image formats, used for generating conversion machinery
// TODO add support for linear formats
#define GU2_IMAGE_FORMATS(GU2_IMAGE_FORMAT) \
    GU2_IMAGE_FORMAT(RGBA_GAMMA)            \
    GU2_IMAGE_FORMAT(RGB_GAMMA)             \
    GU2_IMAGE_FORMAT(BGRA_GAMMA)            \
    GU2_IMAGE_FORMAT(BGR_GAMMA)             \
    GU2_IMAGE_FORMAT(YUV)                   \
    GU2_IMAGE_FORMAT(GRAY)


namespace gu2 {


namespace detail {


namespace imageFormatFlags {
    constexpr uint32_t  gammaBit        {0x01000000};
    constexpr uint32_t  nChannelsMask   {0xf0000000};
    constexpr uint32_t  nChannelsShift  {28};
} // namespace imageFormatFlags;


consteval uint32_t encodeImageFormatNChannels(int nChannels)
{
    return static_cast<uint32_t>(nChannels) << detail::imageFormatFlags::nChannelsShift;
}


} // namespace detail


enum class ImageFormat : uint32_t {
    UNCHANGED   = 0, // Only allowed as an argument to convertImage, used for converting between data types
    RGBA_LINEAR = 1 | detail::encodeImageFormatNChannels(4),
    RGBA_GAMMA  = RGBA_LINEAR | detail::imageFormatFlags::gammaBit,
    RGBA        = RGBA_GAMMA,
    RGB_LINEAR  = 2 | detail::encodeImageFormatNChannels(3),
    RGB_GAMMA   = RGB_LINEAR | detail::imageFormatFlags::gammaBit,
    RGB         = RGB_GAMMA,
    BGRA_LINEAR = 3 | detail::encodeImageFormatNChannels(4),
    BGRA_GAMMA  = BGRA_LINEAR | detail::imageFormatFlags::gammaBit,
    BGRA        = BGRA_GAMMA,
    BGR_LINEAR  = 4 | detail::encodeImageFormatNChannels(3),
    BGR_GAMMA   = BGR_LINEAR | detail::imageFormatFlags::gammaBit,
    BGR         = BGR_GAMMA,
    YUV         = 5 | detail::encodeImageFormatNChannels(3),
    GRAY        = 6 | detail::encodeImageFormatNChannels(1),
    UNKNOWN     = 0x00ffffff
};


constexpr int getImageFormatNChannels(ImageFormat imageFormat)
{
    return static_cast<int>((static_cast<uint32_t>(imageFormat) & detail::imageFormatFlags::nChannelsMask) >>
        detail::imageFormatFlags::nChannelsShift);
}


template <typename T_Data>
class Image;


namespace detail {


// Format conversion matrix aliases
template <ImageFormat T_ImageFormat>
using ToRGBAMatrix = Eigen::Matrix<double, 4, getImageFormatNChannels(T_ImageFormat)>;

template <ImageFormat T_ImageFormat>
using FromRGBAMatrix = Eigen::Matrix<double, getImageFormatNChannels(T_ImageFormat), 4>;

template <typename T_Data, ImageFormat T_SrcImageFormat, ImageFormat T_DestImageFormat>
using ConversionMatrix = Eigen::Matrix<T_Data, getImageFormatNChannels(T_DestImageFormat),
    getImageFormatNChannels(T_SrcImageFormat)>;


// Format conversion parameters
template <ImageFormat T_ImageFormat>
struct ImageFormatConversionParams {};

template <> struct ImageFormatConversionParams<ImageFormat::RGBA_GAMMA> {
    static ToRGBAMatrix<ImageFormat::RGBA_GAMMA>    toRGBAMatrix;
    static FromRGBAMatrix<ImageFormat::RGBA_GAMMA>  fromRGBAMatrix;
    static constexpr int8_t                         toRGBAShuffle[4]    {0, 1, 2, 3};
    static constexpr int8_t                         fromRGBAShuffle[4]  {0, 1, 2, 3};
};
GU2_IMAGE_FORMAT_CONVERSION_TO_RGBA_MATRIX(RGBA_GAMMA,
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
);
GU2_IMAGE_FORMAT_CONVERSION_FROM_RGBA_MATRIX(RGBA_GAMMA,
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
);

template <> struct ImageFormatConversionParams<ImageFormat::RGB_GAMMA> {
    static ToRGBAMatrix<ImageFormat::RGB_GAMMA>     toRGBAMatrix;
    static FromRGBAMatrix<ImageFormat::RGB_GAMMA>   fromRGBAMatrix;
    static constexpr int8_t                         toRGBAShuffle[4]    {0, 1, 2, -1};
    static constexpr int8_t                         fromRGBAShuffle[3]  {0, 1, 2};
};
GU2_IMAGE_FORMAT_CONVERSION_TO_RGBA_MATRIX(RGB_GAMMA,
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,
    0.0, 0.0, 0.0
);
GU2_IMAGE_FORMAT_CONVERSION_FROM_RGBA_MATRIX(RGB_GAMMA,
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0
);

template <> struct ImageFormatConversionParams<ImageFormat::BGRA_GAMMA> {
    static ToRGBAMatrix<ImageFormat::BGRA_GAMMA>    toRGBAMatrix;
    static FromRGBAMatrix<ImageFormat::BGRA_GAMMA>  fromRGBAMatrix;
    static constexpr int8_t                         toRGBAShuffle[4]    {2, 1, 0, 3};
    static constexpr int8_t                         fromRGBAShuffle[3]  {2, 1, 0};
};
GU2_IMAGE_FORMAT_CONVERSION_TO_RGBA_MATRIX(BGRA_GAMMA,
    0.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0
);
GU2_IMAGE_FORMAT_CONVERSION_FROM_RGBA_MATRIX(BGRA_GAMMA,
    0.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0
);

template <> struct ImageFormatConversionParams<ImageFormat::BGR_GAMMA> {
    static ToRGBAMatrix<ImageFormat::BGR_GAMMA>     toRGBAMatrix;
    static FromRGBAMatrix<ImageFormat::BGR_GAMMA>   fromRGBAMatrix;
    static constexpr int8_t                         toRGBAShuffle[4]    {2, 1, 0, -1};
    static constexpr int8_t                         fromRGBAShuffle[3]  {2, 1, 0};
};
GU2_IMAGE_FORMAT_CONVERSION_TO_RGBA_MATRIX(BGR_GAMMA,
    0.0, 0.0, 1.0,
    0.0, 1.0, 0.0,
    1.0, 0.0, 0.0,
    0.0, 0.0, 0.0
);
GU2_IMAGE_FORMAT_CONVERSION_FROM_RGBA_MATRIX(BGR_GAMMA,
    0.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    1.0, 0.0, 0.0, 0.0
);

template <> struct ImageFormatConversionParams<ImageFormat::YUV> {
    static ToRGBAMatrix<ImageFormat::YUV> toRGBAMatrix;
    static FromRGBAMatrix<ImageFormat::YUV> fromRGBAMatrix;
};
GU2_IMAGE_FORMAT_CONVERSION_TO_RGBA_MATRIX(YUV,
    0.99998,    2.03211,        -1.5082e-05,
    1.0,        -0.394646,      -0.580594,
    1.0,        -1.17892e-05,   1.13983,
    0.0,        0.0,            0.0
);
GU2_IMAGE_FORMAT_CONVERSION_FROM_RGBA_MATRIX(YUV,
    0.114,      0.587,      0.299,      0.0,
    0.436,      -0.28886,   -0.14713,   0.0,
    -0.10001,   -0.51499,   0.615,      0.0
);

template <> struct ImageFormatConversionParams<ImageFormat::GRAY> {
    static ToRGBAMatrix<ImageFormat::GRAY>      toRGBAMatrix;
    static FromRGBAMatrix<ImageFormat::GRAY>    fromRGBAMatrix;
    static constexpr int8_t                     toRGBAShuffle[4]    {0, 0, 0, -1};
};
GU2_IMAGE_FORMAT_CONVERSION_TO_RGBA_MATRIX(GRAY,
    1.0,
    1.0,
    1.0,
    0.0
);
GU2_IMAGE_FORMAT_CONVERSION_FROM_RGBA_MATRIX(GRAY,
    0.299,  0.587,  0.114,  0.0
);


// Data type parameters
template <typename T_Data>
struct ImageDataParams {};

template <> struct ImageDataParams<uint8_t> {
    static constexpr uint8_t    pixelSaturation {0xff};
};

template <> struct ImageDataParams<uint16_t> {
    static constexpr uint16_t   pixelSaturation {0xffff};
};

template <> struct ImageDataParams<uint32_t> {
    static constexpr uint32_t   pixelSaturation {0xffffffff};
};


// Data type conversion parameters
template <typename T_DataSrc, typename T_DataDest>
struct ImageDataConversionParams {};


// Helper function for creating combined conversion matrices
template <typename T_Data, ImageFormat T_SrcImageFormat, ImageFormat T_DestImageFormat>
const ConversionMatrix<T_Data, T_SrcImageFormat, T_DestImageFormat>& getImageFormatConversionMatrix()
{
    static ConversionMatrix<T_Data, T_SrcImageFormat, T_DestImageFormat> matrix =
        (ImageFormatConversionParams<T_DestImageFormat>::fromRGBAMatrix *
        ImageFormatConversionParams<T_SrcImageFormat>::toRGBAMatrix).eval().template cast<T_Data>();
    return matrix;
}

// Helper function for creating combined shuffle indices (does not check whether they exist)
template <ImageFormat T_ImageFormatSrc, ImageFormat T_ImageFormatDest>
consteval std::array<int8_t, getImageFormatNChannels(T_ImageFormatDest)> getImageFormatShuffleIndices()
{
    std::array<int8_t, getImageFormatNChannels(T_ImageFormatDest)> shuffleIndices;
    for (int i=0; i<getImageFormatNChannels(T_ImageFormatDest); ++i) {
        shuffleIndices[i] = ImageFormatConversionParams<T_ImageFormatSrc>::toRGBAShuffle[i];
    }
    return shuffleIndices;
}


// Class containing the conversion machinery
class ImageConverter {
public:
    template <typename T_Data>
    static void convertImageFormat(
        ImageFormat srcFormat, ImageFormat destFormat,
        const T_Data* srcBuffer, size_t nSrcBufferElements,
        T_Data* destBuffer, size_t nDestBufferElements);

    template <typename T_Data, ImageFormat T_SrcFormat>
    INLINE static void convertImageFormat_(
        ImageFormat destFormat,
        const T_Data* srcBuffer, size_t nSrcBufferElements,
        T_Data* destBuffer, size_t nDestBufferElements);

    template <typename T_Data, int T_NChannelsSrc, int T_NChannelsDest>
    INLINE static void applyFormatConversion(
        const Eigen::Matrix<T_Data, T_NChannelsDest, T_NChannelsSrc>& matrix,
        const T_Data* srcBuffer, size_t nSrcBufferElements,
        T_Data* destBuffer, size_t nDestBufferElements);


    template <typename T_Data>
    static inline bool shuffle(
        ImageFormat srcFormat,
        ImageFormat destFormat,
        const T_Data* srcBuffer,
        T_Data* destBuffer,
        size_t nPixels
    );

    template <typename T_Data, ImageFormat T_ImageFormatSrc>
    static inline bool shuffle(
        ImageFormat destFormat,
        const T_Data* srcBuffer,
        T_Data* destBuffer,
        size_t nPixels
    );

    template <typename T_Data, ImageFormat T_ImageFormatSrc, ImageFormat T_ImageFormatDest>
    static inline bool shuffle(
        const T_Data* srcBuffer,
        T_Data* destBuffer,
        size_t nPixels
    );
};


} // namespace detail


template <typename T_DataSrc, typename T_DataDest>
void convertImage(
    const Image<T_DataSrc>& srcImage,
    Image<T_DataDest>& destImage,
    ImageFormat destFormat = ImageFormat::UNCHANGED,
    bool allowInternalBuffer = true // Allow fallback to internal buffer on destImage in case the possible external buffer is not of suitable size. Ignored if destImage is using internal buffer
);


#include "ImageConversion.inl"


} // namespace gu2


#undef GU2_IMAGE_FORMAT_CONVERSION_TO_RGBA_MATRIX
#undef GU2_IMAGE_FORMAT_CONVERSION_FROM_RGBA_MATRIX
