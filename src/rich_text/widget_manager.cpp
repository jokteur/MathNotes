#include "widget_manager.h"

using namespace AB;

namespace RichText {
    void Widget::draw() {
        manage_elements();

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

        if (!m_root_elements.empty()) {
            Rect boundaries;
            float y_cursor = m_y_scroll;
            boundaries.h = vMax.y - vMin.y;
            boundaries.w = width;
            m_draw_list.SetImDrawList(ImGui::GetWindowDrawList());

            // Background, ForeGround
            m_draw_list.Split(2);
            m_draw_list.SetCurrentChannel(1);
            for (auto pair : m_root_elements) {
                pair.second->draw(m_draw_list, y_cursor, 0.f, boundaries);
            }
            m_draw_list.Merge();
        }
        if (isInsideRect(mouse_pos, Rect{ vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y })) {
            m_y_scroll += ImGui::GetIO().MouseWheel * 40;
            if (m_y_scroll > 0.f)
                m_y_scroll = 0.f;
        }

        ImVec2 rel_pos = ImVec2(mouse_pos.x - vMin.x, mouse_pos.y - vMin.y);
        ImGui::End();
        ImGui::PopStyleColor();

        if (m_current_width != width) {
            m_current_width = width;
            for (auto pair : m_root_elements) {
                pair.second->setWidth(width);
            }
        }
    }

    void Widget::manage_elements() {
        int half_window = m_config.line_lookahead_window / 2;
        int start_line = m_current_line - half_window;
        int end_line = m_current_line + half_window;
        auto bounds = m_file->getBlocksBoundsContaining(start_line, end_line);

        int start = bounds.start.block_idx;
        int end = bounds.end.block_idx;

        std::unordered_set<AB::RootBlockPtr> to_destroy;

        /* Widget was just created or jumped to another location */
        if (m_block_idx_end == -1 || start >= m_block_idx_end) {
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
                for (int i = m_block_idx_start; i < start;i++) {
                    to_destroy.insert(m_file->m_blocks[i]);
                }
            }
            /* Blocks to build after */
            if (end > m_block_idx_end) {
                ABToWidgets parser;
                parser.parse(m_file, m_block_idx_end, end, &m_root_elements, m_ui_state);
            }
            /* Blocks to destroy after */
            else if (end < m_block_idx_end) {
                for (int i = end; i < m_block_idx_end;i++) {
                    to_destroy.insert(m_file->m_blocks[i]);
                }
            }

            for (auto ptr : to_destroy) {
                m_root_elements.erase(ptr);
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
