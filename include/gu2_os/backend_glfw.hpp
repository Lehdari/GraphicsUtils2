//
// Project: GraphicsUtils2
// File: backend_glfw.hpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include <GLFW/glfw3.h>

#include <stdexcept>


// Macro for querying GLFW error and wrapping it into an exception
#define BACKEND_THROW_ERROR {                   \
    const char* description;                    \
    int code = glfwGetError(&description);      \
    if (code != GLFW_NO_ERROR)                  \
        throw std::runtime_error(description);  \
}


#include "CObjectWrapper.hpp"


namespace gu2 {

struct WindowSettings {
    enum Position : int {
        CENTERED    =   -1, // unsupported for now
        UNDEFINED   =   -1
    };

    std::string name    {"window"};
    int         w       {1280};
    int         h       {720};
    int         x       {Position::CENTERED};
    int         y       {Position::CENTERED};
};


// Predefined GLFW objects
using WindowObject = CObjectWrapper<GLFWwindow, glfwCreateWindow, glfwDestroyWindow>;


namespace detail {

WindowObject createWindowObject(const WindowSettings& settings);

} // namespace detail
} // namespace gu2
