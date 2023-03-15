#pragma once

#include <tempo.h>

#include "state.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"
#include "rich_text/rich_text_widget.h"

// Drawable and widgets
int TextInputCallback(ImGuiInputTextCallbackData* data);

using namespace RichText;
class MainApp: public Tempo::App {
private:
    std::shared_ptr<UIState> m_ui_state = std::make_shared<UIState>();

    RichTextWidget m_rich_text;

    std::string m_in_text;
    std::string m_prev_text;
    std::string m_big_text;
    bool text_set = true;

    //bool m_open = true;
    friend int TextInputCallback(ImGuiInputTextCallback* data);
public:
    int m_insert_at = 0; // Just for
    MainApp();
    virtual ~MainApp() {}

    void InitializationBeforeLoop() override;

    void AfterLoop() override;
    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};