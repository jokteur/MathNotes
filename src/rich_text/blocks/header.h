#pragma once

#include "../widgets.h"

namespace RichText {
    struct HeaderWidget : public AbstractBlock {
        unsigned int hlevel;
        HeaderWidget(UIState_ptr ui_state);
        bool buildAndAddChars(std::vector<WrapCharPtr>& string, int start = -1) override;
    };

    using HeaderWidgetPtr = std::shared_ptr<HeaderWidget>;
}