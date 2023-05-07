#pragma once

#include "span.h"
#include "rich_text/chars/latex_char.h"

namespace RichText {
    struct LatexWidget : public AbstractSpan {
        std::string m_error;
        LatexCharPtr m_latex_char = nullptr;
        WrapString m_error_string;

        LatexWidget() : AbstractSpan() {
            m_category = C_SPAN;
            m_type = T_SPAN_LATEX;
        }

        bool add_chars(WrapParagraph* wrap_string) override;
        bool hk_draw_main(DrawContext* context) override;
        bool hk_build_widget(DrawContext* ctx) override;

        void  hk_debug_attributes() override;
    };
}
