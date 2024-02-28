//
// Project: GraphicsUtils2
// File: ImageConversion.inl
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//


template <typename T_Data>
void detail::ImageConverter::convertImageFormat(
    ImageFormat srcFormat, ImageFormat destFormat,
    const T_Data* srcBuffer, size_t nSrcBufferElements,
    T_Data* destBuffer, size_t nDestBufferElements
) {
    switch (srcFormat) {
        #define GU2_IMAGE_FORMAT(FORMAT)                                                                            \
        case ImageFormat::FORMAT: {                                                                                 \
            convertImageFormat_<T_Data, ImageFormat::FORMAT>(destFormat, srcBuffer, nSrcBufferElements, destBuffer, \
                nDestBufferElements);                                                                               \
        } return;
        GU2_IMAGE_FORMATS(GU2_IMAGE_FORMAT)
        #undef GU2_IMAGE_FORMAT

        default:
            throw std::runtime_error("Requested format conversion not implemented yet");
    }
}

template <typename T_Data, ImageFormat T_SrcFormat>
INLINE void detail::ImageConverter::convertImageFormat_(
    ImageFormat destFormat,
    const T_Data* srcBuffer, size_t nSrcBufferElements,
    T_Data* destBuffer, size_t nDestBufferElements
) {
    constexpr auto srcChannels = getImageFormatNChannels(T_SrcFormat);
    switch (destFormat) {
        #define GU2_IMAGE_FORMAT(FORMAT)                                                                \
        case ImageFormat::FORMAT: {                                                                     \
            auto& conversionMatrix = detail::getImageFormatConversionMatrix                             \
                <T_Data, T_SrcFormat, ImageFormat::FORMAT>();                                           \
            applyFormatConversion<T_Data, srcChannels, getImageFormatNChannels(ImageFormat::FORMAT)>(   \
                conversionMatrix, srcBuffer, nSrcBufferElements, destBuffer, nDestBufferElements);      \
        } return;
        GU2_IMAGE_FORMATS(GU2_IMAGE_FORMAT)
        #undef GU2_IMAGE_FORMAT

        default:
            throw std::runtime_error("Requested format conversion not implemented yet");
    }
}

template <typename T_Data, int T_NChannelsSrc, int T_NChannelsDest>
INLINE void detail::ImageConverter::applyFormatConversion(
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
            detail::ImageConverter::convertImageFormat(srcImage._format, destFormat, data, srcImage._nElements,
                destImage._data, newNElements);
        }
        else {
            destImage.convertImageFormat(destFormat);
        }
    }
}
