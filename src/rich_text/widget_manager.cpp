#include "widget_manager.h"

#include <cmath>

#include "rich_text/chars/im_char.h"

using namespace AB;

namespace RichText {
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
            bool found_top = false;
            for (auto pair : m_root_elements) {
                pair.second->draw(m_draw_list, y_cursor, 0.f, boundaries);
                if (!found_top && pair.second->m_is_visible) {
                    found_top = true;
                    m_top_displayed_ptr = pair.second;
                    m_current_line = m_top_displayed_ptr->m_text_boundaries.front().line_number;
                }
            }
            m_draw_list.Merge();
        }
        if (isInsideRect(mouse_pos, Rect{ vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y })) {
            if (ImGui::IsKeyDown(ImGuiKey_ModShift)) {
                m_y_scroll += ImGui::GetIO().MouseWheel * 70;
            }
            else
                m_y_scroll += ImGui::GetIO().MouseWheel * 20;
            m_y_scroll = m_y_scroll;
            if (m_y_scroll > 0.f)
                m_y_scroll = 0.f;
        }

        ImVec2 rel_pos = ImVec2(mouse_pos.x - vMin.x, mouse_pos.y - vMin.y);
        ImGui::End();
        ImGui::PopStyleColor();

        debug_window();
    }

    void Widget::debug_window() {
        ImGui::Begin("Debug window");
        if (ImGui::CollapsingHeader("Parsed blocks")) {
            for (auto pair : m_root_elements) {
                pair.second->hk_debug(std::to_string(pair.first));
            }
        }
        if (ImGui::CollapsingHeader("Top displayed block")) {
            if (m_top_displayed_ptr != nullptr) {
                m_top_displayed_ptr->hk_debug();
            }
        }
        ImGui::Text(("y_scroll: " + std::to_string(m_y_scroll)).c_str());
        ImGui::Text(("current_line: " + std::to_string(m_current_line)).c_str());
        ImGui::End();
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
                parser.parse(m_file, start, m_block_idx_start, &m_root_elements, m_ui_state);
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
