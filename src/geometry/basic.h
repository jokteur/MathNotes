#pragma once

#include <tempo.h>
#include <string>

struct emfloat {
    float f;

    const emfloat operator*(int rhs) const;

    const emfloat operator+(const emfloat& rhs) const;
    const emfloat operator-(const emfloat& rhs) const;
    const emfloat operator*(const emfloat& rhs) const;
    const emfloat operator/(const emfloat& rhs) const;
    bool operator<(const emfloat& rhs) const;
    bool operator<=(const emfloat& rhs) const;
    bool operator>(const emfloat& rhs) const;
    bool operator>=(const emfloat& rhs) const;
    bool operator==(const emfloat& rhs) const;
    bool operator!=(const emfloat& rhs) const;

    const float getFloat();
};


struct EmVec2 {
    emfloat x;
    emfloat y;

    EmVec2(emfloat x, emfloat y);
    /**
     * @brief Create a new dpi independent vector
     *
     * @param x will be converted with the current scaling (x in real space)
     * @param y will be converted with the current scaling (y in real space)
     */
    EmVec2(float x, float y);
    /**
     * @brief Create a new dpi independent vector
     *
     * @param vec in real space
     */
    EmVec2(const ImVec2& vec);


    const EmVec2 operator+(const EmVec2& rhs) const;
    const EmVec2 operator-(const EmVec2& rhs) const;

    const EmVec2 operator+(const emfloat& rhs) const;
    const EmVec2 operator-(const emfloat& rhs) const;
    const EmVec2 operator*(const emfloat& rhs) const;
    const EmVec2 operator/(const emfloat& rhs) const;

    const ImVec2 getImVec2(const EmVec2& vec);
};

template <typename T>
EmVec2 operator*(T scalar, EmVec2 const& vec) {
    return vec * scalar;
}


const emfloat toEmFloat(const float value);

struct EmRect {
    emfloat x = emfloat{ 0.f };
    emfloat y = emfloat{ 0.f };
    emfloat w = emfloat{ 0.f };
    emfloat h = emfloat{ 0.f };

    const EmVec2 getPos() const {
        return EmVec2{ x, y };
    }
    const EmVec2 getDim() const {
        return EmVec2{ w, h };
    }
};

struct Rect {
    float x = 0.f;
    float y = 0.f;
    float w = 0.f;
    float h = 0.f;

    const ImVec2 getPos() const {
        return ImVec2(x, y);
    }
    const ImVec2 getDim() const {
        return ImVec2(w, h);
    }
};

inline bool isInsideRect(const EmVec2& pos, const EmRect& rect) {
    return pos.x >= rect.x && pos.x <= rect.x + rect.w
        && pos.y >= rect.y && pos.y <= rect.y + rect.h;
}
inline bool isInsideRectX(const EmVec2& pos, const EmRect& rect) {
    return pos.x >= rect.x && pos.x <= rect.x + rect.w;
}
inline bool isInsideRectY(const emfloat pos_y, const EmRect& rect) {
    return pos_y >= rect.y && pos_y <= rect.y + rect.h;
}

inline bool isInsideRect(const ImVec2& pos, const Rect& rect) {
    return pos.x >= rect.x && pos.x <= rect.x + rect.w
        && pos.y >= rect.y && pos.y <= rect.y + rect.h;
}
inline bool isInsideRectX(const ImVec2& pos, const Rect& rect) {
    return pos.x >= rect.x && pos.x <= rect.x + rect.w;
}
inline bool isInsideRectY(const float pos_y, const Rect& rect) {
    return pos_y >= rect.y && pos_y <= rect.y + rect.h;
}