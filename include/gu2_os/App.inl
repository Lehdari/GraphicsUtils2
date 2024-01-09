//
// Project: GraphicsUtils2
// File: App.inl
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

template<typename T_Derived>
void App::addWindow(Window<T_Derived>* window)
{
    #if GU2_BACKEND == GU2_BACKEND_SDL2
    WindowId windowId = SDL_GetWindowID(window->_window.get());
    #elif GU2_BACKEND == GU2_BACKEND_GLFW
    WindowId windowId = window->_window.get();
    #endif
    _windowMap.emplace(std::make_pair<WindowId, WindowStorage>(
        std::move(windowId), {
        window,
        &windowHandleEvent<T_Derived>,
        &windowRender<T_Derived>,
        &windowIsOpen<Window<T_Derived>>
    }));

    #if GU2_BACKEND == GU2_BACKEND_GLFW
    glfwSetWindowCloseCallback(window->_window.get(), &windowCloseCallback<T_Derived>);
    glfwSetWindowSizeCallback(window->_window.get(), &windowSizeCallback<T_Derived>);
    glfwSetKeyCallback(window->_window.get(), &keyCallback<T_Derived>);
    #endif
}

#if GU2_BACKEND == GU2_BACKEND_GLFW

template <typename T_Window>
void App::windowCloseCallback(GLFWwindow* window)
{
    Event event;
    event.type = gu2::Event::WINDOW;
    event.window.action = gu2::WindowEventAction::CLOSE;

    static_cast<T_Window*>(_windowMap.at(window).window)->handleEvent(event);
}

template <typename T_Window>
void App::windowSizeCallback(GLFWwindow* window, int width, int height)
{
    Event event;
    event.type = gu2::Event::WINDOW;
    event.window.action = gu2::WindowEventAction::RESIZE;
    event.window.data1 = width;
    event.window.data2 = height;

    static_cast<T_Window*>(_windowMap.at(window).window)->handleEvent(event);
}

template <typename T_Window>
void App::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Event event;
    event.type = gu2::Event::KEY;
    event.key = KeyEvent(key, scancode, action, mods);

    static_cast<T_Window*>(_windowMap.at(window).window)->handleEvent(event);
}

#endif

template<typename T_Window>
void App::windowHandleEvent(void* window, const Event& event)
{
    static_cast<T_Window*>(window)->handleEvent(event);
}

template<typename T_Window>
void App::windowRender(void* window)
{
    static_cast<T_Window*>(window)->render();
}

template<typename T_Window>
bool App::windowIsOpen(void* window)
{
    return static_cast<T_Window*>(window)->isOpen();
}
