#pragma once

#include <tempo.h>

#include "state.h"
#include "misc/cpp/imgui_stdlib.h"
#include "rich_text/chars/im_char.h"
#include "rich_text/chars/latex_char.h"
#include "latex/latex.h"
#include "imgui_internal.h"

// Drawable and widgets
int TextInputCallback(ImGuiInputTextCallbackData* data);

using namespace RichText;
class MainApp : public Tempo::App {
private:
    std::shared_ptr<UIState> m_ui_state = std::make_shared<UIState>();

    RichTextWidget m_rich_text;
    std::vector<DrawableCharPtr> m_text;
    std::string m_in_text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n"
        "Proin pulvinar urna non eros vehicula efficitur.\n\n"

        "Phasellus eget nunc bibendum lectus dapibus tempor nec vitae neque.\n"
        "Ut euismod augue quis libero vulputate placerat.";

    //bool m_open = true;
    friend int TextInputCallback(ImGuiInputTextCallback* data);

    void updateFontSize(float size);
    void update_text(bool& capture_latex, std::string& tmp_text, std::string& tmp_latex);
    void insertBigString();
public:
    int m_insert_at = 0; // Just for
    MainApp();
    virtual ~MainApp() {}

    void InitializationBeforeLoop() override;

    void AfterLoop() override;
    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};