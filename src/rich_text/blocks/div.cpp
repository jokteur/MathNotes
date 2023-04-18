#include "div.h"

namespace RichText {
    DivWidget::DivWidget(UIState_ptr ui_state): AbstractBlock(ui_state) {
        m_type = T_BLOCK_DIV;
    }
}