//
// Project: GraphicsUtils2
// File: App.hpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include "backend.hpp"
#include "Event.hpp"

#include <cstdint>
#include <vector>
#include <unordered_map>


namespace gu2 {

template <typename T_Derived>
class Window;


class App {
public:
    template <typename T_Derived>
    static void addWindow(Window<T_Derived>* window);

    // Returns false if all windows have been closed
    static bool update();

    #if GU2_BACKEND == GU2_BACKEND_GLFW
    template <typename T_Window>
    static inline void pushGLFWWindowCloseEvent(GLFWwindow* window);
    #endif

private:
    using WindowHandleEventFunction = void(*)(void*, const Event& event);
    using WindowIsOpenFunction = bool(*)(void*);

    // Helper struct needed for type erasure
    struct WindowStorage {
        void*                       window;
        WindowHandleEventFunction   handleEvent;
        WindowIsOpenFunction        isOpen;
    };

    #if GU2_BACKEND == GU2_BACKEND_SDL2
    using WindowId = uint32_t;
    #elif GU2_BACKEND == GU2_BACKEND_GLFW
    using WindowId = GLFWwindow*;
    #endif // GU2_BACKEND
    using WindowMap = std::unordered_map<WindowId, WindowStorage>;

    static WindowMap    _windowMap;    /// Map from backend window pointers to user defined window types for all open windows

    // App provides a fully static interface
    App() = default;

    template <typename T_Window>
    static inline void windowHandleEvent(void* window, const Event& event);

    template <typename T_Window>
    static inline bool windowIsOpen(void* window);

    #if GU2_BACKEND == GU2_BACKEND_SDL2
    static Event convertSDLEvent(const SDL_Event& sdlEvent);
    #endif
};

#include "App.inl"

} // namespace gu2
