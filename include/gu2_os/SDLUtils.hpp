//
// Project: GraphicsUtils2
// File: SDLUtils.hpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include <SDL2/SDL.h>

#include <stdexcept>


// Macro for querying SDL error and wrapping it into an exception
#define SDL_THROW_ERROR {                       \
    std::runtime_error error(SDL_GetError());   \
    SDL_ClearError();                           \
    throw error;                                \
}
