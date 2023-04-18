#pragma once

#include "block.h"

namespace RichText {
    struct DivWidget: public AbstractBlock {
        DivWidget(UIState_ptr ui_state);
    };
}