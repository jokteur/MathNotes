#pragma once
#include "rect.h"
#include "basic.h"

#include <vector>

class MultiBoundaries {
private:
    std::vector<Rect> m_bounds;
public:
    void addBoundary(const Rect& rect);

    void setGlobalPosition(const EmVec2& pos);
    void setGlobalPosition(emfloat x, emfloat y);
    void setGlobalDimensions(const EmVec2& pos);
    void setGlobalPosition(emfloat w, emfloat h);

    Rect getGlobalRect() const;
    EmVec2 getGlobalPosition() const;
    EmVec2 getGlobalDimensions() const;
};