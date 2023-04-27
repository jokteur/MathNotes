#pragma once

#include <tempo.h>
#include <string>

struct EmVec2 {
    float x;
    float y;

    const ImVec2 getImVec() const;
};

struct Rect {
    float x = 0.f;
    float y = 0.f;
    float w = 0.f;
    float h = 0.f;

    const EmVec2 getPos() const {
        return EmVec2(x, y);
    }
    const EmVec2 getDim() const {
        return EmVec2(w, h);
    }
};

inline bool isInsideRect(const EmVec2& pos, const Rect& rect) {
    return pos.x >= rect.x && pos.x <= rect.x + rect.w
        && pos.y >= rect.y && pos.y <= rect.y + rect.h;
}
inline bool isInsideRectX(const EmVec2& pos, const Rect& rect) {
    return pos.x >= rect.x && pos.x <= rect.x + rect.w;
}
inline bool isInsideRectY(const float pos_y, const Rect& rect) {
    return pos_y >= rect.y && pos_y <= rect.y + rect.h;
}