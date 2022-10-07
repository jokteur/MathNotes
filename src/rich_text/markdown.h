#pragma once

#include <string>

#include <md4c.h>
#include <md4c-html.h>

#include "widgets.h"
#include "fonts.h"

namespace RichText {
    // Inspired from https://github.com/mekhontsev/imgui_md
    class MarkdownToWidgets {
    private:
        MD_PARSER m_md;
        std::vector<AbstractWidgetPtr> m_tree;
        const char* m_text;
        int m_text_start_idx = 0;
        int m_text_end_idx = 0;
        int m_text_size;
        float m_base_font_size = 16.f;
        Fonts::FontRequestInfo m_font;

        AbstractWidgetPtr m_current_ptr = nullptr;

        // current state
        bool m_is_underline = false;
        bool m_is_strikethrough = false;
        bool m_is_em = false;
        bool m_is_strong = false;
        bool m_is_table_header = false;
        bool m_is_table_body = false;
        bool m_is_image = false;
        bool m_is_latex = false;
        bool m_is_inline = false;
        bool m_is_code = false;
        std::string m_href;
        unsigned m_hlevel = 0;//0 - no heading

        int text(MD_TEXTTYPE type, const char* str, const char* str_end);
        int block(MD_BLOCKTYPE type, void* detail, bool enter);
        int span(MD_SPANTYPE type, void* detail, bool enter);

        void reset_properties();

        void push_to_tree(AbstractWidgetPtr& node) {
            m_tree.push_back(node);
            if (m_current_ptr != nullptr) m_current_ptr->childrens.push_back(node);
            m_current_ptr = node;
        }
        void set_href(bool enter, const MD_ATTRIBUTE& src);
        void end_block();

        void make_header(MD_TEXTTYPE type);
        void make_quote(MD_TEXTTYPE type);
        void make_list_el(MD_TEXTTYPE type);
        void make_hr(MD_TEXTTYPE type);
        void make_code(MD_TEXTTYPE type);
        void make_p(MD_TEXTTYPE type);
        void make_table(MD_TEXTTYPE type);

        void BLOCK_DOC(bool);
        void BLOCK_QUOTE(bool);
        void BLOCK_UL(const MD_BLOCK_UL_DETAIL*, bool);
        void BLOCK_OL(const MD_BLOCK_OL_DETAIL*, bool);
        void BLOCK_LI(const MD_BLOCK_LI_DETAIL*, bool);
        void BLOCK_HR(bool e);
        void BLOCK_H(const MD_BLOCK_H_DETAIL* d, bool e);
        void BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool);
        void BLOCK_HTML(bool);
        void BLOCK_P(bool);
        void BLOCK_TABLE(const MD_BLOCK_TABLE_DETAIL*, bool);
        void BLOCK_THEAD(bool);
        void BLOCK_TBODY(bool);
        void BLOCK_TR(bool);
        void BLOCK_TH(const MD_BLOCK_TD_DETAIL*, bool);
        void BLOCK_TD(const MD_BLOCK_TD_DETAIL*, bool);

        void SPAN_EM(bool e);
        void SPAN_STRONG(bool e);
        void SPAN_A(const MD_SPAN_A_DETAIL* d, bool e);
        void SPAN_IMG(const MD_SPAN_IMG_DETAIL*, bool);
        void SPAN_CODE(bool);
        void SPAN_DEL(bool);
        void SPAN_LATEXMATH(bool);
        void SPAN_LATEXMATH_DISPLAY(bool);
        void SPAN_WIKILINK(const MD_SPAN_WIKILINK_DETAIL*, bool);
        void SPAN_U(bool);
    public:
        MarkdownToWidgets(unsigned md_flags);

        void setBaseFontSize(float size) { m_base_font_size = size; }

        // Returns 0 on success
        int parse(const std::string& raw_text);
    };
}