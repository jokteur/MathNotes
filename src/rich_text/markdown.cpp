#include <cmath>

#include "markdown.h"

#include "blocks/paragraph.h"
#include "blocks/header.h"
#include "blocks/lists.h"
#include "blocks/table.h"

#include "spans/latex.h"
#include "spans/text.h"
#include "spans/inline_spans.h"
#include "spans/special_spans.h"

namespace RichText {
    void MarkdownToWidgets::configure_parser() {
        m_md.abi_version = 0;

        m_md.enter_block = [](MD_BLOCKTYPE t, void* detail, void* u) {
            return ((MarkdownToWidgets*)u)->block(t, detail, true);
        };
        m_md.leave_block = [](MD_BLOCKTYPE t, void* detail, void* u) {
            return ((MarkdownToWidgets*)u)->block(t, detail, false);
        };
        m_md.enter_span = [](MD_SPANTYPE t, void* detail, MD_SIZE mark_begin, MD_SIZE mark_end, void* u) {
            return ((MarkdownToWidgets*)u)->span(t, detail, mark_begin, mark_end, true);
        };
        m_md.leave_span = [](MD_SPANTYPE t, void* detail, MD_SIZE mark_begin, MD_SIZE mark_end, void* u) {
            return ((MarkdownToWidgets*)u)->span(t, detail, mark_begin, mark_end, false);
        };
        m_md.text = [](MD_TEXTTYPE t, const MD_CHAR* text, MD_SIZE size, MD_SIZE text_pos, void* u) {
            return ((MarkdownToWidgets*)u)->text(t, text, size, text_pos);
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
    int MarkdownToWidgets::text(MD_TEXTTYPE type, const char* str, int size, int text_pos) {
        int prev_text_end_idx = m_text_end_idx;
        m_text_start_idx = text_pos;
        m_text_end_idx = m_text_start_idx + size;

        if (type == MD_TEXT_LATEXMATH) {
        }
        else if (type == MD_TEXT_HTML) {

        }
        else if (type == MD_TEXT_ENTITY) {

        }
        else {
            using namespace Fonts;
            auto span = std::make_shared<TextString>(m_ui_state);
            span->m_processed_text.append(str, size);

            // raw text infos
            span->m_raw_text_info.pre = prev_text_end_idx;
            span->m_raw_text_info.begin = m_text_start_idx;
            span->m_raw_text_info.end = m_text_end_idx;
            span->m_raw_text_info.post = m_text_end_idx;
            span->m_safe_string = m_safe_text;

            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractWidget>(span));

            auto ptr = std::static_pointer_cast<AbstractWidget>(span);
            push_to_tree(ptr);
            tree_up();
        }
        return 0;
    }
    int MarkdownToWidgets::block(MD_BLOCKTYPE type, void* detail, bool enter) {
        AbstractWidgetPtr ptr = nullptr;
        switch (type) {
        case MD_BLOCK_DOC:
            BLOCK_DOC(enter);
            break;
        case MD_BLOCK_QUOTE:
            ptr = BLOCK_QUOTE(enter);
            break;
        case MD_BLOCK_UL:
            ptr = BLOCK_UL((MD_BLOCK_UL_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_OL:
            ptr = BLOCK_OL((MD_BLOCK_OL_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_LI:
            ptr = BLOCK_LI((MD_BLOCK_LI_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_HR:
            ptr = BLOCK_HR(enter);
            break;
        case MD_BLOCK_H:
            ptr = BLOCK_H((MD_BLOCK_H_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_CODE:
            ptr = BLOCK_CODE((MD_BLOCK_CODE_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_HTML:
            ptr = BLOCK_HTML(enter);
            break;
        case MD_BLOCK_P:
            ptr = BLOCK_P(enter);
            break;
        case MD_BLOCK_TABLE:
            ptr = BLOCK_TABLE((MD_BLOCK_TABLE_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_THEAD:
            ptr = BLOCK_THEAD(enter);
            break;
        case MD_BLOCK_TBODY:
            ptr = BLOCK_TBODY(enter);
            break;
        case MD_BLOCK_TR:
            ptr = BLOCK_TR(enter);
            break;
        case MD_BLOCK_TH:
            ptr = BLOCK_TH((MD_BLOCK_TD_DETAIL*)detail, enter);
            break;
        case MD_BLOCK_TD:
            ptr = BLOCK_TD((MD_BLOCK_TD_DETAIL*)detail, enter);
            break;
        default:
            break;
        }
        if (ptr != nullptr) {
            if (enter) {
                push_to_tree(ptr);
            }
            else {
                tree_up();
            }
        }
        return 0;
    }
    int MarkdownToWidgets::span(MD_SPANTYPE type, void* detail, int mark_begin, int mark_end, bool enter) {
        AbstractWidgetPtr ptr = nullptr;
        switch (type) {
        case MD_SPAN_EM:
            ptr = SPAN_EM(enter, mark_begin, mark_end);
            break;
        case MD_SPAN_STRONG:
            ptr = SPAN_STRONG(enter, mark_begin, mark_end);
            break;
        case MD_SPAN_A:
            ptr = SPAN_A((MD_SPAN_A_DETAIL*)detail, enter, mark_begin, mark_end);
            break;
        case MD_SPAN_IMG:
            ptr = SPAN_IMG((MD_SPAN_IMG_DETAIL*)detail, enter, mark_begin, mark_end);
            break;
        case MD_SPAN_CODE:
            ptr = SPAN_CODE(enter, mark_begin, mark_end);
            break;
        case MD_SPAN_DEL:
            ptr = SPAN_DEL(enter, mark_begin, mark_end);
            break;
        case MD_SPAN_LATEXMATH:
            ptr = SPAN_LATEXMATH(enter, mark_begin, mark_end);
            break;
        case MD_SPAN_LATEXMATH_DISPLAY:
            ptr = SPAN_LATEXMATH_DISPLAY(enter, mark_begin, mark_end);
            break;
        case MD_SPAN_WIKILINK:
            ptr = SPAN_WIKILINK((MD_SPAN_WIKILINK_DETAIL*)detail, enter, mark_begin, mark_end);
            break;
        case MD_SPAN_U:
            ptr = SPAN_U(enter, mark_begin, mark_end);
            break;
        default:
            break;
        }
        if (ptr != nullptr) {
            if (enter) {
                ptr->m_raw_text_info.pre = mark_begin;
                ptr->m_raw_text_info.begin = mark_end;
                m_text_end_idx = mark_end;
                ptr->m_style.h_margins = ImVec2(0.f, 0.f);
                ptr->m_style.v_margins = ImVec2(0.f, 0.f);
                push_to_tree(ptr);
            }
            else {
                ptr->m_raw_text_info.end = mark_begin;
                ptr->m_raw_text_info.post = mark_end;
                m_text_end_idx = mark_end;
                tree_up();
            }
        }

        return 0;
    }

    void MarkdownToWidgets::push_to_tree(AbstractWidgetPtr& node) {
        m_tree.push_back(node);
        node->m_parent = m_current_ptr;
        m_last_widget_ptr = node;
        if (m_current_ptr != nullptr) m_current_ptr->m_childrens.push_back(node);
        m_current_ptr = node;
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
        if (m_current_ptr->m_parent != nullptr) {
            m_current_ptr = m_current_ptr->m_parent;
        }
    }
    void MarkdownToWidgets::set_infos(MarkdownConfig::type type, AbstractWidgetPtr ptr) {
        auto& style = m_config.styles[type];
        ptr->m_safe_string = m_safe_text;
        ptr->m_style = style;
        // Inherit potential properties from parents
#define INHERIT_PROPERTY(name_) \
    if (!style.isset_##name_) { ptr->m_style.name_ = m_current_ptr->m_style.name_;}
        INHERIT_PROPERTY(font_size);
        INHERIT_PROPERTY(font_styling);
        INHERIT_PROPERTY(font_color);
        INHERIT_PROPERTY(font_underline);
        INHERIT_PROPERTY(font_strikethrough);
        INHERIT_PROPERTY(font_em);
        INHERIT_PROPERTY(font_strong);
        INHERIT_PROPERTY(line_space);
        INHERIT_PROPERTY(h_margins);
        INHERIT_PROPERTY(v_margins);
        INHERIT_PROPERTY(h_paddings);
        INHERIT_PROPERTY(v_paddings);
        INHERIT_PROPERTY(bg_color);

        if (style.isset_font_strong && style.font_strong) {
            m_config.make_bold(ptr->m_style.font_styling);
        }
        if (style.isset_font_em && style.font_em) {
            m_config.make_em(ptr->m_style.font_styling);
        }
        if (style.isset_font_monospace && style.font_monospace) {
            m_config.make_monospace(ptr->m_style.font_styling);
        }
    }

    void MarkdownToWidgets::BLOCK_DOC(bool enter) {
        if (enter) {
            auto root = std::make_shared<RootNode>(m_ui_state);
            auto ptr = std::static_pointer_cast<AbstractWidget>(root);
            push_to_tree(ptr);
        }
    }
    AbstractWidgetPtr MarkdownToWidgets::BLOCK_UL(const MD_BLOCK_UL_DETAIL* detail, bool enter) {
        if (enter) {
            auto ul_list = std::make_shared<ULWidget>(m_ui_state);
            if (m_current_ptr->m_type == T_BLOCK_UL) {
                ul_list->list_level = std::static_pointer_cast<ULWidget>(m_current_ptr)->list_level + 1;
            }
            else if (m_current_ptr->m_type == T_BLOCK_OL) {
                ul_list->list_level = std::static_pointer_cast<OLWidget>(m_current_ptr)->list_level + 1;
            }
            ul_list->is_tight = (bool)detail->is_tight;
            ul_list->mark = detail->mark;
            ul_list->m_style.h_margins.x = m_config.x_level_offset;
            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractWidget>(ul_list));
            auto ptr = std::static_pointer_cast<AbstractWidget>(ul_list);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractWidgetPtr MarkdownToWidgets::BLOCK_OL(const MD_BLOCK_OL_DETAIL* detail, bool enter) {
        if (enter) {
            auto ol_list = std::make_shared<OLWidget>(m_ui_state);
            if (m_current_ptr->m_type == T_BLOCK_UL) {
                ol_list->list_level = std::static_pointer_cast<ULWidget>(m_current_ptr)->list_level + 1;
            }
            else if (m_current_ptr->m_type == T_BLOCK_OL) {
                ol_list->list_level = std::static_pointer_cast<OLWidget>(m_current_ptr)->list_level + 1;
            }
            ol_list->is_tight = (bool)detail->is_tight;
            ol_list->start = detail->start;
            ol_list->m_style.h_margins.x = m_config.x_level_offset;
            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractWidget>(ol_list));
            auto ptr = std::static_pointer_cast<AbstractWidget>(ol_list);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractWidgetPtr MarkdownToWidgets::BLOCK_LI(const MD_BLOCK_LI_DETAIL* detail, bool enter) {
        if (enter) {
            auto list_el = std::make_shared<LIWidget>(m_ui_state);
            list_el->is_task = detail->is_task;
            list_el->task_mark = detail->task_mark;
            if (m_current_ptr->m_type == T_BLOCK_UL) {
                list_el->list_level = std::static_pointer_cast<ULWidget>(m_current_ptr)->list_level;
            }
            else if (m_current_ptr->m_type == T_BLOCK_OL) {
                list_el->list_level = std::static_pointer_cast<OLWidget>(m_current_ptr)->list_level;
            }
            list_el->m_style.h_margins.x = m_config.x_level_offset;
            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractWidget>(list_el));
            auto ptr = std::static_pointer_cast<AbstractWidget>(list_el);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractWidgetPtr MarkdownToWidgets::BLOCK_HR(bool enter) {
        return nullptr;
    }

    AbstractWidgetPtr MarkdownToWidgets::BLOCK_H(const MD_BLOCK_H_DETAIL* detail, bool enter) {
        if (enter) {
            auto header = std::make_shared<HeaderWidget>(m_ui_state);
            header->hlevel = detail->level;
            set_infos((MarkdownConfig::type)detail->level, std::static_pointer_cast<AbstractWidget>(header));
            auto ptr = std::static_pointer_cast<AbstractWidget>(header);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractWidgetPtr MarkdownToWidgets::BLOCK_QUOTE(bool enter) {
        if (enter) {
            auto quote = std::make_shared<QuoteWidget>(m_ui_state);
            if (m_current_ptr->m_type == T_BLOCK_QUOTE) {
                auto parent = std::static_pointer_cast<QuoteWidget>(m_current_ptr);
                quote->quote_level = parent->quote_level + 1;
            }
            set_infos(MarkdownConfig::QUOTE, std::static_pointer_cast<AbstractWidget>(quote));
            // quote->m_style.h_margins.x += m_config.x_level_offset;
            auto ptr = std::static_pointer_cast<AbstractWidget>(quote);
            return ptr;
        }
        else {
            return m_current_ptr;
        }

    }
    AbstractWidgetPtr MarkdownToWidgets::BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool enter) {
        if (enter) {
            auto code = std::make_shared<CodeWidget>(m_ui_state);
            auto ptr = std::static_pointer_cast<AbstractWidget>(code);
            set_infos(MarkdownConfig::CODE, ptr);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractWidgetPtr MarkdownToWidgets::BLOCK_HTML(bool) {
        return nullptr;
    }

    AbstractWidgetPtr MarkdownToWidgets::BLOCK_P(bool enter) {
        if (enter) {
            auto p = std::make_shared<ParagraphWidget>(m_ui_state);
            auto ptr = std::static_pointer_cast<AbstractWidget>(p);
            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractWidget>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractWidgetPtr MarkdownToWidgets::BLOCK_TABLE(const MD_BLOCK_TABLE_DETAIL*, bool enter) {
        return nullptr;
    }
    AbstractWidgetPtr MarkdownToWidgets::BLOCK_THEAD(bool enter) {
        return nullptr;
    }
    AbstractWidgetPtr MarkdownToWidgets::BLOCK_TBODY(bool enter) {
        return nullptr;
    }
    AbstractWidgetPtr MarkdownToWidgets::BLOCK_TR(bool enter) {
        return nullptr;
    }
    AbstractWidgetPtr MarkdownToWidgets::BLOCK_TH(const MD_BLOCK_TD_DETAIL* detail, bool enter) {
        BLOCK_TD(detail, enter);
        return nullptr;
    }
    AbstractWidgetPtr MarkdownToWidgets::BLOCK_TD(const MD_BLOCK_TD_DETAIL*, bool enter) {
        return nullptr;
    }

    AbstractWidgetPtr MarkdownToWidgets::SPAN_A(const MD_SPAN_A_DETAIL* detail, bool enter, int mark_begin, int mark_end) {
        set_href(enter, detail->href);
        if (enter) {
            auto p = std::make_shared<LinkSpan>(m_ui_state);
            auto ptr = std::static_pointer_cast<AbstractWidget>(p);
            set_infos(MarkdownConfig::HREF, std::static_pointer_cast<AbstractWidget>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_EM(bool enter, int mark_begin, int mark_end) {
        if (enter) {
            auto p = std::make_shared<EmSpan>(m_ui_state);
            auto ptr = std::static_pointer_cast<AbstractWidget>(p);
            set_infos(MarkdownConfig::EM, std::static_pointer_cast<AbstractWidget>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_STRONG(bool enter, int mark_begin, int mark_end) {
        if (enter) {
            auto p = std::make_shared<StrongSpan>(m_ui_state);
            auto ptr = std::static_pointer_cast<AbstractWidget>(p);
            set_infos(MarkdownConfig::STRONG, std::static_pointer_cast<AbstractWidget>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_IMG(const MD_SPAN_IMG_DETAIL* detail, bool enter, int mark_begin, int mark_end) {
        return nullptr;
    }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_CODE(bool enter, int mark_begin, int mark_end) {
        if (enter) {
            auto p = std::make_shared<StrongSpan>(m_ui_state);
            auto ptr = std::static_pointer_cast<AbstractWidget>(p);
            if (m_current_ptr->m_type == T_BLOCK_CODE)
                set_infos(MarkdownConfig::CODE, std::static_pointer_cast<AbstractWidget>(p));
            else
                set_infos(MarkdownConfig::INLINE_CODE, std::static_pointer_cast<AbstractWidget>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_LATEXMATH(bool enter, int mark_begin, int mark_end) {
        return nullptr;
    }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_LATEXMATH_DISPLAY(bool enter, int mark_begin, int mark_end) {
        return nullptr;
    }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_WIKILINK(const MD_SPAN_WIKILINK_DETAIL*, bool, int mark_begin, int mark_end) {
        return nullptr;
    }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_U(bool enter, int mark_begin, int mark_end) {
        return nullptr;
    }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_DEL(bool enter, int mark_begin, int mark_end) {
        return nullptr;
    }
    std::vector<AbstractWidgetPtr> MarkdownToWidgets::parse(const SafeString& str, UIState_ptr ui_state, MarkdownConfig config) {
        m_text_start_idx = 0;
        m_text_end_idx = 0;
        m_tree.clear();
        m_safe_text = str;
        m_text = str->c_str();
        m_text_size = str->size();
        m_current_ptr = nullptr;
        m_config = config;
        m_ui_state = ui_state;
        md_parse(m_text, m_text_size, &m_md, this);
        if (m_last_widget_ptr != nullptr) {
            // m_last_widget_ptr->m_raw_text_after = m_safe_text->size() - 1;
        }
        return m_tree;
    }
}