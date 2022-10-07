#pragma once

#include "../widgets.h"

namespace RichText {
    struct TextString : public AbstractSpan {
        float text_size;
        int color;
        bool is_italic;
        bool is_bold;
        bool is_extra_bold;
    };
}
