#pragma once

#include "markdown.h"
#include "wrapper.h"
#include "ui/drawable.h"

namespace RichText {
    class RichTextWidget : public Drawable {
    private:
        float m_current_width = 0.f;
        WrapAlgorithm m_wrapper;
        MarkdownToWidgets m_md_to_widgets;

        std::string m_raw_text;

        std::vector<WrapCharPtr> m_chars;
        std::vector<AbstractWidgetPtr> m_tree;
    public:
        RichTextWidget(std::shared_ptr<UIState> ui_state);

        void setText(const std::string& text);
        void FrameUpdate() override;
    };
}