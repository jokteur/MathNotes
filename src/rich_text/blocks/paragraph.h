#pragma once

#include "block.h"

namespace RichText {
    struct ParagraphWidget: public AbstractLeafBlock {
    protected:
    public:
        ParagraphWidget(UIState_ptr ui_state);
        // bool hk_build_widget(float x_offset) override;
    };
    struct CodeWidget: public AbstractLeafBlock {
        CodeWidget(UIState_ptr ui_state);
    };
    using ParagraphWidgetPtr = std::shared_ptr<ParagraphWidget>;
    using CodeWidgetPtr = std::shared_ptr<CodeWidget>;
}