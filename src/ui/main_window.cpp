#include <fstream>
#include <chrono>

#include "main_window.h"
#include "style.h"
#include "fonts.h"

#include "imgui_internal.h"
#include "translations/translate.h"
#include "imgui_stdlib.h"

// namespace py = pybind11;

int TextInputCallback(ImGuiInputTextCallbackData* data) {
    auto main_app = static_cast<MainApp*>(data->UserData);
    main_app->m_insert_at = data->CursorPos;
    return 1;
}
MainApp::MainApp() : wrapper(0, 500.f) {
}
void MainApp::InitializationBeforeLoop() {
    buildFonts(m_ui_state);
    m_font_regular32 = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Regular.ttf", 32).value();
    defineStyle();
}
void MainApp::AfterLoop() {
    // ImPlot::DestroyContext();
}

void MainApp::FrameUpdate() {
    ImGui::Begin("My window");
    ImGui::InputTextMultiline("input", &m_in_text, ImVec2(0, 0),
        ImGuiInputTextFlags_CallbackAlways, TextInputCallback, (void*)this);
    ImGui::SliderFloat("Zoom", &m_zoom, 0.2f, 1.f, "%.2f");
    ImGui::SliderInt("Size", &m_text_size, 100, 1000000);

    if (ImGui::Button("Insert")) {
        std::string str = "insertion of $$\\int_a^bx^2dx$$ \na few words";
        m_in_text.insert(m_in_text.begin() + m_insert_at, str.begin(), str.end());
    }
    if (ImGui::Button("Delete 10 chars")) {
        int num = 10;
        if (num + m_insert_at > m_text.size()) {
            num = m_text.size() - m_insert_at;
        }
        if (num > 0) {
            wrapper.deleteAt(m_insert_at, m_insert_at + num);
            m_text.erase(m_text.begin() + m_insert_at, m_text.begin() + m_insert_at + num);
            m_in_text.erase(m_in_text.begin() + m_insert_at, m_in_text.begin() + m_insert_at + num);
            m_prev_text.erase(m_prev_text.begin() + m_insert_at, m_prev_text.begin() + m_insert_at + num);
        }
    }
    ImGui::End();
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.f, 1.f, 1.f, 1.f));
    ImGui::Begin("Second window");
    float width = ImGui::GetWindowContentRegionWidth();
    ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    ImVec2 vMax = ImGui::GetWindowContentRegionMax();

    vMin.x += ImGui::GetWindowPos().x;
    vMin.y += ImGui::GetWindowPos().y;
    vMax.x += ImGui::GetWindowPos().x;
    vMax.y += ImGui::GetWindowPos().y;
    auto mouse_pos = ImGui::GetMousePos();

    if (!m_text.empty()) {
        auto draw_list = ImGui::GetWindowDrawList();
        for (auto c : m_text) {
            c->draw(draw_list);
        }
    }
    ImVec2 rel_pos = ImVec2(mouse_pos.x - vMin.x, mouse_pos.y - vMin.y);
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && mouse_pos.x > vMin.x) {
        wrapper.getCursorIndexFromPosition(rel_pos);
    }
    ImGui::End();
    ImGui::PopStyleColor();
    if (m_in_text != m_prev_text) {
        m_text.clear();
        bool capture_latex = false;
        m_prev_text = m_in_text;
        bool is_prev_dollar = false;
        std::string tmp_text, tmp_latex;
        for (auto s : m_in_text) {
            if (s == '$') {
                if (is_prev_dollar) {
                    update_text(capture_latex, tmp_text, tmp_latex);
                }
                is_prev_dollar = true;
            }
            if (s != '$') {
                is_prev_dollar = false;
                if (capture_latex)
                    tmp_latex.push_back(s);
                else
                    tmp_text.push_back(s);
            }
        }
        update_text(capture_latex, tmp_text, tmp_latex);
        std::vector<WrapCharPtr> str;
        for (auto c : m_text) {
            str.push_back(static_cast<WrapCharPtr>(c));
        }
        wrapper.setString(str);
    }
    if ((m_zoom != m_prev_zoom ||
        Tempo::GetScaling() != m_scaling ||
        m_current_width != width
        )
        && Tempo::GetImFont(m_font_regular32) != nullptr
        ) {
        m_current_width = width;
        m_prev_zoom = m_zoom;
        m_scaling = Tempo::GetScaling();
        auto t = std::chrono::high_resolution_clock::now();
        wrapper.setWidth(m_current_width);
        auto end = std::chrono::high_resolution_clock::now();
        // std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - t).count() << std::endl;
    }
    // ImGui::ShowDemoWindow();
}

void MainApp::updateFontSize(float size) {
    Tempo::RemoveFont(m_font_regular32);
    m_font_regular32 = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Regular.ttf", size).value();
}

void MainApp::update_text(bool& capture_latex, std::string& tmp_text, std::string& tmp_latex) {
    if (!capture_latex) {
        capture_latex = true;
        if (tmp_text.empty())
            return;
        auto res = Utf8StrToImCharStr(
            tmp_text,
            Tempo::GetImFont(m_font_regular32),
            m_font_size * m_zoom * Tempo::GetScaling(),
            microtex::BLACK
        );
        for (auto c : res) {
            m_text.push_back(c);
        }
        tmp_text.clear();
    }
    else {
        capture_latex = false;
        if (tmp_latex.empty())
            return;
        auto c = RichText::ToLatexChar(tmp_latex,
            m_font_size * m_zoom * Tempo::GetScaling(),
            7.f, microtex::BLACK,
            ImVec2(1.f, 1.f),
            ImVec2(m_zoom * 5.f, 0.f)
        );
        m_text.push_back(c);
        tmp_latex.clear();
    }
}
void MainApp::insertBigString() {
    m_in_text.clear();
    for (int i = 0;i < m_text_size;i++) {
        m_in_text += 32 + i % 90;
    }
    auto res = Utf8StrToImCharStr(
        m_in_text,
        Tempo::GetImFont(m_font_regular32),
        m_font_size * m_zoom * Tempo::GetScaling(),
        microtex::BLACK
    );
    wrapper.clear();
}
void MainApp::BeforeFrameUpdate() {
}