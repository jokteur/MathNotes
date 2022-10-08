#pragma once

#include "../widgets.h"
#include "ui/fonts.h"

namespace RichText {
    struct TextString : public AbstractSpan {
        Fonts::FontRequestInfo font_styling;
        int color;
        int text_start_idx = 0;
        int text_end_idx = 0;
    };
}
