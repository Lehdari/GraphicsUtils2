//
// Project: GraphicsUtils2
// File: Event.cpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "Event.hpp"


using namespace gu2;


#if GU2_BACKEND == GU2_BACKEND_SDL2

KeySym::KeySym(const SDL_Keysym& sym) :
    scancode    (static_cast<gu2::ScanCode>(sym.scancode)),
    keycode     (static_cast<gu2::KeyCode>(sym.sym)),
    mods        (sym.mod)
{
}

KeyEvent::KeyEvent(const SDL_KeyboardEvent& sdlKeyboardEvent, uint32_t sdlEventType) :
    state   (sdlEventType == SDL_KEYDOWN ? (sdlKeyboardEvent.repeat ? KeyEventState::REPEATED : KeyEventState::PRESSED)
             : KeyEventState::RELEASED),
    sym     (sdlKeyboardEvent.keysym)
{
}

Event::Event(const SDL_Event& sdlEvent)
{
    switch (sdlEvent.type) {
        case SDL_WINDOWEVENT: {
            type = Event::WINDOW;
            switch (sdlEvent.window.event) {
                case SDL_WINDOWEVENT_CLOSE:
                    window.event = WindowEventID::CLOSE; break;
                default:
                    window.event = WindowEventID::NONE;
            }
        }   break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            type = Event::KEY;
            key = KeyEvent(sdlEvent.key, sdlEvent.type);
        }   break;
        default:
            type = Event::UNDEFINED;
    }
}

#endif