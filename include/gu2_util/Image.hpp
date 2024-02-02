//
// Project: GraphicsUtils2
// File: Image.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include <stb_image.h>
#include <stb_image_write.h>

#include "Macros.hpp"
#include "MathTypes.hpp"
#include "Typedef.hpp"

#include <cstdint>
#include <filesystem>
#include <vector>
#include <type_traits>


// X macro for image formats, arguments:
// Image format name, n. of channels, conversion matrix to RGBA, conversion matrix from RGBA
#define GU2_IMAGE_FORMATS(GU2_IMAGE_FORMAT)             \
    GU2_IMAGE_FORMAT(RGBA, 4,                           \
        GU2_TO_RGBA(                                    \
            1.0, 0.0, 0.0, 0.0,                         \
            0.0, 1.0, 0.0, 0.0,                         \
            0.0, 0.0, 1.0, 0.0,                         \
            0.0, 0.0, 0.0, 1.0),                        \
        GU2_FROM_RGBA(                                  \
            1.0, 0.0, 0.0, 0.0,                         \
            0.0, 1.0, 0.0, 0.0,                         \
            0.0, 0.0, 1.0, 0.0,                         \
            0.0, 0.0, 0.0, 1.0)                         \
    )                                                   \
    GU2_IMAGE_FORMAT(BGRA, 4,                           \
        GU2_TO_RGBA(                                    \
            0.0, 0.0, 1.0, 0.0,                         \
            0.0, 1.0, 0.0, 0.0,                         \
            1.0, 0.0, 0.0, 0.0,                         \
            0.0, 0.0, 0.0, 1.0),                        \
        GU2_FROM_RGBA(                                  \
            0.0, 0.0, 1.0, 0.0,                         \
            0.0, 1.0, 0.0, 0.0,                         \
            1.0, 0.0, 0.0, 0.0,                         \
            0.0, 0.0, 0.0, 1.0)                         \
    )                                                   \
    GU2_IMAGE_FORMAT(RGB, 3,                            \
        GU2_TO_RGBA(                                    \
            1.0, 0.0, 0.0,                              \
            0.0, 1.0, 0.0,                              \
            0.0, 0.0, 1.0,                              \
            0.0, 0.0, 0.0),                             \
        GU2_FROM_RGBA(                                  \
            1.0, 0.0, 0.0, 0.0,                         \
            0.0, 1.0, 0.0, 0.0,                         \
            0.0, 0.0, 1.0, 0.0)                         \
    )                                                   \
    GU2_IMAGE_FORMAT(BGR, 3,                            \
        GU2_TO_RGBA(                                    \
            0.0, 0.0, 1.0,                              \
            0.0, 1.0, 0.0,                              \
            1.0, 0.0, 0.0,                              \
            0.0, 0.0, 0.0),                             \
        GU2_FROM_RGBA(                                  \
            0.0, 0.0, 1.0, 0.0,                         \
            0.0, 1.0, 0.0, 0.0,                         \
            1.0, 0.0, 0.0, 0.0)                         \
    )                                                   \
    GU2_IMAGE_FORMAT(YUV, 3,                            \
        GU2_TO_RGBA(                                    \
            0.99998,    2.03211,        -1.5082e-05,    \
            1.0,        -0.394646,      -0.580594,      \
            1.0,        -1.17892e-05,   1.13983,        \
            0.0,        0.0,            0.0),           \
        GU2_FROM_RGBA(                                  \
            0.114,      0.587,      0.299,      0.0,    \
            0.436,      -0.28886,   -0.14713,   0.0,    \
            -0.10001,   -0.51499,   0.615,      0.0)    \
    )                                                   \
    GU2_IMAGE_FORMAT(GRAY, 1,                           \
        GU2_TO_RGBA(                                    \
            1.0,                                        \
            1.0,                                        \
            1.0,                                        \
            0.0),                                       \
        GU2_FROM_RGBA(                                  \
            0.299,  0.587,  0.114,  0.0)                \
    )


namespace gu2 {


#define GU2_IMAGE_FORMAT(FORMAT, N_CHANNELS, TO_RGBA, FROM_RGBA) FORMAT,
enum class ImageFormat : uint32_t {
    UNCHANGED   = 0, // only allowed as an argument to convertImage
    GU2_IMAGE_FORMATS(GU2_IMAGE_FORMAT)
    UNKNOWN     = 256
};
#undef GU2_IMAGE_FORMAT

constexpr int getImageFormatNChannels(ImageFormat imageFormat);


template <typename T_Data>
class Image {
public:
    // If data is nullptr, internal buffer will be used. Otherwise, the buffer pointed by data will
    // be utilized as the pixel data buffer. Ownership will not be transferred.
    Image(int width=0, int height=0, ImageFormat format=ImageFormat::BGRA, T_Data* data=nullptr);
    Image(const Image<T_Data>& other);
    Image(Image&&) noexcept = default;
    Image& operator=(const Image<T_Data>& other);
    Image& operator=(Image&&) noexcept = default;

    int width() const noexcept;
    int height() const noexcept;
    const ImageFormat& format() const noexcept;
    const T_Data* data() const noexcept;
    size_t nElements() const noexcept; // returns width * height * nchannels (n. of elements pointed to by data())
    T_Data* operator()(int x, int y);
    const T_Data* operator()(int x, int y) const;

    // Set pixel data (will read width * height * nchannels * sizeof(T_Data) bytes from data)
    void copyFrom(const T_Data* data);

    // Convert to other format
    // Note: Only conversion to formats with equal amount of channels is permitted when
    // using external buffer
    void convertImageFormat(ImageFormat destFormat);

    template <typename T_DataOther>
    friend class Image;
    template <typename T_DataSrc, typename T_DataDest>
    friend void convertImage(const Image<T_DataSrc>&, Image<T_DataDest>&, ImageFormat);

private:
    int                 _width;
    int                 _height;
    ImageFormat         _format;

    T_Data*             _data;
    size_t              _nElements;
    std::vector<T_Data> _buffer;

    template <typename T_DataOther>
    void copyParamsFrom(const Image<T_DataOther>& other);

    INLINE bool usingExternalBuffer();

    // Format conversion machinery
    static void convertImageFormat(
        ImageFormat srcFormat, ImageFormat destFormat,
        const T_Data* srcBuffer, size_t nSrcBufferElements,
        T_Data* destBuffer, size_t nDestBufferElements);

    template <ImageFormat T_SrcFormat>
    INLINE static void convertImageFormat_(
        ImageFormat destFormat,
        const T_Data* srcBuffer, size_t nSrcBufferElements,
        T_Data* destBuffer, size_t nDestBufferElements);

    template <int T_NChannelsSrc, int T_NChannelsDest>
    INLINE static void applyFormatConversion(
        const Eigen::Matrix<T_Data, T_NChannelsDest, T_NChannelsSrc>& matrix,
        const T_Data* srcBuffer, size_t nSrcBufferElements,
        T_Data* destBuffer, size_t nDestBufferElements);
};


template <typename T_DataSrc, typename T_DataDest>
inline void convertImage(
    const Image<T_DataSrc>& srcImage,
    Image<T_DataDest>& destImage,
    ImageFormat destFormat = ImageFormat::UNCHANGED);

template <typename T_Data>
inline void writeImageToFile(const Image<T_Data>& image, const Path& filename);

template <typename T_Data>
inline Image<T_Data> readImageFromFile(const Path& filename);


#include "Image.inl"


} // namespace gu2
