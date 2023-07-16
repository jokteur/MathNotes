#include "lists.h"

#include "rich_text/chars/im_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    ULWidget::ULWidget() : AbstractBlock() {
        m_type = T_BLOCK_UL;
    }
    void ULWidget::hk_set_selected(DrawContext* ctx) {
        set_selected_never(ctx);
    }
    OLWidget::OLWidget() : AbstractBlock() {
        m_type = T_BLOCK_OL;
    }
    void OLWidget::hk_set_selected(DrawContext* ctx) {
        set_selected_never(ctx);
    }

    LIWidget::LIWidget() : AbstractBlock() {
        m_type = T_BLOCK_LI;
    }
    bool LIWidget::hk_build_pre_delimiter_chars(DrawContext* ctx) {
        //ZoneScoped;
        bool success = true;
        if (m_is_selected)
            success &= AbstractBlock::hk_build_pre_delimiter_chars(ctx);
        else if (!number.empty()) {
            m_pre_delimiters.clear();
            const auto bounds = m_text_boundaries.front();
            m_pre_delimiters[bounds.line_number] = WrapLine{};
            auto& line = m_pre_delimiters[bounds.line_number];
            auto style = m_style;
            style.set_font_color(Colors::darkslategray);
            success &= Utf8StrToImCharStr(m_ui_state, &line.chars, m_safe_string, bounds.line_number, bounds.pre, bounds.beg, style);

            WrapAlgorithm wrapper;
            wrapper.setWidth(4000.f, false);
            wrapper.recalculate(&line, ctx->x_offset.getOffset(bounds.line_number));

            /* Remove the margin that has been added */
            ctx->x_offset -= m_style.h_margins.x.getFloat() + m_style.h_paddings.x.getFloat();
        }
        return success;
    }
    bool LIWidget::hk_draw_secondary(DrawContext* ctx) {
        if (!m_is_selected && number.empty()) {
            LineInfo line_info;
            hk_get_line_info(m_text_boundaries.front().line_number, line_info);

            auto p1 = m_int_dimensions.getPos() + ImGui::GetCursorScreenPos();
            float radius = emfloat{ 2.5 }.getFloat();
            p1.y += line_info.height / 2.f - radius / 2.f;

            if (list_level == 0) {
                ctx->draw_list->get()->AddCircleFilled(
                    p1, radius, Colors::black
                );
            }
            else {
                ctx->draw_list->get()->AddCircle(
                    p1, radius, Colors::black, 0, emfloat{ 1.0 }.getFloat()
                );
            }
        }
        return true;
    }
}