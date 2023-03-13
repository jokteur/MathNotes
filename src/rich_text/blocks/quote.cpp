#include "quote.h"

namespace RichText {
    QuoteWidget::QuoteWidget(UIState_ptr ui_state): AbstractBlock(ui_state) {
        m_type = T_BLOCK_QUOTE;
    }
    void QuoteWidget::hk_build_widget(float x_offset) {
        if (m_widget_dirty) {
            for (auto bounds : m_text_boundaries) {

            }
            for (auto ptr : m_childrens) {
                ptr->m_position;
            }

            m_widget_dirty = false;
        }
    }
}