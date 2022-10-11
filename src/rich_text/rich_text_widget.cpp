#include "rich_text_widget.h"
#include "chars/drawable_char.h"

namespace RichText {
    RichTextWidget::RichTextWidget(std::shared_ptr<UIState> ui_state) : Drawable(ui_state) {
        unsigned flags = 0;
        flags |= MD_FLAG_LATEXMATHSPANS | MD_FLAG_PERMISSIVEAUTOLINKS;
        flags |= MD_FLAG_PERMISSIVEURLAUTOLINKS | MD_FLAG_PERMISSIVEWWWAUTOLINKS;
        flags |= MD_FLAG_STRIKETHROUGH | MD_FLAG_TABLES | MD_FLAG_TASKLISTS;
        flags |= MD_FLAG_UNDERLINE | MD_FLAG_WIKILINKS;
        m_md_to_widgets.setBaseFontSize(16.f);
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

        if (!m_chars.empty()) {
            auto draw_list = ImGui::GetWindowDrawList();
            for (auto& c : m_chars) {
                auto ptr = std::static_pointer_cast<DrawableChar>(c);
                ptr->draw(draw_list);
            }
        }
        ImVec2 rel_pos = ImVec2(mouse_pos.x - vMin.x, mouse_pos.y - vMin.y);
        ImGui::End();
        ImGui::PopStyleColor();


        if (m_current_width != width) {
            m_current_width = width;
            m_wrapper.setWidth(m_current_width);
        }
        build_chars();
    }
    void RichTextWidget::setText(const std::string& text) {
        m_safe_string = std::make_shared<std::string>(text);
        m_chars.clear();
        m_tree = m_md_to_widgets.parse(m_safe_string, m_ui_state);

        start_build_chars();
    }

    void RichTextWidget::start_build_chars() {
        m_chars_dirty = true;
        m_widget_dirty = false;
        m_number_of_attempts = 0;
    }
    void RichTextWidget::build_chars() {
        if (m_number_of_attempts > 50) {
            m_chars_dirty = false;
            m_widget_dirty = true;
            return;
        }
        if(!m_chars_dirty)
            return;

        m_chars.clear();
        auto it = m_tree.begin();
        for (;it != m_tree.end();it++) {
            bool success = (*it)->buildAndAddChars(m_chars);
            if (!success) {
                break;
            }
        }
        if (it == m_tree.end()) {
            m_chars_dirty = false;
            m_number_of_attempts = 0;
            m_wrapper.setString(m_chars);
        }
        else {
            // If there hasn't been any success, the font may not be constructed yet
            // and may be at the next frame
            m_number_of_attempts++;
            Tempo::PushAnimation("rich_text_widget", 50);
        }
    }
}