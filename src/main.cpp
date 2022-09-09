#include <iostream>

#include <tempo.h>
#include <string>

#include <iostream>
#include <exception>

#include "misc/cpp/imgui_stdlib.h"
#include "graphic_abstract.h"
#include "latex/cairo_painter.h"

#include <chrono>
using namespace std::chrono;

using namespace microtex;

class MainApp : public Tempo::App {
private:
    Tempo::FontID m_font_regular;
    Tempo::FontID m_font_italic;
    Tempo::FontID m_font_bold;

    std::string m_text;
    std::string m_err;
    std::string m_prev_text;

    float m_scale = 1.f;

    Render* m_render = nullptr;
    Graphics2D_abstract m_graphics;
    Image m_image;
    Cairo_Painter m_painter;
    //bool m_open = true;
public:
    virtual ~MainApp() {}

    void InitializationBeforeLoop() override {
        m_font_regular = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Regular.ttf", 16).value();
        // m_font_italic = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Italic.ttf", 16).value();
        // m_font_bold = Tempo::AddFontFromFileTTF("data/fonts/Roboto/Roboto-Bold.ttf", 16).value();

        try {
            // const FontSrcFile math_regular("data/lm-math/latinmodern-math.clm2", "data/lm-math/latinmodern-math.otf");
            // MicroTeX::init(math_regular);
            const FontSrcFile math_regular("data/xits/XITSMath-Regular.clm2", "data/xits/XITSMath-Regular.otf");
            // const FontSrcFile math_bold("data/xits/XITSMath-Bold.clm2", "data/xits/XITSMath-Bold.otf");
            const FontSrcFile xits_boldItalic("data/xits/XITS-BoldItalic.clm2", "data/xits/XITS-BoldItalic.otf");
            const FontSrcFile xits_regular("data/xits/XITS-Regular.clm2", "data/xits/XITS-Regular.otf");
            const FontSrcFile xits_bold("data/xits/XITS-Bold.clm2", "data/xits/XITS-Bold.otf");
            const FontSrcFile xits_talic("data/xits/XITS-Italic.clm2", "data/xits/XITS-Italic.otf");
            MicroTeX::init(math_regular);
            // MicroTeX::addFont(math_bold);
            MicroTeX::addFont(xits_boldItalic);
            MicroTeX::addFont(xits_regular);
            MicroTeX::addFont(xits_bold);
            MicroTeX::addFont(xits_talic);
            MicroTeX::setDefaultMainFont("XITS");

            auto a = MicroTeX::mainFontFamilies();
            auto b = MicroTeX::mathFontNames();

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

        ImGui::InputTextMultiline("Latex input", &m_text);

        if (m_text != m_prev_text) {
            m_prev_text = m_text;

            if (!m_text.empty()) {
                m_graphics.resetCallList();
                auto start0 = high_resolution_clock::now();

                try {
                    m_render = MicroTeX::parse(
                        m_text,
                        500.f, 40.f, 7.f, BLACK, false, "XITS Math", "XITS"
                    );
                    auto stop0 = high_resolution_clock::now();
                    auto duration0 = duration_cast<microseconds>(stop0 - start0);
                    std::cout << "Parse: " << duration0.count() << std::endl;

                    auto start1 = high_resolution_clock::now();
                    m_render->draw(m_graphics, 0.f, 0.f);

                    m_painter.start(m_graphics.getScaledMin(), m_graphics.getScaledMax());
                    m_graphics.distributeCallList(&m_painter);
                    m_painter.finish();

                    auto stop1 = high_resolution_clock::now();
                    auto duration1 = duration_cast<microseconds>(stop1 - start1);
                    std::cout << "Draw: " << duration1.count() << std::endl;
                    m_err = "";
                    m_image.setImage(m_painter.getImageDataPtr(), m_painter.getImageDimensions().x, m_painter.getImageDimensions().y, Image::FILTER_BILINEAR);
                }
                catch (std::exception& e) {
                    m_err = e.what();
                }
            }
        }
        if (!m_err.empty()) {
            ImGui::Text(m_err.c_str());
        }
        ImGui::Text("my latex:\n");

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 0, 0, 0.1));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1, 1, 1, 0.8));
        ImGui::BeginChild("testt");

        if (m_render != nullptr) {

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
                m_painter.start(m_graphics.getScaledMin(), m_graphics.getScaledMax(), ImVec2(m_scale, m_scale));
                m_graphics.distributeCallList(&m_painter);
                m_painter.finish();
                m_image.setImage(m_painter.getImageDataPtr(), m_painter.getImageDimensions().x, m_painter.getImageDimensions().y, Image::FILTER_BILINEAR);
            }
            ImGui::Image(m_image.texture(), ImVec2(m_image.width(), m_image.height()));
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
    MicroTeX::setRenderGlyphUsePath(true);
    Tempo::Config config;
    config.app_name = "TestApp";
    config.app_title = "Hello world";
    config.default_window_height = 1000;
    config.default_window_width = 1650;

    MainApp* app = new MainApp();
    Tempo::Run(app, config);

    return 0;
}