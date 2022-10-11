#include <fstream>
#include <chrono>

#include "main_window.h"
#include "style.h"
#include "fonts.h"

#include "rich_text/rich_text_widget.h"
#include "imgui_internal.h"
#include "translations/translate.h"
#include "imgui_stdlib.h"

// namespace py = pybind11;

void setFonts(UIState_ptr state) {
    using namespace Fonts;
    using Fs = FontStyling;
    // Regular fonts
    state->font_manager.setFontPath(Fs{ F_REGULAR, W_THIN, S_NORMAL }, "data/fonts/Ubuntu/Ubuntu-Th.ttf");
    state->font_manager.setFallBack(Fs{ F_REGULAR, W_THIN, S_ITALIC }, Fs{ F_REGULAR, W_LIGHT, S_ITALIC });
    state->font_manager.setFontPath(Fs{ F_REGULAR, W_LIGHT, S_NORMAL }, "data/fonts/Ubuntu/Ubuntu-L.ttf");
    state->font_manager.setFontPath(Fs{ F_REGULAR, W_LIGHT, S_ITALIC }, "data/fonts/Ubuntu/Ubuntu-Li.ttf");
    state->font_manager.setFontPath(Fs{ F_REGULAR, W_REGULAR, S_NORMAL }, "data/fonts/Ubuntu/Ubuntu-R.ttf");
    state->font_manager.setFontPath(Fs{ F_REGULAR, W_REGULAR, S_ITALIC }, "data/fonts/Ubuntu/Ubuntu-RI.ttf");
    state->font_manager.setFontPath(Fs{ F_REGULAR, W_MEDIUM, S_NORMAL }, "data/fonts/Ubuntu/Ubuntu-M.ttf");
    state->font_manager.setFontPath(Fs{ F_REGULAR, W_MEDIUM, S_ITALIC }, "data/fonts/Ubuntu/Ubuntu-MI.ttf");
    state->font_manager.setFontPath(Fs{ F_REGULAR, W_BOLD, S_NORMAL }, "data/fonts/Ubuntu/Ubuntu-B.ttf");
    state->font_manager.setFontPath(Fs{ F_REGULAR, W_BOLD, S_ITALIC }, "data/fonts/Ubuntu/Ubuntu-BI.ttf");

    // Monospace fonts
    state->font_manager.setWeightFallBack(F_MONOSPACE, W_THIN, W_REGULAR);
    state->font_manager.setWeightFallBack(F_MONOSPACE, W_LIGHT, W_REGULAR);
    state->font_manager.setWeightFallBack(F_MONOSPACE, W_MEDIUM, W_REGULAR);
    state->font_manager.setFontPath(Fs{ F_MONOSPACE, W_LIGHT, S_NORMAL }, "data/fonts/Ubuntu/UbuntuMono-R.ttf");
    state->font_manager.setFontPath(Fs{ F_MONOSPACE, W_LIGHT, S_ITALIC }, "data/fonts/Ubuntu/UbuntuMono-Ri.ttf");
    state->font_manager.setFontPath(Fs{ F_MONOSPACE, W_BOLD, S_NORMAL }, "data/fonts/Ubuntu/UbuntuMono-B.ttf");
    state->font_manager.setFontPath(Fs{ F_MONOSPACE, W_BOLD, S_ITALIC }, "data/fonts/Ubuntu/UbuntuMono-BI.ttf");

    // Condensed fonts
    state->font_manager.setStyleFallBack(F_CONDENSED, S_ITALIC, S_NORMAL);
    state->font_manager.setWeightFallBack(F_CONDENSED, W_THIN, W_REGULAR);
    state->font_manager.setWeightFallBack(F_MONOSPACE, W_LIGHT, W_REGULAR);
    state->font_manager.setWeightFallBack(F_MONOSPACE, W_MEDIUM, W_REGULAR);
    state->font_manager.setWeightFallBack(F_MONOSPACE, W_BOLD, W_REGULAR);
    state->font_manager.setFontPath(Fs{ F_CONDENSED, W_REGULAR, S_NORMAL }, "data/fonts/Ubuntu/Ubuntu-C.ttf");

    FontRequestInfo default_font;
    default_font.size_wish = 18.f;
    default_font.font_styling = Fs{ F_REGULAR, W_REGULAR, S_NORMAL };
    default_font.auto_scaling = true;
    state->font_manager.setDefaultFont(default_font);

    // merge in icons
    ImVector< ImWchar> icons_ranges;
    icons_ranges.push_back(static_cast<ImWchar>(ICON_MIN_MD));
    icons_ranges.push_back(static_cast<ImWchar>(ICON_MAX_MD));
    icons_ranges.push_back(static_cast<ImWchar>(0));
    ImFontConfig icons_config;
    // icons_config.PixelSnapH = true;
    icons_config.GlyphOffset = ImVec2(0, 4.f);
    icons_config.MergeMode = true;

    // Tempo::AddIconsToFont(state->font_regular,
    //     "data/fonts/Icons/material-design-icons/MaterialIcons-Regular.ttf", icons_config, icons_ranges
    // );
}
int TextInputCallback(ImGuiInputTextCallbackData* data) {
    auto main_app = static_cast<MainApp*>(data->UserData);
    main_app->m_insert_at = data->CursorPos;
    return 1;
}
MainApp::MainApp() : m_rich_text(m_ui_state) {
}
void MainApp::InitializationBeforeLoop() {
    setFonts(m_ui_state);
    defineStyle();
}
void MainApp::AfterLoop() {
    // ImPlot::DestroyContext();
}

void MainApp::FrameUpdate() {
    using namespace Fonts;
    FontRequestInfo font;
    font.font_styling = FontStyling{ F_REGULAR, W_REGULAR, S_NORMAL };
    font.size_wish = m_font_size;
    FontInfoOut f_out;
    auto err = m_ui_state->font_manager.requestFont(font, f_out);

    ImGui::Begin("My window");
    ImGui::InputTextMultiline("input", &m_in_text, ImVec2(0, 0),
        ImGuiInputTextFlags_CallbackAlways, TextInputCallback, (void*)this);

    if (ImGui::Button("Markdown")) {
    }
    ImGui::End();
}

void MainApp::updateFontSize(float size) {
}

void MainApp::update_text(bool& capture_latex, std::string& tmp_text, std::string& tmp_latex) {
    // if (!capture_latex) {
    //     capture_latex = true;
    //     if (tmp_text.empty())
    //         return;
    //     using namespace Fonts;
    //     FontRequestInfo font;
    //     font.font_styling = FontStyling{ F_REGULAR, W_REGULAR, S_NORMAL };
    //     font.size_wish = m_font_size;
    //     FontInfoOut out;
    //     auto err = m_ui_state->font_manager.requestFont(font, out);
    //     auto res = Utf8StrToImCharStr(
    //         tmp_text,
    //         Tempo::GetImFont(out.font_id),
    //         out.size * m_zoom * Tempo::GetScaling(),
    //         microtex::BLACK
    //     );
    //     for (auto& c : res) {

    //         m_text.emplace_back(c);
    //     }
    //     tmp_text.clear();
    // }
    // else {
    //     capture_latex = false;
    //     if (tmp_latex.empty())
    //         return;
    //     auto c = RichText::ToLatexChar(tmp_latex,
    //         m_font_size * m_zoom * Tempo::GetScaling(),
    //         7.f, microtex::BLACK,
    //         ImVec2(1.f, 1.f),
    //         ImVec2(m_zoom * 5.f, 0.f)
    //     );
    //     m_text.emplace_back(c);
    //     tmp_latex.clear();
    // }
}
void MainApp::insertBigString() {
    // m_in_text.clear();
    // for (int i = 0;i < m_text_size;i++) {
    //     m_in_text += 32 + i % 90;
    // }
    // auto res = Utf8StrToImCharStr(
    //     m_in_text,
    //     Tempo::GetImFont(m_font_regular32),
    //     m_font_size * m_zoom * Tempo::GetScaling(),
    //     microtex::BLACK
    // );
    // wrapper.clear();
}
void MainApp::BeforeFrameUpdate() {
}