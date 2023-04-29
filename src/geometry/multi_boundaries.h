#pragma once
#include "basic.h"

#include <vector>

class MultiBoundaries {
private:
    std::vector<Rect> m_bounds;
public:
    void addBoundary(const Rect& rect);

    const std::vector<Rect>& getBounds();

    void merge(const MultiBoundaries& bounds);
    void exclude(const MultiBoundaries& bounds);

    Rect getGlobalRect() const;
    ImVec2 getGlobalPosition() const;
    ImVec2 getGlobalDimensions() const;
};