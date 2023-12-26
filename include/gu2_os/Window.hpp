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

template <typename T_Derived>
class Window {
public:
    using Settings = WindowSettings;

    Window(const Settings& settings = Settings());

    void close();

    const Settings& getSettings() const;
    bool isOpen() const;

    void handleEvent(const Event& event);
    void render();

    friend class EventHandler;
protected:
    Settings        _settings;

private:
    WindowObject    _window;
};

#include "Window.inl"

} // namespace gu2
