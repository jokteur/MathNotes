#include "scrollbar.h"

#include "ui/ui_utils.h"

VerticalScrollBar::VerticalScrollBar(UIState_ptr ui_state, Mode mode)
    : Drawable(ui_state), m_display_mode(mode) {
}

void VerticalScrollBar::FrameUpdate(const Rect& b, Draw::DrawList& m_draw_list, float before, float after, const std::string& window_name) {
    /* Scroll bar properties */
    float scroll_width = ImGui::GetStyle().ScrollbarSize;
    float rounding = ImGui::GetStyle().ScrollbarRounding;
    auto color_bg = ImGui::GetColorU32(ImGuiCol_ScrollbarBg);

    float display_height = b.h;
    float elements_height = before + after + display_height;

    float scroll_height = b.h * (display_height / elements_height);
    if (scroll_height < m_min_scroll_height) {
        scroll_height = m_min_scroll_height;
    }
    if (before + after > 0.f)
        m_percentage = before / (before + after);

    float scroll_pos = b.y + (b.h - scroll_height) * m_percentage;

    ImVec2 top_left(5 + b.w + b.x - scroll_width, scroll_pos);
    ImVec2 bottom_right(5 + b.w + b.x, scroll_pos + scroll_height);
    Rect scroll_box = { 5 + b.w + b.x - scroll_width,b.y, scroll_width, b.h };

    /* Check for user input on scrollbar */
    m_has_changed = false;
    ImVec2 mouse_pos = ImGui::GetMousePos();
    bool is_left_down = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool is_mouse_down = is_left_down || ImGui::IsMouseDown(ImGuiMouseButton_Middle);
    bool is_inside = isInsideRect(mouse_pos, scroll_box);
    if (is_inside && is_mouse_down && isOnTop(window_name)) {
        /* First grab */
        if (!m_grabbing) {
            /* User grabbed the scrollbar directly */
            if (mouse_pos.y >= top_left.y && mouse_pos.y <= bottom_right.y) {
                m_y_grab_pos = mouse_pos.y - top_left.y;
            }
            /* User clicked in the scrollbar zone, away from window borders */
            else if (mouse_pos.y >= b.y + scroll_height && mouse_pos.y <= b.y + b.h - scroll_height) {
                m_y_grab_pos = scroll_height / 2.f;
            }
            /* User clicked in the scrollbar zone, near top window border */
            else if (mouse_pos.y <= b.y + scroll_height) {
                m_y_grab_pos = mouse_pos.y - b.y;
            }
            /* User clicked in the scrollbar zone, near bottom window border */
            else {
                m_y_grab_pos = scroll_height - (b.h + b.y - mouse_pos.y);
            }
        }
        m_grabbing = true;
    }
    if (m_grabbing && !is_left_down) {
        m_grabbing = false;
    }

    if (m_grabbing) {
        float relative_top_mouse_pos = mouse_pos.y - m_y_grab_pos - b.y;
        float available_scroll_height = display_height - scroll_height;
        float new_percentage = relative_top_mouse_pos / available_scroll_height;
        if (new_percentage < 0.f)
            new_percentage = 0.f;
        else if (new_percentage > 1.f)
            new_percentage = 1.f;
        if (new_percentage != m_percentage) {
            m_percentage = new_percentage;
            scroll_pos = b.y + (b.h - scroll_height) * m_percentage;
            top_left = ImVec2(5 + b.w + b.x - scroll_width, scroll_pos);
            bottom_right = ImVec2(5 + b.w + b.x, scroll_pos + scroll_height);
            m_has_changed = true;
        }
    }

    /* Draw finaly the scrollbar */
    m_draw_list->AddRectFilled(top_left, bottom_right, color_bg, rounding);
}