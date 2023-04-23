#include "ui_utils.h"
#include "colors.h"
#include "types.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

// Data for resizing from corner
struct ImGuiResizeGripDef {
    ImVec2  CornerPosN;
    ImVec2  InnerDir;
    int     AngleMin12, AngleMax12;
};
static const ImGuiResizeGripDef resize_grip_def[4] = {
    { ImVec2(1, 1), ImVec2(-1, -1), 0, 3 },  // Lower-right
    { ImVec2(0, 1), ImVec2(+1, -1), 3, 6 },  // Lower-left
    { ImVec2(0, 0), ImVec2(+1, +1), 6, 9 },  // Upper-left (Unused)
    { ImVec2(1, 0), ImVec2(-1, +1), 9, 12 }  // Upper-right (Unused)
};

bool isOnTop(const std::string& window_name) {

    ImGuiContext& g = *GImGui;

    auto windows = g.Windows;
    auto current_window = ImGui::FindWindowByName(window_name.c_str());

    auto mouse_pos = ImGui::GetMousePos();

    /* Check for resize grips (code taken from ImGui::UpdateWindowManualResize) */
    {
        const ImGuiResizeGripDef& def = resize_grip_def[0];
        const ImVec2 corner = ImLerp(current_window->Pos, current_window->Pos + current_window->Size, def.CornerPosN);
        static const float WINDOWS_HOVER_PADDING = 4.0f;
        const int resize_grip_count = g.IO.ConfigWindowsResizeFromEdges ? 2 : 1; // Allow resize from lower-left if we have the mouse cursor feedback for it.
        const float grip_draw_size = IM_FLOOR(ImMax(g.FontSize * 1.35f, current_window->WindowRounding + 1.0f + g.FontSize * 0.2f));
        const float grip_hover_inner_size = IM_FLOOR(grip_draw_size * 0.75f);
        const float grip_hover_outer_size = g.IO.ConfigWindowsResizeFromEdges ? WINDOWS_HOVER_PADDING : 0.0f;
        ImRect rect(corner - def.InnerDir * grip_hover_outer_size, corner + def.InnerDir * grip_hover_inner_size);
        if (rect.Min.x > rect.Max.x) ImSwap(rect.Min.x, rect.Max.x);
        if (rect.Min.y > rect.Max.y) ImSwap(rect.Min.y, rect.Max.y);

        if (isInsideRect(mouse_pos, { rect.Min.x, rect.Min.y, rect.Max.x - rect.Min.x, rect.Max.y - rect.Min.y }))
            return false;
    }


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