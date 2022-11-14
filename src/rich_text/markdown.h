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
        AbstractWidgetPtr m_last_text_ptr = nullptr;
        AbstractWidgetPtr m_last_block_ptr = nullptr;

        UIState_ptr m_ui_state = nullptr;

        std::string m_href;

        int text(MD_TEXTTYPE type, const char* str, int size, int text_pos);
        int block(MD_BLOCKTYPE type, void* detail, bool enter, int mark_beg, int mark_end);
        int span(MD_SPANTYPE type, void* detail, int mark_begin, int mark_end, bool enter);

        // Markers
        inline int get_line_number(AbstractWidgetPtr& ptr, int text_pos);

        /**
         * @brief Deduces from text_beg and text_end which lines
         * are covered by the widget
         *
         * @param ptr
         * @param text_beg
         * @param text_end
         */
        inline void propagate_lines_to_parents(AbstractWidgetPtr& ptr, int text_beg, int text_end);
        // inline void set_
        /**
         * @brief Tells the parents which line it is using
         *
         * We use this function to build the complete picture of
         * which block is covered by which line
         * @param ptr current pointer
         * @param line_number line number in the raw text
         */
        void propagate_line_to_parents(AbstractWidgetPtr& ptr, int line_number);
        void extend_pre(AbstractWidgetPtr& ptr);
        void create_intertext_widgets(int start, int end);
        void propagate_begin_to_parents(AbstractWidgetPtr ptr, int begin);
        inline void estimate_end_from_child();

        void push_to_tree(AbstractWidgetPtr& node);
        void set_href(bool enter, const MD_ATTRIBUTE& src);
        void tree_up();
        void set_infos(MarkdownConfig::type type, AbstractWidgetPtr ptr, bool special_style = false);

        void tmp_show_tree();

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