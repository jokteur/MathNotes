#pragma once

#include <tempo.h>
#include <string>

struct Rect {
    float x = 0.f;
    float y = 0.f;
    float w = 0.f;
    float h = 0.f;
};

typedef std::shared_ptr<std::string> SafeString;

inline bool isInsideRect(const ImVec2& pos, const Rect& rect) {
    return pos.x >= rect.x && pos.x <= rect.x + rect.w
        && pos.y >= rect.y && pos.y <= rect.y + rect.h;
}
inline bool isInsideRectX(const ImVec2& pos, const Rect& rect) {
    return pos.x >= rect.x && pos.x <= rect.x + rect.w;
}
inline bool isInsideRectY(const ImVec2& pos, const Rect& rect) {
    return pos.y >= rect.y && pos.y <= rect.y + rect.h;
}