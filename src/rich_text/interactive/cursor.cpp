#include "cursor.h"

namespace RichText {
    void TextCursor::draw(std::map<int, RootNodePtr>* roots, AB::File* file) {
        int text_position = getTextPosition();
        m_root_elements = roots;
        m_file = file;

        manage_events();
    }

    void TextCursor::manage_events() {
        if (ImGui::IsWindowFocused()) {
            // Last key pressed ?
            // Order of priority
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
                go_left(ImGui::IsKeyPressed(ImGuiKey_ModCtrl));
            }
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
                go_right(ImGui::IsKeyPressed(ImGuiKey_ModCtrl));
            }
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                go_up();
            }
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                go_down();
            }
        }
    }

    void TextCursor::go_down() {

    }
    void TextCursor::go_up() {

    }
    void TextCursor::go_left(bool ctrl) {

    }
    void TextCursor::go_right(bool ctrl) {

    }

    int TextCursor::getTextPosition() {
        int text_position = m_start;
        if (m_end >= 0 && m_start > m_end) {
            text_position = m_end;
        }
        return text_position;
    }
}