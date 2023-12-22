//
// Project: GraphicsUtils2
// File: Window.inl
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

template<typename T_Application>
void Window::update(T_Application& application)
{
    SDL_Event event;
    while(SDL_PollEvent(&event) > 0) {
        application.handleEvent(event, *this);
    }
    application.render(*this);
}
