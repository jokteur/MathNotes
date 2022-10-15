#pragma once

#include "../widgets.h"

namespace RichText {
    struct ParagraphWidget : public AbstractBlock {
        ParagraphWidget(UIState_ptr ui_state);
        void virtual draw(ImDrawList* draw_list) override;
    };
    struct CodeWidget : public AbstractBlock {
        CodeWidget(UIState_ptr ui_state);
    };
    struct QuoteWidget : public AbstractBlock {
        QuoteWidget(UIState_ptr ui_state);
        unsigned int quote_level = 0;
    };
    using ParagraphWidgetPtr = std::shared_ptr<ParagraphWidget>;
    using CodeWidgetPtr = std::shared_ptr<CodeWidget>;
    using QuoteWidgetPtr = std::shared_ptr<QuoteWidget>;
}