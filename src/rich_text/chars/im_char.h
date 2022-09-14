#pragma once

#include "../wrapper.h"

namespace RichText {
    /**
     * @brief Character for ImFont
     *
     */
    struct ImChar : public Character {
        Tempo::SafeImFontPtr m_font;
        float m_font_size;
        ImU32 m_color;
        ImWchar m_char;
        float m_y_offset;
    public:
        ImChar(Tempo::SafeImFontPtr font, ImWchar c, float font_size, ImU32 color, bool force_blank = false);

        void draw(ImDrawList* draw_list) override;
    };

    std::vector<CharPtr> Utf8StrToImCharStr(const std::string& str, Tempo::SafeImFontPtr font, float font_size, ImU32 color);
}