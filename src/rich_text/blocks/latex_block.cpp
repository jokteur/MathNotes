#include "latex_block.h"
#include "rich_text/chars/im_char.h"

namespace RichText {
    DisplayLatexWidget::DisplayLatexWidget() : AbstractLeafBlock() {
        m_type = T_BLOCK_LATEX;
    }
    // bool DisplayLatexWidget::hk_draw_secondary(DrawContext* ctx) {
    //     //ZoneScoped;
    //     bool ret = true;

    //     hk_build_widget(ctx);

    //     // Draw all the chars generated in the block
    //     for (auto& pair : ctx->doc->at(this).getParagraph()) {
    //         auto int_pos = m_int_dimensions.getPos();
    //         int_pos.x = ctx->x_offset.getOffset(pair.first);
    //         for (auto ptr : pair.second.chars) {
    //             auto p = std::static_pointer_cast<DrawableChar>(ptr);
    //             if (!p->draw(ctx->draw_list, ctx->boundaries, int_pos))
    //                 ret = false;
    //         }
    //     }

    //     // hk_update_line_info(ctx);

    //     // ctx->cursor_y_pos += m_wrapper.getHeight();


    //     if (m_latex_char != nullptr) {
    //         auto dims = m_latex_char->m_latex_image->getDimensions();
    //         float available_space = m_window_width - ctx->x_offset.getMax() - dims.x;
    //         if (available_space < 0.f)
    //             available_space = 0.f;

    //         m_latex_char->draw(ctx->draw_list, ctx->boundaries, ImVec2(ctx->x_offset.getMax() + available_space / 2.f, ctx->cursor_y_pos));
    //         ctx->cursor_y_pos += m_latex_char->m_latex_image->getDimensions().y;
    //     }
    //     return ret;
    // }
    bool DisplayLatexWidget::hk_build_chars(DrawContext* ctx) {
        bool success = false;
        if (m_widget_dirty & DIRTY_CHARS) {
            m_text_column.clear();

            bool success = true;

            if (m_is_selected) {
                success &= hk_build_pre_delimiter_chars(ctx);
                for (const auto& bounds : m_text_boundaries) {
                    success &= Utf8StrToImCharStr(m_ui_state, &m_text_column, m_safe_string, bounds.line_number, bounds.beg, bounds.end, m_special_chars_style, false);
                }
                success &= hk_build_post_delimiter_chars(ctx);
                m_wrapper.clear();
                m_wrapper.setLineSpace(m_style.line_space, false);
                m_wrapper.setTextColumn(&m_text_column, false);
                m_wrapper.recalculate();
            }

            std::string source;
            AB::str_from_text_boundaries(*m_safe_string, source, m_text_boundaries);

            m_latex_char = ToLatexChar(source, m_style.font_size.f * m_style.scale * Tempo::GetScaling(),
                7.f,
                m_style.font_color,
                ImVec2(1.f, 1.f),
                ImVec2(5.f, 0.f),
                false);

            m_error = m_latex_char->m_latex_image->getLatexErrorMsg();

            if (success)
                m_widget_dirty ^= DIRTY_CHARS;
            success = true;
        }
        if (m_widget_dirty & DIRTY_WIDTH) {
            if (m_is_selected) {
                float internal_size = m_window_width - ctx->x_offset.getMin() - m_style.h_margins.y.getFloat();
                m_wrapper.setWidth(internal_size, false);

                m_wrapper.recalculate();
            }
            m_widget_dirty ^= DIRTY_WIDTH;
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