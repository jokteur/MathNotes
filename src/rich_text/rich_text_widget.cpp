#include "rich_text_widget.h"
#include "chars/drawable_char.h"

#include "ui/draw_commands.h"

namespace RichText {
    RichTextWidget::RichTextWidget(std::shared_ptr<UIState> ui_state): Drawable(ui_state) {
    }
    void RichTextWidget::FrameUpdate() {
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

        if (!m_tree.empty()) {
            Rect boundaries;
            float y_cursor = m_y_scroll;
            boundaries.h = vMax.y - vMin.y;
            boundaries.w = width;
            m_draw_list.SetImDrawList(ImGui::GetWindowDrawList());

            // Background, ForeGround
            m_draw_list.Split(2);
            m_draw_list.SetCurrentChannel(1);
            m_tree[0]->draw(m_draw_list, y_cursor, 0.f, boundaries);
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
            if (!m_tree.empty()) {
                m_tree[0]->setWidth(width);
            }
        }
    }
    void RichTextWidget::setText(const std::string& text) {
        m_safe_string = std::make_shared<std::string>(text);
        m_tree = m_md_to_widgets.parse(m_safe_string, m_ui_state);
        if (!m_tree.empty()) {
            m_tree[0]->setWidth(m_current_width);
        }
    }
}