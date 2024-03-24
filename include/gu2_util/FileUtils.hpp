//
// Project: GraphicsUtils2
// File: FileUtils.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "Typedef.hpp"

#include <cstdio>


namespace gu2 {


inline std::vector<char> readFile(const gu2::Path& filename)
{
    if (!std::filesystem::exists(filename))
        throw std::runtime_error("File " + filename.string() + " does not exist");
    if (!std::filesystem::is_regular_file(filename))
        throw std::runtime_error(filename.string() + " is not a file");

    FILE *f = fopen(GU2_PATH_TO_STRING(filename), "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    std::vector<char> buffer(fsize);
    fread(buffer.data(), fsize, 1, f);
    fclose(f);

    return buffer;
}


} // namespace gu2
