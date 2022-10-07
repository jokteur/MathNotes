#pragma once

#include "../widgets.h"

namespace RichText {
    struct ULWidget : public AbstractBlock {
        unsigned int list_level = 0;
    };
}