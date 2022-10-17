#include "markdown_config.h"

namespace RichText {
    using namespace Fonts;
    MarkdownConfig::MarkdownConfig() {
        // Paragraph
        styles[P].line_space = 1.4f;
        styles[P].v_margins = ImVec2(15.f, 5.f);

        // H1
        styles[H1].font_size = 32.f;
        styles[H1].font_styling = Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL};
        styles[H1].line_space = 1.6f;

        // H2
        styles[H2].font_size = 27.f;
        styles[H2].font_styling = Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL};
        styles[H2].line_space = 1.5f;

        // H3
        styles[H3].font_size = 24.f;
        styles[H3].font_styling = Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL};
        styles[H3].line_space = 1.5f;

        // H4
        styles[H4].font_size = 21.f;
        styles[H4].font_styling = Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL};
        styles[H4].font_underline = false;
        styles[H4].line_space = 1.5f;

        // H5
        styles[H5].font_size = 18.f;
        styles[H5].font_styling = Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL};
        styles[H5].line_space = 1.4f;

        // H6
        styles[H6].font_size = 17.f;
        styles[H6].font_styling = Fonts::FontStyling{Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL};
        styles[H6].line_space = 1.4f;

        // Code
        styles[CODE].font_styling = Fonts::FontStyling{Fonts::F_MONOSPACE, Fonts::W_REGULAR, Fonts::S_NORMAL};
        styles[CODE].line_space = 1.4f;
        styles[CODE].h_margins = ImVec2(15.f, 0.f);
        styles[CODE].v_margins = ImVec2(15.f, 3.f);
        styles[CODE].h_paddings = ImVec2(5.f, 5.f);
        styles[CODE].v_paddings = ImVec2(10.f, 10.f);
        styles[CODE].font_color = Colors::darkgray;
        styles[CODE].bg_color = Colors::silver;

        // Quote
        styles[QUOTE].line_space = 1.4f;
        styles[QUOTE].h_margins = ImVec2(15.f, 0.f);
        styles[QUOTE].v_margins = ImVec2(15.f, 3.f);
        styles[QUOTE].font_color = Colors::darkgray;
        styles[QUOTE].bg_color = Colors::silver;

        // href
        styles[HREF].font_underline = true;
        styles[HREF].font_color = Colors::blue;
        styles[HREF].line_space = 1.4f;
    }
    void MarkdownConfig::make_bold(FontStyling& styling) {
        if (styling.family == F_MONOSPACE) {
            styling.weight = W_BOLD;
        }
        else if (styling.family == F_REGULAR) {
            if (styling.weight == W_LIGHT || styling.weight == W_THIN)
                styling.weight = W_REGULAR;
            else if (styling.weight == W_REGULAR)
                styling.weight = W_MEDIUM;
            else
                styling.weight = W_BOLD;
        }
    }

    void MarkdownConfig::make_em(FontStyling& styling) {
        styling.style = S_ITALIC;
    }
}