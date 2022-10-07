#include "paragraph.h"

namespace RichText {
    ParagraphWidget::ParagraphWidget() {
        type = WidgetsTypes::BLOCK_P;
    }
    CodeWidget::CodeWidget() {
        type = WidgetsTypes::BLOCK_CODE;
    }
    QuoteWidget::QuoteWidget() {
        type = WidgetsTypes::BLOCK_QUOTE;
    }
}