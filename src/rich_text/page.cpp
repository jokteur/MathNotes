#include "page.h"

#include <cmath>

#include "ab/ab_file.h"
#include "ab_converter.h"
#include "ui/ui_utils.h"

namespace RichText {
    void Page::debug_window() {
        ImGui::Begin("Widget info");
        ImGui::Text("y disp: %f", m_y_displacement);
        ImGui::Text("current_line: %d", m_current_line);
        ImGui::Text("current block idx: %d", m_current_block_idx);
        ImGui::Text("Heigts (b, a, t): %f %f %f", m_before_height, m_after_height, m_before_height + m_after_height);
        ImGui::Text("Element count: %d", AbstractElement::count);
        // ImGui::Text("String count: %d", WrapString::count);
        ImGui::Text("Font char count: %d", (int)m_ui_state.font_manager.debugGetChars().size());
        ImGui::Text("Visible count: %d", AbstractElement::visible_count);

        ImGui::Separator();
        ImGui::End();

        ImGui::Begin("Parsed blocks");
        if (ImGui::TreeNode("Show")) {
            for (auto& pair : m_root_elements) {
                pair.second->get().hk_debug(std::to_string(pair.first));
            }
            ImGui::TreePop();
        }
        ImGui::End();

        ImGui::Begin("Top displayed block");
        if (m_current_block_ptr != nullptr) {
            m_current_block_ptr->get().hk_debug();
        }
        ImGui::End();

        ImGui::Begin("AB File");
        if (ImGui::TreeNode("Show")) {
            if (ImGui::CollapsingHeader("Roots")) {
                int num_roots_blocks = m_file->m_blocks.size();
                if (m_debug_root_max > num_roots_blocks) {
                    m_debug_root_max = num_roots_blocks;
                }
                ImGui::DragIntRange2(("Show root idx (max " + std::to_string(num_roots_blocks) + ")").c_str(),
                    &m_debug_root_min,
                    &m_debug_root_max,
                    1.f, 0,
                    num_roots_blocks);
                for (int i = m_debug_root_min;i < m_debug_root_max;i++) {
                    std::string str;
                    auto& block = m_file->m_blocks[i];
                    AB::Boundaries bounds{block->line_start, block->idx_start, block->idx_start, block->idx_end, block->idx_end};
                    AB::str_from_text_boundaries(*m_file->m_safe_txt, str, { bounds });

                    int length = str.length();
                    std::string suffix = str;
                    if (length > 10) {
                        suffix = suffix.substr(0, 10) + "...";
                    }
                    suffix = "  /" + suffix;

                    ImGui::Text("%d %s %s", i, AB::block_to_name(block->type), suffix.c_str());
                }
            }
            if (ImGui::CollapsingHeader("Headers")) {
            }
            if (ImGui::CollapsingHeader("Equations")) {
            }
            ImGui::TreePop();
        }
        ImGui::End();
    }

    void Page::FrameUpdate() {
        //ZoneScoped;

        m_window_name = "RichText window ##drawable " + m_name;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::Begin(m_window_name.c_str());
        float width = ImGui::GetWindowContentRegionWidth();
        width -= ImGui::GetStyle().ScrollbarSize;
        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax = ImGui::GetWindowContentRegionMax();

        vMin.x += ImGui::GetWindowPos().x;
        vMin.y += ImGui::GetWindowPos().y;
        vMax.x += ImGui::GetWindowPos().x;
        vMax.y += ImGui::GetWindowPos().y;
        auto mouse_pos = ImGui::GetMousePos();

        vMin.y += 0;
        vMax.y -= 0;

        /* Once we know the height of the page,
         * we can estimate how many lines we should
         * look ahead for block element construction*/
        m_display_height = vMax.y - vMin.y;
        calculate_heights();
        manage_elements();

        AbstractElement::visible_count = 0;

        {
            // std::lock_guard lk(m_root_mutex);
            find_current_ptr();

            /* Set the width of the blocks recursively, even the ones that are not shown */
            /* After a resize event, we want to make sure that the scroll stays at the same line
             * This is why current_widget_y_size */
            bool resize_event = false;
            float current_widget_y_size = 0.f;
            float updated_current_widget_y_size = 0.f;
            for (auto pair : m_root_elements) {
                if (m_current_width != width && pair.second == m_current_block_ptr) {
                    current_widget_y_size = m_current_block_ptr->get().m_ext_dimensions.h;
                }
                if (m_current_width != width || pair.second->get().m_widget_dirty & pair.second->get().DIRTY_WIDTH) {
                    pair.second->get().setWindowWidth(width);
                }
            }
            if (m_current_width != width) {
                resize_event = true;
                m_current_width = width;
            }

            if (!m_root_elements.empty()) {
                if (!m_scrollbar_grab)
                    manage_scroll(mouse_pos, Rect{ vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y });

                Rect boundaries;
                boundaries.y = 0.f;
                boundaries.h = vMax.y - vMin.y;
                boundaries.w = width;
                m_draw_list.SetImDrawList(ImGui::GetWindowDrawList());

                // m_draw_list->AddRect(vMin, vMax, Colors::red, 0.f, 0, 2.f);

                // Background, ForeGround
                m_draw_list.Split(2);
                m_draw_list.SetCurrentChannel(1);

                /* We want to separate elements in two groups:
                 * - before m_current_line
                 * - after m_current_line
                 *
                 * All the elements before should not be displayed, but they are still important
                 * to be build once to calculate the scroll height
                 *
                 * As the elements height can only be known when displaying them once, we still need
                 * to call the draw fct. This is why we first draw them safely outside of the screen
                 *
                 * Once we reach the m_current_line element, then we can set y_pos properly
                 */
                DrawContext ctx;
                bool found_current = false;
                ctx.cursor_y_pos = m_display_height + 1000.f;
                ctx.draw_list = &m_draw_list;
                ctx.boundaries = boundaries;
                /* Designates the height taken by the elements before the current one */

                TimeCounter::getInstance().startCounter("DisplayAll");
                for (auto pair : m_root_elements) {
                    const auto& bounds = pair.second->get().m_text_boundaries;
                    TimeCounter::getInstance().startCounter("Clear offsets");
                    ctx.x_offset.clear(bounds.front().line_number, bounds.back().line_number);
                    TimeCounter::getInstance().stopCounter("Clear offsets");

                    ctx.lines = &pair.second->m_lines;
                    if (!found_current && pair.first >= m_current_block_idx) {
                        found_current = true;
                        m_before_height = ctx.cursor_y_pos - m_display_height - 1000.f;
                        ctx.cursor_y_pos = -roundf(m_y_displacement);
                        pair.second->get().draw(&ctx);
                        updated_current_widget_y_size = pair.second->get().m_ext_dimensions.h;
                        continue;
                    }
                    pair.second->get().draw(&ctx);
                }
                TimeCounter::getInstance().stopCounter("DisplayAll");
                m_after_height = ctx.cursor_y_pos + roundf(m_y_displacement);
                // m_after_height -= 2 * m_line_height;
                if (m_after_height < 0.f)
                    m_after_height = 0.f;
                m_draw_list.Merge();

                display_scrollbar(Rect{ vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y });
            }
            /* Once all the roots have been displayed, if there has been some resize event, we need to correct
             * for the next frame such that the user doesn't lose the line which was currently being drawn */
            if (resize_event && abs(current_widget_y_size - updated_current_widget_y_size))
                go_to_line(m_current_block_idx);

            ImVec2 rel_pos = ImVec2(mouse_pos.x - vMin.x, mouse_pos.y - vMin.y);
            ImGui::End();
            ImGui::PopStyleColor();

            debug_window();
        }
    }
    void Page::display_scrollbar(const Rect& b) {
        /* Find approximate height before the first parsed block
         * and after the last parsed block */
        int num_lines_before = m_file->m_blocks[m_block_idx_start]->line_start;
        int num_lines_after = m_file->m_blocks.back()->line_end - m_file->m_blocks[m_block_idx_end]->line_end;

        float before = m_before_height + m_y_displacement + num_lines_before * m_line_height;
        float after = m_after_height - m_y_displacement + num_lines_after * m_line_height;

        float old_percentage = m_scrollbar.getPercentage();

        m_scrollbar.setMinScrollHeight(m_config.min_scroll_height);
        m_scrollbar.FrameUpdate(b, m_draw_list, before, after, m_window_name);
        if (m_scrollbar.hasChanged()) {
            float percentage = m_scrollbar.getPercentage();
            float total_height = before + after;
            if (percentage - old_percentage > 0.f) {
                scroll_down((percentage - old_percentage) * total_height);
            }
            else if (percentage - old_percentage < 0.f) {
                scroll_up((old_percentage - percentage) * total_height);
            }
        }
        if (m_scrollbar.isGrabbing())
            m_scrollbar_grab = true;
        else
            m_scrollbar_grab = false;
    }

    void Page::manage_scroll(const ImVec2& mouse_pos, const Rect& box) {
        if (isInsideRect(mouse_pos, box) && isOnTop(m_window_name)) {
            float mouse_wheel = ImGui::GetIO().MouseWheel;
            mouse_wheel *= 60;

            if (mouse_wheel > 0.f) {
                scroll_up(mouse_wheel);
            }
            else if (mouse_wheel < 0.f) {
                scroll_down(mouse_wheel);
            }
        }
        if (m_y_displacement > m_after_height) {
            m_y_displacement = m_after_height;
        }
    }

    void Page::calculate_heights() {
        if (m_recalculate_line_height) {
            using namespace Fonts;
            FontRequestInfo font_request;
            font_request.size_wish = 18.f;
            FontInfoOut font_out;
            m_ui_state.font_manager.requestFont(font_request, font_out);
            auto font = Tempo::GetImFont(font_out.font_id);
            if (font->im_font != nullptr) {
                m_recalculate_line_height = false;
                m_line_height = font_out.size * font_out.ratio * m_scale * Tempo::GetScaling();
            }
        }
        float lines_per_display = m_display_height / m_line_height;
        float num_pages_for_min_scroll = m_display_height / (m_config.min_scroll_height * Tempo::GetScaling());
        m_line_lookahead_window = num_pages_for_min_scroll * lines_per_display;
    }
    std::map<int, RootNodePtr>::iterator Page::find_prev_ptr() {
        //ZoneScoped;
        auto current_it = m_root_elements.find(m_current_block_idx);
        if (current_it == m_root_elements.end() || current_it == m_root_elements.begin())
            return current_it;
        return std::prev(current_it);
    }
    std::map<int, RootNodePtr>::iterator Page::find_next_ptr() {
        //ZoneScoped;
        auto current_it = m_root_elements.find(m_current_block_idx);
        if (current_it == m_root_elements.end()) {
            return current_it;
        }
        auto next = std::next(current_it);
        return next;
    }
    void Page::find_current_ptr() {
        //ZoneScoped;
        auto bounds = m_file->getBlocksBoundsContaining(m_current_line, m_current_line + 1);
        if (m_root_elements.find(bounds.start.block_idx) != m_root_elements.end()) {
            m_current_block_idx = bounds.start.block_idx;
            m_current_block_ptr = m_root_elements[m_current_block_idx];
        }
    }
    void Page::go_to_line(int line_number) {
        //ZoneScoped;
        m_current_line = line_number;
        find_current_ptr();
        if (m_current_block_ptr != nullptr) {
            m_current_line = m_current_block_ptr->get().m_text_boundaries.front().line_number;
        }
    }
    void Page::scroll_down(float pixels) {
        //ZoneScoped;
        if (m_current_block_ptr == nullptr)
            return;

        pixels = abs(pixels);

        /* First check if with the scroll, we stay within the element */
        float remaining_height = m_current_block_ptr->get().m_ext_dimensions.h - m_y_displacement;
        if (pixels < remaining_height) {
            m_y_displacement += pixels;
            return;
        }
        bool arrived_at_end = false;
        int previous_line = m_current_line;
        float total_height = remaining_height;
        while (true) {
            /* Go to the next ptr */
            auto next = find_next_ptr();
            /* We arrived at the end of file */
            if (next == m_root_elements.end()) {
                if (std::prev(next)->first != m_file->m_blocks.size() - 1)
                    arrived_at_end = true;
                break;
            }
            go_to_line(next->second->get().m_text_boundaries.front().line_number);

            float element_height = m_current_block_ptr->get().m_ext_dimensions.h;
            total_height += element_height;
            if (pixels < total_height) {
                m_y_displacement = element_height - (total_height - pixels);
                return;
            }
        }
        if (arrived_at_end && !m_root_elements.empty()) {
            auto last_line = m_root_elements.rbegin()->second->get().m_text_boundaries.back().line_number;
            /* Estimate the average line height with the current informations */
            float estimated_additional_lines = (pixels - total_height) / m_line_height;
            m_current_line = last_line + (int)estimated_additional_lines;
            std::cout << "Change line " << m_current_line << std::endl;
        }
        if (!arrived_at_end) {
            m_y_displacement = m_current_block_ptr->get().m_ext_dimensions.h;
        }
    }
    void Page::scroll_up(float pixels) {
        //ZoneScoped;
        if (m_current_block_ptr == nullptr)
            return;

        /* First check if with the scroll, we stay within the element */
        float remaining_height = m_current_block_ptr->get().m_ext_dimensions.h - m_y_displacement;
        if (pixels <= m_y_displacement || m_current_block_idx == 0) {
            m_y_displacement -= pixels;
            if (m_y_displacement < 0.f)
                m_y_displacement = 0.f;
            return;
        }

        bool arrived_at_beg = false;
        /* Remove y displacement already */
        pixels -= m_y_displacement;
        float total_height = 0.f;

        while (true) {
            auto prev = find_prev_ptr();
            if (prev == m_root_elements.begin() && prev->first > 0 || prev == m_root_elements.end()) {
                arrived_at_beg = true;
                break;
            }
            go_to_line(prev->second->get().m_text_boundaries.front().line_number);
            if (prev->second->get().m_ext_dimensions.h == 0.f && prev->first > 0) {
                continue;
            }
            float element_height = m_current_block_ptr->get().m_ext_dimensions.h;
            total_height += element_height;
            if (pixels <= total_height || prev == m_root_elements.begin()) {
                m_y_displacement = total_height - pixels;
                if (m_y_displacement < 0.f)
                    m_y_displacement = 0.f;
                return;
            }
        }
        if (arrived_at_beg && !m_root_elements.empty()) {
            auto first_line = m_current_block_ptr->get().m_text_boundaries.front().line_number;
            float estimated_additional_lines = (pixels - total_height) / m_line_height;
            int estimated_line = first_line - (int)estimated_additional_lines;
            if (estimated_line < 0)
                estimated_line = 0;
            m_current_line = estimated_line;
            std::cout << "Change line " << m_current_line << std::endl;
        }
    }

    void Page::manage_elements() {
        //ZoneScoped;

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
                m_root_elements.clear();
            }
            m_block_idx_start = start;
            m_block_idx_end = end;
            ABToWidgets parser;
            parser.parse(m_file, m_block_idx_start, m_block_idx_end, &m_root_elements);
            /* Move to the correct block_ptr corresponding to the current line */
            auto current_bounds = m_file->getBlocksBoundsContaining(m_current_line, m_current_line);
            auto it = m_root_elements.find(current_bounds.start.block_idx);
            if (it != m_root_elements.end()) {
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
                    m_root_elements.erase(idx);
                }
            }
            m_block_idx_start = start;
            m_block_idx_end = end;
        }
    }
    void Page::parse_job(int start_idx, int end_idx) {
        ABToWidgets parser;
        std::map<int, RootNodePtr> tmp_roots;
        parser.parse(m_file, start_idx, end_idx, &m_root_elements);
    }
    Page::~Page() {
    }
}