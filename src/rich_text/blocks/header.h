#pragma once

#include "../widgets.h"

namespace RichText {
    struct HeaderWidget : public AbstractBlock {
        unsigned int hlevel;
        HeaderWidget(UIState_ptr ui_state);
        bool buildAndAddChars(std::vector<WrapCharPtr>& wrap_string) override;
        void buildWidget() override;
        // void draw(ImDrawList* draw_list, ImVec2& draw_offset) override;
    };

    using HeaderWidgetPtr = std::shared_ptr<HeaderWidget>;
}