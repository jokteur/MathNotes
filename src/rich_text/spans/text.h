#pragma once

#include "span.h"
#include "fonts/fonts.h"
#include "ui/colors.h"

namespace RichText {
    struct TextString: public AbstractSpan {
        TextString();

        bool add_chars(WrapString* wrap_string) override;

        inline float hk_set_position(float& cursor_y_pos, float& x_offset) override;
        inline void hk_set_dimensions(float last_y_pos, float& cursor_y_pos, float x_offset) override;
        inline bool hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) override;

        void hk_debug(const std::string& prefix = "") override;
    };
}
