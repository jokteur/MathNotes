#include "header.h"
#include "rich_text/chars/im_char.h"
#include "ui/colors.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    HeaderWidget::HeaderWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        m_type = T_BLOCK_H;
    }
    bool HeaderWidget::build_chars() {
        using namespace Fonts;
        FontRequestInfo font_request;
        font_request.font_styling = m_style.font_styling;
        font_request.size_wish = m_style.font_size;

        FontInfoOut font_out;
        m_ui_state->font_manager.requestFont(font_request, font_out);
        float font_size = font_out.size * font_out.ratio * m_scale * Tempo::GetScaling();

        auto font = Tempo::GetImFont(font_out.font_id);
        if (font->im_font == nullptr) {
            return false;
        }
        // for (int i = m_raw_text_info.pre;i < m_raw_text_begin;i++) {
        //     unsigned int c = (unsigned int)(*m_safe_string)[i];
        //     if (c >= 0x80) {
        //         ImTextCharFromUtf8(&c, &((*m_safe_string)[i]), &((*m_safe_string)[m_safe_string->size() - 1]));
        //         if (c == 0) // Malformed UTF-8?
        //             break;
        //     }
        //     auto ptr = std::make_shared<ImChar>(font_out.font_id, (ImWchar)c, font_size, m_style.special_char_color, false);
        //     m_draw_chars.push_back(ptr);
        //     m_wrap_chars.push_back(ptr);
        // }
        // for (int i = 0;i < hlevel;i++) {
        //     auto ptr = std::make_shared<ImChar>(font_out.font_id, (ImWchar)'#', font_size, m_style.special_char_color, false);
        //     m_draw_chars.push_back(ptr);
        //     m_wrap_chars.push_back(ptr);
        // }
        // auto ptr = std::make_shared<ImChar>(font_out.font_id, (ImWchar)' ', font_size, m_style.special_char_color, false);
        // m_draw_chars.push_back(ptr);
        // m_wrap_chars.push_back(ptr);
        return true;
    }
}