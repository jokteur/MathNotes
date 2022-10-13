#pragma once

#include "ui/fonts.h"
#include "ui/colors.h"

namespace RichText {
    struct MarkdownConfig {
        // Everything is specified in pixel (which are scaled to zoom level and monitor scaling)
        enum type { P, H1, H2, H3, H4, H5, H6, CODE, HREF };
        // In order: normal font size, h1, h2, h3, h4, h5, h6
        // Array is compatible with hlevel in md, e.g. font_sizes[4] gives
        // h4 font size
        float font_sizes[9] = { 16.f, 32.f, 27.f, 24.f, 21.f, 18.f, 17.f, 16.f, 16.f };

        Fonts::FontStyling font_stylings[9] = {
            Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_REGULAR, Fonts::S_NORMAL},
            Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL},
            Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL},
            Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL},
            Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL},
            Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL},
            Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL},
            Fonts::FontStyling{Fonts::F_MONOSPACE, Fonts::W_REGULAR, Fonts::S_NORMAL},
            Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_REGULAR, Fonts::S_NORMAL},
        };

        // Relative linespace of different styles
        float line_spaces[9] = { 1.4f, 1.6f, 1.5f, 1.5f, 1.5f, 1.4f, 1.4f, 1.3f, 1.4f };
        float font_underlines[9] = { false, false, false, false, false, false, false, false, true };

        // Colors
        Colors::color default_colors[9] = {
            Colors::BLACK, // Paragraph
            Colors::BLACK, // H1
            Colors::BLACK, // H2
            Colors::BLACK, // H3
            Colors::BLACK, // H4
            Colors::BLACK, // H5
            Colors::BLACK, // H6
            Colors::BLACK, // Code
            Colors::BLUE   // href
        };
        Colors::color bg_colors[9] = {
            Colors::TRANSPARENT, // Paragraph
            Colors::TRANSPARENT, // H1
            Colors::TRANSPARENT, // H2
            Colors::TRANSPARENT, // H3
            Colors::TRANSPARENT, // H4
            Colors::TRANSPARENT, // H5
            Colors::TRANSPARENT, // H6
            Colors::TRANSPARENT, // Code
            Colors::TRANSPARENT  // href
        };

        float x_level_offset = 15.f;

        static void make_bold(Fonts::FontStyling& styling);
        static void make_em(Fonts::FontStyling& styling);
    };
}