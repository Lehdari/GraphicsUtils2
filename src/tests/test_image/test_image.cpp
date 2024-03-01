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
#include <chrono>


static std::default_random_engine rnd(12873545);


TEST(Image, BasicObjectOperations)
{
    constexpr int w = 64;
    constexpr int h = 80;
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

    // TODO add more basic operations (copy, assignment, move)
}

TEST(Image, FormatConversions)
{
    constexpr int w = 4096;
    constexpr int h = 4096;
    gu2::Image<uint8_t> image(w, h, gu2::ImageFormat::BGR);
    for (int j=0; j<h; ++j) {
        for (int i=0; i<w; ++i) {
            image(i, j)[0] = static_cast<uint8_t>(rnd()%256);
            image(i, j)[1] = static_cast<uint8_t>(rnd()%256);
            image(i, j)[2] = static_cast<uint8_t>(rnd()%256);
        }
    }

    gu2::Image<uint8_t> image2;

    auto t1 = std::chrono::system_clock::now();
    gu2::convertImage(image, image2, gu2::ImageFormat::RGBA);
    auto t2 = std::chrono::system_clock::now();
    printf("conversion: %lu us\n", std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count());

    for (int j=0; j<h; ++j) {
        for (int i=0; i<w; ++i) {
            GTEST_ASSERT_EQ(image2(i, j)[0], image(i, j)[2]);
            GTEST_ASSERT_EQ(image2(i, j)[1], image(i, j)[1]);
            GTEST_ASSERT_EQ(image2(i, j)[2], image(i, j)[0]);
            GTEST_ASSERT_EQ(image2(i, j)[3], 255);
        }
    }
}
