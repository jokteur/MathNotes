#include "definition.h"

namespace RichText {
    DefinitionWidget::DefinitionWidget(UIState_ptr ui_state): AbstractBlock(ui_state) {
        m_type = T_BLOCK_DEF;
    }
}