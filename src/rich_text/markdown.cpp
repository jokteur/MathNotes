#include "markdown.h"

#include "blocks/paragraph.h"
#include "blocks/header.h"
#include "blocks/lists.h"
#include "blocks/table.h"

#include "spans/latex.h"
#include "spans/text.h"



namespace RichText {
    MarkdownToWidgets::MarkdownToWidgets(unsigned md_flags) {
        m_md.abi_version = 0;
        m_md.flags = md_flags;

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

    int MarkdownToWidgets::text(MD_TEXTTYPE type, const char* str, const char* str_end) {
        switch (type) {
        case MD_TEXT_NORMAL:
            break;
        case MD_TEXT_CODE:
            break;
        case MD_TEXT_NULLCHAR:
            break;
        case MD_TEXT_BR:
            break;
        case MD_TEXT_SOFTBR:
            break;
        case MD_TEXT_ENTITY:
            break;
        case MD_TEXT_HTML:
            break;
        case MD_TEXT_LATEXMATH:
            break;
        default:
            break;
        }
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
    void MarkdownToWidgets::end_block() {
        if (m_current_ptr->parent != nullptr) {
            m_current_ptr = m_current_ptr->parent;
        }
    }

    void MarkdownToWidgets::BLOCK_DOC(bool enter) {
        if (enter) {
            auto& root = std::make_shared<RootNode>();
            push_to_tree(std::static_pointer_cast<AbstractWidget>(root));
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
            auto& header = std::make_shared<HeaderWidget>();
            header->hlevel = detail->level;
            push_to_tree(std::static_pointer_cast<AbstractWidget>(header));
        }
        else {
            m_hlevel = 0;
            end_block();
        }
    }
    void MarkdownToWidgets::BLOCK_QUOTE(bool enter) {
        if (enter) {
            auto& quote = std::make_shared<QuoteWidget>();
            if (m_current_ptr->type == WidgetsTypes::BLOCK_QUOTE) {
                auto& new_ptr = std::static_pointer_cast<QuoteWidget>(m_current_ptr);
                quote->quote_level = new_ptr->quote_level + 1;
            }
            push_to_tree(std::static_pointer_cast<AbstractWidget>(quote));
        }
        else {
            end_block();
        }

    }
    void MarkdownToWidgets::BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool enter) {
        m_is_code = enter;
        if (enter) {
            auto& code = std::make_shared<CodeWidget>();
            push_to_tree(std::static_pointer_cast<AbstractWidget>(code));
        }
        else {
            end_block();
        }
    }

    void MarkdownToWidgets::BLOCK_HTML(bool) {
    }

    void MarkdownToWidgets::BLOCK_P(bool enter) {
        if (enter) {
            auto& p = std::make_shared<ParagraphWidget>();
            push_to_tree(std::static_pointer_cast<AbstractWidget>(p));
        }
        else {
            end_block();
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
    int MarkdownToWidgets::parse(const std::string& raw_text) {
        m_tree.clear();
        m_text = raw_text.c_str();
        m_text_size = raw_text.size();
        m_current_ptr = nullptr;
        md_parse(m_text, m_text_size, &m_md, this);
        return 0;
    }
}