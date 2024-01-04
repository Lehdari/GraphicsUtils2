//
// Project: GraphicsUtils2
// File: backend.hpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include <vector>


#define GU2_BACKEND_SDL2 1
#define GU2_BACKEND_GLFW 2


#ifndef GU2_BACKEND
#error "GU2_BACKEND is not defined. Something is wrong with the CMake configuration."
#endif // GU2_BACKEND
#ifndef GU2_BACKEND_NAME
#error "GU2_BACKEND_NAME is not defined. Something is wrong with the CMake configuration."
#endif // GU2_BACKEND_NAME

#if GU2_BACKEND == GU2_BACKEND_SDL2
#include "backend_sdl2.hpp"
#elif GU2_BACKEND == GU2_BACKEND_GLFW
#include "backend_glfw.hpp"
#else // GU2_BACKEND
#error "Unsupported OS interfacing backend specified"
#endif // GU2_BACKEND


namespace gu2 {

void sleep(uint32_t ms); // TODO maybe move elsewhere?

} // namespace gu2
