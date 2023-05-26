#include "page.h"

#include <cmath>

#include "ab/ab_file.h"
#include "ab_converter.h"
#include "ui/ui_utils.h"

namespace RichText {
    Page::Page(AB::File* file) : Drawable(), m_scrollbar(VerticalScrollBar::FIT_UNTIL_LAST_LINE), m_mem(file), m_file(file)
    {}

    void Page::debug_window() {
        ImGui::Begin("Widget info");
        ImGui::Text("y disp: %f", m_y_displacement);
        ImGui::Text("current_line: %d", m_mem.getCurrentLine());
        ImGui::Text("current block idx: %d", m_mem.getCurrentBlockIdx());
        ImGui::Text("Heigts (b, a, t): %f %f %f", m_before_height, m_after_height, m_before_height + m_after_height);
        ImGui::Text("Element count: %d", AbstractElement::count);
        // ImGui::Text("String count: %d", WrapString::count);
        ImGui::Text("Font char count: %d", (int)m_ui_state.font_manager.debugGetChars().size());
        ImGui::Text("Visible count: %d", AbstractElement::visible_count);

        ImGui::Separator();
        ImGui::End();

        ImGui::Begin("Parsed blocks");
        if (ImGui::TreeNode("Show")) {
            for (auto& pair : m_mem.getElements()) {
                pair.second->get().hk_debug(std::to_string(pair.first));
            }
            ImGui::TreePop();
        }
        ImGui::End();

        ImGui::Begin("Visible blocks");
        if (ImGui::TreeNode("Show")) {
            for (auto& pair : m_mem.getElements()) {
                auto& ptr = pair.second->get();
                if (ptr.m_is_visible || !ptr.m_is_dimension_set || ptr.m_widget_dirty)
                    ptr.hk_debug(std::to_string(pair.first));
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

    void Page::setLine(int line_number) {
        m_mem.gotoLine(line_number);
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
        m_mem.manage();

        AbstractElement::visible_count = 0;

        {
            /* Set the width of the blocks recursively, even the ones that are not shown */
            /* After a resize event, we want to make sure that the scroll stays at the same line
             * This is why current_widget_y_size */
            bool resize_event = false;
            float current_widget_y_size = 0.f;
            float updated_current_widget_y_size = 0.f;
            for (auto pair : m_mem.getElements()) {
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

            manage_cursors();

            if (!m_mem.empty()) {
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
                for (auto pair : m_mem.getElements()) {
                    const auto& bounds = pair.second->get().m_text_boundaries;
                    TimeCounter::getInstance().startCounter("Clear offsets");
                    ctx.x_offset.clear(bounds.front().line_number, bounds.back().line_number);
                    TimeCounter::getInstance().stopCounter("Clear offsets");

                    ctx.lines = &pair.second->m_lines;
                    if (!found_current && pair.first >= m_mem.getCurrentBlockIdx()) {
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
             // if (resize_event && abs(current_widget_y_size - updated_current_widget_y_size))
             //     m_mem.gotoLine(m_current_block_idx);

            ImVec2 rel_pos = ImVec2(mouse_pos.x - vMin.x, mouse_pos.y - vMin.y);
            ImGui::End();
            ImGui::PopStyleColor();

            debug_window();
        }
    }

    /* ======================
     * Text Cursor management
     * ====================== */
    void Page::manage_cursors() {
        if (m_text_cursors.empty()) {
            m_text_cursors.push_back(TextCursor());
        }
        // for (auto& cursor : m_text_cursors) {
        //     int line_number = cursor.getCurrentLine();
        //     const auto& bounds = m_file->getBlocksBoundsContaining(line_number, line_number);
        //     if (m_root_elements.find(bounds.start.block_idx) == m_root_elements.end())
        //         continue;
        //     cursor.draw(&m_root_elements, m_file);
        // }
    }

    /* ================================
     * Scrolling and element management
     * ================================ */
    void Page::display_scrollbar(const Rect& b) {
        /* Find approximate height before the first parsed block
         * and after the last parsed block */
        int num_lines_before = m_mem.getNumLineBefore();
        int num_lines_after = m_mem.getNumLineAfter();

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
        float total_height = remaining_height;
        while (true) {
            /* Go to the next ptr */
            bool ret = m_mem.nextPtr();
            if (ret) {
                if (m_mem.isCurrentBlockAtTrueEnd())
                    arrived_at_end = true;
                break;
            }
            // auto next = find_next_ptr();
            // /* We arrived at the end of file */
            // if (next == m_root_elements.end()) {
            //     if (std::prev(next)->first != m_file->m_blocks.size() - 1)
            //         arrived_at_end = true;
            //     break;
            // }
            // go_to_line(next->second->get().m_text_boundaries.front().line_number);

            float element_height = m_current_block_ptr->get().m_ext_dimensions.h;
            total_height += element_height;
            if (pixels < total_height) {
                m_y_displacement = element_height - (total_height - pixels);
                return;
            }
        }
        if (arrived_at_end && !m_mem.empty()) {
            auto last_line = m_mem.getLastBlock()->get().m_text_boundaries.back().line_number;
            /* Estimate the average line height with the current informations */
            float estimated_additional_lines = (pixels - total_height) / m_line_height;
            int next_line = last_line + (int)estimated_additional_lines;
            m_mem.gotoLine(next_line);
            std::cout << "Change line " << next_line << std::endl;
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
        if (pixels <= m_y_displacement || m_mem.getCurrentBlockIdx() == 0) {
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
            bool ret = m_mem.prevPtr();
            if (ret) {
                if (m_mem.isCurrentBlockAtTrueBeg())
                    arrived_at_beg = true;
                break;
            }
            // auto prev = find_prev_ptr();
            // if (prev == m_root_elements.begin() && prev->first > 0 || prev == m_root_elements.end()) {
            //     arrived_at_beg = true;
            //     break;
            // }
            // go_to_line(prev->second->get().m_text_boundaries.front().line_number);
            m_current_block_ptr = m_mem.getCurrentBlock();
            if (m_current_block_ptr->get().m_ext_dimensions.h == 0.f) {
                continue;
            }
            float element_height = m_current_block_ptr->get().m_ext_dimensions.h;
            total_height += element_height;
            if (pixels <= total_height) { // || prev == m_root_elements.begin()) {
                m_y_displacement = total_height - pixels;
                if (m_y_displacement < 0.f)
                    m_y_displacement = 0.f;
                return;
            }
        }
        if (arrived_at_beg && !m_mem.empty()) {
            auto first_line = m_current_block_ptr->get().m_text_boundaries.front().line_number;
            float estimated_additional_lines = (pixels - total_height) / m_line_height;
            int estimated_line = first_line - (int)estimated_additional_lines;
            if (estimated_line < 0)
                estimated_line = 0;
            m_mem.gotoLine(estimated_line);
            std::cout << "Change line " << estimated_line << std::endl;
        }
    }
    Page::~Page() {
    }
}