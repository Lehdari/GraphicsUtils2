//
// Project: GraphicsUtils2
// File: EventHandler.hpp
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


class EventHandler {
public:
    using Callback = void(*)(const Event&);

    EventHandler(Callback callback = nullptr);

    template <typename T_Derived>
    void addWindow(Window<T_Derived>* window);

    // Returns false if all windows have been closed
    bool operator()();

    #if GU2_BACKEND == GU2_BACKEND_GLFW
    template <typename T_Window>
    static inline void pushGLFWWindowCloseEvent(GLFWwindow* window);
    #endif

private:
    using WindowHandleEventFunction = void(*)(void*, const Event& event);
    using WindowIsOpenFunction = bool(*)(void*);
    using WindowMap = std::unordered_map<detail::BackendWindow*, void*>;

    // Helper struct needed for type erasure
    struct WindowStorage {
        detail::BackendWindow*      window;
        WindowHandleEventFunction   handleEvent;
        WindowIsOpenFunction        isOpen;
    };

    Callback                    _callback;
    std::vector<WindowStorage>  _windows;
    static WindowMap            _windowMap;    /// Map from backend window pointers to user defined window types for all open windows

    #if GU2_BACKEND == GU2_BACKEND_SDL2
    using SDLWindowIdMap = std::unordered_map<uint32_t, std::vector<WindowStorage>::size_type>;
    SDLWindowIdMap              _sdlWindowIdMap; /// Map from SDL window id to event handler function pointers
    #endif


    template <typename T_Window>
    static inline void windowHandleEvent(void* window, const Event& event);

    template <typename T_Window>
    static inline bool windowIsOpen(void* window);

    #if GU2_BACKEND == GU2_BACKEND_SDL2
    static Event convertSDLEvent(const SDL_Event& sdlEvent);
    #endif
};

#include "EventHandler.inl"

} // namespace gu2
