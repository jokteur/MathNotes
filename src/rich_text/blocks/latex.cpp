#include "latex.h"

namespace RichText {
    DisplayLatexWidget::DisplayLatexWidget(): AbstractLeafBlock() {
        m_type = T_BLOCK_LATEX;
    }
}