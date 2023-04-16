#pragma once

#include <tempo.h>

#include "state.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"
#include "rich_text/page_manager.h"
#include "ab/ab_file.h"

// Drawable and widgets
int TextInputCallback(ImGuiInputTextCallbackData* data);

class MainApp: public Tempo::App {
private:
    std::shared_ptr<UIState> m_ui_state = std::make_shared<UIState>();

    RichText::PageManager* m_widget_manager = nullptr;
    AB::File* m_ab_file = nullptr;
    RichText::WidgetId m_widget_id;

    std::string m_in_text;
    std::string m_txt;
    std::string m_prev_text;
    std::string m_big_text;
    bool text_set = true;

public:
    int m_insert_at = 0; // Just for
    MainApp();
    virtual ~MainApp() {}

    void InitializationBeforeLoop() override;

    void AfterLoop() override;
    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};