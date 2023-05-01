#include "header.h"
#include "rich_text/chars/im_char.h"
#include "ui/colors.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace RichText {
    HeaderWidget::HeaderWidget(): AbstractLeafBlock() {
        m_type = T_BLOCK_H;
    }
}