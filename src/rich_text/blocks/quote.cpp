#include "quote.h"
#include "rich_text/chars/im_char.h"

namespace RichText {
    QuoteWidget::QuoteWidget(): AbstractBlock() {
        m_type = T_BLOCK_QUOTE;
    }
}