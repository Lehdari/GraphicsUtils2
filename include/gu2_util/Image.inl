//
// Project: GraphicsUtils2
// File: image.inl
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//


template <typename T_Data>
Image<T_Data>::Image(int width, int height, ImageFormat format, T_Data* data) :
    _width      (width),
    _height     (height),
    _format     (format),
    _data       (data),
    _nElements  (_width*_height*getImageFormatNChannels(format))
{
    // Check for invalid image formats
    if (_format == ImageFormat::UNCHANGED || _format == ImageFormat::UNKNOWN)
        throw std::runtime_error("Invalid image format");

    // Using internal buffer, allocate it
    if (_data == nullptr) {
        _buffer.resize(_nElements);
        _data = _buffer.data();
    }
}

template <typename T_Data>
Image<T_Data>::Image(const Image<T_Data>& other) :
    _width      (other._width),
    _height     (other._height),
    _format     (other._format),
    _data       (nullptr),
    _nElements  (other._nElements),
    _buffer     (_nElements) // allocate a new, internal buffer
{
    _data = _buffer.data();
    memcpy(_data, other._data, _nElements*sizeof(T_Data)); // make a copy of the pixel data
}

template <typename T_Data>
Image<T_Data>& Image<T_Data>::operator=(const Image<T_Data>& other)
{
    if (this == &other)
        return *this;

    _width = other._width;
    _height = other._height;
    _format = other._format;
    if (_nElements != other._nElements) { // reuse the current buffer in case it's right size
        _nElements = other._nElements;
        _buffer.resize(_nElements);
        _data = _buffer.data();
    }
    memcpy(_data, other._data, _nElements*sizeof(T_Data)); // make a copy of the pixel data

    return *this;
}

template <typename T_Data>
int Image<T_Data>::width() const noexcept
{
    return _width;
}

template <typename T_Data>
int Image<T_Data>::height() const noexcept
{
    return _height;
}

template <typename T_Data>
const ImageFormat& Image<T_Data>::format() const noexcept
{
    return _format;
}

template <typename T_Data>
const T_Data* Image<T_Data>::data() const noexcept
{
    return _data;
}

template <typename T_Data>
size_t Image<T_Data>::nElements() const noexcept
{
    return _nElements;
}

template<typename T_Data>
T_Data* Image<T_Data>::operator()(int x, int y)
{
    return _data + (y*_width + x)*getImageFormatNChannels(_format);
}

template<typename T_Data>
const T_Data* Image<T_Data>::operator()(int x, int y) const
{
    return _data + (y*_width + x)*getImageFormatNChannels(_format);
}

template <typename T_Data>
INLINE bool Image<T_Data>::usingExternalBuffer()
{
    return _data != _buffer.data();
}

template <typename T_Data>
void Image<T_Data>::copyFrom(const T_Data* data)
{
    memcpy(_buffer.data(), data, _nElements * sizeof(T_Data));
}

template <typename T_Data>
void Image<T_Data>::convertImageFormat(ImageFormat destFormat)
{
    if (destFormat == _format || destFormat == ImageFormat::UNCHANGED)
        return;

    if (usingExternalBuffer() && getImageFormatNChannels(destFormat) != getImageFormatNChannels(_format)) {
        throw std::runtime_error("Number of format channels need to match when using external buffer");
    }

    // New number of elements, new buffer
    size_t newNElements = _width*_height*getImageFormatNChannels(destFormat);
    std::vector<T_Data> destBuffer(newNElements);

    // Convert
    detail::ImageConverter::convertImageFormat(_format, destFormat, _data, _nElements, destBuffer.data(), newNElements);

    // Replace the old buffer
    if (usingExternalBuffer()) {
        memcpy(_data, destBuffer.data(), _nElements * sizeof(T_Data));
    }
    else {
        _nElements = newNElements;
        _buffer = std::move(destBuffer);
        _data = _buffer.data();
    }

    _format = destFormat;
}

template <typename T_Data>
template <typename T_DataOther>
void Image<T_Data>::copyParamsFrom(const Image<T_DataOther>& other)
{
    _width = other._width;
    _height = other._height;
    _format = other._format;
    _nElements = other._nElements;
    _buffer.resize(_nElements);
    _data = _buffer.data();
}

template<typename T_Data>
void writeImageToFile(const Image<T_Data>& image, const Path& filename)
{
    // TODO extend, only 8/8/8 RGB PNG supported for now

    Image<T_Data> img;
    convertImage(image, img, ImageFormat::RGB);
    auto nChannels = getImageFormatNChannels(img.format());
    stbi_write_png(GU2_PATH_TO_STRING(filename), img.width(), img.height(), nChannels, img.data(), img.width()*nChannels);
}

template<typename T_Data>
Image<T_Data> readImageFromFile(const Path& filename)
{
    // TODO extend, only 8/8/8 RGB PNG supported for now

    int w, h, c;
    c = 0;
    unsigned char* data = stbi_load(GU2_PATH_TO_STRING(filename), &w, &h, &c, 0);
    if (data == nullptr)
        throw std::runtime_error("Unable to load image from " + filename.string() + ": " + stbi_failure_reason());

    ImageFormat imageFormat = ImageFormat::UNKNOWN;
    switch (c) {
        case 1: imageFormat = ImageFormat::GRAY; break;
        case 3: imageFormat = ImageFormat::RGB; break;
        case 4: imageFormat = ImageFormat::RGBA; break;
        default: {
            throw std::runtime_error("Unable to deduce format from number of channels\n");
            stbi_image_free(data);
        }
    }
    Image<T_Data> img(w, h, imageFormat);
    img.copyFrom(data);
    stbi_image_free(data);

    return img;
}
