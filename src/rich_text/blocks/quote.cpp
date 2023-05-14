#include "quote.h"
#include "rich_text/chars/im_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    QuoteWidget::QuoteWidget() : AbstractBlock() {
        m_type = T_BLOCK_QUOTE;
    }

    bool QuoteWidget::hk_draw_secondary(DrawContext* ctx) {
        bool ret = true;

        if (!m_is_selected) {
            ctx->draw_list->SetCurrentChannel(0);
            auto cursor_pos = ImGui::GetCursorScreenPos();
            auto p1 = cursor_pos + m_ext_dimensions.getPos();
            p1.x += m_style.h_margins.x.getFloat() / 2.f;
            auto p2 = ImVec2(p1.x, p1.y + m_ext_dimensions.getPos().y);
            auto dims = m_ext_dimensions.getDim();
            ctx->draw_list->get()->AddLine(p1, p2, Colors::darkslategray, emfloat{ 1.5 }.getFloat());
            ctx->draw_list->SetCurrentChannel(1);
        }

        return ret;
    }
}