//
// Project: GraphicsUtils2
// File: main.cpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include <gu2_os/App.hpp>
#include <gu2_os/Window.hpp>


class Window1 : public gu2::Window<Window1> {
public:
    Window1(const gu2::WindowSettings& settings) :
        Window<Window1> (settings)
    {
    }

    ~Window1() {}

    void handleEvent(const gu2::Event& event)
    {
        switch (event.type) {
            case gu2::Event::WINDOW:
                switch (event.window.event) {
                    case gu2::WindowEventID::CLOSE: close(); return;
                }
                break;
            default: break;
        }
    }

    void render()
    {
        // TODO
    }
};


int main(void)
{
    gu2::WindowSettings settings1;
    settings1.name = "Little Window";
    settings1.w = 640;
    settings1.h = 480;
    Window1 window1(settings1);

    gu2::WindowSettings settings2;
    settings2.name = "Big Window";
    settings2.w = 800;
    settings2.h = 600;
    Window1 window2(settings2);

    gu2::App::addWindow(&window1);
    gu2::App::addWindow(&window2);

    // Main loop
    while (gu2::App::update());

    return 0;
}