#pragma once
#include "rect.h"

#include <vector>

class MultiBoundaries {
private:
    std::vector<Rect> m_bounds;
public:
    void addBoundary(const Rect& rect);


};