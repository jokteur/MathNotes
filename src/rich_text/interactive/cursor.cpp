#include "cursor.h"

namespace RichText {
    bool TextCursor::is_cursor_in_memory() {
        const auto bound = m_file->getBlocksBoundsContaining(m_current_line, m_current_line + 1);
        return m_mem->isBlockInMemory(bound.start.block_idx);
    }

    void TextCursor::manage() {
        if (ImGui::IsWindowFocused()) {
            // Last key pressed ?
            // Order of priority
            int left = ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true) ? LEFT : 0;
            int right = ImGui::IsKeyPressed(ImGuiKey_RightArrow, true) ? RIGHT : 0;
            int up = ImGui::IsKeyPressed(ImGuiKey_UpArrow, true) ? UP : 0;
            int down = ImGui::IsKeyPressed(ImGuiKey_DownArrow, true) ? DOWN : 0;

            int current_key_presses = left | right | up | down;
            int new_key_presses = current_key_presses & ~m_key_presses;
            int to_press = 0;
            if (new_key_presses != 0) {
                to_press = new_key_presses;
            }
            else if (current_key_presses) {
                to_press = current_key_presses;
            }
            if (to_press & LEFT) {
                go_left(ImGui::GetIO().KeyCtrl);
            }
            else if (to_press & RIGHT) {
                go_right(ImGui::GetIO().KeyCtrl);
            }
            else if (to_press & UP) {
                go_up();
            }
            else if (to_press & DOWN) {
                go_down();
            }
            m_key_presses = current_key_presses;
        }
        else {
            m_key_presses = 0;
        }
    }

    void TextCursor::go_down() {
        std::cout << "go down" << std::endl;
    }
    void TextCursor::go_up() {
        std::cout << "go up" << std::endl;
    }
    void TextCursor::go_left(bool ctrl) {
        // TODO: go to the previous word
        // TODO: UTF-8 support
        if (m_end >= 0 && m_start > m_end) {
            m_start = m_end - 1;
        }
        else {
            m_start--;
        }
        std::cout << "go left" << std::endl;
    }
    void TextCursor::go_right(bool ctrl) {
        if (m_end >= 0 && m_start > m_end) {
            m_start = m_end + 1;
        }
        else {
            m_start++;
        }
        std::cout << "go right" << std::endl;
    }

    int TextCursor::getTextPosition() const {
        int text_position = m_start;
        if (m_end >= 0 && m_start > m_end) {
            text_position = m_end;
        }
        return text_position;
    }
}