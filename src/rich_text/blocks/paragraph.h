#pragma once

#include "block.h"

namespace RichText {
    struct ParagraphWidget: public AbstractLeafBlock {
    protected:
    public:
        ParagraphWidget();
        // bool hk_build_chars(float x_offset) override;
    };
    struct CodeWidget: public AbstractLeafBlock {
        CodeWidget();
    };
    using ParagraphWidgetPtr = std::shared_ptr<ParagraphWidget>;
    using CodeWidgetPtr = std::shared_ptr<CodeWidget>;
}