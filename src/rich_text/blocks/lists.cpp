#include "lists.h"

#include "rich_text/chars/im_char.h"

namespace RichText {
    ULWidget::ULWidget(UIState_ptr ui_state): AbstractBlock(ui_state) {
        m_type = T_BLOCK_UL;
    }
    OLWidget::OLWidget(UIState_ptr ui_state): AbstractBlock(ui_state) {
        m_type = T_BLOCK_OL;
    }
    LIWidget::LIWidget(UIState_ptr ui_state): AbstractBlock(ui_state) {
        m_type = T_BLOCK_LI;
    }
}