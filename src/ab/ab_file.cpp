#include "ab_file.h"
#include "utils.h"

namespace AB {
    void File::add_to_text(std::string& str, const std::vector<AB::Boundaries>& bounds) {
        auto& ref = *m_safe_txt;
        for (auto& bound : bounds) {
            for (int i = bound.beg;i < bound.end;i++) {
                str += ref[i];
            }
        }
    }
    File::File(const std::string& path_or_txt, bool is_path, FileConfig config) {
        m_safe_txt = std::make_shared<std::string>();
        if (is_path) {
            getFileContents(path_or_txt, *m_safe_txt);
        }
        else {
            *m_safe_txt = path_or_txt;
        }
        m_config = config;
        parse(0, -1);
    }

    void File::parse(int start, int end) {
        Parser parser;
        parser.enter_block = [&](AB::BLOCK_TYPE b_type, const std::vector<AB::Boundaries>& bounds, const AB::Attributes& attributes, AB::BlockDetailPtr detail) -> bool {
            m_level++;
            if (m_level == 1) {
                m_blocks.push_back(std::make_shared<RootBlock>(RootBlock{
                    bounds.front().line_number,
                    bounds.front().pre,
                    bounds.back().line_number + 1,
                    bounds.back().post,
                    b_type,
                    0 }));
            }
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
        if (end < 0 || end > m_safe_txt->size()) {
            end = m_safe_txt->size();
        }
        if (start >= end)
            return;

        AB::parse(m_safe_txt.get(), start, end, &parser);
    }

    BlockBounds File::getBlocksBoundsContaining(int line_start, int line_end) {
        if (line_start > line_end || m_blocks.empty() || line_start == 0 && line_start == line_end) {
            return BlockBounds{ };
        }
        BlockBounds bounds;

        /* Start bound */
        bool found = false;
        int start_idx = 0;
        int end_idx = m_blocks.size() - 1;
        int midpoint_idx = end_idx / 2;
        if (line_start <= 0) {
            found = true;
            bounds.start.txt_idx = 0;
            bounds.start.line_idx = 0;
            bounds.start.block_idx = 0;
        }
        while (!found) {
            if (m_blocks[midpoint_idx]->line_start > line_start) {
                end_idx = midpoint_idx;
                midpoint_idx = start_idx + (end_idx - start_idx) / 2;
            }
            else if (m_blocks[midpoint_idx]->line_start < line_start) {
                if (m_blocks[midpoint_idx]->line_end > line_start) {
                    bounds.start.txt_idx = m_blocks[midpoint_idx]->idx_start;
                    bounds.start.line_idx = m_blocks[midpoint_idx]->line_start;
                    bounds.start.block_idx = midpoint_idx;
                    found = true;
                }
                else {
                    start_idx = midpoint_idx;
                    midpoint_idx = start_idx + (end_idx - start_idx) / 2;
                }
            }
            else {
                bounds.start.txt_idx = m_blocks[midpoint_idx]->idx_start;
                bounds.start.line_idx = line_start;
                bounds.start.block_idx = midpoint_idx;
                found = true;
            }
        }

        /* Bounds end */
        found = false;
        start_idx = 0;
        end_idx = m_blocks.size() - 1;
        midpoint_idx = end_idx / 2;
        if (m_blocks.back()->line_end < line_end) {
            found = true;
            bounds.end.txt_idx = m_blocks.back()->idx_end;
            bounds.end.line_idx = m_blocks.back()->line_end;
            bounds.end.block_idx = end_idx;
        }
        while (!found) {
            if (m_blocks[midpoint_idx]->line_end < line_end) {
                start_idx = midpoint_idx;
                midpoint_idx = start_idx + (end_idx - start_idx) / 2;
            }
            else if (m_blocks[midpoint_idx]->line_end > line_end) {
                if (m_blocks[midpoint_idx]->line_start < line_end) {
                    bounds.end.txt_idx = m_blocks[midpoint_idx]->idx_end;
                    bounds.end.line_idx = m_blocks[midpoint_idx]->line_end;
                    bounds.end.block_idx = midpoint_idx;
                    found = true;
                }
                else {
                    end_idx = midpoint_idx;
                    midpoint_idx = start_idx + (end_idx - start_idx) / 2;
                }
            }
            else {
                bounds.end.txt_idx = m_blocks[midpoint_idx]->idx_end;
                bounds.end.line_idx = line_end;
                bounds.end.block_idx = midpoint_idx;
                found = true;
            }
        }
        return bounds;
    }
}