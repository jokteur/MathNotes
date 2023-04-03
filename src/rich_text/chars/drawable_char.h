#pragma once 
#include "rich_text/wrapper.h"
#include "ui/draw_commands.h"
#include "ui/state.h"
#include "types.h"

namespace RichText {
    struct DrawableChar: public WrapCharacter {
        bool virtual draw(Draw::DrawList& draw_list, const Rect& boundaries, ImVec2 draw_offset = ImVec2(0.f, 0.f)) = 0;
    };

    struct NewLine: DrawableChar {
        NewLine() {
            is_linebreak = true;
        }
        bool draw(Draw::DrawList& draw_list, const Rect& boundaries, ImVec2 draw_offset = ImVec2(0.f, 0.f)) override;
    };

    using DrawableCharPtr = std::shared_ptr<DrawableChar>;
}