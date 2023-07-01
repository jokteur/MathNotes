#pragma once

#include "span.h"
#include "rich_text/chars/latex_char.h"

namespace RichText {
    struct LatexSpan : public AbstractSpan {
        std::string m_error;
        LatexCharPtr m_latex_char = nullptr;
        WrapString m_error_string;

        LatexSpan() : AbstractSpan() {
            m_category = C_SPAN;
            m_type = T_SPAN_LATEX;
        }

        bool build_latex_image();

        bool add_chars(WrapColumn* wrap_string) override;
        // bool hk_draw_main(DrawContext* context) override;
        // bool hk_build_chars(DrawContext* ctx) override;

        void  hk_debug_attributes() override;
    };
}
