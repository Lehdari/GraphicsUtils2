//
// Project: GraphicsUtils2
// File: Window.hpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#ifndef GRAPHICSUTILS2_WINDOW_HPP
#define GRAPHICSUTILS2_WINDOW_HPP


#include "SDLObject.hpp"

#include <string>


namespace gu2 {

struct WindowSettings {
    std::string name    {"window"};
    int         w       {1280};
    int         h       {720};
    int         x       {SDL_WINDOWPOS_CENTERED};
    int         y       {SDL_WINDOWPOS_CENTERED};
};

class Window {
public:
    using Settings = WindowSettings;

    Window(const Settings& settings = Settings());

    void update();

    const Settings& getSettings() const;

private:
    Settings    _settings;
    SDLWindow   _window;
};

} // namespace gu2


#endif //GRAPHICSUTILS2_WINDOW_HPP
