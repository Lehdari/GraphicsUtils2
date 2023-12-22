//
// Project: GraphicsUtils2
// File: Application.hpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


union SDL_Event;


namespace gu2 {


class Window;


template <typename T_Derived>
class Application {
public:
    void handleEvent(const SDL_Event& event, const Window& window)
    {
        static_cast<T_Derived*>(this)->handleEvent(event, window);
    }

    void render(const Window& window)
    {
        static_cast<T_Derived*>(this)->render(window);
    }
};


} // namespace gu2
