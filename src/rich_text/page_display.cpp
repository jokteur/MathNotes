#include "page_display.h"

#include "time_counter.h"
#include "ui/ui_utils.h"
#include <cmath>

namespace RichText {

    PageDisplay::PageDisplay(PageMemory* mem) : Drawable(), m_mem(mem), m_scrollbar(VerticalScrollBar::FIT_UNTIL_LAST_LINE) {
    }

    void PageDisplay::debugInfo() {
        ImGui::Text("y disp: %f", m_y_displacement);
        ImGui::Text("Heigts (b, a, t): %f %f %f", m_before_height, m_after_height, m_before_height + m_after_height);
    }

    /* ================================
     * Main display and widget building
     * ================================ */
    void PageDisplay::FrameUpdate(const std::string& window_name, DrawContext* ctx) {
        m_window_name = window_name;

        /* Window boundaries */
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
        Rect boundaries;
        boundaries.y = 0.f;
        boundaries.h = vMax.y - vMin.y;
        boundaries.w = width;
        m_draw_list.SetImDrawList(ImGui::GetWindowDrawList());
        ctx->boundaries = boundaries;

        /* Once we know the height of the page,
         * we can estimate how many lines we should
         * look ahead for block element construction */
        m_display_height = vMax.y - vMin.y;
        calculate_heights();
        m_mem->manage();

        AbstractElement::visible_count = 0;

        /* Resize events may shift vertically the content. To keep the content fixed
         * in place, we may need to reshift after rebuilding the blocks
         *  prev_top_block_idx: record the last block display at the top of the page
         *  prev_top_block_shift: record the vertical shift of the current block if it changes
         * */
        int prev_top_block_idx = m_mem->getCurrentBlockIdx();
        Rect prev_top_block_ext_dimensions;
        float prev_top_block_shift = 0.f;
        float prev_percentage = (-m_y_displacement / m_total_height);
        if (!m_mem->getElements().empty()) {
            const auto& element = m_mem->getCurrentBlock()->get();
            prev_top_block_ext_dimensions = element.m_ext_dimensions;
        }
        /* Now set the new width if necessary */
        for (auto pair : m_mem->getElements()) {
            if (m_current_width != width) {
                pair.second->get().setWindowWidth(width);
            }
        }
        bool resize_event = false;
        if (m_current_width != width) {
            resize_event = true;
            m_current_width = width;
        }

        if (!m_scrollbar_grab)
            manage_scroll(mouse_pos, Rect{ vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y });

        /* Building the widgets (no-op if already built)*/
        bool redo_y_positions = false;
        /* We may not scroll further up than the first block */
        if (m_y_displacement > 0.f) {
            m_y_displacement = 0.f;
        }
        /* We may not scroll further down than the last block */
        if (m_total_height < -m_y_displacement) {
            m_y_displacement = -m_total_height;
        }
        if (m_prev_y_displacement != m_y_displacement) {
            redo_y_positions = true;
            m_prev_y_displacement = m_y_displacement;
        }
        bool found_current = false;
        ctx->cursor_y_pos = m_y_displacement;
        TimeCounter::getInstance().startCounter("BuildAll");
        for (auto pair : m_mem->getElements()) {
            /* Each root block begins with an offset of zero */
            ctx->x_offset.clear();
            auto& element = pair.second->get();
            float y_offset = ctx->cursor_y_pos;
            ctx->lines = LinesInfos();
            if (redo_y_positions) {
                element.resetYOrigin();
            }
            element.hk_build(ctx);
            if (pair.first == prev_top_block_idx && resize_event) {
                prev_top_block_shift = y_offset - prev_top_block_ext_dimensions.y;
            }
            if (!found_current && element.is_in_boundaries(ctx->boundaries)) {
                found_current = true;
                m_before_height = y_offset - m_y_displacement - element.m_ext_dimensions.y;
                if (m_mem->getCurrentBlockIdx() != pair.first) {
                    m_mem->setCurrentBlockIdx(pair.first);
                }
            }
            ctx->cursor_y_pos = y_offset + element.m_ext_dimensions.h;
        }
        m_total_height = ctx->cursor_y_pos - m_y_displacement;
        m_after_height = m_total_height - m_before_height;
        TimeCounter::getInstance().stopCounter("BuildAll");

        if (prev_top_block_shift != 0.f)
            std::cout << m_before_height << " ";

        /* If the user has resized the window, the content may shift vertically
         * As explained above, if m_y_displacement != 0 and resize_event, we need to reshift
         * the blocks to match the previously displayed content, AFTER the blocks have been rebuild */
        if (m_y_displacement < 0.f && resize_event && prev_top_block_shift != 0.f) {
            /* prev_top_block_ext_dimensions.y is always negative or zero */
            m_y_displacement -= prev_top_block_shift;
            ctx->cursor_y_pos = m_y_displacement;
            for (auto pair : m_mem->getElements()) {
                /* Each root block begins with an offset of zero */
                ctx->x_offset.clear();
                auto& element = pair.second->get();
                float y_offset = ctx->cursor_y_pos;
                ctx->lines = LinesInfos();
                element.resetYOrigin();
                element.hk_build(ctx);
                if (pair.first == prev_top_block_idx) {
                    m_before_height = y_offset - m_y_displacement - element.m_ext_dimensions.y;
                    if (m_mem->getCurrentBlockIdx() != pair.first) {
                        m_mem->setCurrentBlockIdx(pair.first);
                    }
                }
                ctx->cursor_y_pos = y_offset + element.m_ext_dimensions.h;
            }
            // m_total_height = ctx->cursor_y_pos - m_y_displacement - prev_top_block_shift;
            m_after_height = m_total_height - m_before_height;
        }
        if (prev_top_block_shift != 0.f)
            std::cout << m_before_height << std::endl;

        /* Drawing all the widgets */
        // Background, ForeGround
        m_draw_list.Split(2);
        m_draw_list.SetCurrentChannel(1);

        ctx->draw_list = &m_draw_list;
        /* Designates the height taken by the elements before the current one */
        TimeCounter::getInstance().startCounter("DisplayAll");
        for (auto pair : m_mem->getElements()) {
            pair.second->get().draw(ctx);
        }
        TimeCounter::getInstance().stopCounter("DisplayAll");
        m_draw_list.Merge();

        /* Scrollbar has to be displayed after it has been built, to estime the heights of the page */
        display_scrollbar(Rect{ vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y });
    }

    /* ================================
     * Scrolling and element management
     * ================================ */
    void PageDisplay::display_scrollbar(const Rect& b) {
        if (m_mem->empty())
            return;
        /* Find approximate height before the first parsed block
         * and after the last parsed block */
        int num_lines_before = m_mem->getNumLineBefore();
        int num_lines_after = m_mem->getNumLineAfter();

        float before = m_before_height + num_lines_before * m_line_height;
        float after = m_after_height + num_lines_after * m_line_height;

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

        m_y_displacement -= roundf(pixels);
    }
    void PageDisplay::scrollUp(float pixels) {
        //ZoneScoped;
        if (m_mem->getCurrentBlock() == nullptr)
            return;

        m_y_displacement += roundf(pixels);
    }

}