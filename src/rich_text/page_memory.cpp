#include "page_memory.h"
#include "ab_converter.h"

namespace RichText {
    PageMemory::PageMemory(AB::File* file) : m_file(file) {}

    RootElements::iterator PageMemory::find_prev_ptr() {
        //ZoneScoped;
        auto current_it = m_elements.find(m_current_block_idx);
        if (current_it == m_elements.end() || current_it == m_elements.begin())
            return current_it;
        return std::prev(current_it);
    }
    RootElements::iterator PageMemory::find_next_ptr() {
        //ZoneScoped;
        auto current_it = m_elements.find(m_current_block_idx);
        if (current_it == m_elements.end()) {
            return current_it;
        }
        auto next = std::next(current_it);
        return next;
    }
    void PageMemory::find_current_ptr() {
        //ZoneScoped;
        auto bounds = m_file->getBlocksBoundsContaining(m_current_line, m_current_line + 1);
        if (m_elements.find(bounds.start.block_idx) != m_elements.end()) {
            m_current_block_idx = bounds.start.block_idx;
            m_current_block_ptr = m_elements[m_current_block_idx];
        }
    }
    void PageMemory::parse_job(int start_idx, int end_idx) {
        ABToWidgets parser;
        std::map<int, RootNodePtr> tmp_roots;
        parser.parse(m_file, start_idx, end_idx, &m_elements);
    }
    void PageMemory::gotoLine(int line_number) {
        m_current_line = line_number;
        find_current_ptr();
        if (m_current_block_ptr != nullptr) {
            m_current_line = m_current_block_ptr->get().m_text_boundaries.front().line_number;
        }
    }
    int PageMemory::getNumLineBefore() const {
        return m_file->m_blocks[m_block_idx_start]->line_start;
    }
    int PageMemory::getNumLineAfter() const {
        return m_file->m_blocks.back()->line_end - m_file->m_blocks[m_block_idx_end]->line_end;
    }

    bool PageMemory::isBlockInMemory(int block_idx) const {
        return m_elements.find(block_idx) != m_elements.end();
    }
    bool PageMemory::isCurrentBlockAtMemBeg() const {
        return m_current_block_idx == m_block_idx_start;
    }
    bool PageMemory::isCurrentBlockAtMemEnd() const {
        return m_current_block_idx == m_block_idx_end;
    }
    bool PageMemory::isCurrentBlockAtTrueBeg() const {
        return m_current_block_idx == 0;
    }
    bool PageMemory::isCurrentBlockAtTrueEnd() const {
        return m_current_block_idx == m_file->m_blocks.size() - 1;
    }

    bool PageMemory::prevPtr() {
        //ZoneScoped;
        if (m_current_block_idx == 0)
            return true;
        auto prev_it = find_prev_ptr();
        if (prev_it == m_elements.end())
            return true;
        m_current_block_idx = prev_it->first;
        m_current_block_ptr = prev_it->second;
        m_current_line = m_current_block_ptr->get().m_text_boundaries.front().line_number;
        return false;
    }
    bool PageMemory::nextPtr() {
        //ZoneScoped;
        if (m_current_block_idx == m_file->m_blocks.size() - 1)
            return true;
        auto next_it = find_next_ptr();
        if (next_it == m_elements.end())
            return true;
        m_current_block_idx = next_it->first;
        m_current_block_ptr = next_it->second;
        m_current_line = m_current_block_ptr->get().m_text_boundaries.front().line_number;
        return false;
    }

    void PageMemory::manage() {
        find_current_ptr();
        int half_window = 0.9 * m_line_lookahead_window / 2;
        /* We want a minimum half window for super tiny pages */
        if (half_window < 1500) {
            half_window = 1500;
        }
        int start_line = m_current_line - half_window;
        int end_line = m_current_line + half_window;
        if (start_line < 0) {
            end_line += -start_line;
            start_line = 0;
        }
        auto bounds = m_file->getBlocksBoundsContaining(start_line, end_line);

        if (bounds.start.block_idx == -1)
            return;

        int start = bounds.start.block_idx;
        int end = bounds.end.block_idx;

        std::unordered_set<int> to_destroy;

        /* Widget was just created or jumped to another location */
        if (m_block_idx_end == -1 || (start >= m_block_idx_end && start != 0) || end < m_block_idx_start) {
            std::lock_guard<std::mutex> lk(m_root_mutex);
            if (start >= m_block_idx_end || end < m_block_idx_start) {
                m_elements.clear();
            }
            m_block_idx_start = start;
            m_block_idx_end = end;
            ABToWidgets parser;
            parser.parse(m_file, m_block_idx_start, m_block_idx_end, &m_elements);
            /* Move to the correct block_ptr corresponding to the current line */
            auto current_bounds = m_file->getBlocksBoundsContaining(m_current_line, m_current_line);
            auto it = m_elements.find(current_bounds.start.block_idx);
            if (it != m_elements.end()) {
                m_current_block_idx = it->first;
                m_current_block_ptr = it->second;
            }
        }
        else {
            /* Blocks to build before */
            if (start < m_block_idx_start) {
                parse_job(start, m_block_idx_start);
            }
            /* Blocks to destroy before */
            else if (start > m_block_idx_start) {
                std::cout << "Destroy " << m_block_idx_start << " to " << start << " (pre)" << std::endl;
                for (int i = m_block_idx_start; i < start;i++) {
                    to_destroy.insert(i);
                }
            }
            /* Blocks to build after */
            if (end > m_block_idx_end) {
                parse_job(m_block_idx_end, end);
            }
            /* Blocks to destroy after */
            else if (end < m_block_idx_end) {
                std::cout << "Destroy " << end << " to " << m_block_idx_end << std::endl;
                for (int i = end + 1; i <= m_block_idx_end;i++) {
                    to_destroy.insert(i);
                }
            }

            {
                // std::lock_guard<std::mutex> lk(m_root_mutex);
                for (auto idx : to_destroy) {
                    // m_lastly_destroyed_elements.insert(idx);
                    m_elements.erase(idx);
                }
            }
            m_block_idx_start = start;
            m_block_idx_end = end;
        }
    }
}