#include <fstream>
#include <chrono>

#include "main_window.h"
#include "style.h"
#include "fonts.h"
#include "utils.h"

#include "ab/ab_file.h"
#include "imgui_internal.h"
#include "translations/translate.h"
#include "imgui_stdlib.h"

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
    state->font_manager.setFontPath(Fs{ F_MONOSPACE, W_REGULAR, S_NORMAL }, "data/fonts/Ubuntu/UbuntuMono-R.ttf");
    state->font_manager.setFontPath(Fs{ F_MONOSPACE, W_REGULAR, S_ITALIC }, "data/fonts/Ubuntu/UbuntuMono-Ri.ttf");
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
MainApp::MainApp() {

}
void MainApp::InitializationBeforeLoop() {
    setFonts(m_ui_state);
    defineStyle();
    getFileContents("data/bigfile.md", m_big_text);
    // for (int i = 0;i < 3;i++)
    //     m_big_text += m_big_text;
}
void MainApp::AfterLoop() {
}

void MainApp::FrameUpdate() {
    using namespace RichText;
    ImGui::Begin("My window");
    ImGui::InputTextMultiline("input", &m_in_text, ImVec2(0, 0),
        ImGuiInputTextFlags_CallbackAlways, TextInputCallback, (void*)this);

    if (!text_set) {
        auto t1 = std::chrono::high_resolution_clock::now();
        if (m_ab_file != nullptr)
            delete m_ab_file;

        m_ab_file = new AB::File(m_big_text, false);

        auto t2 = std::chrono::high_resolution_clock::now();
        auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
        std::cout << ms_int.count() << "ms (parse file) ";

        t1 = std::chrono::high_resolution_clock::now();
        m_ab_file->getBlocksBoundsContaining(30, 135488);
        t2 = std::chrono::high_resolution_clock::now();
        auto mu_int = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        std::cout << mu_int.count() << "mus (test)";
        std::cout << " " << sizeof(*m_ab_file) << std::endl;
        std::cout << std::endl;

        std::cout << m_ab_file->m_blocks.size() * (sizeof(AB::RootBlock) + 24) << std::endl;

        if (m_widget_manager != nullptr) {
            delete m_widget_manager;
        }
        t1 = std::chrono::high_resolution_clock::now();
        m_widget_manager = new WidgetManager(*m_ab_file, m_ui_state);
        m_widget_id = m_widget_manager->createWidget(WidgetConfig{ 0.f, 1000, true });
        m_widget_manager->createWidget(WidgetConfig{ 0.f, 1000, true });
        m_widget_manager->createWidget(WidgetConfig{ 0.f, 1000, true });

        text_set = true;
    }

    if (m_widget_manager != nullptr) {
        auto widget = m_widget_manager->getWidget(m_widget_id);
        m_widget_manager->manage();
        widget.draw();
    }

    // if (m_in_text != m_prev_text) {
    //     m_prev_text = m_in_text;
    //     m_rich_text.setText(m_in_text);
    // }

    if (ImGui::Button("SetBigText")) {
        text_set = false;
    }
    ImGui::End();

    // m_rich_text.FrameUpdate();
}

void MainApp::BeforeFrameUpdate() {
}