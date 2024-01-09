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
#include "Window.hpp"

#include <cstdint>
#include <vector>
#include <unordered_map>


namespace gu2 {

class App {
public:
    template <typename T_Derived>
    static void addWindow(Window<T_Derived>* window);

    // Returns false if all windows have been closed
    static bool update();

    #if GU2_BACKEND == GU2_BACKEND_GLFW
    template <typename T_Window>
    static inline void windowCloseCallback(GLFWwindow* window);
    template <typename T_Window>
    static inline void windowSizeCallback(GLFWwindow* window, int width, int height);
    template <typename T_Window>
    static inline void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    #endif

private:
    using WindowHandleEventFunction = void(*)(void*, const Event& event);
    using WindowRenderFunction = void(*)(void*);
    using WindowIsOpenFunction = bool(*)(void*);

    // Helper struct needed for type erasure
    struct WindowStorage {
        void*                       window;
        WindowHandleEventFunction   handleEvent;
        WindowRenderFunction        render;
        WindowIsOpenFunction        isOpen;
    };

    using WindowMap = std::unordered_map<WindowId, WindowStorage>;

    static WindowMap    _windowMap;    /// Map from backend window pointers to user defined window types for all open windows

    // App provides a fully static interface
    App() = default;

    template <typename T_Window>
    static inline void windowHandleEvent(void* window, const Event& event);

    template <typename T_Window>
    static inline void windowRender(void* window);

    template <typename T_Window>
    static inline bool windowIsOpen(void* window);
};

#include "App.inl"

} // namespace gu2
