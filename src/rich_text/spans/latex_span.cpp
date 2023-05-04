#include "latex_span.h"

#include "rich_text/chars/latex_char.h"

#include "profiling.h"

namespace RichText {
    /* =====
     * SPANS
     * ===== */
    bool LatexWidget::add_chars(WrapString* wrap_chars) {
        //ZoneScoped;
        bool is_not_null = m_latex_char != nullptr;
        bool success = is_not_null;

        success &= hk_add_pre_chars(wrap_chars);
        if (is_not_null)
            wrap_chars->push_back(m_latex_char);
        success &= hk_add_post_chars(wrap_chars);
        return success;
    }

    bool LatexWidget::hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) {
        //ZoneScoped;
        bool ret = true;

        /* Build widget must be called after drawing the children, because we need to know
         * the positions of the first chars in line in childrens before displaying
         * the delimiters */
         // hk_build_widget(x_offset);

        return ret;
    }
    bool LatexWidget::hk_build_widget(float x_offset) {
        bool success = false;
        if (m_widget_dirty & DIRTY_CHARS) {
            std::string source;
            AB::str_from_text_boundaries(*m_safe_string, source, m_text_boundaries);

            m_latex_char = ToLatexChar(source, m_style.font_size.f * m_style.scale * Tempo::GetScaling(),
                7.f,
                m_style.font_color,
                ImVec2(1.f, 1.f),
                ImVec2(5.f, 0.f),
                false);

            m_error = m_latex_char->m_latex_image->getLatexErrorMsg();

            m_widget_dirty ^= DIRTY_CHARS;
            success = true;
        }
        return success;
    }

    void LatexWidget::hk_debug_attributes() {
        AbstractSpan::hk_debug_attributes();
        if (!m_error.empty()) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(Colors::red), "Error: %s", m_error.c_str());
        }
    }
}