#include <iostream>

#include <tempo.h>
#include <string>

#include <iostream>
#include <exception>

#include "misc/cpp/imgui_stdlib.h"
#include "rich_text/wrapper.h"
#include "latex/latex.h"

#include <chrono>
using namespace std::chrono;
using namespace RichText;
struct CharText : public Character {
public:
    char c;
    void draw(ImDrawList*) override {
        if (c != '\n')
            std::cout << c;
    }
};

void draw(const std::vector<CharPtr> string, std::set<int> line_positions) {

}

CharText toChar(char t) {
    CharText c;
    c.c = t;
    c.advance = 1.0;
    c.bearing = ImVec2(0.2f, 0.2f);
    c.dimensions = ImVec2(0.8f, 0.8f);
    c.is_linebreak = false;
    c.breakable = false;
    if (t == '\n') {
        c.is_linebreak = true;
    }
    if (t == ' ') {
        c.breakable = true;
    }
    return c;
}

class MainApp : public Tempo::App {
private:
    Tempo::FontID m_font_regular;
    Tempo::FontID m_font_italic;
    Tempo::FontID m_font_bold;

    float text_size = 10.f;
    float prev_size = 0.f;

    std::vector<CharPtr> m_text;
    TextWrapper wrapper;
    //bool m_open = true;
public:
    virtual ~MainApp() {}

    void InitializationBeforeLoop() override {
        m_font_regular = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Regular.ttf", 16).value();
        // m_font_italic = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Italic.ttf", 16).value();
        // m_font_bold = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Bold.ttf", 16).value();

        Latex::init();

        std::string text = "a     abcdefghijklmnopqrstuvwxyz0123456789.";

        for (auto s : text) {
            m_text.push_back(std::make_shared<CharText>(toChar(s)));
        }
        wrapper.insertAt(m_text, 0);
    }

    void FrameUpdate() override {
        ImGui::Begin("My window");

        ImGui::DragFloat("text_size", &text_size, 0.5f, 0.f, 20.f);
        if (ImGui::Button("draw")) {
            wrapper.setWidth(text_size);
            auto& lines = wrapper.getLines();
            int line_idx = 0;
            int i = 0;

            std::cout << "Start text" << std::endl;
            for (auto c : m_text) {
                if (line_idx < lines.size() && lines[line_idx].start == i) {
                    line_idx++;
                    if (line_idx > 0)
                        std::cout << std::endl;
                }

                c->draw(ImGui::GetWindowDrawList());
                i++;
            }
            std::cout << "End text" << std::endl;
        }
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
    config.default_window_width = 1200;
    config.default_window_height = 700;

    MainApp* app = new MainApp();
    Tempo::Run(app, config);

    return 0;
}