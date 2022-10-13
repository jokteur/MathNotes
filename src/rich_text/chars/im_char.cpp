#include "im_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    ImChar::ImChar(Tempo::SafeImFontPtr font, ImWchar c, float font_size, ImU32 color, bool force_breakable) {
        m_font = font;
        m_font_size = font_size;
        m_color = color;
        m_char = c;

        if (m_font->im_font == nullptr)
            return;

        const ImFontGlyph* glyph = font->im_font->FindGlyph(c);
        if (glyph == NULL)
            return;
        if (c == '\r')
            return;
        if (c == '\n')
            is_linebreak = true;


        if (ImCharIsBlankW(c)) {
            breakable = true;
            is_whitespace = true;
        }
        if (force_breakable)
            breakable = true;

        float scale = (font_size >= 0.0f) ? (font_size / font->im_font->FontSize) : 1.0f;
        dimensions = ImVec2(scale * (glyph->X1 - glyph->X0), scale * (glyph->Y1 - glyph->Y0));

        ascent = scale * font->im_font->Ascent;
        descent = scale * font->im_font->Descent;
        offset = ImVec2(scale * glyph->X0, scale * glyph->Y0);
        advance = scale * glyph->AdvanceX;
    }

    void ImChar::draw(ImDrawList* draw_list, ImVec2 draw_offset) {
        if (m_font->im_font == nullptr)
            return;
        auto cursor_pos = ImGui::GetCursorScreenPos();

        if (!is_linebreak) {
            // ImGui RenderChar takes offset into account, this is why it is substracted
            ImVec2 position = _calculated_position + cursor_pos - offset + draw_offset;
            m_font->im_font->RenderChar(draw_list,
                m_font_size,
                position,
                m_color, m_char);
        }
    }
}