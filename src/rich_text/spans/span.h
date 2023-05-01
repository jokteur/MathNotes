#pragma once

#include <tempo.h>
#include <unordered_set>
#include "ab_parser.h"

#include "types.h"
#include "ui/drawable.h"
#include "ui/fonts.h"
#include "ui/draw_commands.h"
#include "ui/colors.h"

#include "rich_text/element.h"

namespace RichText {
    struct AbstractSpan: public AbstractElement {
        AbstractSpan(): AbstractElement() {
            m_category = C_SPAN;
        }
        bool hk_add_pre_chars(std::vector<WrapCharPtr>& wrap_chars);
        bool hk_add_post_chars(std::vector<WrapCharPtr>& wrap_chars);
        bool add_chars(std::vector<WrapCharPtr>& wrap_chars) override;
        void hk_debug_attributes() override;
    };
}