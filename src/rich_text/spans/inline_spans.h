#pragma once

#include "rich_text/widgets.h"
#include "ui/fonts.h"
#include "ui/colors.h"

namespace RichText {
    struct EmSpan: public AbstractSpan {
        EmSpan(UIState_ptr ui_state): AbstractSpan(ui_state) {
            m_type = T_SPAN_EM;
        }
    };
    struct StrongSpan: public AbstractSpan {
        StrongSpan(UIState_ptr ui_state): AbstractSpan(ui_state) {
            m_type = T_SPAN_STRONG;
        }
    };
    struct HighlightSpan: public AbstractSpan {
        HighlightSpan(UIState_ptr ui_state): AbstractSpan(ui_state) {
            m_type = T_SPAN_HIGHLIGHT;
        }
    };
    struct DelSpan: public AbstractSpan {
        DelSpan(UIState_ptr ui_state): AbstractSpan(ui_state) {
            m_type = T_SPAN_DEL;
        }
    };
}
