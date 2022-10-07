#pragma once

#include "../widgets.h"
#include "fonts.h"

namespace RichText {
    struct TextString : public AbstractSpan {
        Fonts::FontRequestInfo font_styling;
        float text_size;
        int color;
    };
}
