#pragma once

#include <string_view>

#include "markdown.h"
#include "wrapper.h"
#include "ui/drawable.h"
#include "ui/draw_commands.h"
#include "rich_text_context.h"

namespace RichText {
    class RichTextWidget: public Drawable {
    private:
        float m_current_width = 0.f;
        bool m_redo_positions = false;
        MarkdownToWidgets m_md_to_widgets;

        SafeString m_safe_string = nullptr;
        RichTextInfo m_info;

        std::vector<AbstractWidgetPtr> m_tree;
        Draw::DrawList m_draw_list;
        float m_y_scroll = 0.f;

    public:
        RichTextWidget(std::shared_ptr<UIState> ui_state);

        void setText(const std::string& text);
        void FrameUpdate() override;
    };
}