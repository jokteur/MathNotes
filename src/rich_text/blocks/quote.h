#pragma once

#include "block.h"

namespace RichText {
    struct QuoteWidget: public AbstractBlock {
        QuoteWidget();

        emfloat m_delimiter_offset = emfloat{ 0.f };

        // void hk_build_widget(float x_offset) override;
    };
    using QuoteWidgetPtr = std::shared_ptr<QuoteWidget>;
}