#include <iostream>

#include <tempo.h>
#include <string>

#include <chrono>
#include <iostream>
#include <exception>

#include "misc/cpp/imgui_stdlib.h"
#include "rich_text/wrapper.h"
#include "rich_text/chars/im_char.h"
#include "rich_text/chars/latex_char.h"
#include "latex/latex.h"

#include <chrono>
using namespace std::chrono;
using namespace RichText;


int TextInputCallback(ImGuiInputTextCallbackData* data);

class MainApp : public Tempo::App {
private:
    Tempo::FontID m_font_regular;
    Tempo::FontID m_font_regular32;
    Tempo::FontID m_font_italic;
    Tempo::FontID m_font_bold;


    float text_size = 300.f;
    float prev_size = 0.f;

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
public:
    int m_insert_at = 0;
    virtual ~MainApp() {}
    MainApp() : wrapper(0.f, 500.f) {}

    void InitializationBeforeLoop() override {
        m_font_regular = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Regular.ttf", 16).value();
        m_font_regular32 = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Regular.ttf", 32).value();
        // m_font_italic = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Italic.ttf", 16).value();
        // m_font_bold = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Bold.ttf", 16).value();

        Latex::init();
    }

    void updateFontSize(float size) {
        Tempo::RemoveFont(m_font_regular32);
        m_font_regular32 = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Regular.ttf", size).value();
    }

    void update_text(bool& capture_latex, std::string& tmp_text, std::string& tmp_latex) {
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
    void insertBigString() {
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
    void FrameUpdate() override {
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

        if (!m_text.empty()) {
            auto draw_list = ImGui::GetWindowDrawList();
            for (auto c : m_text) {
                c->draw(draw_list);
            }
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
    void BeforeFrameUpdate() override {}
};

int TextInputCallback(ImGuiInputTextCallbackData* data) {
    auto main_app = static_cast<MainApp*>(data->UserData);
    main_app->m_insert_at = data->CursorPos;
    return 1;
}

int main() {
    std::string err = Latex::init();
    Tempo::Config config;
    config.app_name = "TestApp";
    config.app_title = "Hello world";
    config.default_window_width = 1200;
    config.default_window_height = 700;

    MainApp* app = new MainApp();
    Tempo::Run(app, config);

    return 0;
}