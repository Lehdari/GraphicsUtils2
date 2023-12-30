//
// Project: GraphicsUtils2
// File: KeyCode.hpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include <cstdint>


namespace gu2 {

// Copied from SDL2
enum class ScanCode : uint32_t {
    UNKNOWN = 0,

    /**
     *  \name Usage page 0x07
     *
     *  These values are from usage page 0x07 (USB keyboard page).
     */
    /*@{*/

    A = 4,
    B = 5,
    C = 6,
    D = 7,
    E = 8,
    F = 9,
    G = 10,
    H = 11,
    I = 12,
    J = 13,
    K = 14,
    L = 15,
    M = 16,
    N = 17,
    O = 18,
    P = 19,
    Q = 20,
    R = 21,
    S = 22,
    T = 23,
    U = 24,
    V = 25,
    W = 26,
    X = 27,
    Y = 28,
    Z = 29,

    N_1 = 30,
    N_2 = 31,
    N_3 = 32,
    N_4 = 33,
    N_5 = 34,
    N_6 = 35,
    N_7 = 36,
    N_8 = 37,
    N_9 = 38,
    N_0 = 39,

    RETURN = 40,
    ESCAPE = 41,
    BACKSPACE = 42,
    TAB = 43,
    SPACE = 44,

    MINUS = 45,
    EQUALS = 46,
    LEFTBRACKET = 47,
    RIGHTBRACKET = 48,
    BACKSLASH = 49, /**< Located at the lower left of the return
                                  *   key on ISO keyboards and at the right end
                                  *   of the QWERTY row on ANSI keyboards.
                                  *   Produces REVERSE SOLIDUS (backslash) and
                                  *   VERTICAL LINE in a US layout, REVERSE
                                  *   SOLIDUS and VERTICAL LINE in a UK Mac
                                  *   layout, NUMBER SIGN and TILDE in a UK
                                  *   Windows layout, DOLLAR SIGN and POUND SIGN
                                  *   in a Swiss German layout, NUMBER SIGN and
                                  *   APOSTROPHE in a German layout, GRAVE
                                  *   ACCENT and POUND SIGN in a French Mac
                                  *   layout, and ASTERISK and MICRO SIGN in a
                                  *   French Windows layout.
                                  */
    NONUSHASH = 50, /**< ISO USB keyboards actually use this code
                                  *   instead of 49 for the same key, but all
                                  *   OSes I've seen treat the two codes
                                  *   identically. So, as an implementor, unless
                                  *   your keyboard generates both of those
                                  *   codes and your OS treats them differently,
                                  *   you should generate BACKSLASH
                                  *   instead of this code. As a user, you
                                  *   should not rely on this code because SDL
                                  *   will never generate it with most (all?)
                                  *   keyboards.
                                  */
    SEMICOLON = 51,
    APOSTROPHE = 52,
    GRAVE = 53, /**< Located in the top left corner (on both ANSI
                              *   and ISO keyboards). Produces GRAVE ACCENT and
                              *   TILDE in a US Windows layout and in US and UK
                              *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                              *   and NOT SIGN in a UK Windows layout, SECTION
                              *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                              *   layouts on ISO keyboards, SECTION SIGN and
                              *   DEGREE SIGN in a Swiss German layout (Mac:
                              *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                              *   DEGREE SIGN in a German layout (Mac: only on
                              *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                              *   French Windows layout, COMMERCIAL AT and
                              *   NUMBER SIGN in a French Mac layout on ISO
                              *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                              *   SIGN in a Swiss German, German, or French Mac
                              *   layout on ANSI keyboards.
                              */
    COMMA = 54,
    PERIOD = 55,
    SLASH = 56,

    CAPSLOCK = 57,

    F1 = 58,
    F2 = 59,
    F3 = 60,
    F4 = 61,
    F5 = 62,
    F6 = 63,
    F7 = 64,
    F8 = 65,
    F9 = 66,
    F10 = 67,
    F11 = 68,
    F12 = 69,

    PRINTSCREEN = 70,
    SCROLLLOCK = 71,
    PAUSE = 72,
    INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
                                   does send code 73, not 117) */
    HOME = 74,
    PAGEUP = 75,
    DELETE = 76,
    END = 77,
    PAGEDOWN = 78,
    RIGHT = 79,
    LEFT = 80,
    DOWN = 81,
    UP = 82,

    NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                                     */
    KP_DIVIDE = 84,
    KP_MULTIPLY = 85,
    KP_MINUS = 86,
    KP_PLUS = 87,
    KP_ENTER = 88,
    KP_1 = 89,
    KP_2 = 90,
    KP_3 = 91,
    KP_4 = 92,
    KP_5 = 93,
    KP_6 = 94,
    KP_7 = 95,
    KP_8 = 96,
    KP_9 = 97,
    KP_0 = 98,
    KP_PERIOD = 99,

    NONUSBACKSLASH = 100, /**< This is the additional key that ISO
                                        *   keyboards have over ANSI ones,
                                        *   located between left shift and Y.
                                        *   Produces GRAVE ACCENT and TILDE in a
                                        *   US or UK Mac layout, REVERSE SOLIDUS
                                        *   (backslash) and VERTICAL LINE in a
                                        *   US or UK Windows layout, and
                                        *   LESS-THAN SIGN and GREATER-THAN SIGN
                                        *   in a Swiss German, German, or French
                                        *   layout. */
    APPLICATION = 101, /**< windows contextual menu, compose */
    POWER = 102, /**< The USB document says this is a status flag,
                               *   not a physical key - but some Mac keyboards
                               *   do have a power key. */
    KP_EQUALS = 103,
    F13 = 104,
    F14 = 105,
    F15 = 106,
    F16 = 107,
    F17 = 108,
    F18 = 109,
    F19 = 110,
    F20 = 111,
    F21 = 112,
    F22 = 113,
    F23 = 114,
    F24 = 115,
    EXECUTE = 116,
    HELP = 117,
    MENU = 118,
    SELECT = 119,
    STOP = 120,
    AGAIN = 121,   /**< redo */
    UNDO = 122,
    CUT = 123,
    COPY = 124,
    PASTE = 125,
    FIND = 126,
    MUTE = 127,
    VOLUMEUP = 128,
    VOLUMEDOWN = 129,
/* not sure whether there's a reason to enable these */
/*     LOCKINGCAPSLOCK = 130,  */
/*     LOCKINGNUMLOCK = 131, */
/*     LOCKINGSCROLLLOCK = 132, */
    KP_COMMA = 133,
    KP_EQUALSAS400 = 134,

    INTERNATIONAL1 = 135, /**< used on Asian keyboards, see
                                            footnotes in USB doc */
    INTERNATIONAL2 = 136,
    INTERNATIONAL3 = 137, /**< Yen */
    INTERNATIONAL4 = 138,
    INTERNATIONAL5 = 139,
    INTERNATIONAL6 = 140,
    INTERNATIONAL7 = 141,
    INTERNATIONAL8 = 142,
    INTERNATIONAL9 = 143,
    LANG1 = 144, /**< Hangul/English toggle */
    LANG2 = 145, /**< Hanja conversion */
    LANG3 = 146, /**< Katakana */
    LANG4 = 147, /**< Hiragana */
    LANG5 = 148, /**< Zenkaku/Hankaku */
    LANG6 = 149, /**< reserved */
    LANG7 = 150, /**< reserved */
    LANG8 = 151, /**< reserved */
    LANG9 = 152, /**< reserved */

    ALTERASE = 153, /**< Erase-Eaze */
    SYSREQ = 154,
    CANCEL = 155,
    CLEAR = 156,
    PRIOR = 157,
    RETURN2 = 158,
    SEPARATOR = 159,
    OUT = 160,
    OPER = 161,
    CLEARAGAIN = 162,
    CRSEL = 163,
    EXSEL = 164,

    KP_00 = 176,
    KP_000 = 177,
    THOUSANDSSEPARATOR = 178,
    DECIMALSEPARATOR = 179,
    CURRENCYUNIT = 180,
    CURRENCYSUBUNIT = 181,
    KP_LEFTPAREN = 182,
    KP_RIGHTPAREN = 183,
    KP_LEFTBRACE = 184,
    KP_RIGHTBRACE = 185,
    KP_TAB = 186,
    KP_BACKSPACE = 187,
    KP_A = 188,
    KP_B = 189,
    KP_C = 190,
    KP_D = 191,
    KP_E = 192,
    KP_F = 193,
    KP_XOR = 194,
    KP_POWER = 195,
    KP_PERCENT = 196,
    KP_LESS = 197,
    KP_GREATER = 198,
    KP_AMPERSAND = 199,
    KP_DBLAMPERSAND = 200,
    KP_VERTICALBAR = 201,
    KP_DBLVERTICALBAR = 202,
    KP_COLON = 203,
    KP_HASH = 204,
    KP_SPACE = 205,
    KP_AT = 206,
    KP_EXCLAM = 207,
    KP_MEMSTORE = 208,
    KP_MEMRECALL = 209,
    KP_MEMCLEAR = 210,
    KP_MEMADD = 211,
    KP_MEMSUBTRACT = 212,
    KP_MEMMULTIPLY = 213,
    KP_MEMDIVIDE = 214,
    KP_PLUSMINUS = 215,
    KP_CLEAR = 216,
    KP_CLEARENTRY = 217,
    KP_BINARY = 218,
    KP_OCTAL = 219,
    KP_DECIMAL = 220,
    KP_HEXADECIMAL = 221,

    LCTRL = 224,
    LSHIFT = 225,
    LALT = 226, /**< alt, option */
    LGUI = 227, /**< windows, command (apple), meta */
    RCTRL = 228,
    RSHIFT = 229,
    RALT = 230, /**< alt gr, option */
    RGUI = 231, /**< windows, command (apple), meta */

    MODE = 257,    /**< I'm not sure if this is really not covered
                                 *   by any of the above, but since there's a
                                 *   special KMOD_MODE for it I'm adding it here
                                 */

    /*@}*//*Usage page 0x07*/

    /**
     *  \name Usage page 0x0C
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /*@{*/

    AUDIONEXT = 258,
    AUDIOPREV = 259,
    AUDIOSTOP = 260,
    AUDIOPLAY = 261,
    AUDIOMUTE = 262,
    MEDIASELECT = 263,
    WWW = 264,
    MAIL = 265,
    CALCULATOR = 266,
    COMPUTER = 267,
    AC_SEARCH = 268,
    AC_HOME = 269,
    AC_BACK = 270,
    AC_FORWARD = 271,
    AC_STOP = 272,
    AC_REFRESH = 273,
    AC_BOOKMARKS = 274,

    /*@}*//*Usage page 0x0C*/

    /**
     *  \name Walther keys
     *
     *  These are values that Christian Walther added (for mac keyboard?).
     */
    /*@{*/

    BRIGHTNESSDOWN = 275,
    BRIGHTNESSUP = 276,
    DISPLAYSWITCH = 277, /**< display mirroring/dual display
                                           switch, video mode switch */
    KBDILLUMTOGGLE = 278,
    KBDILLUMDOWN = 279,
    KBDILLUMUP = 280,
    EJECT = 281,
    SLEEP = 282,

    APP1 = 283,
    APP2 = 284,

    /*@}*//*Walther keys*/

    /* Add any other keys here. */

    NUM_SCANCODES = 512 /**< not a key, just marks the number of scancodes
                                 for array bounds */
};

constexpr uint32_t scanCodeMask (1<<30);
constexpr uint32_t scanCodeToKeyCode(ScanCode x) {
    return static_cast<uint32_t>(x) | scanCodeMask;
}

enum class KeyCode : uint32_t
{
    UNKNOWN = 0,

    RETURN = '\r',
    ESCAPE = '\033',
    BACKSPACE = '\b',
    TAB = '\t',
    SPACE = ' ',
    EXCLAIM = '!',
    QUOTEDBL = '"',
    HASH = '#',
    PERCENT = '%',
    DOLLAR = '$',
    AMPERSAND = '&',
    QUOTE = '\'',
    LEFTPAREN = '(',
    RIGHTPAREN = ')',
    ASTERISK = '*',
    PLUS = '+',
    COMMA = ',',
    MINUS = '-',
    PERIOD = '.',
    SLASH = '/',
    N_0 = '0',
    N_1 = '1',
    N_2 = '2',
    N_3 = '3',
    N_4 = '4',
    N_5 = '5',
    N_6 = '6',
    N_7 = '7',
    N_8 = '8',
    N_9 = '9',
    COLON = ':',
    SEMICOLON = ';',
    LESS = '<',
    EQUALS = '=',
    GREATER = '>',
    QUESTION = '?',
    AT = '@',
    /* 
       Skip uppercase letters
     */
    LEFTBRACKET = '[',
    BACKSLASH = '\\',
    RIGHTBRACKET = ']',
    CARET = '^',
    UNDERSCORE = '_',
    BACKQUOTE = '`',
    a = 'a',
    b = 'b',
    c = 'c',
    d = 'd',
    e = 'e',
    f = 'f',
    g = 'g',
    h = 'h',
    i = 'i',
    j = 'j',
    k = 'k',
    l = 'l',
    m = 'm',
    n = 'n',
    o = 'o',
    p = 'p',
    q = 'q',
    r = 'r',
    s = 's',
    t = 't',
    u = 'u',
    v = 'v',
    w = 'w',
    x = 'x',
    y = 'y',
    z = 'z',

    CAPSLOCK = scanCodeToKeyCode(gu2::ScanCode::CAPSLOCK),

    F1 = scanCodeToKeyCode(gu2::ScanCode::F1),
    F2 = scanCodeToKeyCode(gu2::ScanCode::F2),
    F3 = scanCodeToKeyCode(gu2::ScanCode::F3),
    F4 = scanCodeToKeyCode(gu2::ScanCode::F4),
    F5 = scanCodeToKeyCode(gu2::ScanCode::F5),
    F6 = scanCodeToKeyCode(gu2::ScanCode::F6),
    F7 = scanCodeToKeyCode(gu2::ScanCode::F7),
    F8 = scanCodeToKeyCode(gu2::ScanCode::F8),
    F9 = scanCodeToKeyCode(gu2::ScanCode::F9),
    F10 = scanCodeToKeyCode(gu2::ScanCode::F10),
    F11 = scanCodeToKeyCode(gu2::ScanCode::F11),
    F12 = scanCodeToKeyCode(gu2::ScanCode::F12),

    PRINTSCREEN = scanCodeToKeyCode(gu2::ScanCode::PRINTSCREEN),
    SCROLLLOCK = scanCodeToKeyCode(gu2::ScanCode::SCROLLLOCK),
    PAUSE = scanCodeToKeyCode(gu2::ScanCode::PAUSE),
    INSERT = scanCodeToKeyCode(gu2::ScanCode::INSERT),
    HOME = scanCodeToKeyCode(gu2::ScanCode::HOME),
    PAGEUP = scanCodeToKeyCode(gu2::ScanCode::PAGEUP),
    DELETE = '\177',
    END = scanCodeToKeyCode(gu2::ScanCode::END),
    PAGEDOWN = scanCodeToKeyCode(gu2::ScanCode::PAGEDOWN),
    RIGHT = scanCodeToKeyCode(gu2::ScanCode::RIGHT),
    LEFT = scanCodeToKeyCode(gu2::ScanCode::LEFT),
    DOWN = scanCodeToKeyCode(gu2::ScanCode::DOWN),
    UP = scanCodeToKeyCode(gu2::ScanCode::UP),

    NUMLOCKCLEAR = scanCodeToKeyCode(gu2::ScanCode::NUMLOCKCLEAR),
    KP_DIVIDE = scanCodeToKeyCode(gu2::ScanCode::KP_DIVIDE),
    KP_MULTIPLY = scanCodeToKeyCode(gu2::ScanCode::KP_MULTIPLY),
    KP_MINUS = scanCodeToKeyCode(gu2::ScanCode::KP_MINUS),
    KP_PLUS = scanCodeToKeyCode(gu2::ScanCode::KP_PLUS),
    KP_ENTER = scanCodeToKeyCode(gu2::ScanCode::KP_ENTER),
    KP_1 = scanCodeToKeyCode(gu2::ScanCode::KP_1),
    KP_2 = scanCodeToKeyCode(gu2::ScanCode::KP_2),
    KP_3 = scanCodeToKeyCode(gu2::ScanCode::KP_3),
    KP_4 = scanCodeToKeyCode(gu2::ScanCode::KP_4),
    KP_5 = scanCodeToKeyCode(gu2::ScanCode::KP_5),
    KP_6 = scanCodeToKeyCode(gu2::ScanCode::KP_6),
    KP_7 = scanCodeToKeyCode(gu2::ScanCode::KP_7),
    KP_8 = scanCodeToKeyCode(gu2::ScanCode::KP_8),
    KP_9 = scanCodeToKeyCode(gu2::ScanCode::KP_9),
    KP_0 = scanCodeToKeyCode(gu2::ScanCode::KP_0),
    KP_PERIOD = scanCodeToKeyCode(gu2::ScanCode::KP_PERIOD),

    APPLICATION = scanCodeToKeyCode(gu2::ScanCode::APPLICATION),
    POWER = scanCodeToKeyCode(gu2::ScanCode::POWER),
    KP_EQUALS = scanCodeToKeyCode(gu2::ScanCode::KP_EQUALS),
    F13 = scanCodeToKeyCode(gu2::ScanCode::F13),
    F14 = scanCodeToKeyCode(gu2::ScanCode::F14),
    F15 = scanCodeToKeyCode(gu2::ScanCode::F15),
    F16 = scanCodeToKeyCode(gu2::ScanCode::F16),
    F17 = scanCodeToKeyCode(gu2::ScanCode::F17),
    F18 = scanCodeToKeyCode(gu2::ScanCode::F18),
    F19 = scanCodeToKeyCode(gu2::ScanCode::F19),
    F20 = scanCodeToKeyCode(gu2::ScanCode::F20),
    F21 = scanCodeToKeyCode(gu2::ScanCode::F21),
    F22 = scanCodeToKeyCode(gu2::ScanCode::F22),
    F23 = scanCodeToKeyCode(gu2::ScanCode::F23),
    F24 = scanCodeToKeyCode(gu2::ScanCode::F24),
    EXECUTE = scanCodeToKeyCode(gu2::ScanCode::EXECUTE),
    HELP = scanCodeToKeyCode(gu2::ScanCode::HELP),
    MENU = scanCodeToKeyCode(gu2::ScanCode::MENU),
    SELECT = scanCodeToKeyCode(gu2::ScanCode::SELECT),
    STOP = scanCodeToKeyCode(gu2::ScanCode::STOP),
    AGAIN = scanCodeToKeyCode(gu2::ScanCode::AGAIN),
    UNDO = scanCodeToKeyCode(gu2::ScanCode::UNDO),
    CUT = scanCodeToKeyCode(gu2::ScanCode::CUT),
    COPY = scanCodeToKeyCode(gu2::ScanCode::COPY),
    PASTE = scanCodeToKeyCode(gu2::ScanCode::PASTE),
    FIND = scanCodeToKeyCode(gu2::ScanCode::FIND),
    MUTE = scanCodeToKeyCode(gu2::ScanCode::MUTE),
    VOLUMEUP = scanCodeToKeyCode(gu2::ScanCode::VOLUMEUP),
    VOLUMEDOWN = scanCodeToKeyCode(gu2::ScanCode::VOLUMEDOWN),
    KP_COMMA = scanCodeToKeyCode(gu2::ScanCode::KP_COMMA),
    KP_EQUALSAS400 = scanCodeToKeyCode(gu2::ScanCode::KP_EQUALSAS400),

    ALTERASE = scanCodeToKeyCode(gu2::ScanCode::ALTERASE),
    SYSREQ = scanCodeToKeyCode(gu2::ScanCode::SYSREQ),
    CANCEL = scanCodeToKeyCode(gu2::ScanCode::CANCEL),
    CLEAR = scanCodeToKeyCode(gu2::ScanCode::CLEAR),
    PRIOR = scanCodeToKeyCode(gu2::ScanCode::PRIOR),
    RETURN2 = scanCodeToKeyCode(gu2::ScanCode::RETURN2),
    SEPARATOR = scanCodeToKeyCode(gu2::ScanCode::SEPARATOR),
    OUT = scanCodeToKeyCode(gu2::ScanCode::OUT),
    OPER = scanCodeToKeyCode(gu2::ScanCode::OPER),
    CLEARAGAIN = scanCodeToKeyCode(gu2::ScanCode::CLEARAGAIN),
    CRSEL = scanCodeToKeyCode(gu2::ScanCode::CRSEL),
    EXSEL = scanCodeToKeyCode(gu2::ScanCode::EXSEL),

    KP_00 = scanCodeToKeyCode(gu2::ScanCode::KP_00),
    KP_000 = scanCodeToKeyCode(gu2::ScanCode::KP_000),
    THOUSANDSSEPARATOR = scanCodeToKeyCode(gu2::ScanCode::THOUSANDSSEPARATOR),
    DECIMALSEPARATOR = scanCodeToKeyCode(gu2::ScanCode::DECIMALSEPARATOR),
    CURRENCYUNIT = scanCodeToKeyCode(gu2::ScanCode::CURRENCYUNIT),
    CURRENCYSUBUNIT = scanCodeToKeyCode(gu2::ScanCode::CURRENCYSUBUNIT),
    KP_LEFTPAREN = scanCodeToKeyCode(gu2::ScanCode::KP_LEFTPAREN),
    KP_RIGHTPAREN = scanCodeToKeyCode(gu2::ScanCode::KP_RIGHTPAREN),
    KP_LEFTBRACE = scanCodeToKeyCode(gu2::ScanCode::KP_LEFTBRACE),
    KP_RIGHTBRACE = scanCodeToKeyCode(gu2::ScanCode::KP_RIGHTBRACE),
    KP_TAB = scanCodeToKeyCode(gu2::ScanCode::KP_TAB),
    KP_BACKSPACE = scanCodeToKeyCode(gu2::ScanCode::KP_BACKSPACE),
    KP_A = scanCodeToKeyCode(gu2::ScanCode::KP_A),
    KP_B = scanCodeToKeyCode(gu2::ScanCode::KP_B),
    KP_C = scanCodeToKeyCode(gu2::ScanCode::KP_C),
    KP_D = scanCodeToKeyCode(gu2::ScanCode::KP_D),
    KP_E = scanCodeToKeyCode(gu2::ScanCode::KP_E),
    KP_F = scanCodeToKeyCode(gu2::ScanCode::KP_F),
    KP_XOR = scanCodeToKeyCode(gu2::ScanCode::KP_XOR),
    KP_POWER = scanCodeToKeyCode(gu2::ScanCode::KP_POWER),
    KP_PERCENT = scanCodeToKeyCode(gu2::ScanCode::KP_PERCENT),
    KP_LESS = scanCodeToKeyCode(gu2::ScanCode::KP_LESS),
    KP_GREATER = scanCodeToKeyCode(gu2::ScanCode::KP_GREATER),
    KP_AMPERSAND = scanCodeToKeyCode(gu2::ScanCode::KP_AMPERSAND),
    KP_DBLAMPERSAND = scanCodeToKeyCode(gu2::ScanCode::KP_DBLAMPERSAND),
    KP_VERTICALBAR = scanCodeToKeyCode(gu2::ScanCode::KP_VERTICALBAR),
    KP_DBLVERTICALBAR = scanCodeToKeyCode(gu2::ScanCode::KP_DBLVERTICALBAR),
    KP_COLON = scanCodeToKeyCode(gu2::ScanCode::KP_COLON),
    KP_HASH = scanCodeToKeyCode(gu2::ScanCode::KP_HASH),
    KP_SPACE = scanCodeToKeyCode(gu2::ScanCode::KP_SPACE),
    KP_AT = scanCodeToKeyCode(gu2::ScanCode::KP_AT),
    KP_EXCLAM = scanCodeToKeyCode(gu2::ScanCode::KP_EXCLAM),
    KP_MEMSTORE = scanCodeToKeyCode(gu2::ScanCode::KP_MEMSTORE),
    KP_MEMRECALL = scanCodeToKeyCode(gu2::ScanCode::KP_MEMRECALL),
    KP_MEMCLEAR = scanCodeToKeyCode(gu2::ScanCode::KP_MEMCLEAR),
    KP_MEMADD = scanCodeToKeyCode(gu2::ScanCode::KP_MEMADD),
    KP_MEMSUBTRACT = scanCodeToKeyCode(gu2::ScanCode::KP_MEMSUBTRACT),
    KP_MEMMULTIPLY = scanCodeToKeyCode(gu2::ScanCode::KP_MEMMULTIPLY),
    KP_MEMDIVIDE = scanCodeToKeyCode(gu2::ScanCode::KP_MEMDIVIDE),
    KP_PLUSMINUS = scanCodeToKeyCode(gu2::ScanCode::KP_PLUSMINUS),
    KP_CLEAR = scanCodeToKeyCode(gu2::ScanCode::KP_CLEAR),
    KP_CLEARENTRY = scanCodeToKeyCode(gu2::ScanCode::KP_CLEARENTRY),
    KP_BINARY = scanCodeToKeyCode(gu2::ScanCode::KP_BINARY),
    KP_OCTAL = scanCodeToKeyCode(gu2::ScanCode::KP_OCTAL),
    KP_DECIMAL = scanCodeToKeyCode(gu2::ScanCode::KP_DECIMAL),
    KP_HEXADECIMAL = scanCodeToKeyCode(gu2::ScanCode::KP_HEXADECIMAL),

    LCTRL = scanCodeToKeyCode(gu2::ScanCode::LCTRL),
    LSHIFT = scanCodeToKeyCode(gu2::ScanCode::LSHIFT),
    LALT = scanCodeToKeyCode(gu2::ScanCode::LALT),
    LGUI = scanCodeToKeyCode(gu2::ScanCode::LGUI),
    RCTRL = scanCodeToKeyCode(gu2::ScanCode::RCTRL),
    RSHIFT = scanCodeToKeyCode(gu2::ScanCode::RSHIFT),
    RALT = scanCodeToKeyCode(gu2::ScanCode::RALT),
    RGUI = scanCodeToKeyCode(gu2::ScanCode::RGUI),

    MODE = scanCodeToKeyCode(gu2::ScanCode::MODE),

    AUDIONEXT = scanCodeToKeyCode(gu2::ScanCode::AUDIONEXT),
    AUDIOPREV = scanCodeToKeyCode(gu2::ScanCode::AUDIOPREV),
    AUDIOSTOP = scanCodeToKeyCode(gu2::ScanCode::AUDIOSTOP),
    AUDIOPLAY = scanCodeToKeyCode(gu2::ScanCode::AUDIOPLAY),
    AUDIOMUTE = scanCodeToKeyCode(gu2::ScanCode::AUDIOMUTE),
    MEDIASELECT = scanCodeToKeyCode(gu2::ScanCode::MEDIASELECT),
    WWW = scanCodeToKeyCode(gu2::ScanCode::WWW),
    MAIL = scanCodeToKeyCode(gu2::ScanCode::MAIL),
    CALCULATOR = scanCodeToKeyCode(gu2::ScanCode::CALCULATOR),
    COMPUTER = scanCodeToKeyCode(gu2::ScanCode::COMPUTER),
    AC_SEARCH = scanCodeToKeyCode(gu2::ScanCode::AC_SEARCH),
    AC_HOME = scanCodeToKeyCode(gu2::ScanCode::AC_HOME),
    AC_BACK = scanCodeToKeyCode(gu2::ScanCode::AC_BACK),
    AC_FORWARD = scanCodeToKeyCode(gu2::ScanCode::AC_FORWARD),
    AC_STOP = scanCodeToKeyCode(gu2::ScanCode::AC_STOP),
    AC_REFRESH = scanCodeToKeyCode(gu2::ScanCode::AC_REFRESH),
    AC_BOOKMARKS = scanCodeToKeyCode(gu2::ScanCode::AC_BOOKMARKS),

    BRIGHTNESSDOWN = scanCodeToKeyCode(gu2::ScanCode::BRIGHTNESSDOWN),
    BRIGHTNESSUP = scanCodeToKeyCode(gu2::ScanCode::BRIGHTNESSUP),
    DISPLAYSWITCH = scanCodeToKeyCode(gu2::ScanCode::DISPLAYSWITCH),
    KBDILLUMTOGGLE = scanCodeToKeyCode(gu2::ScanCode::KBDILLUMTOGGLE),
    KBDILLUMDOWN = scanCodeToKeyCode(gu2::ScanCode::KBDILLUMDOWN),
    KBDILLUMUP = scanCodeToKeyCode(gu2::ScanCode::KBDILLUMUP),
    EJECT = scanCodeToKeyCode(gu2::ScanCode::EJECT),
    SLEEP = scanCodeToKeyCode(gu2::ScanCode::SLEEP)
};



} // namespace gu2
