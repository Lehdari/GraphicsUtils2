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

#include "ImageConversion.hpp"
#include "MathTypes.hpp"
#include "Typedef.hpp"

#include <cstdint>


namespace gu2 {


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
    INLINE bool usingExternalBuffer();

    // Set pixel data (will read width * height * nchannels * sizeof(T_Data) bytes from data)
    void copyFrom(const T_Data* data);

    // Convert to other format
    // Note: Only conversion to formats with equal amount of channels is permitted when
    // using external buffer
    void convertImageFormat(ImageFormat destFormat); // TODO absorb to convertImage

    template <typename T_DataOther>
    friend class Image;
    template <typename T_DataSrc, typename T_DataDest>
    friend void convertImage(const Image<T_DataSrc>&, Image<T_DataDest>&, ImageFormat, bool);
    friend class ImageConverter;

private:
    int                 _width;
    int                 _height;
    ImageFormat         _format;

    T_Data*             _data;
    size_t              _nElements;
    std::vector<T_Data> _buffer;

    template <typename T_DataOther>
    void copyParamsFrom(const Image<T_DataOther>& other);

};


template <typename T_Data>
inline void writeImageToFile(const Image<T_Data>& image, const Path& filename);

template <typename T_Data>
inline Image<T_Data> readImageFromFile(const Path& filename);


#include "Image.inl"


} // namespace gu2
