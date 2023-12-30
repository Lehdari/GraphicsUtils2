//
// Project: GraphicsUtils2
// File: Window.hpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "backend.hpp"
#include "CObjectWrapper.hpp"

#include <string>


namespace gu2 {

#if GU2_BACKEND == GU2_BACKEND_SDL2

using WindowId = uint32_t;
constexpr WindowId defaultWindowId = 0;

#elif GU2_BACKEND == GU2_BACKEND_GLFW

using WindowId = GLFWwindow*;
constexpr WindowId defaultWindowId = nullptr;

#endif // GU2_BACKEND


template <typename T_Derived>
class Window {
public:
    using Settings = WindowSettings;

    Window(const Settings& settings = Settings());

    void close();

    const Settings& getSettings() const;
    WindowId getId() const;
    bool isOpen() const;

    void handleEvent(const Event& event);
    void render();

    friend class App;
protected:
    Settings        _settings;

private:
    WindowObject    _window;

protected:
    WindowId        _id;
};


#include "Window.inl"

} // namespace gu2
