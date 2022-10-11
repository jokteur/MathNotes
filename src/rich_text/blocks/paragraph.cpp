#include "paragraph.h"

namespace RichText {
    ParagraphWidget::ParagraphWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        type = T_BLOCK_P;
    }
    bool ParagraphWidget::buildAndAddChars(std::vector<WrapCharPtr>& string, int start) {
        auto char_ptr = std::make_shared<NewLine>();
        auto char_ptr2 = std::make_shared<NewLine>();
        string.push_back(char_ptr);
        chars.push_back(char_ptr);
        string.push_back(char_ptr2);
        chars.push_back(char_ptr2);
        return true;
    }
    CodeWidget::CodeWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        type = T_BLOCK_CODE;
    }
    QuoteWidget::QuoteWidget(UIState_ptr ui_state) : AbstractBlock(ui_state) {
        type = T_BLOCK_QUOTE;
    }
}