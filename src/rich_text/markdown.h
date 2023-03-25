#pragma once

#include <string>

// #include <md4c.h>
// #include <md4c-html.h>


#include <ab_parser.h>
#include "element.h"
#include "ui/fonts.h"
#include "markdown_config.h"
#include "rich_text_context.h"

namespace RichText {
    // Inspired from https://github.com/mekhontsev/imgui_md

    void str_from_text_boundaries(const std::string& raw_text, std::string& str, const std::vector<AB::Boundaries>& text_boundaries);
    struct RootBlock {
        int line_start;
        int line_end;
        Type type;
    };


    class ABToWidgets {
    private:
        AB::Parser m_parser;
        std::vector<RootBlock> m_tree;
        MarkdownConfig m_config;

        RichTextInfo* m_rt_info;
        std::string m_text;
        int m_text_start_idx = 0;
        int m_text_end_idx = 0;
        int m_text_size;

        int level = 0;

        SimpleWidgetPtr m_current_ptr = nullptr;
        SimpleWidgetPtr m_last_text_ptr = nullptr;
        SimpleWidgetPtr m_last_block_ptr = nullptr;

        UIState_ptr m_ui_state = nullptr;

        std::string m_href;

        int text(AB::TEXT_TYPE t_type, const std::vector<AB::Boundaries>& bounds);
        int block(AB::BLOCK_TYPE type, bool enter, const std::vector<AB::Boundaries>& bounds = {}, const AB::Attributes& attributes = {}, AB::BlockDetailPtr detail = nullptr);
        int span(AB::SPAN_TYPE type, bool enter, const std::vector<AB::Boundaries>& bounds = {}, const AB::Attributes& attributes = {}, AB::SpanDetailPtr detail = nullptr);

        void configure_parser();
    public:
        ABToWidgets();

        std::vector<RootBlock> parse(const std::string& str, int start, int end, UIState_ptr ui_state, MarkdownConfig config = MarkdownConfig());
    };
}