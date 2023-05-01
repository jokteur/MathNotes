#include "definition.h"

namespace RichText {
    DefinitionWidget::DefinitionWidget(): AbstractBlock() {
        m_type = T_BLOCK_DEF;
    }
}