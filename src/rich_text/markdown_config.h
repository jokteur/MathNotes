#pragma once

#include "ui/fonts.h"
#include "ui/colors.h"

namespace RichText {
#define NEW_STYLE_ELEMENT(name_, type_, default_) \
    type_ name_ = (default_); \
    bool isset_##name_ = false; \
    void set_##name_(type_ _##name_) { name_ = _##name_; isset_##name_ = true; }
#define COMMA ,

    struct Style {
        NEW_STYLE_ELEMENT(font_size, float, 16.f);
        NEW_STYLE_ELEMENT(font_styling, Fonts::FontStyling, Fonts::FontStyling{ Fonts::F_REGULAR COMMA Fonts::W_REGULAR COMMA Fonts::S_NORMAL });
        NEW_STYLE_ELEMENT(font_underline, bool, false);
        NEW_STYLE_ELEMENT(font_strikethrough, bool, false);
        NEW_STYLE_ELEMENT(font_color, Colors::color, Colors::black);
        NEW_STYLE_ELEMENT(font_bg_color, Colors::color, Colors::transparent);
        NEW_STYLE_ELEMENT(font_strong, bool, false);
        NEW_STYLE_ELEMENT(font_em, bool, false);
        NEW_STYLE_ELEMENT(font_monospace, bool, false);

        NEW_STYLE_ELEMENT(line_space, float, 1.4f);
        NEW_STYLE_ELEMENT(h_margins, ImVec2, ImVec2(0.f COMMA 0.f));
        NEW_STYLE_ELEMENT(v_margins, ImVec2, ImVec2(0.f COMMA 0.f));
        NEW_STYLE_ELEMENT(h_paddings, ImVec2, ImVec2(0.f COMMA 0.f));
        NEW_STYLE_ELEMENT(v_paddings, ImVec2, ImVec2(0.f COMMA 0.f));

        NEW_STYLE_ELEMENT(bg_color, Colors::color, Colors::transparent);

        NEW_STYLE_ELEMENT(scale, float, 1.f);
    };
    struct MarkdownConfig {
        MarkdownConfig();

        // Everything is specified in pixel (which are scaled to zoom level and monitor scaling)
        enum type { P, H1, H2, H3, H4, H5, H6, CODE, INLINE_CODE, QUOTE, HREF, EM, STRONG, SPECIAL };
        // In order: normal font size, h1, h2, h3, h4, h5, h6
        // Array is compatible with hlevel in md, e.g. font_sizes[4] gives
        // h4 font size
        Style styles[14];

        float x_level_offset = 15.f;

        static void make_bold(Fonts::FontStyling& styling);
        static void make_em(Fonts::FontStyling& styling);
        static void make_monospace(Fonts::FontStyling& styling);
    };
}