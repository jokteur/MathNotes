#pragma once

#include "block.h"

namespace RichText {
    struct QuoteWidget : public AbstractBlock {
        QuoteWidget();

        bool hk_draw_secondary(DrawContext* context) override;

        emfloat m_delimiter_offset = emfloat{ 0.f };

        // void hk_build_widget(float x_offset) override;
    };
    using QuoteWidgetPtr = std::shared_ptr<QuoteWidget>;
}