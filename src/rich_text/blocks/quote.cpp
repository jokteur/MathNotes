#include "quote.h"
#include "rich_text/chars/im_char.h"

namespace RichText {
    QuoteWidget::QuoteWidget(UIState_ptr ui_state): AbstractBlock(ui_state) {
        m_type = T_BLOCK_QUOTE;
    }
}