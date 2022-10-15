#include "rich_text_widget.h"
#include "chars/drawable_char.h"

namespace RichText {
    RichTextWidget::RichTextWidget(std::shared_ptr<UIState> ui_state) : Drawable(ui_state) {
        unsigned flags = 0;
        flags |= MD_FLAG_LATEXMATHSPANS | MD_FLAG_PERMISSIVEAUTOLINKS;
        flags |= MD_FLAG_PERMISSIVEURLAUTOLINKS | MD_FLAG_PERMISSIVEWWWAUTOLINKS;
        flags |= MD_FLAG_STRIKETHROUGH | MD_FLAG_TABLES | MD_FLAG_TASKLISTS;
        flags |= MD_FLAG_UNDERLINE | MD_FLAG_WIKILINKS;
        m_md_to_widgets.setFlags(flags);
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
            ImVec2 draw_offset;
            m_tree[0]->draw(ImGui::GetWindowDrawList(), draw_offset);
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
            m_tree[0]->buildWidget();
        }
    }
}