#include "markdown_config.h"

namespace RichText {
    using namespace Fonts;
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