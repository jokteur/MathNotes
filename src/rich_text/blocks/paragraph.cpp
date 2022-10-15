#include "paragraph.h"

namespace RichText {
    ParagraphWidget::ParagraphWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        m_type = T_BLOCK_P;
    }
    void ParagraphWidget::draw(ImDrawList* draw_list) {
        if (m_widget_dirty) {
            m_wrap_chars.clear();
            bool success = true;
            for (auto ptr : m_childrens) {
                // The first block should break the loop
                if (ptr->m_category != C_SPAN) {
                    break;
                }
                if (!ptr->buildAndAddChars(m_draw_chars, m_wrap_chars)) {
                    success = false;
                    break;
                }
            }
            if (success) {
                m_widget_dirty = false;
                m_wrapper.setString(m_wrap_chars);
            }
        }
        for (auto ptr : m_childrens) {
            ptr->draw(draw_list);
        }
    }
    CodeWidget::CodeWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        m_type = T_BLOCK_CODE;
    }
    QuoteWidget::QuoteWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        m_type = T_BLOCK_QUOTE;
    }
}