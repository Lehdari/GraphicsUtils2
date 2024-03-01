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

template<typename T_Data>
bool detail::ImageConverter::shuffle(
    ImageFormat srcFormat,
    ImageFormat destFormat,
    const T_Data* srcBuffer,
    T_Data* destBuffer,
    size_t nPixels
) {
    switch (srcFormat) {
        #define GU2_IMAGE_FORMAT(FORMAT)                                                            \
        case ImageFormat::FORMAT:                                                                   \
            return shuffle<T_Data, ImageFormat::FORMAT>(destFormat, srcBuffer, destBuffer, nPixels);
        GU2_IMAGE_FORMATS(GU2_IMAGE_FORMAT)
        #undef GU2_IMAGE_FORMAT

        default:
            throw std::runtime_error("Requested format conversion not implemented yet");
    }
}

template<typename T_Data, ImageFormat T_ImageFormatSrc>
bool detail::ImageConverter::shuffle(
    ImageFormat destFormat,
    const T_Data* srcBuffer,
    T_Data* destBuffer,
    size_t nPixels
) {
    if constexpr (!(requires { detail::ImageFormatConversionParams<T_ImageFormatSrc>::toRGBAShuffle; })) {
        return false;
    }
    else {
        switch (destFormat) {
            #define GU2_IMAGE_FORMAT(FORMAT)                                                                    \
            case ImageFormat::FORMAT:                                                                           \
                return shuffle<T_Data, T_ImageFormatSrc, ImageFormat::FORMAT>(srcBuffer, destBuffer, nPixels);
                GU2_IMAGE_FORMATS(GU2_IMAGE_FORMAT)
            #undef GU2_IMAGE_FORMAT

            default:
                throw std::runtime_error("Requested format conversion not implemented yet");
        }
    }
}

#if defined(__AVX512F__) && defined(__AVX512VBMI__) && defined(__AVX512BW__)
// TODO move somewhere else, add support for 256-bit and 128-bit instructions as well
template <ImageFormat T_ImageFormatSrc, ImageFormat T_ImageFormatDest>
INLINE __m512i createShuffleIndicesAVX512()
{
    constexpr auto shuffleIndices = detail::getImageFormatShuffleIndices<T_ImageFormatSrc, T_ImageFormatDest>();
    std::array<uint8_t, 64> indicesOut{};
    std::fill(indicesOut.begin(), indicesOut.end(), 0b01000000);
    uint8_t i=0;
    uint8_t j=0;
    while (i+getImageFormatNChannels(T_ImageFormatDest) <= 64 && j+getImageFormatNChannels(T_ImageFormatDest) <= 64) {
        for (int c=0; c<getImageFormatNChannels(T_ImageFormatDest); ++c) {
            if (shuffleIndices[c] >= 0)
                indicesOut[j+c] = i+shuffleIndices[c];
        }
        i += getImageFormatNChannels(T_ImageFormatSrc);
        j += getImageFormatNChannels(T_ImageFormatDest);
    }
    return _mm512_loadu_epi8(indicesOut.data());
}
#endif

template<typename T_Data, ImageFormat T_ImageFormatSrc, ImageFormat T_ImageFormatDest>
bool detail::ImageConverter::shuffle(const T_Data* srcBuffer, T_Data* destBuffer, size_t nPixels)
{
    // Check if necessary shuffle indices are defined
    if constexpr (!(requires { detail::ImageFormatConversionParams<T_ImageFormatDest>::fromRGBAShuffle; })) {
        return false;
    }
    else {
        constexpr auto shuffleIndices = getImageFormatShuffleIndices<T_ImageFormatSrc, T_ImageFormatDest>();

        size_t pixel = 0;

        // AVX-512 SIMD shuffle
        #if defined(__AVX512F__) && defined(__AVX512VBMI__) && defined(__AVX512BW__)
        if constexpr (std::is_same_v<T_Data, uint8_t>) {
            int pixelsPerInstruction = 64 / (sizeof(uint8_t)*std::max(
                getImageFormatNChannels(T_ImageFormatSrc), getImageFormatNChannels(T_ImageFormatDest)));

            auto idx = createShuffleIndicesAVX512<T_ImageFormatSrc, T_ImageFormatDest>();
            auto saturation = _mm512_set1_epi8(ImageDataParams<T_Data>::pixelSaturation);

            int destIncr = pixelsPerInstruction * getImageFormatNChannels(T_ImageFormatDest);
            int srcIncr = pixelsPerInstruction * getImageFormatNChannels(T_ImageFormatSrc);
            int nIters = nPixels / pixelsPerInstruction;
            for (int i=0; i<nIters; ++i) {
                _mm512_storeu_epi8(destBuffer+i*destIncr, _mm512_permutex2var_epi8(
                    _mm512_loadu_epi8(srcBuffer+i*srcIncr), idx, saturation));
            }
            pixel = nIters*pixelsPerInstruction;
        }
        #endif

        // Perform the channel shuffle
        for (size_t i = pixel; i < nPixels; ++i) {
            for (int c = 0; c < getImageFormatNChannels(T_ImageFormatDest); ++c) {
                destBuffer[i*getImageFormatNChannels(T_ImageFormatDest) + c] = shuffleIndices[c] < 0 ?
                    ImageDataParams<T_Data>::pixelSaturation :
                    srcBuffer[i*getImageFormatNChannels(T_ImageFormatSrc) + shuffleIndices[c]];
            }
        }
    }
    return true;
}


template <typename T_DataSrc, typename T_DataDest>
void convertImage(
    const Image<T_DataSrc>& srcImage,
    Image<T_DataDest>& destImage,
    ImageFormat destFormat,
    bool allowInternalBuffer
) {
    // Target format
    if (destFormat == ImageFormat::UNCHANGED)
        destFormat = srcImage._format; // We're performing pure data type conversion

    // If the data types and image formats are unchanged, just make a copy
    if constexpr (std::is_same_v<T_DataSrc, T_DataDest>) {
        if (destFormat == srcImage._format) {
            destImage = srcImage;
            return;
        }
    }

    // Calculate number of pixels and required number of elements for the target buffer
    auto nPixels = srcImage._width * srcImage._height;
    auto nElementsRequired = nPixels * getImageFormatNChannels(destFormat);

    // Check whether the operation is disallowed
    if (destImage._nElements != nElementsRequired && destImage.usingExternalBuffer() && !allowInternalBuffer)
        throw std::runtime_error("Destination image using external buffer of incompatible size and fallback to internal buffer is disabled.");

    // Check if we need to use a temporary buffer
    // TODO   reasons: images are the same, need elevated bit depth for conversion (YUV, GRAY)
    //  reasons not to: shuffle operation is sufficient, output in elevated bit depth
    //  (in that case just use the dest buffer directly
    std::vector<T_DataDest> tempBuffer; // TODO temp buffer type, might need elevated bit depth
    bool usingTempBuffer = false;
    T_DataDest* destBuffer = destImage._data;
    if (&srcImage == &destImage) {
        tempBuffer.resize(nElementsRequired);
        usingTempBuffer = true;
        destBuffer = tempBuffer.data();
    }

    // If we're not using temp buffer, reallocate the destination image internal buffer if it's not the correct size
    if (!usingTempBuffer && destImage._nElements != nElementsRequired) {
        destImage._buffer.resize(nElementsRequired);
        destImage._data = destImage._buffer.data();
        destBuffer = destImage._data;
    }

    // Try to perform the format conversion by shuffle
    bool shuffled = detail::ImageConverter::shuffle(srcImage._format, destFormat, srcImage._data, destBuffer,
        nPixels);

    // Do upcasting if it's required
    // TODO

    // Do gamma correction if it's required
    // TODO

    // Do conversion with conversion matrix if shuffle was not available
    if (!shuffled) {
        // TODO
    }

    // Temp buffer was in use, prepare the destination image for the final copy and cast
    if (usingTempBuffer) {
        // Reallocate the destination image internal buffer if it's not the correct size
        if (destImage._nElements != nElementsRequired) {
            destImage._buffer.resize(nElementsRequired);
            destImage._data = destImage._buffer.data();
            destBuffer = destImage._data;
        }

        // Do downcasting if it's required
        // TODO

        // Copy to destination image
        memcpy(destBuffer, tempBuffer.data(), nElementsRequired*sizeof(T_DataDest));
    }

    // Set destination image parameters to finish things off
    destImage._width = srcImage._width;
    destImage._height = srcImage._height;
    destImage._format = destFormat;
    destImage._nElements = nElementsRequired;

#if 0
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
#endif
}
