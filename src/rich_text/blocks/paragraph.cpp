#include "paragraph.h"

namespace RichText {
    ParagraphWidget::ParagraphWidget() {
        type = T_BLOCK_P;
    }
    CodeWidget::CodeWidget() {
        type = T_BLOCK_CODE;
    }
    QuoteWidget::QuoteWidget() {
        type = T_BLOCK_QUOTE;
    }
}