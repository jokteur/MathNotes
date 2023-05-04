#pragma once

#include "span.h"
#include "rich_text/chars/latex_char.h"

namespace RichText {
    struct LatexWidget : public AbstractSpan {
        std::string m_error;
        LatexWidget() : AbstractSpan() {
            m_category = C_SPAN;
            m_type = T_SPAN_LATEX;
        }

        bool add_chars(WrapString* wrap_string) override;
        // bool hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) override;
        // bool hk_build_widget(float x_offset);

        void  hk_debug_attributes() override;
    };
}
