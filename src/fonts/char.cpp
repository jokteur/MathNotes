#include "char.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace Fonts {
    void fillCharInfos(Character* char_ptr, ImWchar c, float font_size, Tempo::SafeImFontPtr font, bool force_breakable) {
        const ImFontGlyph* glyph = font->im_font->FindGlyph(c);
        if (glyph == NULL)
            return;
        if (c == '\r')
            return;
        if (c == '\n')
            char_ptr->is_linebreak = true;


        if (ImCharIsBlankW(c)) {
            char_ptr->breakable = true;
            char_ptr->is_whitespace = true;
        }
        if (force_breakable)
            char_ptr->breakable = true;

        float scale = (font_size >= 0.0f) ? (font_size / font->im_font->FontSize) : 1.0f;
        char_ptr->dimensions = ImVec2(scale * (glyph->X1 - glyph->X0), scale * (glyph->Y1 - glyph->Y0));

        char_ptr->ascent = scale * font->im_font->Ascent;
        char_ptr->descent = scale * font->im_font->Descent;
        char_ptr->offset = ImVec2(scale * glyph->X0, scale * glyph->Y0);
        char_ptr->advance = scale * glyph->AdvanceX;
    }
}