#pragma once

#include "block.h"

namespace RichText {
    struct DisplayLatexWidget: public AbstractLeafBlock {
        DisplayLatexWidget(UIState_ptr ui_state);
    };
}