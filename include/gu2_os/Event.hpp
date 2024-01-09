//
// Project: GraphicsUtils2
// File: Event.hpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "backend.hpp"
#include "KeyCode.hpp"


namespace gu2 {

enum class WindowEventAction : uint8_t {
    UNKNOWN,
    CLOSE
};

struct WindowEvent {
    WindowEventAction   action;
};

enum class KeyEventAction : uint8_t {
    UNKNOWN,
    PRESSED,
    RELEASED,
    REPEATED
};

struct KeySym {
    ScanCode    scancode;
    KeyCode     keycode;
    KeyMod      mod;

    KeySym() = default;
#if GU2_BACKEND == GU2_BACKEND_SDL2
    KeySym(const SDL_Keysym& sym);
#elif GU2_BACKEND == GU2_BACKEND_GLFW
    KeySym(int key, int scancode, int mods);
#endif // GU2_BACKEND
};

struct KeyEvent {
    KeyEventAction   state;
    KeySym          sym;

    KeyEvent() = default;
#if GU2_BACKEND == GU2_BACKEND_SDL2
    KeyEvent(const SDL_KeyboardEvent& sdlKeyboardEvent, uint32_t sdlEventType);
#elif GU2_BACKEND == GU2_BACKEND_GLFW
    KeyEvent(int key, int scancode, int action, int mods);
#endif // GU2_BACKEND
};

struct Event {
    enum Type : uint32_t {
        UNDEFINED,
        WINDOW,
        KEY,
        QUIT
    };

    Type type;

    union {
        WindowEvent window;
        KeyEvent    key;
    };

#if GU2_BACKEND == GU2_BACKEND_SDL2
    Event(const SDL_Event& sdlEvent);
#endif // GU2_BACKEND
};

} // namespace gu2
