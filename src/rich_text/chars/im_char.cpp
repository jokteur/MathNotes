#include "im_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "ui/state.h"
#include <iostream>

#include "profiling.h"

namespace RichText {
    ImChar::ImChar(Tempo::FontID font_id, ImWchar c, float font_size, ImU32 color, Fonts::CharPtr char_ptr, int text_pos) : DrawableChar(char_ptr) {
        m_font_id = font_id;
        m_font_size = font_size;
        m_color = color;
        m_char = c;
        text_position = text_pos;
    }

    bool ImChar::draw(Draw::DrawList* draw_list, const Rect& boundaries, ImVec2 draw_offset) {
        auto font = Tempo::GetImFont(m_font_id);

        if (font->im_font == nullptr)
            return false;
        auto cursor_pos = ImGui::GetCursorScreenPos();

        if (!info->is_linebreak) {
            // ImGui RenderChar takes offset into account, this is why it is substracted
            ImVec2 position = calculated_position + cursor_pos - info->offset + draw_offset;
            font->im_font->RenderChar(**draw_list,
                m_font_size,
                position,
                m_color, m_char);
        }
        return true;
    }

    bool Utf8StrToImCharStr(UIState& ui_state, WrapParagraph* wrap_p, SafeString str, int line, int start, int end, const Style& style, bool replace_spaces_by_points) {
        //ZoneScoped;
        if (start == end)
            return true;

        std::vector<Fonts::FontCharOut> characters;
        bool ret = ui_state.font_manager.requestCharString(characters, *str, start, end, style.font_styling, style.font_size, replace_spaces_by_points);
        if (!ret)
            return false;

        for (auto& ch : characters) {
            wrap_p->push_back(std::make_shared<ImChar>(ch.char_id.m_font_id, ch.char_id.m_char, ch.char_id.m_font_size, style.font_color, ch.character, ch.text_pos), line);
        }

        return true;
    }
    bool Utf8StrToImCharStr(UIState& ui_state, WrapString* wrap_str, SafeString str, int line, int start, int end, const Style& style, bool replace_spaces_by_points) {
        if (start == end)
            return true;

        std::vector<Fonts::FontCharOut> characters;
        bool ret = ui_state.font_manager.requestCharString(characters, *str, start, end, style.font_styling, style.font_size, replace_spaces_by_points);
        if (!ret)
            return false;

        for (auto& ch : characters) {
            wrap_str->push_back(std::make_shared<ImChar>(ch.char_id.m_font_id, ch.char_id.m_char, ch.char_id.m_font_size, style.font_color, ch.character, ch.text_pos));
        }
        return true;
    }
}
