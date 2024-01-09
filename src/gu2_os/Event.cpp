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

#include <array>


using namespace gu2;


#if GU2_BACKEND == GU2_BACKEND_SDL2

KeySym::KeySym(const SDL_Keysym& sym) :
    scancode    (static_cast<ScanCode>(sym.scancode)),
    keycode     (static_cast<KeyCode>(sym.sym)),
    mod         (static_cast<KeyMod>(sym.mod))
{
}

KeyEvent::KeyEvent(const SDL_KeyboardEvent& sdlKeyboardEvent, uint32_t sdlEventType) :
    state   (sdlEventType == SDL_KEYDOWN ? (sdlKeyboardEvent.repeat ? KeyEventAction::REPEATED : KeyEventAction::PRESSED)
             : KeyEventAction::RELEASED),
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
                    window.action = WindowEventAction::CLOSE; break;
                default:
                    window.action = WindowEventAction::UNKNOWN;
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

#elif GU2_BACKEND == GU2_BACKEND_GLFW
#define GU2_GLFW_KEYMAP(GLFW_KEY, KEYCODE, SCANCODE) case GLFW_KEY_##GLFW_KEY: {\
    *keyCodeMapIndex = KeyCode::KEYCODE;\
    *scanCodeMapIndex = ScanCode::SCANCODE;\
} return;

namespace {
    inline KeyMod convertGLFWKeyMod(int glfwKeyMods)
    {
        auto mod = static_cast<uint16_t>(KeyMod::NONE);
        if (glfwKeyMods & GLFW_MOD_SHIFT)
            mod |= static_cast<uint16_t>(KeyMod::SHIFT);
        if (glfwKeyMods & GLFW_MOD_CONTROL)
            mod |= static_cast<uint16_t>(KeyMod::CTRL);
        if (glfwKeyMods & GLFW_MOD_ALT)
            mod |= static_cast<uint16_t>(KeyMod::ALT);
        if (glfwKeyMods & GLFW_MOD_SUPER)
            mod |= static_cast<uint16_t>(KeyMod::SUPER);
        if (glfwKeyMods & GLFW_MOD_CAPS_LOCK)
            mod |= static_cast<uint16_t>(KeyMod::CAPS_LOCK);
        if (glfwKeyMods & GLFW_MOD_NUM_LOCK)
            mod |= static_cast<uint16_t>(KeyMod::NUM_LOCK);
        return static_cast<KeyMod>(mod);
    }

    inline KeyEventAction convertGLFWKeyState(int glfwKeyAction)
    {
        switch (glfwKeyAction) {
            case GLFW_PRESS: return KeyEventAction::PRESSED;
            case GLFW_RELEASE: return KeyEventAction::RELEASED;
            case GLFW_REPEAT: return KeyEventAction::REPEATED;
            default:
                return KeyEventAction::UNKNOWN;
        }
        return KeyEventAction::UNKNOWN;
    }

    inline void storeGLFWKeyMapValues(int key, KeyCode* keyCodeMapIndex, ScanCode* scanCodeMapIndex)
    {
        switch (key) {
            GU2_GLFW_KEYMAP(A, a, A)
            GU2_GLFW_KEYMAP(B, b, B)
            GU2_GLFW_KEYMAP(C, c, C)
            GU2_GLFW_KEYMAP(D, d, D)
            GU2_GLFW_KEYMAP(E, e, E)
            GU2_GLFW_KEYMAP(F, f, F)
            GU2_GLFW_KEYMAP(G, g, G)
            GU2_GLFW_KEYMAP(H, h, H)
            GU2_GLFW_KEYMAP(I, i, I)
            GU2_GLFW_KEYMAP(J, j, J)
            GU2_GLFW_KEYMAP(K, k, K)
            GU2_GLFW_KEYMAP(L, l, L)
            GU2_GLFW_KEYMAP(M, m, M)
            GU2_GLFW_KEYMAP(N, n, N)
            GU2_GLFW_KEYMAP(O, o, O)
            GU2_GLFW_KEYMAP(P, p, P)
            GU2_GLFW_KEYMAP(Q, q, Q)
            GU2_GLFW_KEYMAP(R, r, R)
            GU2_GLFW_KEYMAP(S, s, S)
            GU2_GLFW_KEYMAP(T, t, T)
            GU2_GLFW_KEYMAP(U, u, U)
            GU2_GLFW_KEYMAP(V, v, V)
            GU2_GLFW_KEYMAP(W, w, W)
            GU2_GLFW_KEYMAP(X, x, X)
            GU2_GLFW_KEYMAP(Y, y, Y)
            GU2_GLFW_KEYMAP(Z, z, Z)
            GU2_GLFW_KEYMAP(0, N_0, N_0)
            GU2_GLFW_KEYMAP(1, N_1, N_1)
            GU2_GLFW_KEYMAP(2, N_2, N_2)
            GU2_GLFW_KEYMAP(3, N_3, N_3)
            GU2_GLFW_KEYMAP(4, N_4, N_4)
            GU2_GLFW_KEYMAP(5, N_5, N_5)
            GU2_GLFW_KEYMAP(6, N_6, N_6)
            GU2_GLFW_KEYMAP(7, N_7, N_7)
            GU2_GLFW_KEYMAP(8, N_8, N_8)
            GU2_GLFW_KEYMAP(9, N_9, N_9)
            GU2_GLFW_KEYMAP(SPACE, SPACE, SPACE)
            GU2_GLFW_KEYMAP(APOSTROPHE, QUOTE, APOSTROPHE)
            GU2_GLFW_KEYMAP(COMMA, COMMA, COMMA)
            GU2_GLFW_KEYMAP(MINUS, MINUS, MINUS)
            GU2_GLFW_KEYMAP(PERIOD, PERIOD, PERIOD)
            GU2_GLFW_KEYMAP(SLASH, SLASH, SLASH)
            GU2_GLFW_KEYMAP(SEMICOLON, SEMICOLON, SEMICOLON)
            GU2_GLFW_KEYMAP(EQUAL, EQUALS, EQUALS)
            GU2_GLFW_KEYMAP(LEFT_BRACKET, LEFTBRACKET, LEFTBRACKET)
            GU2_GLFW_KEYMAP(RIGHT_BRACKET, RIGHTBRACKET, RIGHTBRACKET)
            GU2_GLFW_KEYMAP(BACKSLASH, BACKSLASH, BACKSLASH)
            GU2_GLFW_KEYMAP(GRAVE_ACCENT, BACKQUOTE, GRAVE)
            GU2_GLFW_KEYMAP(WORLD_1, UNKNOWN, INTERNATIONAL1)
            GU2_GLFW_KEYMAP(WORLD_2, UNKNOWN, INTERNATIONAL2)
            GU2_GLFW_KEYMAP(ESCAPE, ESCAPE, ESCAPE)
            GU2_GLFW_KEYMAP(ENTER, RETURN, RETURN)
            GU2_GLFW_KEYMAP(TAB, TAB, TAB)
            GU2_GLFW_KEYMAP(BACKSPACE, BACKSPACE, BACKSPACE)
            GU2_GLFW_KEYMAP(INSERT, INSERT, INSERT)
            GU2_GLFW_KEYMAP(DELETE, DELETE, DELETE)
            GU2_GLFW_KEYMAP(RIGHT, RIGHT, RIGHT)
            GU2_GLFW_KEYMAP(LEFT, LEFT, LEFT)
            GU2_GLFW_KEYMAP(DOWN, DOWN, DOWN)
            GU2_GLFW_KEYMAP(UP, UP, UP)
            GU2_GLFW_KEYMAP(PAGE_UP, PAGEUP, PAGEUP)
            GU2_GLFW_KEYMAP(PAGE_DOWN, PAGEDOWN, PAGEDOWN)
            GU2_GLFW_KEYMAP(HOME, HOME, HOME)
            GU2_GLFW_KEYMAP(END, END, END)
            GU2_GLFW_KEYMAP(CAPS_LOCK, CAPSLOCK, CAPSLOCK)
            GU2_GLFW_KEYMAP(SCROLL_LOCK, SCROLLLOCK, SCROLLLOCK)
            GU2_GLFW_KEYMAP(NUM_LOCK, NUMLOCKCLEAR, NUMLOCKCLEAR)
            GU2_GLFW_KEYMAP(PRINT_SCREEN, PRINTSCREEN, PRINTSCREEN)
            GU2_GLFW_KEYMAP(PAUSE, PAUSE, PAUSE)
            GU2_GLFW_KEYMAP(F1, F1, F1)
            GU2_GLFW_KEYMAP(F2, F2, F2)
            GU2_GLFW_KEYMAP(F3, F3, F3)
            GU2_GLFW_KEYMAP(F4, F4, F4)
            GU2_GLFW_KEYMAP(F5, F5, F5)
            GU2_GLFW_KEYMAP(F6, F6, F6)
            GU2_GLFW_KEYMAP(F7, F7, F7)
            GU2_GLFW_KEYMAP(F8, F8, F8)
            GU2_GLFW_KEYMAP(F9, F9, F9)
            GU2_GLFW_KEYMAP(F10, F10, F10)
            GU2_GLFW_KEYMAP(F11, F11, F11)
            GU2_GLFW_KEYMAP(F12, F12, F12)
            GU2_GLFW_KEYMAP(F13, F13, F13)
            GU2_GLFW_KEYMAP(F14, F14, F14)
            GU2_GLFW_KEYMAP(F15, F15, F15)
            GU2_GLFW_KEYMAP(F16, F16, F16)
            GU2_GLFW_KEYMAP(F17, F17, F17)
            GU2_GLFW_KEYMAP(F18, F18, F18)
            GU2_GLFW_KEYMAP(F19, F19, F19)
            GU2_GLFW_KEYMAP(F20, F20, F20)
            GU2_GLFW_KEYMAP(F21, F21, F21)
            GU2_GLFW_KEYMAP(F22, F22, F22)
            GU2_GLFW_KEYMAP(F23, F23, F23)
            GU2_GLFW_KEYMAP(F24, F24, F24)
            GU2_GLFW_KEYMAP(KP_0, KP_0, KP_0)
            GU2_GLFW_KEYMAP(KP_1, KP_1, KP_1)
            GU2_GLFW_KEYMAP(KP_2, KP_2, KP_2)
            GU2_GLFW_KEYMAP(KP_3, KP_3, KP_3)
            GU2_GLFW_KEYMAP(KP_4, KP_4, KP_4)
            GU2_GLFW_KEYMAP(KP_5, KP_5, KP_5)
            GU2_GLFW_KEYMAP(KP_6, KP_6, KP_6)
            GU2_GLFW_KEYMAP(KP_7, KP_7, KP_7)
            GU2_GLFW_KEYMAP(KP_8, KP_8, KP_8)
            GU2_GLFW_KEYMAP(KP_9, KP_9, KP_9)
            GU2_GLFW_KEYMAP(KP_DECIMAL, KP_DECIMAL, KP_DECIMAL)
            GU2_GLFW_KEYMAP(KP_DIVIDE, KP_DIVIDE, KP_DIVIDE)
            GU2_GLFW_KEYMAP(KP_MULTIPLY, KP_MULTIPLY, KP_MULTIPLY)
            GU2_GLFW_KEYMAP(KP_SUBTRACT, KP_MEMSUBTRACT, KP_MEMSUBTRACT)
            GU2_GLFW_KEYMAP(KP_ADD, KP_MEMADD, KP_MEMADD)
            GU2_GLFW_KEYMAP(KP_ENTER, KP_ENTER, KP_ENTER)
            GU2_GLFW_KEYMAP(KP_EQUAL, KP_EQUALS, KP_EQUALS)
            GU2_GLFW_KEYMAP(LEFT_SHIFT, LSHIFT, LSHIFT)
            GU2_GLFW_KEYMAP(LEFT_CONTROL, LCTRL, LCTRL)
            GU2_GLFW_KEYMAP(LEFT_ALT, LALT, LALT)
            GU2_GLFW_KEYMAP(LEFT_SUPER, LGUI, LGUI)
            GU2_GLFW_KEYMAP(RIGHT_SHIFT, RSHIFT, RSHIFT)
            GU2_GLFW_KEYMAP(RIGHT_CONTROL, RCTRL, RCTRL)
            GU2_GLFW_KEYMAP(RIGHT_ALT, RALT, RALT)
            GU2_GLFW_KEYMAP(RIGHT_SUPER, RGUI, RGUI)
            GU2_GLFW_KEYMAP(MENU, MENU, MENU)
            default: {
                *keyCodeMapIndex = KeyCode::UNKNOWN;
                *scanCodeMapIndex = ScanCode::UNKNOWN;
            }   return;
        }
    }

    auto [glfwKeyCodeMap, glfwScanCodeMap] = [](){
        glfwInit();

        std::array<KeyCode, GLFW_KEY_LAST> keyCodeMap{};
        std::fill(keyCodeMap.begin(), keyCodeMap.end(), KeyCode::UNKNOWN);
        constexpr uint32_t nScancodes = 512;
        std::array<ScanCode, nScancodes> scanCodeMap{};
        std::fill(scanCodeMap.begin(), scanCodeMap.end(), ScanCode::UNKNOWN);

        int maxScancode = 0;
        for (int keyCode=0; keyCode<GLFW_KEY_LAST; ++keyCode) {
            auto scanCode = glfwGetKeyScancode(keyCode);
            if (scanCode >= nScancodes)
                continue; // shouldn't happen
            storeGLFWKeyMapValues(keyCode, &keyCodeMap[keyCode], &scanCodeMap[scanCode]);
        }
        return std::pair{keyCodeMap, scanCodeMap};
    }();
}

KeySym::KeySym(int key, int scancode, int mods) :
    scancode    (static_cast<ScanCode>(glfwScanCodeMap[scancode >= 0 ? scancode : glfwGetKeyScancode(key)])),
    keycode     (static_cast<KeyCode>(glfwKeyCodeMap[key])),
    mod         (static_cast<KeyMod>(convertGLFWKeyMod(mods)))
{
}

KeyEvent::KeyEvent(int key, int scancode, int action, int mods) :
    state   (convertGLFWKeyState(action)),
    sym     (key, scancode, mods)
{
}

#endif