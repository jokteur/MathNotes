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
                m_blocks.push_back(RootBlock{ bounds.front().line_number, bounds.back().line_number, b_type });
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
        AB::parse(m_txt.c_str(), (SIZE)m_txt.size(), &parser);
    }
}