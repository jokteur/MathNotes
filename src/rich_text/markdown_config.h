#pragma once

#include "ui/fonts.h"
#include "ui/colors.h"

namespace RichText {
    struct Style {
        float font_size = 16.f;
        Fonts::FontStyling font_styling = Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_REGULAR, Fonts::S_NORMAL};
        bool font_underline = false;
        Colors::color font_color = Colors::black;

        float line_space = 1.4f;
        ImVec2 h_margins;
        ImVec2 v_margins;
        ImVec2 h_paddings;
        ImVec2 v_paddings;

        Colors::color bg_color = Colors::transparent;
    };
    struct MarkdownConfig {
        MarkdownConfig();

        // Everything is specified in pixel (which are scaled to zoom level and monitor scaling)
        enum type { P, H1, H2, H3, H4, H5, H6, CODE, QUOTE, HREF };
        // In order: normal font size, h1, h2, h3, h4, h5, h6
        // Array is compatible with hlevel in md, e.g. font_sizes[4] gives
        // h4 font size
        Style styles[10];

        float x_level_offset = 15.f;

        static void make_bold(Fonts::FontStyling& styling);
        static void make_em(Fonts::FontStyling& styling);
    };
}