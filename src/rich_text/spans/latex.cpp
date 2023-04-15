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
        m_char = ToLatexChar(source, m_style.font_size * m_style.scale * Tempo::GetScaling(),
            7.f,
            m_style.font_color,
            ImVec2(1.f, 1.f),
            ImVec2(5.f, 0.f),
            false);

        wrap_chars.push_back(m_char);
        m_draw_chars.push_back(m_char);
        hk_add_post_chars(wrap_chars);
        return success;
    }

    void LatexWidget::hk_debug_attributes() {
        AbstractSpan::hk_debug_attributes();
        std::string err = m_char->m_latex_image->getLatexErrorMsg();
        if (!err.empty()) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(Colors::red), ("Error: " + err).c_str());
        }
    }
}