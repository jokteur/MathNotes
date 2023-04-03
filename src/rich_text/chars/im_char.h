#pragma once

#include "drawable_char.h"
#include "rich_text/element.h"

namespace RichText {
    /**
     * @brief Character for ImFont
     *
     */
    struct ImChar: public DrawableChar {
        Tempo::FontID m_font_id;
        float m_font_size;
        ImU32 m_color;
        ImWchar m_char;
        float m_y_offset;
    public:
        ImChar(Tempo::FontID font_id, ImWchar c, float font_size, ImU32 color, bool force_blank = false);

        bool draw(Draw::DrawList& draw_list, const Rect& boundaries, ImVec2 draw_offset = ImVec2(0.f, 0.f)) override;
    };

    bool Utf8StrToImCharStr(UIState_ptr ui_state, std::vector<WrapCharPtr>& wrap_chars, std::vector<DrawableCharPtr>& draw_chars, SafeString str, int start, int end, Style style, bool replace_spaces_by_points = false);
}