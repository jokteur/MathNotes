#pragma once

#include "../widgets.h"
#include "ui/fonts.h"
#include "ui/colors.h"

namespace RichText {
    struct TextString : public AbstractSpan {
        TextString(UIState_ptr ui_state);

        bool buildAndAddChars(std::vector<WrapCharPtr>& wrap_string) override;
        void draw(ImDrawList* draw_list, ImVec2& draw_offset) override;
    };
}
