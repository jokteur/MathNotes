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
    void str_from_text_boundaries(const std::string& raw_text, std::string& str, const std::vector<AB::Boundaries>& text_boundaries) {
        int j = 0;
        for (auto bound : text_boundaries) {
            if (j > 0)
                str += '\n';
            for (int i = bound.beg;i < bound.end;i++) {
                str += raw_text[i];
            }
            j++;
        }
    }

    void MarkdownToWidgets::configure_parser() {
        m_parser.enter_block = [&](AB::BLOCK_TYPE b_type, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, AB::BlockDetailPtr detail) -> bool {
            return this->block(b_type, true, bounds, attributes, detail);
        };
        m_parser.leave_block = [&](AB::BLOCK_TYPE b_type) -> bool {
            return this->block(b_type, false);
        };
        m_parser.enter_span = [&](AB::SPAN_TYPE s_type, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, AB::SpanDetailPtr detail) {
            return this->span(s_type, true, bounds, attributes, detail);
        };
        m_parser.leave_span = [&](AB::SPAN_TYPE s_type) {
            return this->span(s_type, false);
        };
        m_parser.text = [&](AB::TEXT_TYPE t_type, const std::vector<AB::Boundaries>& bounds) {
            return this->text(t_type, bounds);
        };
    }
    MarkdownToWidgets::MarkdownToWidgets() {
        configure_parser();
    }
    int MarkdownToWidgets::text(AB::TEXT_TYPE t_type, const std::vector<AB::Boundaries>& bounds) {
        if (t_type == AB::TEXT_LATEX) {
        }
        else if (t_type == AB::TEXT_CODE) {

        }
        else {
            using namespace Fonts;
            auto text = std::make_shared<TextString>(m_ui_state);
            text->m_text_boundaries = bounds;
            str_from_text_boundaries(*m_safe_text, text->m_processed_text, bounds);
            // text->m_processed_text.append(str, size);


            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractWidget>(text));

            auto ptr = std::static_pointer_cast<AbstractWidget>(text);

            // Text are separated line by line by md_parse
            // Only need to push the line number of the start of the text widget
            // propagate_line_to_parents(ptr, get_line_number(ptr, m_text_start_idx));
            push_to_tree(ptr);


            tree_up();
            m_last_text_ptr = ptr;
        }
        return true;
    }
    int MarkdownToWidgets::block(AB::BLOCK_TYPE type, bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, AB::BlockDetailPtr detail) {
        AbstractWidgetPtr ptr = nullptr;
        switch (type) {
        case AB::BLOCK_DOC:
            BLOCK_DOC(enter);
            break;
        case AB::BLOCK_QUOTE:
            ptr = BLOCK_QUOTE(enter, bounds, attributes);
            break;
        case AB::BLOCK_UL:
            ptr = BLOCK_UL(enter, bounds, attributes, *std::static_pointer_cast<AB::BlockUlDetail>(detail));
            break;
        case AB::BLOCK_OL:
            ptr = BLOCK_OL(enter, bounds, attributes, *std::static_pointer_cast<AB::BlockOlDetail>(detail));
            break;
        case AB::BLOCK_LI:
            ptr = BLOCK_LI(enter, bounds, attributes, *std::static_pointer_cast<AB::BlockLiDetail>(detail));
            break;
        case AB::BLOCK_HR:
            ptr = BLOCK_HR(enter, bounds, attributes);
            break;
        case AB::BLOCK_H:
            ptr = BLOCK_H(enter, bounds, attributes, *std::static_pointer_cast<AB::BlockHDetail>(detail));
            break;
        case AB::BLOCK_CODE:
            ptr = BLOCK_CODE(enter, bounds, attributes, *std::static_pointer_cast<AB::BlockCodeDetail>(detail));
            break;
        case AB::BLOCK_P:
            ptr = BLOCK_P(enter, bounds, attributes);
            break;
            // case AB::BLOCK_DIV:
            //     ptr = BLOCK_TABLE((MD_BLOCK_TABLE_DETAIL*)detail, enter);
            //     break;
        default:
            break;
        }
        if (ptr != nullptr) {
            if (enter) {
                push_to_tree(ptr);
                m_last_block_ptr = ptr;
            }
            else {
                tree_up();
            }
        }
        return true;
    }
    int MarkdownToWidgets::span(AB::SPAN_TYPE type, bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, AB::SpanDetailPtr detail) {
        AbstractWidgetPtr ptr = nullptr;
        switch (type) {
        case AB::SPAN_EM:
            ptr = SPAN_EM(enter, bounds, attributes);
            break;
        case AB::SPAN_STRONG:
            ptr = SPAN_STRONG(enter, bounds, attributes);
            break;
        case AB::SPAN_URL:
            ptr = SPAN_A(enter, bounds, attributes, *std::static_pointer_cast<AB::SpanADetail>(detail));
            break;
        case AB::SPAN_IMG:
            ptr = SPAN_IMG(enter, bounds, attributes, *std::static_pointer_cast<AB::SpanImgDetail>(detail));
            break;
        case AB::SPAN_CODE:
            ptr = SPAN_CODE(enter, bounds, attributes);
            break;
        case AB::SPAN_DEL:
            ptr = SPAN_DEL(enter, bounds, attributes);
            break;
        case AB::SPAN_LATEXMATH:
            ptr = SPAN_LATEXMATH(enter, bounds, attributes);
            break;
            // case AB::SPAN_REF:
            //     ptr = SPAN_WIKILINK((MD_SPAN_WIKILINK_DETAIL*)detail, enter, mark_begin, mark_end);
            //     break;
        // case MD_SPAN_U:
        //     ptr = SPAN_U(enter, mark_begin, mark_end);
        //     break;
        default:
            break;
        }
        if (ptr != nullptr) {
            if (enter) {
                // propagate_lines_to_parents(ptr, m_text_end_idx, mark_begin);
                // create_intertext_widgets(m_text_end_idx, mark_begin);
                // ptr->m_raw_text_info.pre = mark_begin;
                // ptr->m_raw_text_info.begin = mark_end;
                // m_text_end_idx = mark_end;
                ptr->m_style.h_margins = ImVec2(0.f, 0.f);
                ptr->m_style.v_margins = ImVec2(0.f, 0.f);
                push_to_tree(ptr);

            }
            else {
                // propagate_lines_to_parents(ptr, mark_begin, mark_end);
                // ptr->m_raw_text_info.end = mark_begin;
                // ptr->m_raw_text_info.post = mark_end;
                // m_text_end_idx = mark_end;
                tree_up();
            }
        }

        return true;
    }

    void MarkdownToWidgets::push_to_tree(AbstractWidgetPtr& node) {
        m_tree.push_back(node);
        node->m_safe_string = m_safe_text;
        if (m_current_ptr != nullptr)
            set_infos(MarkdownConfig::SPECIAL, node, true);
        node->m_parent = m_current_ptr;
        if (m_current_ptr != nullptr) {
            m_current_ptr->m_childrens.push_back(node);
            node->m_child_number = m_current_ptr->m_childrens.size() - 1;
            node->m_textpos_to_lines = m_current_ptr->m_textpos_to_lines;
        }
        m_current_ptr = node;
    }
    void MarkdownToWidgets::tree_up() {
        if (m_current_ptr->m_parent != nullptr) {
            m_current_ptr = m_current_ptr->m_parent;
        }
    }
    void MarkdownToWidgets::set_href(bool enter, const std::string& src) {
        if (enter) {
            m_href = src;
        }
        else {
            m_href.clear();
        }
    }
    void MarkdownToWidgets::set_infos(MarkdownConfig::type type, AbstractWidgetPtr ptr, bool special_style) {
        auto style = m_config.styles[type];
        Style* current_style = nullptr;

        if (special_style) {
            ptr->m_special_chars_style = style;
            current_style = &(ptr->m_special_chars_style);
        }
        else {
            ptr->m_style = style;
            current_style = &(ptr->m_style);
        }
        // Inherit potential properties from parents
#define INHERIT_PROPERTY(name_) \
    if (!style.isset_##name_) { current_style->name_ = m_current_ptr->m_style.name_;}
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
            m_config.make_bold(current_style->font_styling);
        }
        if (style.isset_font_em && style.font_em) {
            m_config.make_em(current_style->font_styling);
        }
        if (style.isset_font_monospace && style.font_monospace) {
            m_config.make_monospace(current_style->font_styling);
        }
    }

    void MarkdownToWidgets::BLOCK_DOC(bool enter) {
        if (enter) {
            auto root = std::make_shared<RootNode>(m_ui_state);
            auto ptr = std::static_pointer_cast<AbstractWidget>(root);

            ptr->m_textpos_to_lines = std::make_shared<std::vector<int>>();
            ptr->m_lines_selected = std::make_shared<std::vector<bool>>();

            auto textpos_to_lines = ptr->m_textpos_to_lines.get();
            auto lines_selected = ptr->m_lines_selected.get();
            lines_selected->push_back(true); // The is always at least one line
            textpos_to_lines->reserve(m_text_size);
            int line_counter = 0;
            for (auto i = 0;i < m_text_size;i++) {
                if (m_text[i] == '\n') {
                    line_counter++;
                    lines_selected->push_back(true);
                }
                textpos_to_lines->push_back(line_counter);
            }
            push_to_tree(ptr);
            // Build raw text line information
        }
    }
    AbstractWidgetPtr MarkdownToWidgets::BLOCK_UL(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::BlockUlDetail& detail) {
        if (enter) {
            auto ul_list = std::make_shared<ULWidget>(m_ui_state);
            ul_list->m_text_boundaries = bounds;
            ul_list->m_attributes = attributes;
            if (m_current_ptr->m_type == T_BLOCK_UL) {
                ul_list->list_level = std::static_pointer_cast<ULWidget>(m_current_ptr)->list_level + 1;
            }
            else if (m_current_ptr->m_type == T_BLOCK_OL) {
                ul_list->list_level = std::static_pointer_cast<OLWidget>(m_current_ptr)->list_level + 1;
            }
            ul_list->mark = detail.marker;
            ul_list->m_style.h_margins.x = m_config.x_level_offset;
            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractWidget>(ul_list));
            auto ptr = std::static_pointer_cast<AbstractWidget>(ul_list);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractWidgetPtr MarkdownToWidgets::BLOCK_OL(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::BlockOlDetail& detail) {
        if (enter) {
            auto ol_list = std::make_shared<OLWidget>(m_ui_state);
            ol_list->m_text_boundaries = bounds;
            ol_list->m_attributes = attributes;
            if (m_current_ptr->m_type == T_BLOCK_UL) {
                ol_list->list_level = std::static_pointer_cast<ULWidget>(m_current_ptr)->list_level + 1;
            }
            else if (m_current_ptr->m_type == T_BLOCK_OL) {
                ol_list->list_level = std::static_pointer_cast<OLWidget>(m_current_ptr)->list_level + 1;
            }
            // ol_list->start = detail->start;
            ol_list->m_style.h_margins.x = m_config.x_level_offset;
            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractWidget>(ol_list));
            auto ptr = std::static_pointer_cast<AbstractWidget>(ol_list);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractWidgetPtr MarkdownToWidgets::BLOCK_LI(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::BlockLiDetail& detail) {
        if (enter) {
            auto list_el = std::make_shared<LIWidget>(m_ui_state);
            list_el->m_text_boundaries = bounds;
            list_el->m_attributes = attributes;
            list_el->is_task = detail.is_task;
            // list_el->task_mark = detail.task_mark;
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

    AbstractWidgetPtr MarkdownToWidgets::BLOCK_HR(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        return nullptr;
    }

    AbstractWidgetPtr MarkdownToWidgets::BLOCK_H(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::BlockHDetail& detail) {
        if (enter) {
            auto header = std::make_shared<HeaderWidget>(m_ui_state);
            header->m_text_boundaries = bounds;
            header->m_attributes = attributes;
            header->hlevel = detail.level;
            set_infos((MarkdownConfig::type)detail.level, std::static_pointer_cast<AbstractWidget>(header));
            auto ptr = std::static_pointer_cast<AbstractWidget>(header);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractWidgetPtr MarkdownToWidgets::BLOCK_QUOTE(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        if (enter) {
            auto quote = std::make_shared<QuoteWidget>(m_ui_state);
            quote->m_text_boundaries = bounds;
            quote->m_attributes = attributes;
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
    AbstractWidgetPtr MarkdownToWidgets::BLOCK_CODE(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::BlockCodeDetail& detail) {
        if (enter) {
            auto code = std::make_shared<CodeWidget>(m_ui_state);
            code->m_text_boundaries = bounds;
            code->m_attributes = attributes;
            auto ptr = std::static_pointer_cast<AbstractWidget>(code);
            set_infos(MarkdownConfig::CODE, ptr);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractWidgetPtr MarkdownToWidgets::BLOCK_P(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        if (enter) {
            auto p = std::make_shared<ParagraphWidget>(m_ui_state);
            p->m_text_boundaries = bounds;
            p->m_attributes = attributes;
            auto ptr = std::static_pointer_cast<AbstractWidget>(p);
            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractWidget>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractWidgetPtr MarkdownToWidgets::SPAN_A(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::SpanADetail& detail) {
        set_href(enter, detail.href);
        if (enter) {
            auto p = std::make_shared<LinkSpan>(m_ui_state);
            p->m_text_boundaries = bounds;
            p->m_attributes = attributes;
            auto ptr = std::static_pointer_cast<AbstractWidget>(p);
            set_infos(MarkdownConfig::HREF, std::static_pointer_cast<AbstractWidget>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_EM(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        if (enter) {
            auto p = std::make_shared<EmSpan>(m_ui_state);
            p->m_text_boundaries = bounds;
            p->m_attributes = attributes;
            auto ptr = std::static_pointer_cast<AbstractWidget>(p);
            set_infos(MarkdownConfig::EM, std::static_pointer_cast<AbstractWidget>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_STRONG(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        if (enter) {
            auto p = std::make_shared<StrongSpan>(m_ui_state);
            p->m_text_boundaries = bounds;
            p->m_attributes = attributes;
            auto ptr = std::static_pointer_cast<AbstractWidget>(p);
            set_infos(MarkdownConfig::STRONG, std::static_pointer_cast<AbstractWidget>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_IMG(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::SpanImgDetail& detail) {
        return nullptr;
    }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_CODE(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        if (enter) {
            auto p = std::make_shared<StrongSpan>(m_ui_state);
            p->m_text_boundaries = bounds;
            p->m_attributes = attributes;
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
    AbstractWidgetPtr MarkdownToWidgets::SPAN_LATEXMATH(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        return nullptr;
    }
    // AbstractWidgetPtr MarkdownToWidgets::SPAN_WIKILINK(const MD_SPAN_WIKILINK_DETAIL*, bool) {
    //     return nullptr;
    // }
    // AbstractWidgetPtr MarkdownToWidgets::SPAN_U(bool enter) {
    //     return nullptr;
    // }
    AbstractWidgetPtr MarkdownToWidgets::SPAN_DEL(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
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

        // md_parse(m_text, m_text_size, &m_md, this);
        AB::parse(m_text, m_text_size, &m_parser);

        // int level = 0;
        // for (auto ptr : m_tree) {
        //     // Find out level of widget
        //     // Not efficient but do not care
        //     int level = 0;
        //     auto tmp_ptr = ptr;
        //     while (tmp_ptr->m_parent != nullptr) {
        //         tmp_ptr = tmp_ptr->m_parent;
        //         std::cout << "  ";
        //         level++;
        //     }
        //     std::cout << AB::type_to_name(ptr->m_type);
        //     std::cout << " Pre: " << ptr->m_raw_text_info.pre;
        //     std::cout << " Begin: " << ptr->m_raw_text_info.begin;
        //     std::cout << " End: " << ptr->m_raw_text_info.end;
        //     std::cout << " Post: " << ptr->m_raw_text_info.post;
        //     std::cout << std::endl;
        // }
        // std::cout << "-----" << std::endl;

        return m_tree;
    }
}