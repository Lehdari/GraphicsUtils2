//
// Project: GraphicsUtils2
// File: App.cpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "App.hpp"
#include "Window.hpp"


using namespace gu2;


App::WindowMap App::_windowMap;


bool App::update()
{
    #if GU2_BACKEND == GU2_BACKEND_SDL2
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        switch (sdlEvent.type) {
            case SDL_WINDOWEVENT: {
                auto& window = _windowMap.at(sdlEvent.window.windowID);
                window.handleEvent(window.window, convertSDLEvent(sdlEvent));
            }   break;
        }
    }
    #elif GU2_BACKEND == GU2_BACKEND_GLFW
    glfwPollEvents();
    #endif

    // TODO remove closed windows from _windows vector (and _windowMap)

    for (const auto& [windowId, windowStorage] : _windowMap) {
        if (windowStorage.isOpen(windowStorage.window))
            return true;
    }

    return false;
}

#if GU2_BACKEND == GU2_BACKEND_SDL2

Event App::convertSDLEvent(const SDL_Event& sdlEvent)
{
    Event event;
    switch (sdlEvent.type) {
        case SDL_WINDOWEVENT: {
            event.type = Event::WINDOW;
            switch (sdlEvent.window.event) {
                case SDL_WINDOWEVENT_CLOSE:
                    event.window.event = WINDOWEVENT_CLOSE; break;
                default:
                    event.window.event = WINDOWEVENT_NONE;
            }
        }   break;
        default:
            event.type = Event::UNDEFINED;
    }
    return event;
}

#endif // GU2_BACKEND
