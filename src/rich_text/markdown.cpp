#include <cmath>

#include "markdown.h"

#include "blocks/paragraph.h"
#include "blocks/header.h"
#include "blocks/lists.h"
#include "blocks/table.h"

#include "spans/latex.h"
#include "spans/text.h"



namespace RichText {
    void MarkdownToWidgets::configure_parser() {
        m_md.abi_version = 0;

        m_md.enter_block = [](MD_BLOCKTYPE t, void* detail, void* u) {
            return ((MarkdownToWidgets*)u)->block(t, detail, true);
        };
        m_md.leave_block = [](MD_BLOCKTYPE t, void* detail, void* u) {
            return ((MarkdownToWidgets*)u)->block(t, detail, false);
        };
        m_md.enter_span = [](MD_SPANTYPE t, void* detail, void* u) {
            return ((MarkdownToWidgets*)u)->span(t, detail, true);
        };
        m_md.leave_span = [](MD_SPANTYPE t, void* detail, void* u) {
            return ((MarkdownToWidgets*)u)->span(t, detail, false);
        };
        m_md.text = [](MD_TEXTTYPE t, const MD_CHAR* text, MD_SIZE size, void* u) {
            return ((MarkdownToWidgets*)u)->text(t, text, text + size);
        };
        m_md.debug_log = nullptr;
        m_md.syntax = nullptr;
    }
    MarkdownToWidgets::MarkdownToWidgets(unsigned md_flags) {
        m_md.flags = md_flags;
        configure_parser();
    }
    MarkdownToWidgets::MarkdownToWidgets() {
        m_md.flags = 0;
        m_md.flags |= MD_FLAG_LATEXMATHSPANS | MD_FLAG_PERMISSIVEAUTOLINKS;
        m_md.flags |= MD_FLAG_PERMISSIVEURLAUTOLINKS | MD_FLAG_PERMISSIVEWWWAUTOLINKS;
        m_md.flags |= MD_FLAG_STRIKETHROUGH | MD_FLAG_TABLES | MD_FLAG_TASKLISTS;
        m_md.flags |= MD_FLAG_UNDERLINE | MD_FLAG_WIKILINKS;
        configure_parser();
    }
    void MarkdownToWidgets::setFlags(unsigned md_flags) {
        m_md.flags = md_flags;
    }

    int MarkdownToWidgets::text(MD_TEXTTYPE type, const char* str, const char* str_end) {
        m_text_start_idx = (int)(str - m_text);
        m_text_end_idx = (int)(str_end - m_text);

        using namespace Fonts;
        m_font = FontRequestInfo();
        m_font.auto_scaling = true;
        m_font.font_styling = FontStyling{ F_REGULAR, W_REGULAR, S_NORMAL };
        if (m_is_code)
            m_font.font_styling.family = F_MONOSPACE;
        if (m_is_em)
            m_font.font_styling.style = S_ITALIC;
        if (m_is_strong)
            m_font.font_styling.weight = W_MEDIUM;

        AbstractWidgetPtr ptr;
        if (type == MD_TEXT_LATEXMATH) {

        }
        else if (type == MD_TEXT_HTML) {

        }
        else if (type == MD_TEXT_ENTITY) {

        }
        else {
            auto span = std::make_shared<TextString>(m_ui_state);
            if (m_hlevel > 0)
                span->font_styling.size_wish = calculate_text_size();
            else
                span->font_styling.size_wish = m_base_font_size;
            span->text_begin = m_text_start_idx;
            span->text_end = m_text_end_idx;
            span->safe_string = m_safe_text;
            ptr = std::static_pointer_cast<AbstractWidget>(span);
        }

        push_to_tree(ptr);
        tree_up();
        return 0;
    }
    int MarkdownToWidgets::block(MD_BLOCKTYPE type, void* detail, bool enter) {
        switch (type) {
        case MD_BLOCK_DOC:
            BLOCK_DOC(enter);
            break;
        case MD_BLOCK_QUOTE:
            BLOCK_QUOTE(enter);
            break;
        case MD_BLOCK_UL:
            BLOCK_UL((MD_BLOCK_UL_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_OL:
            BLOCK_OL((MD_BLOCK_OL_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_LI:
            BLOCK_LI((MD_BLOCK_LI_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_HR:
            BLOCK_HR(enter);
            break;
        case MD_BLOCK_H:
            BLOCK_H((MD_BLOCK_H_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_CODE:
            BLOCK_CODE((MD_BLOCK_CODE_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_HTML:
            BLOCK_HTML(enter);
            break;
        case MD_BLOCK_P:
            BLOCK_P(enter);
            break;
        case MD_BLOCK_TABLE:
            BLOCK_TABLE((MD_BLOCK_TABLE_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_THEAD:
            BLOCK_THEAD(enter);
            break;
        case MD_BLOCK_TBODY:
            BLOCK_TBODY(enter);
            break;
        case MD_BLOCK_TR:
            BLOCK_TR(enter);
            break;
        case MD_BLOCK_TH:
            BLOCK_TH((MD_BLOCK_TD_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_TD:
            BLOCK_TD((MD_BLOCK_TD_DETAIL*)detail, enter);
            break;
        default:
            break;
        }
        return 0;
    }
    int MarkdownToWidgets::span(MD_SPANTYPE type, void* detail, bool enter) {
        switch (type) {
        case MD_SPAN_EM:
            SPAN_EM(enter);
            break;
        case MD_SPAN_STRONG:
            SPAN_STRONG(enter);
            break;
        case MD_SPAN_A:
            SPAN_A((MD_SPAN_A_DETAIL*)detail, enter);
            break;
        case MD_SPAN_IMG:
            SPAN_IMG((MD_SPAN_IMG_DETAIL*)detail, enter);
            break;
        case MD_SPAN_CODE:
            SPAN_CODE(enter);
            break;
        case MD_SPAN_DEL:
            SPAN_DEL(enter);
            break;
        case MD_SPAN_LATEXMATH:
            SPAN_LATEXMATH(enter);
            break;
        case MD_SPAN_LATEXMATH_DISPLAY:
            SPAN_LATEXMATH_DISPLAY(enter);
            break;
        case MD_SPAN_WIKILINK:
            SPAN_WIKILINK((MD_SPAN_WIKILINK_DETAIL*)detail, enter);
            break;
        case MD_SPAN_U:
            SPAN_U(enter);
            break;
        default:
            break;
        }
        return 0;
    }

    void MarkdownToWidgets::set_href(bool enter, const MD_ATTRIBUTE& src) {
        if (enter) {
            m_href.assign(src.text, src.size);
        }
        else {
            m_href.clear();
        }
    }
    void MarkdownToWidgets::tree_up() {
        if (m_current_ptr->parent != nullptr) {
            m_current_ptr = m_current_ptr->parent;
        }
    }

    float MarkdownToWidgets::calculate_text_size() {
        return round(2 + m_base_font_size * ((6 - m_hlevel) / 5 * 2));
    }
    void MarkdownToWidgets::make_p(MD_TEXTTYPE type) {
        auto ptr = std::static_pointer_cast<HeaderWidget>(m_current_ptr);
        m_font.size_wish = m_base_font_size;
    }

    void MarkdownToWidgets::BLOCK_DOC(bool enter) {
        if (enter) {
            auto root = std::make_shared<RootNode>(m_ui_state);
            auto ptr = std::static_pointer_cast<AbstractWidget>(root);
            push_to_tree(ptr);
        }
    }
    void MarkdownToWidgets::BLOCK_UL(const MD_BLOCK_UL_DETAIL* detail, bool enter) {
    }

    void MarkdownToWidgets::BLOCK_OL(const MD_BLOCK_OL_DETAIL* detail, bool enter) {
    }

    void MarkdownToWidgets::BLOCK_LI(const MD_BLOCK_LI_DETAIL*, bool enter) {
    }

    void MarkdownToWidgets::BLOCK_HR(bool enter) {
    }

    void MarkdownToWidgets::BLOCK_H(const MD_BLOCK_H_DETAIL* detail, bool enter) {
        if (enter) {
            m_hlevel = detail->level;
            auto header = std::make_shared<HeaderWidget>(m_ui_state);
            header->hlevel = detail->level;
            auto ptr = std::static_pointer_cast<AbstractWidget>(header);
            push_to_tree(ptr);
        }
        else {
            m_hlevel = 0;
            tree_up();
        }
    }
    void MarkdownToWidgets::BLOCK_QUOTE(bool enter) {
        if (enter) {
            auto quote = std::make_shared<QuoteWidget>(m_ui_state);
            if (m_current_ptr->type == T_BLOCK_QUOTE) {
                auto new_ptr = std::static_pointer_cast<QuoteWidget>(m_current_ptr);
                quote->quote_level = new_ptr->quote_level + 1;
            }
            auto ptr = std::static_pointer_cast<AbstractWidget>(quote);
            push_to_tree(ptr);
        }
        else {
            tree_up();
        }

    }
    void MarkdownToWidgets::BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool enter) {
        m_is_code = enter;
        if (enter) {
            auto code = std::make_shared<CodeWidget>(m_ui_state);
            auto ptr = std::static_pointer_cast<AbstractWidget>(code);
            push_to_tree(ptr);
        }
        else {
            tree_up();
        }
    }

    void MarkdownToWidgets::BLOCK_HTML(bool) {
    }

    void MarkdownToWidgets::BLOCK_P(bool enter) {
        if (enter) {
            auto p = std::make_shared<ParagraphWidget>(m_ui_state);
            auto ptr = std::static_pointer_cast<AbstractWidget>(p);
            push_to_tree(ptr);
        }
        else {
            tree_up();
        }
    }
    void MarkdownToWidgets::BLOCK_TABLE(const MD_BLOCK_TABLE_DETAIL*, bool enter) {
    }
    void MarkdownToWidgets::BLOCK_THEAD(bool enter) {
        m_is_table_header = enter;
    }
    void MarkdownToWidgets::BLOCK_TBODY(bool enter) {
        m_is_table_body = enter;
    }
    void MarkdownToWidgets::BLOCK_TR(bool enter) {
    }
    void MarkdownToWidgets::BLOCK_TH(const MD_BLOCK_TD_DETAIL* detail, bool enter) {
        BLOCK_TD(detail, enter);
    }
    void MarkdownToWidgets::BLOCK_TD(const MD_BLOCK_TD_DETAIL*, bool enter) {
    }

    void MarkdownToWidgets::SPAN_A(const MD_SPAN_A_DETAIL* detail, bool enter) {
        set_href(enter, detail->href);
    }
    void MarkdownToWidgets::SPAN_EM(bool enter) {
        m_is_em = enter;
    }
    void MarkdownToWidgets::SPAN_STRONG(bool enter) {
        m_is_strong = enter;
    }
    void MarkdownToWidgets::SPAN_IMG(const MD_SPAN_IMG_DETAIL* detail, bool enter) {
        // TODO
    }
    void MarkdownToWidgets::SPAN_CODE(bool enter) {
        m_is_code = enter;
    }
    void MarkdownToWidgets::SPAN_LATEXMATH(bool enter) {
        m_is_latex = enter;
        m_is_inline = enter;
    }
    void MarkdownToWidgets::SPAN_LATEXMATH_DISPLAY(bool enter) {
        m_is_latex = enter;
    }
    void MarkdownToWidgets::SPAN_WIKILINK(const MD_SPAN_WIKILINK_DETAIL*, bool) {
    }
    void MarkdownToWidgets::SPAN_U(bool enter) {
        m_is_underline = enter;
    }
    void MarkdownToWidgets::SPAN_DEL(bool enter) {
        m_is_strikethrough = enter;
    }
    std::vector<AbstractWidgetPtr> MarkdownToWidgets::parse(const SafeString& str, UIState_ptr ui_state) {
        m_tree.clear();
        m_safe_text = str;
        m_text = str->c_str();
        m_text_size = str->size();
        m_current_ptr = nullptr;
        m_ui_state = ui_state;
        md_parse(m_text, m_text_size, &m_md, this);
        return m_tree;
    }
}