#include "latex.h"

#include "rich_text/chars/latex_char.h"

namespace RichText {
    /* =====
     * SPANS
     * ===== */
    bool LatexWidget::add_chars(std::vector<WrapCharPtr>& wrap_chars) {
        bool success = true;
        m_draw_chars.clear();

        std::string source;
        AB::str_from_text_boundaries(*m_safe_string, source, m_text_boundaries);

        hk_add_pre_chars(wrap_chars);
        m_char = ToLatexChar(source, m_style.font_size * m_style.scale * Tempo::GetScaling(), m_style.font_color);
        wrap_chars.push_back(m_char);
        m_draw_chars.push_back(m_char);
        hk_add_post_chars(wrap_chars);
        return success;
    }
}