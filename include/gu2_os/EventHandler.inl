//
// Project: GraphicsUtils2
// File: EventHandler.inl
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

template<typename T_Derived>
void EventHandler::addWindow(Window<T_Derived>* window)
{
    _windows.emplace_back(
        window->_window.get(),
        &windowHandleEvent<T_Derived>,
        &windowIsOpen<Window<T_Derived>>
    );
    _windowMap[window->_window.get()] = window;

    #if GU2_BACKEND == GU2_BACKEND_SDL2
    _sdlWindowIdMap[SDL_GetWindowID(window->_window.get())] = _windows.size() - 1;
    #elif GU2_BACKEND == GU2_BACKEND_GLFW
    glfwSetWindowCloseCallback(window->_window.get(), &pushGLFWWindowCloseEvent<T_Derived>);
    #endif
}

#if GU2_BACKEND == GU2_BACKEND_GLFW

template <typename T_Window>
void EventHandler::pushGLFWWindowCloseEvent(GLFWwindow* window)
{
    Event event;
    event.type = gu2::Event::WINDOW;
    event.window.event = gu2::WINDOWEVENT_CLOSE;

    static_cast<T_Window*>(_windowMap.at(window))->handleEvent(event);
}

#endif

template<typename T_Window>
void EventHandler::windowHandleEvent(void* window, const Event& event)
{
    static_cast<T_Window*>(window)->handleEvent(event);
}

template<typename T_Window>
bool EventHandler::windowIsOpen(void* window)
{
    return static_cast<T_Window*>(window)->isOpen();
}
