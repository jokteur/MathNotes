#include "text.h"
#include "rich_text/chars/im_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    TextString::TextString(UIState_ptr ui_state) : AbstractSpan(ui_state) {
        m_type = T_TEXT;
    }

    bool TextString::buildAndAddChars(std::vector<WrapCharPtr>& wrap_string) {
        using namespace Fonts;
        FontInfoOut font_out;
        m_ui_state->font_manager.requestFont(m_font_request, font_out);
        auto font = Tempo::GetImFont(font_out.font_id);

        if (m_safe_string == nullptr || font->im_font == nullptr) {
            return false;
        }

        float font_size = font_out.size * font_out.ratio * m_scale * Tempo::GetScaling();
        for (int i = m_raw_text_begin;i < m_raw_text_end;i++) {
            unsigned int c = (unsigned int)(*m_safe_string)[i];
            if (c >= 0x80) {
                ImTextCharFromUtf8(&c, &(*m_safe_string)[i], &(*m_safe_string)[m_safe_string->size() - 1]);
                if (c == 0) // Malformed UTF-8?
                    break;
            }
            bool force_breakable = false;
            if (c == ',' || c == '|' || c == '-' || c == '.' || c == '!' || c == '?')
                force_breakable = true;
            auto char_ptr = std::make_shared<ImChar>(font, (ImWchar)c, font_size, m_font_color, force_breakable);
            m_draw_chars.push_back(std::static_pointer_cast<DrawableChar>(char_ptr));
            wrap_string.push_back(std::static_pointer_cast<WrapCharacter>(char_ptr));
        }
        return true;
    }
    void TextString::draw(ImDrawList* draw_list, ImVec2& draw_offset) {
        for (auto& c : m_draw_chars) {
            c->draw(draw_list, draw_offset);
        }
    }
}