#include "latex.h"

namespace RichText {
    DisplayLatexWidget::DisplayLatexWidget(UIState_ptr ui_state): AbstractLeafBlock(ui_state) {
        m_type = T_BLOCK_LATEX;
    }
}