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

    float text_size = 300.f;
    float prev_size = 0.f;

    Tempo::FontID m_font_regular32;

    std::vector<DrawableCharPtr> m_text;
    std::string m_in_text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n"
        "Proin pulvinar urna non eros vehicula efficitur.\n\n"

        "Phasellus eget nunc bibendum lectus dapibus tempor nec vitae neque.\n"
        "Ut euismod augue quis libero vulputate placerat.";
    std::string m_prev_text = "";
    float m_current_width = 0.f;
    float m_font_size = 32.f;
    float m_zoom = 1.f;
    float m_prev_zoom = 1.f;
    float m_scaling = 1.f;
    WrapAlgorithm wrapper;
    int m_text_size = 1000;
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