#include "lists.h"

#include "rich_text/chars/im_char.h"

namespace RichText {
    ULWidget::ULWidget(): AbstractBlock() {
        m_type = T_BLOCK_UL;
    }
    OLWidget::OLWidget(): AbstractBlock() {
        m_type = T_BLOCK_OL;
    }
    LIWidget::LIWidget(): AbstractBlock() {
        m_type = T_BLOCK_LI;
    }
}