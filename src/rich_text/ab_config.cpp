#include "ab_config.h"

namespace RichText {
    using namespace Fonts;
    ABConfig::ABConfig() {
        // Paragraph
        styles[P].set_line_space(1.4f);
        styles[P].set_v_margins(EmVec2(emfloat{ 0.f }, emfloat{ 0.f }));
        styles[P].set_h_margins(EmVec2(emfloat{ 0.f }, emfloat{ 0.f }));
        // styles[P].set_v_paddings(EmVec2(emfloat{0.f},emfloat{ 0.f}));
        // styles[P].set_h_paddings(EmVec2(emfloat{0.f},emfloat{ 0.f}));

        // H1
        styles[H1].set_font_size(emfloat{ 42.f });
        styles[H1].set_font_styling(Fonts::FontStyling{ Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL });
        styles[H1].set_line_space(1.6f);

        // H2
        styles[H2].set_font_size(emfloat{ 36.f });
        styles[H2].set_font_styling(Fonts::FontStyling{ Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL });
        styles[H2].set_line_space(1.5f);

        // H3
        styles[H3].set_font_size(emfloat{ 32.f });
        styles[H3].set_font_styling(Fonts::FontStyling{ Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL });
        styles[H3].set_line_space(1.5f);

        // H4
        styles[H4].set_font_size(emfloat{ 27.f });
        styles[H4].set_font_styling(Fonts::FontStyling{ Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL });
        styles[H4].set_font_underline(false);
        styles[H4].set_line_space(1.5f);

        // H5
        styles[H5].set_font_size(emfloat{ 24.f });
        styles[H5].set_font_styling(Fonts::FontStyling{ Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL });
        styles[H5].set_line_space(1.4f);

        // H6
        styles[H6].set_font_size(emfloat{ 21.f });
        styles[H6].set_font_styling(Fonts::FontStyling{ Fonts::F_REGULAR, Fonts::W_MEDIUM, Fonts::S_NORMAL });
        styles[H6].set_line_space(1.4f);

        // Code
        styles[CODE].set_font_monospace(true);
        styles[CODE].set_h_margins(EmVec2(emfloat{ 15.f }, emfloat{ 0.f }));
        styles[CODE].set_v_margins(EmVec2(emfloat{ 10.f }, emfloat{ 0.f }));
        // styles[CODE].set_h_paddings(EmVec2(emfloat{15.f},emfloat{ 5.f}));
        // styles[CODE].set_v_paddings(EmVec2(emfloat{0.f},emfloat{ 0.f}));
        styles[CODE].set_font_color(Colors::dimgray);
        styles[CODE].set_bg_color(Colors::gainsboro);

        // Inline code
        styles[INLINE_CODE].set_font_monospace(true);
        styles[INLINE_CODE].set_font_color(Colors::dimgray);
        styles[INLINE_CODE].set_font_bg_color(Colors::gainsboro);

        // Quote
        styles[QUOTE].set_line_space(1.4f);
        styles[QUOTE].set_h_margins(EmVec2(emfloat{ 15.f }, emfloat{ 0.f }));
        styles[QUOTE].set_v_margins(EmVec2(emfloat{ 5.f }, emfloat{ 0.f }));
        // styles[QUOTE].set_v_paddings(EmVec2(emfloat{0.f},emfloat{ 0.f}));

        // Display latex
        styles[B_LATEX].set_h_margins(EmVec2(emfloat{ 0.f }, emfloat{ 0.f }));
        styles[B_LATEX].set_v_margins(EmVec2(emfloat{ 15.f }, emfloat{ 15.f }));
        // styles[B_LATEX].set_v_paddings(EmVec2(emfloat{0.f},emfloat{ 0.f}));

        styles[UL].set_h_margins(EmVec2(emfloat{ 15.f }, emfloat{ 0.f }));
        styles[OL].set_h_margins(EmVec2(emfloat{ 15.f }, emfloat{ 0.f }));
        styles[LI].set_v_margins(EmVec2(emfloat{ 5.f }, emfloat{ 0.f }));

        // div
        styles[DIV].set_h_margins(EmVec2(emfloat{ 15.f }, emfloat{ 0.f }));
        styles[DIV].set_v_margins(EmVec2(emfloat{ 0.f }, emfloat{ 0.f }));
        // styles[DIV].set_v_paddings(EmVec2(emfloat{0.f},emfloat{ 0.f}));

        // definition
        styles[DEF].set_h_margins(EmVec2(emfloat{ 15.f }, emfloat{ 0.f }));
        styles[DEF].set_v_margins(EmVec2(emfloat{ 0.f }, emfloat{ 0.f }));
        // styles[DEF].set_v_paddings(EmVec2(emfloat{0.f},emfloat{ 0.f}));

        // href
        styles[HREF].set_font_underline(true);
        styles[HREF].set_font_color(Colors::blue);
        styles[HREF].set_line_space(1.4f);

        // em
        styles[EM].set_font_em(true);
        // strong
        styles[STRONG].set_font_strong(true);

        // highlight
        styles[HIGHLIGHT].set_font_bg_color(Colors::yellow);

        // highlight
        // styles[P].set_line_space(1.4f);

        // markdown characters (non-visible usually)
        styles[SPECIAL].set_font_monospace(true);
        styles[SPECIAL].set_font_color(Colors::gray);
    }
    void ABConfig::make_bold(FontStyling& styling) {
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

    void ABConfig::make_em(FontStyling& styling) {
        styling.style = S_ITALIC;
        if (styling.family == F_MONOSPACE) {
            styling.weight = W_REGULAR;
        }
    }
    void ABConfig::make_monospace(FontStyling& styling) {
        styling.family = F_MONOSPACE;
        if (styling.weight != W_REGULAR) {
            styling.weight = W_BOLD;
        }
        if (styling.style == S_ITALIC) {
            styling.weight = W_REGULAR;
        }
    }
}