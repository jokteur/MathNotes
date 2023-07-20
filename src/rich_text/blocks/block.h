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
            m_cursor_set = &AbstractElement::set_selected_pre_only;
        }
        WrapColumn m_pre_delimiters;

        void set_pre_margins(DrawContext* context);
        // void place_text_cursor(DrawContext* ctx, int line_number, int text_pos, float x_pos, const WrapString& chars, TextCursor& cursor);

        bool hk_build_hlayout(DrawContext* context) override;
        bool hk_build_vlayout(DrawContext* context, int line_number = -1) override;

        // bool hk_build_main(DrawContext* context) override;
        // bool hk_draw_main(DrawContext* context) override;
        void hk_draw_background(Draw::DrawList* draw_list) override;
        void hk_draw_text_cursor(DrawContext* context) override;
        bool draw(DrawContext* ctx) override;
        void hk_debug_attributes() override;
        void hk_get_line_info(DrawContext* context, int line_number, LineInfo& line_info) override;


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
            m_cursor_set = &AbstractElement::set_selected_all;
        }

        bool hk_build_pre_delimiter_chars(DrawContext* context) override;
        bool hk_build_post_delimiter_chars(DrawContext* context) override;
        // void hk_update_line_info(DrawContext* context) override;
        // bool hk_build_main(DrawContext* context) override;
        bool hk_build_chars(DrawContext* context) override;
        bool hk_build_hlayout(DrawContext* context) override;
        bool hk_build_vlayout(DrawContext* context, int line_number = -1) override;

        // bool hk_draw_main(DrawContext* context) override;
        void hk_draw_text_cursor(DrawContext* context) override;
    };

    struct HiddenSpace : public AbstractLeafBlock {
        HiddenSpace() : AbstractLeafBlock() {
            m_category = C_BLOCK;
            m_type = T_BLOCK_HIDDENSPACE;
        }
        bool hk_build_chars(DrawContext* context) override;
        // bool hk_draw_main(DrawContext* context) override;
        bool add_chars(WrapColumn* wrap_chars) override;
    };
    struct HrBlock : public AbstractLeafBlock {
        HrBlock() : AbstractLeafBlock() {
            m_category = C_BLOCK;
            m_type = T_BLOCK_HR;
        }
        // bool hk_build_chars(float x_offset);
        // bool add_chars(std::vector<WrapCharPtr>& wrap_chars) override;
    };
}