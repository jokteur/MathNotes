#include "lists.h"

#include "rich_text/chars/im_char.h"

namespace RichText {
    ULWidget::ULWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        m_type = T_BLOCK_UL;
    }
    OLWidget::OLWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        m_type = T_BLOCK_OL;
    }
    LIWidget::LIWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        m_type = T_BLOCK_LI;
    }
    bool LIWidget::build_chars() {
        // TODO make OL
        using namespace Fonts;
        FontInfoOut font_out;
        m_ui_state->font_manager.requestFont(m_font_request, font_out);
        float font_size = font_out.size * font_out.ratio * m_scale * Tempo::GetScaling();

        auto font = Tempo::GetImFont(font_out.font_id);
        if (font->im_font == nullptr) {
            return false;
        }
        auto ptr = std::make_shared<ImChar>(font_out.font_id, (ImWchar)'-', font_size, m_font_color, false);
        m_draw_chars.push_back(ptr);
        m_wrap_chars.push_back(ptr);
        auto ptr2 = std::make_shared<ImChar>(font_out.font_id, (ImWchar)' ', font_size, m_font_color, false);
        m_draw_chars.push_back(ptr2);
        m_wrap_chars.push_back(ptr2);
        return true;
    }
}