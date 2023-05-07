#include "drawable_char.h"

namespace RichText {
    bool NewLine::draw(Draw::DrawList*, const Rect&, ImVec2) {
        return true;
    }
}