//
// Project: DooT2
// File: ImageUtils.hpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include "util/Image.hpp"


namespace Detail {

template <typename T_Data>
struct DownscaleUtil {
    using Type = T_Data;
};
template<> struct DownscaleUtil<uint8_t> { using Type = int; };

} // namespace Detail


// xDownscale, yDownscale: factors of size reduction
template <typename T_Data>
Image<T_Data> downscaleImage(const Image<T_Data>& image, int xDownscale, int yDownscale)
{
    int newWidth = image.width() / xDownscale;
    int newHeight = image.height() / yDownscale;
    Image<T_Data> newImage(newWidth, newHeight, image.format());

    typename Detail::DownscaleUtil<T_Data>::Type kernelSize = xDownscale*yDownscale;
    int nChannels = getImageFormatNChannels(image.format());

    #pragma omp parallel for default(shared)
    for (int j=0; j<newHeight; ++j) {
        int j2 = j*yDownscale;
        typename Detail::DownscaleUtil<T_Data>::Type p;
        for (int i=0; i<newWidth; ++i) {
            int i2 = i*xDownscale;
            for (int c=0; c<nChannels; ++c) {
                p = 0;
                for (int j3=0; j3<yDownscale; ++j3) {
                    for (int i3=0; i3<xDownscale; ++i3) {
                        p += image(i2+i3, j2+j3)[c];
                    }
                }
                newImage(i, j)[c] = p / kernelSize;
            }
        }
    }

    return newImage;
}
