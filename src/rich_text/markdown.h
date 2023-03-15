#pragma once

#include <string>

// #include <md4c.h>
// #include <md4c-html.h>


#include <ab_parser.h>
#include "widgets.h"
#include "ui/fonts.h"
#include "markdown_config.h"
#include "rich_text_context.h"

namespace RichText {
    // Inspired from https://github.com/mekhontsev/imgui_md

    void str_from_text_boundaries(const std::string& raw_text, std::string& str, const std::vector<AB::Boundaries>& text_boundaries);

    class MarkdownToWidgets {
    private:
        AB::Parser m_parser;
        std::vector<AbstractWidgetPtr> m_tree;
        MarkdownConfig m_config;

        const char* m_text;
        SafeString m_safe_text;
        RichTextInfo* m_rt_info;
        int m_text_start_idx = 0;
        int m_text_end_idx = 0;
        int m_text_size;

        AbstractWidgetPtr m_current_ptr = nullptr;
        AbstractWidgetPtr m_last_text_ptr = nullptr;
        AbstractWidgetPtr m_last_block_ptr = nullptr;

        UIState_ptr m_ui_state = nullptr;

        std::string m_href;

        int text(AB::TEXT_TYPE t_type, const std::vector<AB::Boundaries>& bounds);
        int block(AB::BLOCK_TYPE type, bool enter, const std::vector<AB::Boundaries>& bounds = {}, const AB::Attributes& attributes = {}, AB::BlockDetailPtr detail = nullptr);
        int span(AB::SPAN_TYPE type, bool enter, const std::vector<AB::Boundaries>& bounds = {}, const AB::Attributes& attributes = {}, AB::SpanDetailPtr detail = nullptr);

        void push_to_tree(AbstractWidgetPtr& node);
        void set_href(bool enter, const std::string& src);
        void tree_up();
        void set_infos(MarkdownConfig::type type, AbstractWidgetPtr ptr, bool special_style = false);

        void BLOCK_DOC(bool);
        AbstractWidgetPtr BLOCK_QUOTE(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractWidgetPtr BLOCK_UL(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::BlockUlDetail&);
        AbstractWidgetPtr BLOCK_OL(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::BlockOlDetail&);
        AbstractWidgetPtr BLOCK_LI(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::BlockLiDetail&);
        AbstractWidgetPtr BLOCK_HR(bool e, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractWidgetPtr BLOCK_H(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::BlockHDetail& d);
        AbstractWidgetPtr BLOCK_CODE(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::BlockCodeDetail&);
        AbstractWidgetPtr BLOCK_P(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractWidgetPtr BLOCK_HIDDENSPACE(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);

        AbstractWidgetPtr SPAN_EM(bool e, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractWidgetPtr SPAN_STRONG(bool e, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractWidgetPtr SPAN_A(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::SpanADetail&);
        AbstractWidgetPtr SPAN_IMG(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&, const AB::SpanImgDetail&);
        AbstractWidgetPtr SPAN_CODE(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractWidgetPtr SPAN_DEL(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractWidgetPtr SPAN_HIGHLIGHT(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        AbstractWidgetPtr SPAN_LATEXMATH(bool, const std::vector<AB::Boundaries>&, const AB::Attributes&);
        // AbstractWidgetPtr SPAN_WIKILINK(const MD_SPAN_WIKILINK_DETAIL*, bool, int mark_begin, int mark_end);
        // AbstractWidgetPtr SPAN_U(bool, int mark_begin, int mark_end);

        void configure_parser();
    public:
        MarkdownToWidgets();

        std::vector<AbstractWidgetPtr> parse(const SafeString& str, UIState_ptr ui_state, RichTextInfo* rt_info, MarkdownConfig config = MarkdownConfig());
    };

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

        const char* m_text;
        SafeString m_safe_text;
        RichTextInfo* m_rt_info;
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

        std::vector<RootBlock> parse(const SafeString& str, UIState_ptr ui_state, RichTextInfo* rt_info, MarkdownConfig config = MarkdownConfig());
    };
}