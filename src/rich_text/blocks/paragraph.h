#pragma once

#include "../widgets.h"

namespace RichText {
    struct ParagraphWidget : public AbstractBlock {
        ParagraphWidget();
    };
    struct CodeWidget : public AbstractBlock {
        CodeWidget();
    };
    struct QuoteWidget : public AbstractBlock {
        QuoteWidget();
        unsigned int quote_level = 0;
    };
    using ParagraphWidgetPtr = std::shared_ptr<ParagraphWidget>;
    using CodeWidgetPtr = std::shared_ptr<CodeWidget>;
    using QuoteWidgetPtr = std::shared_ptr<QuoteWidget>;
}