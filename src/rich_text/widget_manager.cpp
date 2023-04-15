#include "widget_manager.h"

#include <cmath>

#include "rich_text/chars/im_char.h"

using namespace AB;

namespace RichText {
    void Widget::debug_window() {
        ImGui::Begin("Widget info");
        ImGui::Text("y disp: %f", m_y_displacement);
        ImGui::Text("current_line: %d", m_current_line);
        ImGui::Text("current block idx: %d", m_current_block_idx);
        ImGui::End();

        ImGui::Begin("Parsed blocks");
        if (ImGui::TreeNode("Show")) {
            for (auto pair : m_root_elements) {
                pair.second->hk_debug(std::to_string(pair.first));
            }
            ImGui::TreePop();
        }
        ImGui::End();

        ImGui::Begin("Top displayed block");
        if (m_current_block_ptr != nullptr) {
            m_current_block_ptr->hk_debug();
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

    void Widget::draw() {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::Begin("RichText window");
        float width = ImGui::GetWindowContentRegionWidth();
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
        find_current_ptr();

        /* Set the width of the blocks recursively, even the ones that are not shown */
        for (auto pair : m_root_elements) {
            if (m_current_width != width || pair.second->m_widget_dirty) {
                pair.second->setWidth(width);
            }
        }
        if (m_current_width != width) {
            m_current_width = width;
        }


        if (!m_root_elements.empty()) {
            Rect boundaries;
            float y_cursor = roundf(m_y_scroll);
            boundaries.y = 0.f;
            boundaries.h = vMax.y - vMin.y;
            boundaries.w = width;
            m_draw_list.SetImDrawList(ImGui::GetWindowDrawList());

            m_draw_list->AddRect(vMin, vMax, Colors::red, 0.f, 0, 2.f);

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
            float y_pos = m_display_height + 1000.f;
            /* Designates the height taken by the elements before the current one */
            float before_height = 0.f;
            for (auto pair : m_root_elements) {
                if (!found_current && pair.first >= m_current_block_idx) {
                    found_current = true;
                    before_height = y_pos - m_display_height - 1000.f;
                    y_pos = -m_y_displacement;
                }
                pair.second->draw(m_draw_list, y_pos, 0.f, boundaries);
            }
            m_draw_list.Merge();
        }
        if (isInsideRect(mouse_pos, Rect{ vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y })) {
            float mouse_wheel = ImGui::GetIO().MouseWheel;
            if (ImGui::IsKeyDown(ImGuiKey_ModShift))
                mouse_wheel *= 70;
            else
                mouse_wheel *= 20;

            if (mouse_wheel > 0.f) {
                scroll_up(mouse_wheel);
            }
            else if (mouse_wheel < 0.f) {
                scroll_down(mouse_wheel);
            }
        }

        ImVec2 rel_pos = ImVec2(mouse_pos.x - vMin.x, mouse_pos.y - vMin.y);
        ImGui::End();
        ImGui::PopStyleColor();

        debug_window();
    }

    void Widget::calculate_heights() {
        if (m_recalculate_line_height) {
            using namespace Fonts;
            FontRequestInfo font_request;
            font_request.size_wish = 18.f;
            FontInfoOut font_out;
            m_ui_state->font_manager.requestFont(font_request, font_out);
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
    std::map<int, AbstractElementPtr>::iterator Widget::find_prev_ptr() {
        auto current_it = m_root_elements.find(m_current_block_idx);
        if (current_it == m_root_elements.end() || current_it == m_root_elements.begin())
            return current_it;
        return std::prev(current_it);
    }
    std::map<int, AbstractElementPtr>::iterator Widget::find_next_ptr() {
        auto current_it = m_root_elements.find(m_current_block_idx);
        if (current_it == m_root_elements.end()) {
            return current_it;
        }
        auto next = std::next(current_it);
        return next;
    }
    void Widget::find_current_ptr() {
        auto bounds = m_file->getBlocksBoundsContaining(m_current_line, m_current_line + 1);
        if (m_root_elements.find(bounds.start.block_idx) != m_root_elements.end()) {
            m_current_block_idx = bounds.start.block_idx;
            m_current_block_ptr = m_root_elements[m_current_block_idx];
        }
    }
    void Widget::go_to_line(int line_number) {
        m_current_line = line_number;
        find_current_ptr();
        if (m_current_block_ptr != nullptr) {
            m_current_line = m_current_block_ptr->m_text_boundaries.front().line_number;
        }
    }
    void Widget::scroll_down(float pixels) {
        if (m_current_block_ptr == nullptr)
            return;

        pixels = abs(pixels);

        /* First check if with the scroll, we stay within the element */
        float remaining_height = m_current_block_ptr->m_dimensions.y - m_y_displacement;
        if (pixels < remaining_height) {
            m_y_displacement += pixels;
            return;
        }
        bool arrived_at_end = false;
        float total_height = remaining_height;
        while (true) {
            /* Go to the next ptr */
            auto next = find_next_ptr();
            if (next == m_root_elements.end()) {
                arrived_at_end = true;
                break;
            }
            go_to_line(next->second->m_text_boundaries.front().line_number);
            float element_height = m_current_block_ptr->m_dimensions.y;
            total_height += element_height;
            if (pixels < total_height) {
                m_y_displacement = element_height - (total_height - pixels);
                return;
            }
        }
    }
    void Widget::scroll_up(float pixels) {
        if (m_current_block_ptr == nullptr)
            return;

        /* First check if with the scroll, we stay within the element */
        float remaining_height = m_current_block_ptr->m_dimensions.y - m_y_displacement;
        if (pixels < m_y_displacement) {
            m_y_displacement -= pixels;
            if (m_y_displacement < 0.f)
                m_y_displacement = 0.f;
            return;
        }
        bool arrived_at_beg = false;
        float total_height = remaining_height;
        while (true) {
            /* Go to the next ptr */
            auto prev = find_prev_ptr();
            if (prev == m_root_elements.begin() && prev->first > 0) {
                arrived_at_beg = true;
                break;
            }
            go_to_line(prev->second->m_text_boundaries.front().line_number);
            float element_height = m_current_block_ptr->m_dimensions.y;
            total_height += element_height;
            if (pixels < total_height) {
                m_y_displacement = 0.f; //total_height - pixels;
                return;
            }
        }
    }

    void Widget::manage_elements() {
        int half_window = m_line_lookahead_window / 2;
        int start_line = m_current_line - half_window;
        int end_line = m_current_line + half_window;
        auto bounds = m_file->getBlocksBoundsContaining(start_line, end_line);

        if (bounds.start.block_idx == -1)
            return;

        int start = bounds.start.block_idx;
        int end = bounds.end.block_idx;

        std::unordered_set<int> to_destroy;

        /* Widget was just created or jumped to another location */
        if (m_block_idx_start >= 0 && (m_block_idx_end == -1 || start >= m_block_idx_end)) {
            if (start >= m_block_idx_end) {
                m_root_elements.clear();
            }
            m_block_idx_start = start;
            m_block_idx_end = end;
            ABToWidgets parser;
            auto t1 = std::chrono::high_resolution_clock::now();
            parser.parse(m_file, m_block_idx_start, m_block_idx_end, &m_root_elements, m_ui_state);
            auto t2 = std::chrono::high_resolution_clock::now();
            auto ms_int = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
            std::cout << ms_int.count() << "mus (parse widgets) " << std::endl;
        }
        else {
            /* Blocks to build before */
            if (start < m_block_idx_start) {
                ABToWidgets parser;
                parser.parse(m_file, start, m_block_idx_start, &m_root_elements, m_ui_state, RichText::MarkdownConfig(), true);
            }
            /* Blocks to destroy before */
            else if (start > m_block_idx_start) {
                std::cout << "Destroy " << m_block_idx_start << " to " << start << " (pre)" << std::endl;
                for (int i = m_block_idx_start; i < start;i++) {
                    if (m_root_elements.find(i) != m_root_elements.end())
                        m_y_scroll += m_root_elements[i]->m_dimensions.y;
                    to_destroy.insert(i);
                }
            }
            /* Blocks to build after */
            if (end > m_block_idx_end) {
                ABToWidgets parser;
                parser.parse(m_file, m_block_idx_end, end, &m_root_elements, m_ui_state);
            }
            /* Blocks to destroy after */
            else if (end < m_block_idx_end) {
                std::cout << "Destroy " << end << " to " << m_block_idx_end << std::endl;
                for (int i = end + 1; i <= m_block_idx_end;i++) {
                    to_destroy.insert(i);
                }
            }

            for (auto idx : to_destroy) {
                m_root_elements.erase(idx);
            }
            m_block_idx_start = start;
            m_block_idx_end = end;
        }
    }

    WidgetManager::WidgetManager(const File& file, UIState_ptr ui_state): m_file(file), m_empty_widget(nullptr) {
        m_ui_state = ui_state;
    }

    WidgetId WidgetManager::createWidget(const WidgetConfig& config) {
        WidgetId widget_id = 1;

        bool available_slot = false;
        for (int i = 1;i <= 32;i++) {
            if (m_current_widgets ^ widget_id) {
                available_slot = true;
                break;
            }
            widget_id = widget_id << 1;
        }

        if (!available_slot)
            return 0;

        m_current_widgets |= widget_id;

        auto widget = std::make_shared<Widget>(m_ui_state);
        widget->m_config = config;
        widget->m_file = &m_file;
        widget->m_current_line = config.line_start;
        m_widgets[widget_id] = widget;
        return widget_id;
    }
    void WidgetManager::removeWidget(WidgetId id) {
        auto it = m_widgets.find(id);
        if (it != m_widgets.end()) {
            m_widgets.erase(it);
            m_current_widgets ^= id;
        }
    }
    WidgetPtr WidgetManager::getWidget(WidgetId id) {
        auto it = m_widgets.find(id);
        if (it != m_widgets.end()) {
            return it->second;
        }
        return nullptr;
    }
    WidgetManager::~WidgetManager() {
    }

    void WidgetManager::manage() {
        int widget_id = 1;
        for (int i = 1;i <= 32;i++) {
            if (m_current_widgets & widget_id) {
                // std::cout << "Managing " << widget_id << " ";
                // auto& widget = getWidget(widget_id);
                // int current_line = widget.m_current_line;
            }
            widget_id = widget_id << 1;
        }
    }
}
