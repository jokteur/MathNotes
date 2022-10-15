#pragma once

#include <string_view>

#include "markdown.h"
#include "wrapper.h"
#include "ui/drawable.h"

namespace RichText {
    class RichTextWidget : public Drawable {
    private:
        float m_current_width = 0.f;
        MarkdownToWidgets m_md_to_widgets;

        SafeString m_safe_string = nullptr;

        std::vector<AbstractWidgetPtr> m_tree;

    public:
        RichTextWidget(std::shared_ptr<UIState> ui_state);

        void setText(const std::string& text);
        void FrameUpdate() override;
    };
}