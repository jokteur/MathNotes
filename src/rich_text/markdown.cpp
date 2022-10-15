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

        AbstractWidgetPtr ptr;
        if (type == MD_TEXT_LATEXMATH) {

        }
        else if (type == MD_TEXT_HTML) {

        }
        else if (type == MD_TEXT_ENTITY) {

        }
        else {
            using namespace Fonts;
            m_font.auto_scaling = true;
            auto span = std::make_shared<TextString>(m_ui_state);
            span->m_font_request = m_font;
            // If parent is header, font inherints header properties
            if (m_current_ptr->m_type == T_BLOCK_H) {
                span->m_font_request = m_current_ptr->m_font_request;
            }

            // Span properties
            if (m_is_code) {
                set_font_infos(MarkdownConfig::CODE, std::static_pointer_cast<AbstractWidget>(span));
            }
            if (m_is_em) {
                MarkdownConfig::make_em(span->m_font_request.font_styling);
            }
            if (m_is_strong) {
                MarkdownConfig::make_bold(span->m_font_request.font_styling);
            }
            if (m_is_underline) {
                span->m_font_underline = true;
            }
            // Override set_font_info on the size
            span->m_font_request.size_wish = m_config.font_sizes[m_hlevel];
            span->m_raw_text_begin = m_text_start_idx;
            span->m_raw_text_end = m_text_end_idx;
            span->m_safe_string = m_safe_text;
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
            m_color = m_config.default_colors[MarkdownConfig::HREF];
        }
        else {
            m_href.clear();
            m_color = m_config.default_colors[MarkdownConfig::P];
        }
    }
    void MarkdownToWidgets::tree_up() {
        if (m_current_ptr->m_parent != nullptr) {
            m_current_ptr = m_current_ptr->m_parent;
        }
    }
    void MarkdownToWidgets::set_font_infos(MarkdownConfig::type type, AbstractWidgetPtr ptr) {
        ptr->m_font_request.font_styling = m_config.font_stylings[type];
        ptr->m_font_request.size_wish = m_config.font_sizes[type];
        ptr->m_line_space = m_config.line_spaces[type];
        ptr->m_font_underline = m_config.font_underlines[type];
        ptr->m_font_color = m_config.default_colors[type];
        ptr->m_bg_color = m_config.bg_colors[type];
    }
    void MarkdownToWidgets::make_p(MD_TEXTTYPE type) {
        auto ptr = std::static_pointer_cast<HeaderWidget>(m_current_ptr);
        set_font_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractWidget>(ptr));
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
            auto header = std::make_shared<HeaderWidget>(m_ui_state);
            m_hlevel = detail->level;
            header->hlevel = detail->level;
            set_font_infos((MarkdownConfig::type)m_hlevel, std::static_pointer_cast<AbstractWidget>(header));
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
            if (m_current_ptr->m_type == T_BLOCK_QUOTE) {
                auto parent = std::static_pointer_cast<QuoteWidget>(m_current_ptr);
                quote->quote_level = parent->quote_level + 1;
                set_font_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractWidget>(quote));
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
            set_font_infos(MarkdownConfig::CODE, std::static_pointer_cast<AbstractWidget>(code));
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
            set_font_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractWidget>(p));
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
    std::vector<AbstractWidgetPtr> MarkdownToWidgets::parse(const SafeString& str, UIState_ptr ui_state, MarkdownConfig config) {
        m_tree.clear();
        m_safe_text = str;
        m_text = str->c_str();
        m_text_size = str->size();
        m_current_ptr = nullptr;
        m_config = config;
        m_ui_state = ui_state;
        md_parse(m_text, m_text_size, &m_md, this);
        return m_tree;
    }
}