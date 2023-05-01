#include "paragraph.h"

namespace RichText {
    ParagraphWidget::ParagraphWidget(): AbstractLeafBlock() {
        m_type = T_BLOCK_P;
    }

    // bool ParagraphWidget::hk_build_widget(float x_offset) {
    //     if (m_widget_dirty) {
    //         m_rt_info->line_to_char[m_text_boundaries.front().line_number] = m_wrap_chars.front();
    //     }
    //     return true;
    // }

    CodeWidget::CodeWidget(): AbstractLeafBlock() {
        m_type = T_BLOCK_CODE;
    }
}