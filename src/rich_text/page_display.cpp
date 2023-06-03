#include "page_display.h"

#include "time_counter.h"
#include "ui/ui_utils.h"

namespace RichText {

    PageDisplay::PageDisplay(PageMemory* mem) : Drawable(), m_mem(mem), m_scrollbar(VerticalScrollBar::FIT_UNTIL_LAST_LINE) {
    }

    void PageDisplay::debugInfo() {
        ImGui::Text("y disp: %f", m_y_displacement);
        ImGui::Text("Heigts (b, a, t): %f %f %f", m_before_height, m_after_height, m_before_height + m_after_height);
    }

    void PageDisplay::FrameUpdate(const std::string& window_name, DrawContext* ctx) {
        m_window_name = window_name;

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
        m_mem->manage();

        AbstractElement::visible_count = 0;

        /* Set the width of the blocks recursively, even the ones that are not shown */
        /* After a resize event, we want to make sure that the scroll stays at the same line
         * This is why current_widget_y_size */
        bool resize_event = false;
        float current_widget_y_size = 0.f;
        float updated_current_widget_y_size = 0.f;
        for (auto pair : m_mem->getElements()) {
            if (m_current_width != width && pair.second == m_mem->getCurrentBlock()) {
                current_widget_y_size = m_mem->getCurrentBlock()->get().m_ext_dimensions.h;
            }
            if (m_current_width != width || pair.second->get().m_widget_dirty & pair.second->get().DIRTY_WIDTH) {
                pair.second->get().setWindowWidth(width);
            }
        }
        if (m_current_width != width) {
            resize_event = true;
            m_current_width = width;
        }

        if (!m_mem->empty()) {
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
            bool found_current = false;
            ctx->cursor_y_pos = m_display_height + 1000.f;
            ctx->draw_list = &m_draw_list;
            ctx->boundaries = boundaries;
            ctx->doc = &m_mem->getWrapDocument();
            /* Designates the height taken by the elements before the current one */

            TimeCounter::getInstance().startCounter("DisplayAll");
            for (auto pair : m_mem->getElements()) {
                const auto& bounds = pair.second->get().m_text_boundaries;
                TimeCounter::getInstance().startCounter("Clear offsets");
                ctx->x_offset.clear(bounds.front().line_number, bounds.back().line_number);
                TimeCounter::getInstance().stopCounter("Clear offsets");

                if (!found_current && pair.first >= m_mem->getCurrentBlockIdx()) {
                    found_current = true;
                    m_before_height = ctx->cursor_y_pos - m_display_height - 1000.f;
                    ctx->cursor_y_pos = -roundf(m_y_displacement);
                    pair.second->get().draw(ctx);
                    updated_current_widget_y_size = pair.second->get().m_ext_dimensions.h;
                    continue;
                }
                pair.second->get().draw(ctx);
            }
            TimeCounter::getInstance().stopCounter("DisplayAll");

            m_after_height = ctx->cursor_y_pos + roundf(m_y_displacement);
            // m_after_height -= 2 * m_line_height;
            if (m_after_height < 0.f)
                m_after_height = 0.f;
            m_draw_list.Merge();

            display_scrollbar(Rect{ vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y });
        }
    }

    /* ================================
     * Scrolling and element management
     * ================================ */
    void PageDisplay::display_scrollbar(const Rect& b) {
        /* Find approximate height before the first parsed block
         * and after the last parsed block */
        int num_lines_before = m_mem->getNumLineBefore();
        int num_lines_after = m_mem->getNumLineAfter();

        float before = m_before_height + m_y_displacement + num_lines_before * m_line_height;
        float after = m_after_height - m_y_displacement + num_lines_after * m_line_height;

        float old_percentage = m_scrollbar.getPercentage();

        m_scrollbar.setMinScrollHeight(m_min_scroll_height);
        m_scrollbar.FrameUpdate(b, m_draw_list, before, after, m_window_name);
        if (m_scrollbar.hasChanged()) {
            float percentage = m_scrollbar.getPercentage();
            float total_height = before + after;
            if (percentage - old_percentage > 0.f) {
                scrollDown((percentage - old_percentage) * total_height);
            }
            else if (percentage - old_percentage < 0.f) {
                scrollUp((old_percentage - percentage) * total_height);
            }
        }
        if (m_scrollbar.isGrabbing())
            m_scrollbar_grab = true;
        else
            m_scrollbar_grab = false;
    }

    void PageDisplay::manage_scroll(const ImVec2& mouse_pos, const Rect& box) {
        if (isInsideRect(mouse_pos, box) && isOnTop(m_window_name)) {
            float mouse_wheel = ImGui::GetIO().MouseWheel;
            mouse_wheel *= 60;

            if (mouse_wheel > 0.f) {
                scrollUp(mouse_wheel);
            }
            else if (mouse_wheel < 0.f) {
                scrollDown(mouse_wheel);
            }
        }
        if (m_y_displacement > m_after_height) {
            m_y_displacement = m_after_height;
        }
    }

    void PageDisplay::calculate_heights() {
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
        float num_pages_for_min_scroll = m_display_height / (m_min_scroll_height * Tempo::GetScaling());
        m_line_lookahead_window = num_pages_for_min_scroll * lines_per_display;
    }
    void PageDisplay::scrollDown(float pixels) {
        //ZoneScoped;
        if (m_mem->getCurrentBlock() == nullptr)
            return;

        pixels = abs(pixels);

        /* First check if with the scroll, we stay within the element */
        float remaining_height = m_mem->getCurrentBlock()->get().m_ext_dimensions.h - m_y_displacement;
        if (pixels < remaining_height) {
            m_y_displacement += pixels;
            return;
        }
        bool arrived_at_end = false;
        float total_height = remaining_height;
        while (true) {
            /* Go to the next ptr */
            bool ret = m_mem->nextPtr();
            if (ret) {
                if (!m_mem->isCurrentBlockAtTrueEnd())
                    arrived_at_end = true;
                break;
            }

            float element_height = m_mem->getCurrentBlock()->get().m_ext_dimensions.h;
            total_height += element_height;
            if (pixels < total_height) {
                m_y_displacement = element_height - (total_height - pixels);
                return;
            }
        }
        if (arrived_at_end && !m_mem->empty()) {
            auto last_line = m_mem->getLastBlock()->get().m_text_boundaries.back().line_number;
            /* Estimate the average line height with the current informations */
            float estimated_additional_lines = (pixels - total_height) / m_line_height;
            int next_line = last_line + (int)estimated_additional_lines;
            m_mem->gotoLine(next_line);
            std::cout << "Change line " << next_line << std::endl;
        }
        if (!arrived_at_end) {
            m_y_displacement = m_mem->getCurrentBlock()->get().m_ext_dimensions.h;
        }
    }
    void PageDisplay::scrollUp(float pixels) {
        //ZoneScoped;
        if (m_mem->getCurrentBlock() == nullptr)
            return;

        /* First check if with the scroll, we stay within the element */
        float remaining_height = m_mem->getCurrentBlock()->get().m_ext_dimensions.h - m_y_displacement;
        if (pixels <= m_y_displacement || m_mem->getCurrentBlockIdx() == 0) {
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
            bool ret = m_mem->prevPtr();
            if (ret) {
                if (!m_mem->isCurrentBlockAtTrueBeg())
                    arrived_at_beg = true;
                break;
            }
            auto current_block_ptr = m_mem->getCurrentBlock();
            if (current_block_ptr->get().m_ext_dimensions.h == 0.f) {
                continue;
            }
            float element_height = current_block_ptr->get().m_ext_dimensions.h;
            total_height += element_height;
            if (pixels <= total_height) { // || prev == m_root_elements.begin()) {
                m_y_displacement = total_height - pixels;
                if (m_y_displacement < 0.f)
                    m_y_displacement = 0.f;
                return;
            }
        }
        if (arrived_at_beg && !m_mem->empty()) {
            auto first_line = m_mem->getCurrentBlock()->get().m_text_boundaries.front().line_number;
            float estimated_additional_lines = (pixels - total_height) / m_line_height;
            int estimated_line = first_line - (int)estimated_additional_lines;
            if (estimated_line < 0)
                estimated_line = 0;
            m_mem->gotoLine(estimated_line);
            std::cout << "Change line " << estimated_line << std::endl;
        }
    }

}