#include <iostream>

#include <tempo.h>
#include <string>

#include <iostream>
#include <exception>

#include "misc/cpp/imgui_stdlib.h"
#include "latex/latex.h"

#include <chrono>
using namespace std::chrono;

class MainApp : public Tempo::App {
private:
    Tempo::FontID m_font_regular;
    Tempo::FontID m_font_italic;
    Tempo::FontID m_font_bold;

    std::string m_text;
    std::string m_err;
    std::string m_prev_text;

    float m_scale = 1.f;

    std::shared_ptr <Latex::LatexImage> m_latex;
    std::string m_latex_err;
    //bool m_open = true;
public:
    virtual ~MainApp() {}

    void InitializationBeforeLoop() override {
        m_font_regular = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Regular.ttf", 16).value();
        // m_font_italic = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Italic.ttf", 16).value();
        // m_font_bold = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Bold.ttf", 16).value();

        Latex::init();
    }

    void FrameUpdate() override {
        ImGui::Begin("My window");

        ImGui::InputTextMultiline("Latex input", &m_text);

        if (m_text != m_prev_text) {
            m_prev_text = m_text;

            if (!m_text.empty()) {
                m_latex = std::make_shared<Latex::LatexImage>(m_text, 20.f, 7.f, microtex::BLACK, ImVec2(m_scale, m_scale));
            }
        }

        ImGui::Text("my latex:\n");


        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 0, 0, 0.1));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1, 1, 1, 0.8));
        ImGui::BeginChild("testt");
        if (!m_text.empty()) {
            if (m_latex->getLatexErrorMsg().empty()) {
                auto& io = ImGui::GetIO();
                float mouse_wheel = io.MouseWheel;
                if (mouse_wheel != 0.f) {
                    if (mouse_wheel < 0.f) {
                        m_scale *= 0.99;
                    }
                    else {
                        m_scale *= 1.01;
                    }
                    if (m_scale >= 5.f)
                        m_scale = 5.f;
                    if (m_scale <= 0.5)
                        m_scale = 0.5;

                    m_latex->redraw(ImVec2(m_scale, m_scale));
                }
                ImGui::Image(
                    m_latex->getImage()->texture(),
                    m_latex->getDimensions()
                );
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Text, microtex::BLACK);
                ImGui::Text(m_latex->getLatexErrorMsg().c_str());
                ImGui::PopStyleColor();
            }
        }



        // ImGui::Text("Test");
        ImGui::EndChild();
        ImGui::PopStyleColor(2);
        ImGui::End();
        ImGui::ShowDemoWindow();

    }
    void BeforeFrameUpdate() override {}
};

int main() {
    std::string err = Latex::init();
    Tempo::Config config;
    config.app_name = "TestApp";
    config.app_title = "Hello world";
    config.default_window_height = 1000;
    config.default_window_width = 1650;

    MainApp* app = new MainApp();
    Tempo::Run(app, config);

    return 0;
}