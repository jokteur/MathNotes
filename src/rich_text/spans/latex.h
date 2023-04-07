#pragma once

#include "span.h"
#include "rich_text/chars/latex_char.h"

namespace RichText {
    struct LatexWidget: public AbstractSpan {
        LatexCharPtr m_char;
        LatexWidget(UIState_ptr ui_state): AbstractSpan(ui_state) {
            m_category = C_SPAN;
            m_type = T_SPAN_LATEX;
        }

        bool add_chars(std::vector<WrapCharPtr>& wrap_string) override;

        void  hk_debug_attributes() override;
    };
}
