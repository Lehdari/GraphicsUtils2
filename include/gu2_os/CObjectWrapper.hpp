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


#include <utility>


namespace gu2 {

// SDLObject adds RAII wrapping for SDL object pointers
template <typename T_Object, auto Creator, auto Destroyer>
class CObjectWrapper {
public:
    template <typename... T_Args>
    CObjectWrapper(T_Args&&... args) :
        _object (Creator(std::forward<T_Args>(args)...))
    {
        if (_object == nullptr) // An error occurred during object construction
            BACKEND_THROW_ERROR
    }
    ~CObjectWrapper()
    {
        if (_object)
            Destroyer(_object);
    }

    T_Object& operator*()
    {
        return *_object;
    }

    const T_Object& operator*() const
    {
        return *_object;
    }

    T_Object* operator->()
    {
        return _object;
    }

    const T_Object* operator->() const
    {
        return _object;
    }

    T_Object* get()
    {
        return _object;
    }

    const T_Object* get() const
    {
        return _object;
    }

    void destroy()
    {
        if (_object)
            Destroyer(_object);
        _object = nullptr;
    }

    void create()
    {
        if (_object == nullptr)
            Creator(_object);
    }

private:
    T_Object*    _object;
};

} // namespace gu2
