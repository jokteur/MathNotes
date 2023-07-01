#include "latex_span.h"

#include "rich_text/chars/latex_char.h"
#include "rich_text/chars/im_char.h"

#include "profiling.h"

namespace RichText {
    /* =====
     * SPANS
     * ===== */

    bool LatexSpan::build_latex_image() {
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
    bool LatexSpan::add_chars(WrapColumn* wrap_chars) {
        //ZoneScoped;
        build_latex_image();

        bool is_not_null = m_latex_char != nullptr;
        bool success = is_not_null;


        if (m_is_selected) {
            success &= hk_add_pre_chars(wrap_chars);
            for (const auto& bounds : m_text_boundaries) {
                success &= Utf8StrToImCharStr(m_ui_state, wrap_chars, m_safe_string, bounds.line_number, bounds.beg, bounds.end, m_special_chars_style, false);
            }
        }
        if (is_not_null)
            wrap_chars->push_back(m_latex_char, m_text_boundaries.front().line_number);
        if (m_is_selected) {
            success &= hk_add_post_chars(wrap_chars);
        }
        return success;
    }

    // bool LatexSpan::hk_draw_main(DrawContext* ctx) {
    //     //ZoneScoped;
    //     bool ret = true;

    //     /* Build widget must be called after drawing the children, because we need to know
    //      * the positions of the first chars in line in childrens before displaying
    //      * the delimiters */
    //     hk_build_chars(ctx);

    //     return ret;
    // }
    // bool LatexSpan::hk_build_chars(DrawContext* ctx) {
    //     // build_latex_image();
    //     return true;
    // }

    void LatexSpan::hk_debug_attributes() {
        AbstractSpan::hk_debug_attributes();
        if (!m_error.empty()) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(Colors::red), "Error: %s", m_error.c_str());
        }
    }
}