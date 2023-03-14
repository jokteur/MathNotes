#pragma once

#include "span.h"

namespace RichText {
    struct LatexWidget: public AbstractSpan {
        float text_size;
    };
}
