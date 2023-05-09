#pragma once

#include "span.h"
#include "fonts/fonts.h"
#include "ui/colors.h"

namespace RichText {
    struct TextString : public AbstractSpan {
        TextString();

        bool add_chars(WrapParagraph* wrap_string) override;

        inline float hk_set_position(float& cursor_y_pos, MultiOffset& x_offset) override;
        inline void hk_set_dimensions(float last_y_pos, float& cursor_y_pos, const MultiOffset& x_offset) override;
        inline bool hk_draw_main(DrawContext* context) override;

        void hk_debug(const std::string& prefix = "") override;
    };
}
