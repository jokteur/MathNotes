#pragma once

#include "../widgets.h"
#include "ui/fonts.h"
#include "ui/colors.h"

namespace RichText {
    struct TextString : public AbstractSpan {
        TextString(UIState_ptr ui_state);

        bool buildAndAddChars(std::vector<WrapCharPtr>& string, int start = -1) override;
        void draw(ImDrawList* draw_list) override;

        Fonts::FontRequestInfo font_styling;
        int color = Colors::BLACK; // TODO: Remove microtex dependence
    };
}
