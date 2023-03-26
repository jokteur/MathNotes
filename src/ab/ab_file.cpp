#include "ab_file.h"
#include "utils.h"

namespace AB {
    void File::add_to_text(std::string& str, const std::vector<AB::Boundaries>& bounds) {
        for (auto& bound : bounds) {
            for (int i = bound.beg;i < bound.end;i++) {
                str += m_txt[i];
            }
        }
    }
    File::File(const std::string& path_or_txt, bool is_path, FileConfig config) {
        if (is_path) {
            getFileContents(path_or_txt, m_txt);
        }
        else {
            m_txt = path_or_txt;
        }
        m_config = config;
        parse(0, -1);

    }

    void File::parse(int start, int end) {
        Parser parser;
        parser.enter_block = [&](AB::BLOCK_TYPE b_type, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, AB::BlockDetailPtr detail) -> bool {
            m_level++;
            if (m_level == 1)
                m_blocks.push_back(RootBlock{
                    bounds.front().line_number,
                    bounds.front().pre,
                    bounds.back().line_number,
                    bounds.back().post,
                    b_type });
            if (m_config.track_headers > 0 && b_type == BLOCK_H) {
                m_h_accumulator.clear();
                m_h_accumulating = true;
                m_headers.push_back(Header{ bounds, "" });
            }

            return true;
        };
        parser.leave_block = [&](AB::BLOCK_TYPE b_type) -> bool {
            m_level--;
            if (m_config.track_headers > 0 && b_type == BLOCK_H) {
                m_h_accumulating = false;
                m_headers.back().content = m_h_accumulator;
            }
            return true;
        };
        parser.enter_span = [&](AB::SPAN_TYPE s_type, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, AB::SpanDetailPtr detail) {
            return true;
        };
        parser.leave_span = [&](AB::SPAN_TYPE s_type) {
            return true;
        };
        parser.text = [&](AB::TEXT_TYPE t_type, const std::vector<AB::Boundaries>& bounds) {
            if (m_h_accumulating) {
                add_to_text(m_h_accumulator, bounds);
            }
            return true;
        };
        if (end < 0 || end > m_txt.size()) {
            end = m_txt.size();
        }
        if (start > end)
            return;

        AB::parse((m_txt.c_str() + start), (SIZE)end, &parser);
    }

    BlockBounds File::getBlocksBoundsContaining(int line_start, int line_end) {
        if (line_start > line_end) {
            return BlockBounds{ };
        }
        BlockBounds bounds;

        /* Start bound
         * Linear search - we don't expect more than 100'000 blocks, this function
         * should stay fast (~0.2ms for 135'000 root blocks) */
        auto it = m_blocks.begin();
        if (line_start > 0)
            for (;it != m_blocks.end();it++) {
                if (it->line_end >= line_start) {
                    break;
                }
            }
        bounds.start = it;

        if (it == m_blocks.end()) {
            bounds.end = it;
            return bounds;
        }

        /* End bound */
        for (;it != m_blocks.end();it++) {
            auto next = std::next(it);
            if (next != m_blocks.end() && next->line_start > line_end) {
                break;
            }
        }
        if (it == m_blocks.end()) {
            it--;
        }
        bounds.end = it;
        return bounds;
    }
}