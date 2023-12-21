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


namespace gu2 {

class Window {
public:
    Window();

    void update();

private:
    SDLWindow   _window;
};

} // namespace gu2


#endif //GRAPHICSUTILS2_WINDOW_HPP
