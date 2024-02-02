//
// Project: GraphicsUtils2
// File: image.inl
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

namespace detail {


template <typename T_Data, int T_NChannelsSrc, int T_NChannelsDest>
Eigen::Matrix<T_Data, T_NChannelsDest, T_NChannelsSrc> getImageFormatConversionMatrix(
    ImageFormat srcFormat, ImageFormat destFormat
) {
    Eigen::Matrix<T_Data, 4, T_NChannelsSrc> srcToRgbaMatrix;
    switch (srcFormat) {
        #define GU2_TO_RGBA(...) __VA_ARGS__
        #define GU2_IMAGE_FORMAT(FORMAT, N_CHANNELS, TO_RGBA, FROM_RGBA)    \
        case ImageFormat::FORMAT: srcToRgbaMatrix << TO_RGBA; break;
        GU2_IMAGE_FORMATS(GU2_IMAGE_FORMAT)
        #undef GU2_IMAGE_FORMAT
        #undef GU2_TO_RGBA
        default:
            throw std::runtime_error("BUG: Support for the image format not implemented!");
    }

    Eigen::Matrix<T_Data, T_NChannelsDest, 4> rgbaToDestMatrix;
    switch (destFormat) {
        #define GU2_FROM_RGBA(...) __VA_ARGS__
        #define GU2_IMAGE_FORMAT(FORMAT, N_CHANNELS, TO_RGBA, FROM_RGBA)    \
        case ImageFormat::FORMAT: rgbaToDestMatrix << FROM_RGBA; break;
        GU2_IMAGE_FORMATS(GU2_IMAGE_FORMAT)
        #undef GU2_IMAGE_FORMAT
        #undef GU2_FROM_RGBA
        default:
            throw std::runtime_error("BUG: Support for the image format not implemented!");
    }

    return rgbaToDestMatrix*srcToRgbaMatrix;
}


} // namespace detail


constexpr int getImageFormatNChannels(ImageFormat imageFormat)
{
    switch (imageFormat) {
        #define GU2_IMAGE_FORMAT(FORMAT, N_CHANNELS, TO_RGBA, FROM_RGBA) case ImageFormat::FORMAT: return N_CHANNELS;
        GU2_IMAGE_FORMATS(GU2_IMAGE_FORMAT)
        #undef GU2_IMAGE_FORMAT
        default:
            throw std::runtime_error("BUG: Support for the image format not implemented!");
    }
    return -1;
}


template <typename T_Data>
Image<T_Data>::Image(int width, int height, ImageFormat format, T_Data* data) :
    _width      (width),
    _height     (height),
    _format     (format),
    _data       (data),
    _nElements  (_width*_height*getImageFormatNChannels(format))
{
    // Unchanged only allowed in convertImage
    if (_format == ImageFormat::UNCHANGED)
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
    _nElements  (other._nElements), // allocate a new, internal buffer
    _buffer     (_nElements)
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
    convertImageFormat(_format, destFormat, _data, _nElements, destBuffer.data(), newNElements);

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

template <typename T_Data>
INLINE bool Image<T_Data>::usingExternalBuffer()
{
    return _data != _buffer.data();
}

template <typename T_Data>
void Image<T_Data>::convertImageFormat(
    ImageFormat srcFormat, ImageFormat destFormat,
    const T_Data* srcBuffer, size_t nSrcBufferElements,
    T_Data* destBuffer, size_t nDestBufferElements
) {
    switch (srcFormat) {
        #define GU2_IMAGE_FORMAT(FORMAT, N_CHANNELS, TO_RGBA, FROM_RGBA)                                    \
        case ImageFormat::FORMAT: {                                                                         \
            convertImageFormat_<ImageFormat::FORMAT>(destFormat, srcBuffer, nSrcBufferElements, destBuffer, \
                nDestBufferElements);                                                                       \
        } return;
        GU2_IMAGE_FORMATS(GU2_IMAGE_FORMAT)
        #undef GU2_IMAGE_FORMAT

        default:
            throw std::runtime_error("Requested format conversion not implemented yet");
    }
}

template <typename T_Data>
template <ImageFormat T_SrcFormat>
INLINE void Image<T_Data>::convertImageFormat_(
    ImageFormat destFormat,
    const T_Data* srcBuffer, size_t nSrcBufferElements,
    T_Data* destBuffer, size_t nDestBufferElements
) {
    constexpr auto srcChannels = getImageFormatNChannels(T_SrcFormat);
    switch (destFormat) {
        #define GU2_IMAGE_FORMAT(FORMAT, N_CHANNELS, TO_RGBA, FROM_RGBA)                                            \
        case ImageFormat::FORMAT: {                                                                                 \
            static const auto conversionMatrix = detail::getImageFormatConversionMatrix<T_Data, srcChannels,        \
                N_CHANNELS>(T_SrcFormat, ImageFormat::FORMAT);                                                      \
            applyFormatConversion<srcChannels, N_CHANNELS>(conversionMatrix, srcBuffer, nSrcBufferElements,         \
                destBuffer, nDestBufferElements);                                                                   \
        } return;
        GU2_IMAGE_FORMATS(GU2_IMAGE_FORMAT)
        #undef GU2_IMAGE_FORMAT

        default:
            throw std::runtime_error("Requested format conversion not implemented yet");
    }
}

template <typename T_Data>
template<int T_NChannelsSrc, int T_NChannelsDest>
INLINE void Image<T_Data>::applyFormatConversion(
    const Eigen::Matrix<T_Data, T_NChannelsDest, T_NChannelsSrc>& matrix,
    const T_Data* srcBuffer, size_t nSrcBufferElements,
    T_Data* destBuffer, size_t nDestBufferElements
) {
    using SrcPixels = Eigen::Matrix<T_Data, T_NChannelsSrc, Eigen::Dynamic>;
    using SrcPixelsMap = Eigen::Map<const SrcPixels>;
    using DestPixels = Eigen::Matrix<T_Data, T_NChannelsDest, Eigen::Dynamic>;
    using DestPixelsMap = Eigen::Map<DestPixels>;
    using namespace Eigen::placeholders;

    SrcPixelsMap srcPixels(srcBuffer, T_NChannelsSrc, nSrcBufferElements / T_NChannelsSrc);
    DestPixelsMap destPixels(destBuffer, T_NChannelsDest, nDestBufferElements / T_NChannelsDest);
    destPixels = matrix * srcPixels;

    // set alpha channel to 1 (this will break if alpha channel is other than the last one)
    if constexpr (T_NChannelsSrc < 4 && T_NChannelsDest == 4) {
        destPixels(last, all).setOnes();
    }
}


template <typename T_DataSrc, typename T_DataDest>
struct ImageConversionParams {};
template <> struct ImageConversionParams<uint8_t, float> {
    static constexpr bool   prescale    {false};
    static constexpr float  scaleRatio  {1.0f / 255.0f};
};
template <> struct ImageConversionParams<float, uint8_t> {
    static constexpr bool   prescale    {true};
    static constexpr float  scaleRatio  {255.0f};
};


template <typename T_DataSrc, typename T_DataDest>
inline void convertImage(
    const Image<T_DataSrc>& srcImage,
    Image<T_DataDest>& destImage,
    ImageFormat destFormat
) {
    T_DataDest* data = destImage._data;
    std::vector<T_DataDest> tempBuffer;
    if (destImage.usingExternalBuffer()) {
        if (srcImage._width != destImage._width || srcImage._height != destImage._height) {
            throw std::runtime_error("Image width and height need to match when using external buffer");
        }

        if (getImageFormatNChannels(srcImage._format) != getImageFormatNChannels(destImage._format)) {
            // Allocate a temp buffer
            tempBuffer.resize(srcImage._nElements);
            data = tempBuffer.data();
        }
        else
            destImage._format = srcImage.format();
    }
    else {
        destImage.copyParamsFrom(srcImage);
        data = destImage._data;
    }

    using Params = ImageConversionParams<T_DataSrc, T_DataDest>;

    if constexpr (std::is_same_v<T_DataSrc, T_DataDest>) {
        // No conversion required, only copy
        if (data != srcImage._data)
            memcpy(data, srcImage._data, srcImage._nElements * sizeof(T_DataSrc));
    }
    else {
        // Perform type conversion if the types differ
        if constexpr (ImageConversionParams<T_DataSrc, T_DataDest>::prescale) {
            // prescale
            for (int i = 0; i < srcImage._nElements; ++i) {
                data[i] = static_cast<T_DataDest>(srcImage._data[i] * Params::scaleRatio);
            }
        } else {
            // postscale
            for (int i = 0; i < srcImage._nElements; ++i) {
                data[i] = static_cast<T_DataDest>(srcImage._data[i]) * Params::scaleRatio;
            }
        }
    }

    // Perform the potential image format conversion
    if (destFormat != ImageFormat::UNCHANGED) {
        if (data == tempBuffer.data()) { // using temp buffer for conversion
            size_t newNElements = destImage._width*destImage._height*getImageFormatNChannels(destFormat);
            destImage.convertImageFormat(srcImage._format, destFormat, data, srcImage._nElements,
                destImage._data, newNElements);
        }
        else {
            destImage.convertImageFormat(destFormat);
        }
    }
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
