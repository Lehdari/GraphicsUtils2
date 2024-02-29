//
// Project: GraphicsUtils2
// File: test_image.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include <gtest/gtest.h>

#include <gu2_util/Image.hpp>

#include <random>


TEST(Image, BasicObjectOperations)
{
    constexpr int w = 64;
    constexpr int h = 80;
    std::default_random_engine rnd(12873545);
    gu2::Image<uint8_t> image(w, h, gu2::ImageFormat::RGBA);
    for (int j=0; j<h; ++j) {
        for (int i=0; i<w; ++i) {
            image(i, j)[0] = static_cast<uint8_t>(rnd()%256);
            image(i, j)[1] = static_cast<uint8_t>(rnd()%256);
            image(i, j)[2] = static_cast<uint8_t>(rnd()%256);
            image(i, j)[3] = static_cast<uint8_t>(rnd()%256);
        }
    }
    auto image2(image);
    for (int j=0; j<h; ++j) {
        for (int i=0; i<w; ++i) {
            GTEST_ASSERT_EQ(image(i, j)[0], image2(i, j)[0]);
            GTEST_ASSERT_EQ(image(i, j)[1], image2(i, j)[1]);
            GTEST_ASSERT_EQ(image(i, j)[2], image2(i, j)[2]);
            GTEST_ASSERT_EQ(image(i, j)[3], image2(i, j)[3]);
        }
    }
}
