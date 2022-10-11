#pragma once

#include <cstdint>

namespace Colors {
    using color = std::uint32_t;
    static constexpr color transparent = 0x00000000;
    static constexpr color black = 0xff000000;
    static constexpr color white = 0xffffffff;
    static constexpr color red = 0xffff0000;
    static constexpr color green = 0xff00ff00;
    static constexpr color blue = 0xff0000ff;
    static constexpr color yellow = 0xffffff00;
    static constexpr color cyan = 0xff00ffff;
    static constexpr color magenta = 0xffff00ff;

    static constexpr color TRANSPARENT = transparent;
    static constexpr color BLACK = black;
    static constexpr color WHITE = white;
    static constexpr color RED = red;
    static constexpr color GREEN = green;
    static constexpr color BLUE = blue;
    static constexpr color YELLOW = yellow;
    static constexpr color CYAN = cyan;
    static constexpr color MAGENTA = magenta;
}