#include "latex.h"

#include "rich_text/chars/latex_char.h"

#include "profiling.h"

namespace RichText {
    /* =====
     * SPANS
     * ===== */
    bool LatexWidget::add_chars(std::vector<WrapCharPtr>& wrap_chars) {
        //ZoneScoped;
        bool success = true;
        m_draw_chars.clear();

        std::string source;
        AB::str_from_text_boundaries(*m_safe_string, source, m_text_boundaries);

        hk_add_pre_chars(wrap_chars);
        auto ch = ToLatexChar(source, m_style.font_size * m_style.scale * Tempo::GetScaling(),
            7.f,
            m_style.font_color,
            ImVec2(1.f, 1.f),
            ImVec2(5.f, 0.f),
            false);

        m_error = ch->m_latex_image->getLatexErrorMsg();

        wrap_chars.push_back(ch);
        m_draw_chars.push_back(ch);
        hk_add_post_chars(wrap_chars);
        return success;
    }

    void LatexWidget::hk_debug_attributes() {
        AbstractSpan::hk_debug_attributes();
        if (!m_error.empty()) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(Colors::red), "Error: %s", m_error.c_str());
        }
    }
}