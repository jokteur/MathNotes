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
    }
    ABToWidgets::ABToWidgets() {
        configure_parser();
    }
    int ABToWidgets::text(AB::TEXT_TYPE t_type, const std::vector<AB::Boundaries>& bounds) {
        return true;
    }
    int ABToWidgets::block(AB::BLOCK_TYPE type, bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, AB::BlockDetailPtr detail) {
        if (level == 1 && enter) {
            m_tree.push_back(RootBlock{ bounds.front().line_number, bounds.back().line_number + 1, T_ROOT });
        }
        if (enter) {
            level++;
        }
        else {
            level--;
        }
        return true;
    }
    int ABToWidgets::span(AB::SPAN_TYPE type, bool enter, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, AB::SpanDetailPtr detail) {
        return true;
    }

    std::vector<RootBlock> ABToWidgets::parse(const std::string& str, int start, int end, UIState_ptr ui_state, MarkdownConfig config) {
        m_text_start_idx = start;
        m_text_end_idx = end;
        m_text = str;
        m_tree.clear();
        m_current_ptr = nullptr;
        m_config = config;
        m_ui_state = ui_state;

        AB::parse(m_text.c_str() + start, end - start, &m_parser);

        return m_tree;
    }
}