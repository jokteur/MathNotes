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
    struct AbstractBlock: public AbstractWidget {
    public:
        AbstractBlock(UIState_ptr ui_state): AbstractWidget(ui_state) {
            m_category = C_BLOCK;
        }

        bool m_widget_dirty = true;

        bool hk_build_delimiter_chars();
        void hk_build_widget(float x_offset) override;
        void hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) override;
        void hk_draw_background(Draw::DrawList& draw_list) override;

        void setWidth(float width) override;
    };

    struct AbstractLeafBlock: public AbstractBlock {
    public:
        AbstractLeafBlock(UIState_ptr ui_state): AbstractBlock(ui_state) {
            m_category = C_BLOCK;
        }

        bool m_widget_dirty = true;

        void hk_build_widget(float x_offset) override;
        void hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) override;
    };

    struct HiddenSpace: public AbstractLeafBlock {
        HiddenSpace(UIState_ptr ui_state): AbstractLeafBlock(ui_state) {
            m_category = C_BLOCK;
            m_type = T_BLOCK_HIDDENSPACE;
        }
        void hk_build_widget(float x_offset) override;
        bool add_chars(std::vector<WrapCharPtr>& wrap_chars) override;
    };
}