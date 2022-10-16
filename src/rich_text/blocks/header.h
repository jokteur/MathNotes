#pragma once

#include "../widgets.h"

namespace RichText {
    struct HeaderWidget : public AbstractBlock {
        protected:
        bool build_chars() override;

        public:
        unsigned int hlevel;
        HeaderWidget(UIState_ptr ui_state);
    };

    using HeaderWidgetPtr = std::shared_ptr<HeaderWidget>;
}