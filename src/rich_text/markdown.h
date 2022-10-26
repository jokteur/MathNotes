#pragma once

#include <string>

#include <md4c.h>
#include <md4c-html.h>


#include "widgets.h"
#include "ui/fonts.h"
#include "markdown_config.h"

namespace RichText {
    // Inspired from https://github.com/mekhontsev/imgui_md

    class MarkdownToWidgets {
    private:
        MD_PARSER m_md;
        MarkdownConfig m_config;
        std::vector<AbstractWidgetPtr> m_tree;

        const char* m_text;
        SafeString m_safe_text;
        int m_text_start_idx = 0;
        int m_text_end_idx = 0;
        int m_text_size;

        AbstractWidgetPtr m_current_ptr = nullptr;
        AbstractWidgetPtr m_last_block_ptr = nullptr;

        UIState_ptr m_ui_state = nullptr;

        std::string m_href;

        int text(MD_TEXTTYPE type, const char* str, int size, int text_pos);
        int block(MD_BLOCKTYPE type, void* detail, bool enter);
        int span(MD_SPANTYPE type, void* detail, int mark_begin, int mark_end, bool enter);

        // Markers
        void propagate_begins_to_parents(AbstractWidgetPtr& ptr, int pre, int begin);

        void push_to_tree(AbstractWidgetPtr& node);
        void set_href(bool enter, const MD_ATTRIBUTE& src);
        void tree_up();
        void set_infos(MarkdownConfig::type type, AbstractWidgetPtr ptr, bool special_style = false);

        void BLOCK_DOC(bool);
        AbstractWidgetPtr BLOCK_QUOTE(bool);
        AbstractWidgetPtr BLOCK_UL(const MD_BLOCK_UL_DETAIL*, bool);
        AbstractWidgetPtr BLOCK_OL(const MD_BLOCK_OL_DETAIL*, bool);
        AbstractWidgetPtr BLOCK_LI(const MD_BLOCK_LI_DETAIL*, bool);
        AbstractWidgetPtr BLOCK_HR(bool e);
        AbstractWidgetPtr BLOCK_H(const MD_BLOCK_H_DETAIL* d, bool e);
        AbstractWidgetPtr BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool);
        AbstractWidgetPtr BLOCK_HTML(bool);
        AbstractWidgetPtr BLOCK_P(bool);
        AbstractWidgetPtr BLOCK_TABLE(const MD_BLOCK_TABLE_DETAIL*, bool);
        AbstractWidgetPtr BLOCK_THEAD(bool);
        AbstractWidgetPtr BLOCK_TBODY(bool);
        AbstractWidgetPtr BLOCK_TR(bool);
        AbstractWidgetPtr BLOCK_TH(const MD_BLOCK_TD_DETAIL*, bool);
        AbstractWidgetPtr BLOCK_TD(const MD_BLOCK_TD_DETAIL*, bool);

        AbstractWidgetPtr SPAN_EM(bool e, int mark_begin, int mark_end);
        AbstractWidgetPtr SPAN_STRONG(bool e, int mark_begin, int mark_end);
        AbstractWidgetPtr SPAN_A(const MD_SPAN_A_DETAIL* d, bool e, int mark_begin, int mark_end);
        AbstractWidgetPtr SPAN_IMG(const MD_SPAN_IMG_DETAIL*, bool, int mark_begin, int mark_end);
        AbstractWidgetPtr SPAN_CODE(bool, int mark_begin, int mark_end);
        AbstractWidgetPtr SPAN_DEL(bool, int mark_begin, int mark_end);
        AbstractWidgetPtr SPAN_LATEXMATH(bool, int mark_begin, int mark_end);
        AbstractWidgetPtr SPAN_LATEXMATH_DISPLAY(bool, int mark_begin, int mark_end);
        AbstractWidgetPtr SPAN_WIKILINK(const MD_SPAN_WIKILINK_DETAIL*, bool, int mark_begin, int mark_end);
        AbstractWidgetPtr SPAN_U(bool, int mark_begin, int mark_end);

        void configure_parser();
    public:
        MarkdownToWidgets(unsigned md_flags);
        MarkdownToWidgets();

        void setFlags(unsigned md_flags);

        std::vector<AbstractWidgetPtr> parse(const SafeString& str, UIState_ptr ui_state, MarkdownConfig config = MarkdownConfig());
    };
}