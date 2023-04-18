#pragma once

#include "block.h"

namespace RichText {
    struct DefinitionWidget: public AbstractBlock {
        DefinitionWidget(UIState_ptr ui_state);
    };
}
