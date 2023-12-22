//
// Project: GraphicsUtils2
// File: main.cpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include <gu2_os/Application.hpp>
#include <gu2_os/Window.hpp>


class App : gu2::Application<App> {
public:
    App() :
        _running    (true)
    {}

    void handleEvent(const SDL_Event& event, const gu2::Window& window)
    {
        switch(event.type) {
            case SDL_QUIT:
                _running = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    _running = false;
                break;
        }
    }

    void render(const gu2::Window& window)
    {
        // TODO
    }

    void loop(gu2::Window& window)
    {
        while (_running) {
            window.update(*this);
            SDL_Delay(10);
        }
    }

private:
    bool    _running;
};


int main(void)
{
    gu2::WindowSettings settings;
    settings.w = 800;
    settings.h = 600;
    gu2::Window window(settings);
    App app;
    app.loop(window);

    return 0;
}