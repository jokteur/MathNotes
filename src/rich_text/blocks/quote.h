#pragma once

#include "../widgets.h"

namespace RichText {
    struct QuoteWidget : public AbstractBlock {
        QuoteWidget(UIState_ptr ui_state);
        
        void hk_build_widget(float x_offset) override;
    };
    using QuoteWidgetPtr = std::shared_ptr<QuoteWidget>;
}