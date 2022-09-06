#include <iostream>

#include <tempo.h>
#include <string>

#include <iostream>
#include <exception>

#include "graphic_abstract.h"
#include "latex/imgui_cv_painter.h"

#include <chrono>
using namespace std::chrono;

using namespace microtex;

class MainApp : public Tempo::App {
private:
    Tempo::FontID m_font_regular;
    Tempo::FontID m_font_italic;
    Tempo::FontID m_font_bold;

    Render* m_render = nullptr;
    Graphics2D_abstract m_graphics;
    ImGuiCV_Painter m_painter;
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

        if (ImGui::Button("Click me")) {
            m_graphics.resetCallList();
            auto start0 = high_resolution_clock::now();

            m_render = MicroTeX::parse(
                "\\int dV = \\mathbf{g}_{(i)}\\cdot (\\mathbf{g}_{(j)}\\times \\mathbf{g}_{(k)})d\\xi_id\\xi_jd\\xi_k \\textrm{Hello} \\textbf{world!}",
                // "\\begin{array}{l}"
                // "  \\forall\\varepsilon\\in\\mathbb{R}_+^*\\ \\exists\\eta>0\\ |x-x_0|\\leq\\eta\\Longrightarrow|f(x)-f(x_0)|\\leq\\varepsilon\\\\"
                // "  \\det"
                // "  \\begin{bmatrix}"
                // "      a_{11}&a_{12}&\\cdots&a_{1n}\\\\"
                // "      a_{21}&\\ddots&&\\vdots\\\\"
                // "      \\vdots&&\\ddots&\\vdots\\\\"
                // "      a_{n1}&\\cdots&\\cdots&a_{nn}"
                // "  \\end{bmatrix}"
                // "  \\overset{\\mathrm{def}}{=}\\sum_{\\sigma\\in\\mathfrak{S}_n}\\varepsilon(\\sigma)\\prod_{k=1}^n a_{k\\sigma(k)}\\\\"
                // "  \\sideset{_\\alpha^\\beta}{_\\gamma^\\delta}{\\begin{pmatrix}a&b\\\\c&d\\end{pmatrix}}\\\\"
                // "  \\int_0^\\infty{x^{2n} e^{-a x^2}\\,dx} = \\frac{2n-1}{2a} "
                // "      \\int_0^\\infty{x^{2(n-1)} e^{-a x^2}\\,dx} = \\frac{(2n-1)!!}{2^{n+1}} \\sqrt{\\frac{\\pi}{a^{2n+1}}}\\\\"
                // "  \\int_a^b{f(x)\\,dx} = (b - a) \\sum\\limits_{n = 1}^\\infty  "
                // "      {\\sum\\limits_{m = 1}^{2^n  - 1} {\\left( { - 1} \\right)^{m + 1} } } 2^{ - n} f(a + m\\left( {b - a} \\right)2^{-n} )\\\\"
                // "  \\int_{-\\pi}^{\\pi} \\sin(\\alpha x) \\sin^n(\\beta x) dx = \\textstyle{\\left \\{"
                // "      \\begin{array}{cc}"
                // "          (-1)^{(n+1)/2} (-1)^m \\frac{2 \\pi}{2^n} \\binom{n}{m} & n \\mbox{ odd},\\ \\alpha = \\beta (2m-n) \\\\"
                // "          0 & \\mbox{otherwise} \\\\ \\end{array} \\right .}\\\\"
                // "  L = \\int_a^b \\sqrt{ \\left|\\sum_{i,j=1}^ng_{ij}(\\gamma(t))\\left(\\frac{d}{dt}x^i\\circ\\gamma(t)\\right)"
                // "      \\left(\\frac{d}{dt}x^j\\circ\\gamma(t)\\right)\\right|}\\,dt\\\\"
                // "  \\begin{array}{rl}"
                // "      s &= \\int_a^b\\left\\|\\frac{d}{dt}\\vec{r}\\,(u(t),v(t))\\right\\|\\,dt \\\\"
                // "      &= \\int_a^b \\sqrt{u'(t)^2\\,\\vec{r}_u\\cdot\\vec{r}_u + 2u'(t)v'(t)\\, \\vec{r}_u\\cdot"
                // "          \\vec{r}_v+ v'(t)^2\\,\\vec{r}_v\\cdot\\vec{r}_v}\\,\\,\\, dt. "
                // "  \\end{array}\\\\"
                // "\\end{array}",
                720.f, 40.f, 7.f, BLACK, false, "XITS Math", "XITS"
            );
            auto stop0 = high_resolution_clock::now();
            auto duration0 = duration_cast<microseconds>(stop0 - start0);
            std::cout << "Parse: " << duration0.count() << std::endl;

            auto start1 = high_resolution_clock::now();
            m_render->draw(m_graphics, 0.f, 0.f);
            auto stop1 = high_resolution_clock::now();
            auto duration1 = duration_cast<microseconds>(stop1 - start1);
            std::cout << "Draw: " << duration1.count() << std::endl;

            m_painter.setPhantomDrawList(ImGui::GetWindowDrawList());
            m_painter.start(m_graphics.getScaledMin(), m_graphics.getScaledMax());
            m_graphics.distributeCallList(&m_painter);
            m_painter.finish();
        }
        ImGui::End();

        ImGui::Begin("Draw Window");
        ImGui::Text("my latex:\n");

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 0, 0, 0.1));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1, 1, 1, 0.8));
        ImGui::BeginChild("testt");

        if (m_render != nullptr) {
            m_painter.draw(ImGui::GetWindowDrawList());
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