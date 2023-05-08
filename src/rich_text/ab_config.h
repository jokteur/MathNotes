#pragma once

#include "fonts/fonts.h"
#include "ui/colors.h"

#include "geometry/basic.h"

namespace RichText {
#define NEW_STYLE_ELEMENT(name_, type_, default_) \
    type_ name_ = (default_); \
    bool isset_##name_ = false; \
    void set_##name_(type_ _##name_) { name_ = _##name_; isset_##name_ = true; }
#define COMMA ,

    struct Style {
        NEW_STYLE_ELEMENT(font_size, emfloat, emfloat{ 18.f });
        NEW_STYLE_ELEMENT(font_styling, Fonts::FontStyling, Fonts::FontStyling{ Fonts::F_REGULAR COMMA Fonts::W_REGULAR COMMA Fonts::S_NORMAL });
        NEW_STYLE_ELEMENT(font_underline, bool, false);
        NEW_STYLE_ELEMENT(font_strikethrough, bool, false);
        NEW_STYLE_ELEMENT(font_color, Colors::color, Colors::black);
        NEW_STYLE_ELEMENT(font_bg_color, Colors::color, Colors::transparent);
        NEW_STYLE_ELEMENT(font_strong, bool, false);
        NEW_STYLE_ELEMENT(font_em, bool, false);
        NEW_STYLE_ELEMENT(font_monospace, bool, false);

        NEW_STYLE_ELEMENT(line_space, float, 1.4f);
        NEW_STYLE_ELEMENT(h_margins, EmVec2, EmVec2(0.f COMMA 0.f));
        NEW_STYLE_ELEMENT(v_margins, EmVec2, EmVec2(0.f COMMA 0.f));
        NEW_STYLE_ELEMENT(h_paddings, EmVec2, EmVec2(0.f COMMA 0.f));
        NEW_STYLE_ELEMENT(v_paddings, EmVec2, EmVec2(0.f COMMA 0.f));

        NEW_STYLE_ELEMENT(pre_indent, bool, true);

        NEW_STYLE_ELEMENT(bg_color, Colors::color, Colors::transparent);

        NEW_STYLE_ELEMENT(scale, float, 1.f);
    };
    struct ABConfig {
        ABConfig();

        // Everything is specified in pixel (which are scaled to zoom level and monitor scaling)
        enum type { P, H1, H2, H3, H4, H5, H6, CODE, INLINE_CODE, QUOTE, B_LATEX, UL, OL, LI, DIV, DEF, HREF, EM, STRONG, HIGHLIGHT, LATEX, SPECIAL, NUMBER_OF_TYPES };
        // In order: normal font size, h1, h2, h3, h4, h5, h6
        // Array is compatible with hlevel in md, e.g. font_sizes[4] gives
        // h4 font size
        Style styles[NUMBER_OF_TYPES];

        emfloat x_level_offset = emfloat{ 15.f };

        static void make_bold(Fonts::FontStyling& styling);
        static void make_em(Fonts::FontStyling& styling);
        static void make_monospace(Fonts::FontStyling& styling);
    };
}