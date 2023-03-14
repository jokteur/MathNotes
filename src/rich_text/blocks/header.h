#pragma once

#include "block.h"

namespace RichText {
    struct HeaderWidget: public AbstractLeafBlock {
    protected:
    public:
        unsigned int hlevel;
        HeaderWidget(UIState_ptr ui_state);
    };

    using HeaderWidgetPtr = std::shared_ptr<HeaderWidget>;
}