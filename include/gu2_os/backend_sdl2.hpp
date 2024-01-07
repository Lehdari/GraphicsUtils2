//
// Project: GraphicsUtils2
// File: backend_sdl2.hpp
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
#define BACKEND_THROW_ERROR {                   \
    std::runtime_error error(SDL_GetError());   \
    SDL_ClearError();                           \
    throw error;                                \
}


#include "CObjectWrapper.hpp"


namespace gu2 {

struct WindowSettings {
    enum Position : int {
        CENTERED    =   SDL_WINDOWPOS_CENTERED,
        UNDEFINED   =   SDL_WINDOWPOS_UNDEFINED
    };

    std::string name    {"window"};
    int         w       {1280};
    int         h       {720};
    int         x       {Position::CENTERED};
    int         y       {Position::CENTERED};
};


// Predefined SDL objects
using WindowObject = CObjectWrapper<SDL_Window, SDL_CreateWindow, SDL_DestroyWindow>;

} // namespace gu2
