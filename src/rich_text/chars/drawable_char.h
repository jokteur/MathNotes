#pragma once 
#include "../wrapper.h"

namespace RichText {
    struct DrawableChar : public WrapCharacter {
        void virtual draw(ImDrawList* draw_list) = 0;
    };

    struct NewLine : DrawableChar {
        NewLine() {
            is_linebreak = true;
        }
        void draw(ImDrawList* draw_list) override {}
    };

    using DrawableCharPtr = std::shared_ptr<DrawableChar>;
}