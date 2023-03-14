#pragma once

#include <tempo.h>
#include <unordered_set>
#include "ab_parser.h"

#include "types.h"
#include "ui/drawable.h"
#include "ui/fonts.h"
#include "ui/draw_commands.h"
#include "ui/colors.h"

#include "../widgets.h"

namespace RichText {
    struct AbstractSpan: public AbstractWidget {
        std::string m_processed_text;
        AbstractSpan(UIState_ptr ui_state): AbstractWidget(ui_state) {
            m_category = C_SPAN;
        }
        bool hk_add_pre_chars(std::vector<WrapCharPtr>& wrap_chars);
        bool hk_add_post_chars(std::vector<WrapCharPtr>& wrap_chars);
        bool add_chars(std::vector<WrapCharPtr>& wrap_chars) override;
        void hk_draw_background(Draw::DrawList& draw_list) override;
    };
}