#pragma once

#include <string_view>

#include "markdown.h"
#include "wrapper.h"
#include "ui/drawable.h"

namespace RichText {
    class RichTextWidget : public Drawable {
    private:
        float m_current_width = 0.f;
        WrapAlgorithm m_wrapper;
        MarkdownToWidgets m_md_to_widgets;

        SafeString m_safe_string = nullptr;

        bool m_chars_dirty = false;
        bool m_widget_dirty = false;
        int m_number_of_attempts = 0;
        std::vector<WrapCharPtr> m_chars;
        std::vector<AbstractWidgetPtr> m_tree;

        void start_build_chars();
        void build_chars();
    public:
        RichTextWidget(std::shared_ptr<UIState> ui_state);

        void setText(const std::string& text);
        void FrameUpdate() override;
    };
}