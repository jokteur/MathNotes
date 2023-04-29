#include "basic.h"

const emfloat emfloat::operator*(int rhs) const {
    return emfloat{ f * (float)rhs };
}
const emfloat emfloat::operator+(const emfloat& rhs) const {
    return emfloat{ f + rhs.f };
}
const emfloat emfloat::operator-(const emfloat& rhs) const {
    return emfloat{ f - rhs.f };
}
const emfloat emfloat::operator*(const emfloat& rhs) const {
    return emfloat{ f * rhs.f };
}
const emfloat emfloat::operator/(const emfloat& rhs) const {
    return emfloat{ f / rhs.f };
}
bool emfloat::operator<(const emfloat& rhs) const {
    return f < rhs.f;
}
bool emfloat::operator<=(const emfloat& rhs) const {
    return f <= rhs.f;
}
bool emfloat::operator>(const emfloat& rhs) const {
    return f > rhs.f;
}
bool emfloat::operator>=(const emfloat& rhs) const {
    return f >= rhs.f;
}
bool emfloat::operator==(const emfloat& rhs) const {
    return f == rhs.f;
}
bool emfloat::operator!=(const emfloat& rhs) const {
    return f != rhs.f;
}

const float emfloat::getFloat() {
    return f * Tempo::GetScaling();
}

EmVec2::EmVec2(emfloat xx, emfloat yy) {
    x = xx; y = yy;
}
EmVec2::EmVec2(float xx, float yy) {
    const float scaling = Tempo::GetScaling();
    x = emfloat{ xx / scaling };
    y = emfloat{ yy / scaling };
}
EmVec2::EmVec2(const ImVec2& vec) {
    const float scaling = Tempo::GetScaling();
    x = emfloat{ vec.x / scaling };
    y = emfloat{ vec.y / scaling };
}
const EmVec2 EmVec2::operator+(const EmVec2& rhs) const {
    return EmVec2{ x + rhs.x, y + rhs.y };
}
const EmVec2 EmVec2::operator-(const EmVec2& rhs) const {
    return EmVec2{ x - rhs.x, y - rhs.y };
}
const EmVec2 EmVec2::operator+(const emfloat& rhs) const {
    return EmVec2{ x + rhs, y + rhs };
}
const EmVec2 EmVec2::operator-(const emfloat& rhs) const {
    return EmVec2{ x - rhs, y - rhs };
}
const EmVec2 EmVec2::operator*(const emfloat& rhs) const {
    return EmVec2{ x * rhs, y * rhs };
}
const EmVec2 EmVec2::operator/(const emfloat& rhs) const {
    return EmVec2{ x / rhs, y / rhs };
}

const ImVec2 EmVec2::getImVec2(const EmVec2& vec) {
    return ImVec2(x.getFloat(), y.getFloat());
}