#pragma once

#include <tempo.h>
#include <unordered_set>
#include "ab_parser.h"

#include "types.h"
#include "ui/drawable.h"
#include "fonts/fonts.h"
#include "ui/draw_commands.h"
#include "ui/colors.h"

#include "rich_text/element.h"

namespace RichText {
    struct AbstractBlock: public AbstractElement {
    public:
        AbstractBlock(): AbstractElement() {
            m_category = C_BLOCK;
        }

        bool hk_build_delimiter_chars();
        bool hk_build_widget(float x_offset);
        bool hk_build_widget_post(float x_offset);
        bool hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) override;
        void hk_draw_background(Draw::DrawList& draw_list) override;
        void hk_debug_attributes() override;
    };

    struct AbstractLeafBlock: public AbstractBlock {
    public:
        AbstractLeafBlock(): AbstractBlock() {
            m_category = C_BLOCK;
        }

        bool hk_build_widget(float x_offset);
        bool hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) override;
    };

    struct HiddenSpace: public AbstractLeafBlock {
        HiddenSpace(): AbstractLeafBlock() {
            m_category = C_BLOCK;
            m_type = T_BLOCK_HIDDENSPACE;
        }
        bool hk_build_widget(float x_offset);
        bool add_chars(std::vector<WrapCharPtr>& wrap_chars) override;
    };
    struct HrBlock: public AbstractLeafBlock {
        HrBlock(): AbstractLeafBlock() {
            m_category = C_BLOCK;
            m_type = T_BLOCK_HR;
        }
        // bool hk_build_widget(float x_offset);
        // bool add_chars(std::vector<WrapCharPtr>& wrap_chars) override;
    };
}