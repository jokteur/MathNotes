#include "quote.h"
#include "rich_text/chars/im_char.h"

namespace RichText {
    QuoteWidget::QuoteWidget(UIState_ptr ui_state): AbstractBlock(ui_state) {
        m_type = T_BLOCK_QUOTE;
    }
    void QuoteWidget::hk_build_widget(float x_offset) {
        if (m_widget_dirty && m_is_selected) {
            bool success = true;
            m_wrap_chars.clear();
            for (auto bounds : m_text_boundaries) {
                m_wrap_chars.clear();
                auto res = Utf8StrToImCharStr(m_ui_state, m_wrap_chars, m_draw_chars, m_safe_string, bounds.pre, bounds.beg, m_special_chars_style, true);

                m_wrapper.clear();
                float internal_size = m_window_width - x_offset - m_style.h_paddings.x - m_style.h_paddings.y;
                m_wrapper.setWidth(1000.f); // Large enough number
                auto last_char = m_wrap_chars.back();
                m_wrapper.setString(m_wrap_chars);

                if (res) {
                    success = false;
                }
            }
            if (success)
                m_widget_dirty = false;
        }
    }
}