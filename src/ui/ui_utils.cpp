#include "ui_utils.h"
#include "colors.h"
#include "types.h"

#include "imgui_internal.h"

bool isOnTop(const std::string& window_name) {

    ImGuiContext& g = *GImGui;

    auto windows = g.Windows;
    auto current_window = ImGui::FindWindowByName(window_name.c_str());

    auto mouse_pos = ImGui::GetMousePos();

    /* First check if no popups in front of our window */
    for (int i = 0; i < g.OpenPopupStack.Size; i++) {
        ImGuiWindow* window = g.OpenPopupStack[i].Window;
        Rect window_rect = { window->Pos.x, window->Pos.y, window->Size.x, window->Size.y };
        if (isInsideRect(mouse_pos, window_rect))
            return false;
    }

    bool found_current = false;
    for (int i = g.Windows.Size - 1; i >= 0; i--) {
        ImGuiWindow* window = g.Windows[i];

        Rect window_rect = { window->Pos.x, window->Pos.y, window->Size.x, window->Size.y };
        if (current_window->ID == window->ID) {
            found_current = true;
        }
        else if (!found_current && isInsideRect(mouse_pos, window_rect))
            return false;
    }
    return true;
}