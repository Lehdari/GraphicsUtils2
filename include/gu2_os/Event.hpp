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


namespace gu2 {

enum WindowEventID : uint8_t {
    WINDOWEVENT_NONE,
    WINDOWEVENT_CLOSE
};

struct WindowEvent {
    WindowEventID   event;
};

struct Event {
    enum Type : uint32_t {
        UNDEFINED,
        WINDOW,
        QUIT
    };

    Type type;

    union {
        WindowEvent window;
    };
};

} // namespace gu2
