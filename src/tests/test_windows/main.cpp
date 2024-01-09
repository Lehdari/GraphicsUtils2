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


// Window closed by window close event
GU2_WINDOW(WindowCloseWindow) {
public:
    WindowCloseWindow(const gu2::WindowSettings& settings) :
        Window<WindowCloseWindow> (settings)
    {}

    void handleEvent(const gu2::Event& event)
    {
        if (event.type == gu2::Event::WINDOW &&
            event.window.action == gu2::WindowEventAction::CLOSE) {
            close();
        }
    }

    void render() {}
};

// Window closed by escape key press
GU2_WINDOW(EscapeKeyWindow) {
public:
    EscapeKeyWindow(const gu2::WindowSettings& settings) :
        Window<EscapeKeyWindow> (settings)
    {}

    void handleEvent(const gu2::Event& event)
    {
        if (event.type == gu2::Event::KEY &&
            event.key.state == gu2::KeyEventAction::PRESSED &&
            event.key.sym.scancode == gu2::ScanCode::ESCAPE) {
            close();
        }
    }

    void render() {}
};


int main(void)
{
    gu2::WindowSettings settings1;
    settings1.name = "Little WindowCloseWindow";
    settings1.w = 300;
    settings1.h = 200;

    gu2::WindowSettings settings2;
    settings2.name = "Big WindowCloseWindow";
    settings2.w = 400;
    settings2.h = 300;

    gu2::WindowSettings settings3;
    settings3.name = "Little EscapeKeyWindow";
    settings3.w = 300;
    settings3.h = 200;

    gu2::WindowSettings settings4;
    settings4.name = "Big EscapeKeyWindow";
    settings4.w = 400;
    settings4.h = 300;

    {
        WindowCloseWindow window1(settings1);
        WindowCloseWindow window2(settings2);
        EscapeKeyWindow window3(settings3);
        EscapeKeyWindow window4(settings4);

        gu2::App::addWindow(&window1);
        gu2::App::addWindow(&window2);
        gu2::App::addWindow(&window3);
        gu2::App::addWindow(&window4);

        #if GU2_BACKEND == GU2_BACKEND_SDL2
        {
            // Send SDL events to close the windows
            SDL_Event event;
            event.type = SDL_WINDOWEVENT;
            event.window.event = SDL_WindowEventID::SDL_WINDOWEVENT_CLOSE;
            event.window.windowID = window1.getId();
            SDL_PushEvent(&event);
            event.window.windowID = window2.getId();
            SDL_PushEvent(&event);

            event.type = SDL_KEYDOWN;
            event.key.keysym.scancode = SDL_SCANCODE_ESCAPE;
            event.window.windowID = window3.getId();
            SDL_PushEvent(&event);
            event.window.windowID = window4.getId();
            SDL_PushEvent(&event);
        }
        #elif GU2_BACKEND == GU2_BACKEND_GLFW
        {
            // GLFW uses callbacks, call them directly
            gu2::App::windowCloseCallback<WindowCloseWindow>(window1.getId());
            gu2::App::windowCloseCallback<WindowCloseWindow>(window2.getId());
            gu2::App::keyCallback<EscapeKeyWindow>(window3.getId(), GLFW_KEY_ESCAPE, -1, GLFW_PRESS, 0);
            gu2::App::keyCallback<EscapeKeyWindow>(window4.getId(), GLFW_KEY_ESCAPE, -1, GLFW_PRESS, 0);
        }
        #endif

        // Main loop, should return after 1 update
        if (gu2::App::update())
            return EXIT_FAILURE;
    }

    gu2::cleanupBackend();

    return EXIT_SUCCESS;
}