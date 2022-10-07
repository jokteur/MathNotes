#pragma once

#include "../widgets.h"

namespace RichText {
    struct HeaderWidget : public AbstractBlock {
        unsigned int hlevel;
        HeaderWidget();
    };

    using HeaderWidgetPtr = std::shared_ptr<HeaderWidget>;
}