#include "lists.h"

#include "rich_text/chars/im_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    ULWidget::ULWidget() : AbstractBlock() {
        m_type = T_BLOCK_UL;
    }
    OLWidget::OLWidget() : AbstractBlock() {
        m_type = T_BLOCK_OL;
    }

    LIWidget::LIWidget() : AbstractBlock() {
        m_type = T_BLOCK_LI;
    }
    bool LIWidget::hk_draw_secondary(DrawContext* ctx) {
        int line_number = m_text_boundaries.front().line_number;
        auto it = ctx->lines->find(line_number);
        if (it != ctx->lines->end() && !m_is_selected && number.empty()) {
            auto p1 = m_ext_dimensions.getPos() + ImGui::GetCursorScreenPos();
            float radius = emfloat{ 2.0 }.getFloat();
            p1.y += it->second.height / 2.f;

            if (list_level == 0) {
                ctx->draw_list->get()->AddCircleFilled(
                    p1, radius, Colors::black
                );
            }
            else {
                ctx->draw_list->get()->AddCircle(
                    p1, radius, Colors::black, emfloat{ 1.0 }.getFloat()
                );
            }
        }
        return true;
    }
}