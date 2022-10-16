#pragma once

#include <cstdint>

namespace Colors {
    using color = std::uint32_t;
    // Basic
    static constexpr color transparent = 0x00000000;
    static constexpr color black = 0xff000000;
    static constexpr color white = 0xffffffff;
    static constexpr color red = 0xff0000ff;
    static constexpr color green = 0xff00ff00;
    static constexpr color blue = 0xffff0000;
    static constexpr color yellow = 0xffffff00;
    static constexpr color cyan = 0xff00ffff;
    static constexpr color magenta = 0xffff00ff;

    // Gray
    static constexpr color gainsboro = 0xffdcdcdc;
    static constexpr color lightgray = 0xffd3d3d3;
    static constexpr color silver = 0xffc0c0c0;
    static constexpr color darkgray = 0xffa9a9a9;
    static constexpr color gray = 0xffa9a9a9;
    static constexpr color dimgray = 0xff696969;
    static constexpr color lightslategray = 0xff778899;
    static constexpr color slategray = 0xff708090;
    static constexpr color darkslategray = 0xff2f4f4f;
}