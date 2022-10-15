#pragma once

#include "../widgets.h"
#include "ui/fonts.h"
#include "ui/colors.h"

namespace RichText {
    struct TextString : public AbstractSpan {
        TextString(UIState_ptr ui_state);

        bool buildAndAddChars(std::vector<DrawableCharPtr>& draw_string, std::vector<WrapCharPtr>& wrap_string, int start = -1) override;
        void draw(ImDrawList* draw_list) override;
    };
}
