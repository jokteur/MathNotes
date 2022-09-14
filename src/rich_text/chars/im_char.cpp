#include "im_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    ImChar::ImChar(Tempo::SafeImFontPtr font, ImWchar c, float font_size, ImU32 color, bool force_blank) {
        m_font = font;
        m_font_size = font_size;
        m_color = color;
        m_char = c;

        if (m_font == nullptr)
            return;

        const ImFontGlyph* glyph = font->im_font->FindGlyph(c);
        if (glyph == NULL)
            return;
        if (c == '\r')
            return;
        if (c == '\n')
            is_linebreak = true;


        if (ImCharIsBlankW(c))
            breakable = true;
        if (force_blank)
            breakable = true;

        float scale = (font_size >= 0.0f) ? (font_size / font->im_font->FontSize) : 1.0f;
        dimensions = ImVec2(scale * (glyph->X1 - glyph->X0), scale * (glyph->Y1 - glyph->Y0));

        bearing = ImVec2(scale * glyph->X0, scale * (font->im_font->Ascent - glyph->Y0));
        advance = scale * glyph->AdvanceX;
        m_y_offset = scale * glyph->Y0;
    }

    void ImChar::draw(ImDrawList* draw_list) {
        if (m_font->im_font == nullptr)
            return;
        auto& cursor_pos = ImGui::GetCursorScreenPos();
        if (!is_linebreak) {
            ImVec2 position = _calculated_position + cursor_pos;
            position.y -= m_y_offset;
            m_font->im_font->RenderChar(draw_list,
                m_font_size,
                position,
                m_color, m_char);
        }
    }

    std::vector<CharPtr> Utf8StrToImCharStr(const std::string& str, Tempo::SafeImFontPtr font, float font_size, ImU32 color) {
        std::vector<CharPtr> out_string;

        int i = 0;
        for (auto s : str) {
            unsigned int c = (unsigned int)s;
            if (c >= 0x80) {
                ImTextCharFromUtf8(&c, &str[i], &str[str.size() - 1]);
                if (c == 0) // Malformed UTF-8?
                    break;
            }
            bool force_blank = false;
            if (c == ',' || c == '|' || c == '-' || c == '.' || c == '!' || c == '?')
                force_blank = true;
            out_string.push_back(std::make_shared<ImChar>(font, (ImWchar)c, font_size, color, force_blank));
            i++;

        }
        return out_string;
    }
}