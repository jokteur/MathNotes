#pragma once 
#include "../wrapper.h"

namespace RichText {
    struct DrawableChar : public WrapCharacter {
        void virtual draw(ImDrawList* draw_list, ImVec2 draw_offset = ImVec2(0.f, 0.f)) = 0;
    };

    struct NewLine : DrawableChar {
        NewLine() {
            is_linebreak = true;
        }
        void draw(ImDrawList* draw_list, ImVec2 draw_offset = ImVec2(0.f, 0.f)) override {}
    };

    using DrawableCharPtr = std::shared_ptr<DrawableChar>;
}