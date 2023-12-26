//
// Project: GraphicsUtils2
// File: Window.inl
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

template <typename T_Derived>
Window<T_Derived>::Window(const WindowSettings& settings) :
    _settings   (settings),
    _window     (detail::createWindowObject(_settings))
{
}

template<typename T_Derived>
void Window<T_Derived>::close()
{
    _window.destroy();
}

template <typename T_Derived>
const WindowSettings& Window<T_Derived>::getSettings() const
{
    return _settings;
}

template<typename T_Derived>
bool Window<T_Derived>::isOpen() const
{
    return _window.get() != nullptr;
}

template <typename T_Derived>
void Window<T_Derived>::handleEvent(const Event& event)
{
    static_cast<T_Derived*>(this)->handleEvent(event);
}

template <typename T_Derived>
void Window<T_Derived>::render()
{
    static_cast<T_Derived*>(this)->render();
}
