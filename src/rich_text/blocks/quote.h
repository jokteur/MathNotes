#pragma once

#include "../widgets.h"

namespace RichText {
    struct QuoteWidget: public AbstractBlock {
        QuoteWidget(UIState_ptr ui_state);

        float m_delimiter_offset = 0.f;

        void hk_build_widget(float x_offset) override;
    };
    using QuoteWidgetPtr = std::shared_ptr<QuoteWidget>;
}