//
// Project: GraphicsUtils2
// File: Typedef.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include <nlohmann/json.hpp>

#include <filesystem>

#ifdef _WIN32
#define GU2_PATH_TO_STRING(PATH) (PATH).string().c_str()
#else // _WIN32
#define GU2_PATH_TO_STRING(PATH) (PATH).c_str()
#endif // _WIN32


namespace gu2 {


using Json = nlohmann::json;
using Path = std::filesystem::path;


} // namespace gu2
