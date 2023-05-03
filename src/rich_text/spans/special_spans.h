#pragma once

#include "span.h"
#include "fonts/fonts.h"
#include "ui/colors.h"

namespace RichText {
    struct LinkSpan: public AbstractSpan {
        LinkSpan(): AbstractSpan() {
            m_type = T_SPAN_A;
        }
    };
    struct CodeSpan: public AbstractSpan {
        CodeSpan(): AbstractSpan() {
            m_type = T_SPAN_A;
        }
    };
}
