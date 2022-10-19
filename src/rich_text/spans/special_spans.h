#pragma once

#include "rich_text/widgets.h"
#include "ui/fonts.h"
#include "ui/colors.h"

namespace RichText {
    struct LinkSpan : public AbstractSpan {
        LinkSpan(UIState_ptr ui_state) : AbstractSpan(ui_state) {
            m_type = T_SPAN_A;
        }
    };
    struct CodeSpan : public AbstractSpan {
        CodeSpan(UIState_ptr ui_state) : AbstractSpan(ui_state) {
            m_type = T_SPAN_A;
        }
    };
}
