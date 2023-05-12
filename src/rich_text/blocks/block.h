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
    struct AbstractBlock : public AbstractElement {
    public:
        AbstractBlock() : AbstractElement() {
            m_category = C_BLOCK;
        }
        std::vector<DelimiterInfo> m_pre_delimiters;
        WrapString m_post_delimiters;

        void set_pre_y_position(DrawContext* context);
        void set_pre_margins(DrawContext* context);
        void get_line_height_from_delimiters(DrawContext* context);
        bool draw_pre_line(DrawContext* ctx, DelimiterInfo& del_info, int line_number, const MultiOffset& x_offset, float y_pos);

        bool hk_build_widget(DrawContext* context) override;
        bool hk_draw_main(DrawContext* context) override;
        void hk_draw_background(Draw::DrawList* draw_list) override;
        void hk_debug_attributes() override;


        /* Blocks can be defined by vertical marker, like quotes:
         * > abc    <- hk_add_pre_chars(line=0)
         * > def    <- hk_add_pre_chars(line=1)
         *
         * */
        bool virtual hk_build_pre_delimiter_chars(DrawContext* context);
        bool virtual hk_build_post_delimiter_chars(DrawContext* context) { return true; }
    };

    struct AbstractLeafBlock : public AbstractBlock {
    public:
        AbstractLeafBlock() : AbstractBlock() {
            m_category = C_BLOCK;
        }

        bool hk_build_pre_delimiter_chars(DrawContext* context) override;
        bool hk_build_post_delimiter_chars(DrawContext* context) override;
        void hk_update_line_info(DrawContext* context) override;
        bool hk_build_widget(DrawContext* context) override;
        bool hk_draw_main(DrawContext* context) override;
    };

    struct HiddenSpace : public AbstractLeafBlock {
        HiddenSpace() : AbstractLeafBlock() {
            m_category = C_BLOCK;
            m_type = T_BLOCK_HIDDENSPACE;
        }
        bool hk_build_widget(DrawContext* context) override;
        bool hk_draw_main(DrawContext* context) override;
        bool add_chars(WrapParagraph* wrap_chars) override;
    };
    struct HrBlock : public AbstractLeafBlock {
        HrBlock() : AbstractLeafBlock() {
            m_category = C_BLOCK;
            m_type = T_BLOCK_HR;
        }
        // bool hk_build_widget(float x_offset);
        // bool add_chars(std::vector<WrapCharPtr>& wrap_chars) override;
    };
}