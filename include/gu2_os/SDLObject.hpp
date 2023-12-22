//
// Project: GraphicsUtils2
// File: SDLObject.hpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "SDLUtils.hpp"

#include <utility>


namespace gu2 {

// SDLObject adds RAII wrapping for SDL object pointers
template <typename T_SDLObject, auto Creator, auto Destroyer>
class SDLObject {
public:
    template <typename... T_Args>
    SDLObject(T_Args&&... args) :
        _object (Creator(std::forward<T_Args>(args)...))
    {
        if (_object == nullptr) // An error occurred during object construction
            SDL_THROW_ERROR
    }
    ~SDLObject()
    {
        if (_object)
            Destroyer(_object);
    }

    T_SDLObject& operator*()
    {
        return *_object;
    }

    const T_SDLObject& operator*() const
    {
        return *_object;
    }

    T_SDLObject* operator->()
    {
        return _object;
    }

    const T_SDLObject* operator->() const
    {
        return _object;
    }

    T_SDLObject* get()
    {
        return _object;
    }

    const T_SDLObject* get() const
    {
        return _object;
    }

private:
    T_SDLObject*    _object;
};


// Predefined SDL objects
using SDLWindow = SDLObject<SDL_Window, SDL_CreateWindow, SDL_DestroyWindow>;


} // namespace gu2
