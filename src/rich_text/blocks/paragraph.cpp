#include "paragraph.h"

namespace RichText {
    ParagraphWidget::ParagraphWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        m_type = T_BLOCK_P;
    }

    CodeWidget::CodeWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        m_type = T_BLOCK_CODE;
    }
    QuoteWidget::QuoteWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        m_type = T_BLOCK_QUOTE;
    }
}