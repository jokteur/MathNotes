#pragma once

#include "span.h"
#include "ui/fonts.h"
#include "ui/colors.h"

namespace RichText {
    struct EmSpan: public AbstractSpan {
        EmSpan(): AbstractSpan() {
            m_type = T_SPAN_EM;
        }
    };
    struct StrongSpan: public AbstractSpan {
        StrongSpan(): AbstractSpan() {
            m_type = T_SPAN_STRONG;
        }
    };
    struct HighlightSpan: public AbstractSpan {
        HighlightSpan(): AbstractSpan() {
            m_type = T_SPAN_HIGHLIGHT;
        }
    };
    struct DelSpan: public AbstractSpan {
        DelSpan(): AbstractSpan() {
            m_type = T_SPAN_DEL;
        }
    };
}
