#pragma once

#include "block.h"
#include "rich_text/chars/latex_char.h"

namespace RichText {
    struct DisplayLatexWidget : public AbstractLeafBlock {
        std::string m_error;
        LatexCharPtr m_latex_char = nullptr;
        WrapString m_error_string;

        DisplayLatexWidget();

        bool hk_draw_main(DrawContext* context) override;
        bool hk_build_widget(DrawContext* context) override;

        void  hk_debug_attributes() override;
    };
}