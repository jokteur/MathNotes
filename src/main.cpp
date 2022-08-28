#include <iostream>
#include <tempo.h>
#include <string>

#include <iostream>
#include <exception>

#include "graphic_abstract.h"
#include "latex/imgui_painter.h"

using namespace microtex;

class MainApp : public Tempo::App {
private:
    Tempo::FontID m_font_regular;
    Tempo::FontID m_font_italic;
    Tempo::FontID m_font_bold;

    Render* m_render = nullptr;
    Graphics2D_abstract m_graphics;
    ImGui_Painter m_painter;
    //bool m_open = true;
public:
    virtual ~MainApp() {}

    void InitializationBeforeLoop() override {
        m_font_regular = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Regular.ttf", 16).value();
        m_font_italic = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Italic.ttf", 16).value();
        m_font_bold = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Bold.ttf", 16).value();

        try {
            const FontSrcFile math("data/xits/XITSMath-Regular.clm2", "data/xits/XITSMath-Regular.otf");
            MicroTeX::init(math);
            PlatformFactory::registerFactory("abstract", std::make_unique<PlatformFactory_abstract>());
            PlatformFactory::activate("abstract");

        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            // TODO: quit
        }
    }

    void FrameUpdate() override {
        ImGui::Begin("My window");

        if (ImGui::Button("Click me")) {
            m_render = MicroTeX::parse(
                "\\int_{ now }^ {+\\infty} f(x)^2 dx",
                200, 1.f, 1.f / 3.f, 0xff424242
            );
            m_render->draw(m_graphics, 0.f, 0.f);
            auto calls = m_graphics.getCallList();
            m_graphics.distributeCallList(&m_painter);
            // std::wstring code = L"\\int_{now}^{+\\infty} \\text{Keep trying}";
            // // Convert the code to a paintable object (TeXRender)
            // auto r = LaTeX::parse(
            //     code,   // LaTeX code to parse
            //     720,    // logical width of the graphics context (in pixel)
            //     20,     // font size (in point)
            //     10,     // space between 2 lines (in pixel)
            //     BLACK   // foreground color
            // );
            // TeXFormula::setDPITarget(74);
        }
        ImGui::Text("Welcome to the multi-font application");
        Tempo::PushFont(m_font_bold);
        ImGui::Text("This is bold");
        Tempo::PopFont();
        ImGui::End();

        ImGui::Begin("Draw Window");

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.f, 1.f, 1.f, 1.f));
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImVec2 p0 = ImGui::GetCursorScreenPos();

        ImGui::PopStyleColor();
        ImGui::End();
        ImGui::ShowDemoWindow();

    }
    void BeforeFrameUpdate() override {}
};

int main() {
    Tempo::Config config;
    config.app_name = "TestApp";
    config.app_title = "Hello world";

    MainApp* app = new MainApp();
    Tempo::Run(app, config);

    return 0;
}