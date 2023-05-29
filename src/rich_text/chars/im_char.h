#pragma once

#include "drawable_char.h"
#include "rich_text/element.h"

namespace RichText {
    /**
     * @brief Character for ImFont
     *
     */
    struct ImChar : public DrawableChar {
        Tempo::FontID m_font_id;
        float m_font_size;
        ImU32 m_color;
        ImWchar m_char;
    public:
        ImChar(Tempo::FontID font_id, ImWchar c, float font_size, ImU32 color, Fonts::CharPtr char_ptr, int text_pos);

        bool draw(Draw::DrawList* draw_list, const Rect& boundaries, ImVec2 draw_offset = ImVec2(0.f, 0.f)) override;
    };

    bool Utf8StrToImCharStr(UIState& ui_state, WrapParagraph* wrap_p, SafeString str, int line, int start, int end, const Style& style, bool replace_spaces_by_points = false);
    bool Utf8StrToImCharStr(UIState& ui_state, WrapString* wrap_str, SafeString str, int line, int start, int end, const Style& style, bool replace_spaces_by_points = false);
}
