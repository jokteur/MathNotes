#pragma once

#include "drawable_char.h"

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
        float m_y_offset;
    public:
        ImChar(Tempo::FontID font_id, ImWchar c, float font_size, ImU32 color, bool force_blank = false);

        void draw(ImDrawList* draw_list, ImVec2 draw_offset = ImVec2(0.f, 0.f)) override;
    };

    std::vector<DrawableCharPtr> Utf8StrToImCharStr(const std::string& str, Tempo::SafeImFontPtr font, float font_size, ImU32 color);
}