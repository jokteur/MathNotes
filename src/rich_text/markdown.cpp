#include <cmath>

#include "markdown.h"

#include "blocks/paragraph.h"
#include "blocks/header.h"
#include "blocks/lists.h"
#include "blocks/table.h"
#include "blocks/quote.h"

#include "spans/latex.h"
#include "spans/text.h"
#include "spans/inline_spans.h"
#include "spans/special_spans.h"

namespace RichText {
    void str_from_text_boundaries(const std::string& raw_text, std::string& str, const std::vector<AB::Boundaries>& text_boundaries) {
        int j = 0;
        for (auto& bound : text_boundaries) {
            if (j > 0)
                str += '\n';
            for (int i = bound.beg;i < bound.end;i++) {
                str += raw_text[i];
            }
            j++;
        }
    }

    void ABToWidgets::configure_parser() {
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
        m_root_ptr = std::make_shared<RootNode>(m_ui_state);
    }
    ABToWidgets::ABToWidgets() {
        configure_parser();
    }
    int ABToWidgets::text(AB::TEXT_TYPE t_type, const std::vector<AB::Boundaries>& bounds) {
        if (t_type == AB::TEXT_LATEX) {
        }
        else if (t_type == AB::TEXT_CODE) {

        }
        else {
            using namespace Fonts;
            auto text = std::make_shared<TextString>(m_ui_state);
            text->m_text_boundaries = bounds;
            str_from_text_boundaries(*m_safe_text, text->m_processed_text, bounds);

            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractElement>(text));

            auto ptr = std::static_pointer_cast<AbstractElement>(text);

            m_level++;
            push_to_tree(ptr);
            m_level--;
            tree_up();
            m_last_text_ptr = ptr;
        }
        return true;
    }
    int ABToWidgets::block(AB::BLOCK_TYPE type, bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, AB::BlockDetailPtr detail) {
        AbstractElementPtr ptr = nullptr;
        if (enter) {
            m_level++;
        }
        else {
            m_level--;
        }
        if (m_level == 1) {
            m_current_ptr = m_root_ptr;
        }
        switch (type) {
        case AB::BLOCK_DOC:
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
        case AB::BLOCK_HIDDEN:
            ptr = BLOCK_HIDDENSPACE(enter, bounds, attributes);
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
    int ABToWidgets::span(AB::SPAN_TYPE type, bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, AB::SpanDetailPtr detail) {
        AbstractElementPtr ptr = nullptr;
        if (enter) {
            m_level++;
        }
        else {
            m_level--;
        }
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
        case AB::SPAN_HIGHLIGHT:
            ptr = SPAN_HIGHLIGHT(enter, bounds, attributes);
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
                ptr->m_style.h_margins = ImVec2(0.f, 0.f);
                ptr->m_style.v_margins = ImVec2(0.f, 0.f);
                push_to_tree(ptr);

            }
            else {
                tree_up();
            }
        }

        return true;
    }

    void ABToWidgets::push_to_tree(AbstractElementPtr& node) {
        // m_tree.push_back(node);
        if (m_level == 1) {
            m_current_ptr = nullptr;
            (*m_root_elements)[m_root_idx_current] = node;
            m_root_idx_current++;
        }
        // node->m_txt_offset = m_text_start_idx;
        // node->m_line_offset = m_line_offset;
        node->m_safe_string = m_safe_text;
        node->m_rt_info = m_rt_info;
        if (m_current_ptr != nullptr)
            set_infos(MarkdownConfig::SPECIAL, node, true);
        node->m_parent = m_current_ptr;
        if (m_current_ptr != nullptr) {
            m_current_ptr->m_childrens.push_back(node);
            node->m_child_number = m_current_ptr->m_childrens.size() - 1;
        }
        m_current_ptr = node;
    }
    void ABToWidgets::tree_up() {
        if (m_current_ptr->m_parent.lock() != nullptr) {
            m_current_ptr = m_current_ptr->m_parent.lock();
        }
    }
    void ABToWidgets::set_href(bool enter, const std::string& src) {
        if (enter) {
            m_href = src;
        }
        else {
            m_href.clear();
        }
    }
    void ABToWidgets::set_infos(MarkdownConfig::type type, AbstractElementPtr ptr, bool special_style) {
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

        if (m_current_ptr != nullptr) {
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
        }

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

    AbstractElementPtr ABToWidgets::BLOCK_UL(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::BlockUlDetail& detail) {
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
            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractElement>(ul_list));
            auto ptr = std::static_pointer_cast<AbstractElement>(ul_list);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractElementPtr ABToWidgets::BLOCK_OL(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::BlockOlDetail& detail) {
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
            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractElement>(ol_list));
            auto ptr = std::static_pointer_cast<AbstractElement>(ol_list);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractElementPtr ABToWidgets::BLOCK_LI(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::BlockLiDetail& detail) {
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
            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractElement>(list_el));
            auto ptr = std::static_pointer_cast<AbstractElement>(list_el);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractElementPtr ABToWidgets::BLOCK_HR(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        return nullptr;
    }

    AbstractElementPtr ABToWidgets::BLOCK_H(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::BlockHDetail& detail) {
        if (enter) {
            auto header = std::make_shared<HeaderWidget>(m_ui_state);
            header->m_text_boundaries = bounds;
            header->m_attributes = attributes;
            header->hlevel = detail.level;
            set_infos((MarkdownConfig::type)detail.level, std::static_pointer_cast<AbstractElement>(header));
            auto ptr = std::static_pointer_cast<AbstractElement>(header);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractElementPtr ABToWidgets::BLOCK_QUOTE(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        if (enter) {
            auto quote = std::make_shared<QuoteWidget>(m_ui_state);
            quote->m_text_boundaries = bounds;
            quote->m_attributes = attributes;
            if (m_current_ptr->m_type == T_BLOCK_QUOTE) {
                auto parent = std::static_pointer_cast<QuoteWidget>(m_current_ptr);
            }
            set_infos(MarkdownConfig::QUOTE, std::static_pointer_cast<AbstractElement>(quote));
            auto ptr = std::static_pointer_cast<AbstractElement>(quote);
            return ptr;
        }
        else {
            return m_current_ptr;
        }

    }
    AbstractElementPtr ABToWidgets::BLOCK_CODE(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::BlockCodeDetail& detail) {
        if (enter) {
            auto code = std::make_shared<CodeWidget>(m_ui_state);
            code->m_text_boundaries = bounds;
            code->m_attributes = attributes;
            auto ptr = std::static_pointer_cast<AbstractElement>(code);
            set_infos(MarkdownConfig::CODE, ptr);
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractElementPtr ABToWidgets::BLOCK_P(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        if (enter) {
            auto p = std::make_shared<ParagraphWidget>(m_ui_state);
            p->m_text_boundaries = bounds;
            p->m_attributes = attributes;
            auto ptr = std::static_pointer_cast<AbstractElement>(p);
            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractElement>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractElementPtr ABToWidgets::BLOCK_HIDDENSPACE(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        if (enter) {
            auto p = std::make_shared<HiddenSpace>(m_ui_state);
            p->m_text_boundaries = bounds;
            p->m_attributes = attributes;
            auto ptr = std::static_pointer_cast<AbstractElement>(p);
            set_infos(MarkdownConfig::P, std::static_pointer_cast<AbstractElement>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }

    AbstractElementPtr ABToWidgets::SPAN_A(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::SpanADetail& detail) {
        set_href(enter, detail.href);
        if (enter) {
            auto p = std::make_shared<LinkSpan>(m_ui_state);
            p->m_text_boundaries = bounds;
            p->m_attributes = attributes;
            auto ptr = std::static_pointer_cast<AbstractElement>(p);
            set_infos(MarkdownConfig::HREF, std::static_pointer_cast<AbstractElement>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractElementPtr ABToWidgets::SPAN_EM(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        if (enter) {
            auto p = std::make_shared<EmSpan>(m_ui_state);
            p->m_text_boundaries = bounds;
            p->m_attributes = attributes;
            auto ptr = std::static_pointer_cast<AbstractElement>(p);
            set_infos(MarkdownConfig::EM, std::static_pointer_cast<AbstractElement>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractElementPtr ABToWidgets::SPAN_STRONG(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        if (enter) {
            auto p = std::make_shared<StrongSpan>(m_ui_state);
            p->m_text_boundaries = bounds;
            p->m_attributes = attributes;
            auto ptr = std::static_pointer_cast<AbstractElement>(p);
            set_infos(MarkdownConfig::STRONG, std::static_pointer_cast<AbstractElement>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractElementPtr ABToWidgets::SPAN_IMG(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, const AB::SpanImgDetail& detail) {
        return nullptr;
    }
    AbstractElementPtr ABToWidgets::SPAN_CODE(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        if (enter) {
            auto p = std::make_shared<StrongSpan>(m_ui_state);
            p->m_text_boundaries = bounds;
            p->m_attributes = attributes;
            auto ptr = std::static_pointer_cast<AbstractElement>(p);
            if (m_current_ptr->m_type == T_BLOCK_CODE)
                set_infos(MarkdownConfig::CODE, std::static_pointer_cast<AbstractElement>(p));
            else
                set_infos(MarkdownConfig::INLINE_CODE, std::static_pointer_cast<AbstractElement>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    AbstractElementPtr ABToWidgets::SPAN_LATEXMATH(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        return nullptr;
    }
    // AbstractElementPtr MarkdownToWidgets::SPAN_WIKILINK(const MD_SPAN_WIKILINK_DETAIL*, bool) {
    //     return nullptr;
    // }
    // AbstractElementPtr MarkdownToWidgets::SPAN_U(bool enter) {
    //     return nullptr;
    // }
    AbstractElementPtr ABToWidgets::SPAN_DEL(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        return nullptr;
    }
    AbstractElementPtr ABToWidgets::SPAN_HIGHLIGHT(bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes) {
        if (enter) {
            auto p = std::make_shared<HighlightSpan>(m_ui_state);
            p->m_text_boundaries = bounds;
            p->m_attributes = attributes;
            auto ptr = std::static_pointer_cast<AbstractElement>(p);
            set_infos(MarkdownConfig::HIGHLIGHT, std::static_pointer_cast<AbstractElement>(p));
            return ptr;
        }
        else {
            return m_current_ptr;
        }
    }
    void ABToWidgets::parse(AB::File* file, int root_idx_start, int root_idx_end, std::map<int, AbstractElementPtr>* root_elements, UIState_ptr ui_state, MarkdownConfig config) {
        if (root_idx_end < 0 || root_idx_start < 0)
            return;
        m_ab_file = file;
        m_root_idx_start = root_idx_start;
        m_root_idx_current = root_idx_start;
        m_root_idx_end = root_idx_end;

        m_root_elements = root_elements;
        m_current_ptr = nullptr;
        m_config = config;
        m_ui_state = ui_state;
        m_safe_text = file->m_safe_txt;


        int idx_start = m_ab_file->m_blocks[root_idx_start]->idx_start;
        int idx_end = m_ab_file->m_blocks[root_idx_end]->idx_end;

        std::cout << "Parse " << root_idx_start << " to " << root_idx_end <<
            " or idx: " << idx_start << " " << idx_end << std::endl;

        AB::parse(m_safe_text.get(), idx_start, idx_end, &m_parser);
    }
}