#pragma once

#include "../widgets.h"

namespace RichText {
    struct HeaderWidget : public AbstractBlock {
        unsigned int hlevel;
        HeaderWidget(UIState_ptr ui_state);
        bool buildAndAddChars(std::vector<DrawableCharPtr>& draw_string, std::vector<WrapCharPtr>& wrap_string, int start = -1) override;
        void buildWidget() override;
        void draw(ImDrawList* draw_list) override;
    };

    using HeaderWidgetPtr = std::shared_ptr<HeaderWidget>;
}