#include "im_char.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include <iostream>

namespace RichText {
    ImChar::ImChar(Tempo::FontID font_id, ImWchar c, float font_size, ImU32 color, bool force_breakable) {
        m_font_id = font_id;
        m_font_size = font_size;
        m_color = color;
        m_char = c;

        auto font = Tempo::GetImFont(font_id);

        if (font->im_font == nullptr)
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

    void ImChar::draw(Draw::DrawList& draw_list, ImVec2 draw_offset) {
        auto font = Tempo::GetImFont(m_font_id);

        if (font->im_font == nullptr)
            return;
        auto cursor_pos = ImGui::GetCursorScreenPos();

        if (!is_linebreak) {
            // ImGui RenderChar takes offset into account, this is why it is substracted
            ImVec2 position = _calculated_position + cursor_pos - offset + draw_offset;
            font->im_font->RenderChar(*draw_list,
                m_font_size,
                position,
                m_color, m_char);
        }
    }

    bool Utf8StrToImCharStr(UIState_ptr ui_state, std::vector<WrapCharPtr>& wrap_chars, std::vector<DrawableCharPtr>& draw_chars, SafeString str, int start, int end, Style style) {
        using namespace Fonts;
        FontRequestInfo font_request;
        font_request.font_styling = style.font_styling;
        font_request.size_wish = style.font_size;

        FontInfoOut font_out;
        ui_state->font_manager.requestFont(font_request, font_out);
        float font_size = font_out.size * font_out.ratio * style.scale * Tempo::GetScaling();

        auto font = Tempo::GetImFont(font_out.font_id);
        if (font->im_font == nullptr) {
            return false;
        }

        for (int i = start;i < end;i++) {
            unsigned int c = (unsigned int)(*str)[i];
            if (c >= 0x80) {
                ImTextCharFromUtf8(&c, &((*str)[i]), &((*str)[str->size() - 1]));
                if (c == 0) // Malformed UTF-8?
                    break;
            }
            auto ptr = std::make_shared<ImChar>(font_out.font_id, (ImWchar)c, font_size, style.font_color, false);
            draw_chars.push_back(ptr);
            wrap_chars.push_back(ptr);
        }
        return true;
    }
}