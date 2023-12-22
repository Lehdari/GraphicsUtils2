//
// Project: GraphicsUtils2
// File: Window.cpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include <Window.hpp>


using namespace gu2;


Window::Window(const WindowSettings& settings) :
    _settings   (settings),
    _window     ("window", _settings.x, _settings.y, _settings.w, _settings.h, SDL_WINDOW_VULKAN)
{
}

void Window::update()
{

}

const WindowSettings& Window::getSettings() const
{
    return _settings;
}
