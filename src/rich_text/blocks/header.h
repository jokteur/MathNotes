#pragma once

#include "block.h"

namespace RichText {
    struct HeaderWidget: public AbstractLeafBlock {
    protected:
    public:
        unsigned int hlevel;
        HeaderWidget();
    };

    using HeaderWidgetPtr = std::shared_ptr<HeaderWidget>;
}