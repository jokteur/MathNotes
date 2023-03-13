#pragma once

#include "../widgets.h"

namespace RichText {
    struct ParagraphWidget: public AbstractBlock {
    protected:
    public:
        ParagraphWidget(UIState_ptr ui_state);
        void hk_build_widget(float x_offset) override;
    };
    struct CodeWidget: public AbstractBlock {
        CodeWidget(UIState_ptr ui_state);
    };
    using ParagraphWidgetPtr = std::shared_ptr<ParagraphWidget>;
    using CodeWidgetPtr = std::shared_ptr<CodeWidget>;
}