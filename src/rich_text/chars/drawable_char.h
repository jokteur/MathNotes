#pragma once 
#include "../wrapper.h"

namespace RichText {
    struct DrawableChar : public WrapCharacter {
        void virtual draw(ImDrawList* draw_list) = 0;
    };

    using DrawableCharPtr = std::shared_ptr<DrawableChar>;
}