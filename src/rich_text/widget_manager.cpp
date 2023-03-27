#include "widget_manager.h"

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

    void Widget::build_elements() {
    }

    WidgetManager::WidgetManager(const File& file, UIState_ptr ui_state): m_file(file), m_empty_widget(nullptr) {
        m_ui_state = ui_state;
    }

    WidgetId WidgetManager::createWidget(const WidgetConfig& config) {
        WidgetId widget_id = 1;

        bool available_slot = false;
        for (int i = 1;i <= 32;i++) {
            if (m_current_widgets & widget_id) {
                available_slot = true;
                break;
            }
            widget_id << 1;
        }

        if (!available_slot)
            return 0;

        m_current_widgets |= widget_id;

        Widget widget(m_ui_state);
        widget.m_config = config;
        widget.m_file = &m_file;
        widget.m_current_line = config.line_start;
        auto pair = m_widgets.emplace(std::pair<WidgetId, Widget>(widget_id, widget));
        return widget_id;
    }
    void WidgetManager::removeWidget(WidgetId id) {
        auto it = m_widgets.find(id);
        if (it != m_widgets.end()) {
            m_widgets.erase(it);
            m_current_widgets ^= id;
        }
    }
    Widget& WidgetManager::getWidget(WidgetId id) {
        auto it = m_widgets.find(id);
        if (it != m_widgets.end()) {
            return it->second;
        }
        return m_empty_widget;
    }
    WidgetManager::~WidgetManager() {
    }

    void WidgetManager::manage() {
        int widget_id = 1;
        for (int i = 1;i <= 32;i++) {
            widget_id << 1;
        }
    }
}
