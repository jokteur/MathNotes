#include "latex_block.h"

namespace RichText {
    DisplayLatexWidget::DisplayLatexWidget() : AbstractLeafBlock() {
        m_type = T_BLOCK_LATEX;
    }
    bool DisplayLatexWidget::hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) {
        //ZoneScoped;
        bool ret = true;

        /* Build widget must be called after drawing the children, because we need to know
         * the positions of the first chars in line in childrens before displaying
         * the delimiters */
        hk_build_widget(x_offset);

        if (m_latex_char != nullptr) {
            auto dims = m_latex_char->m_latex_image->getDimensions();
            float available_space = m_window_width - x_offset - dims.x;
            if (available_space < 0.f)
                available_space = 0.f;

            m_latex_char->draw(draw_list, boundaries, ImVec2(x_offset + available_space / 2.f, cursor_y_pos));
            cursor_y_pos += m_latex_char->m_latex_image->getDimensions().y;
        }
        return ret;
    }
    bool DisplayLatexWidget::hk_build_widget(float x_offset) {
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

    void DisplayLatexWidget::hk_debug_attributes() {
        AbstractLeafBlock::hk_debug_attributes();
        if (!m_error.empty()) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(Colors::red), "Error: %s", m_error.c_str());
        }
    }
}