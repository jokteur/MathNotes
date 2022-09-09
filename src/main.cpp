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

    Render* m_render = nullptr;
    Graphics2D_abstract m_graphics;
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

        if (ImGui::Button("Click me")) {
            m_graphics.resetCallList();
            auto start0 = high_resolution_clock::now();

            m_render = MicroTeX::parse(
                "\\definecolor{gris}{gray}{0.9}"
                "\\definecolor{noir}{rgb}{0,0,0}"
                "\\definecolor{bleu}{rgb}{0,0,1}"
                "\\fatalIfCmdConflict{false}"
                "\\newcommand{\\pa}{\\left|}"
                "\\begin{array}{c}"
                "  \\LaTeX\\\\"
                "  \\begin{split}"
                "      &Тепловой\\ поток\\ \\mathrm{Тепловой\\ поток}\\ \\mathtt{Тепловой\\ поток}\\\\"
                "      &\\boldsymbol{\\mathrm{Тепловой\\ поток}}\\ \\mathsf{Тепловой\\ поток}\\\\"
                "      |I_2| &= \\pa\\int_0^T\\psi(t)\\left\\{ u(a,t)-\\int_{\\gamma(t)}^a \\frac{d\\theta}{k} (\\theta,t) \\int_a^\\theta c(\\xi) "
                "          u_t (\\xi,t)\\,d\\xi\\right\\}dt\\right|\\\\"
                "      &\\le C_6 \\Bigg|\\pa f \\int_\\Omega \\pa\\widetilde{S}^{-1,0}_{a,-}"
                "          W_2(\\Omega, \\Gamma_1)\\right|\\ \\right|\\left| |u|\\overset{\\circ}{\\to} W_2^{\\widetilde{A}}(\\Omega\\Gamma_r,T)\\right|\\Bigg|\\\\"
                "      &\\\\"
                "      &\\begin{pmatrix}"
                "          \\alpha&\\beta&\\gamma&\\delta\\\\"
                "          \\aleph&\\beth&\\gimel&\\daleth\\\\"
                "          \\mathfrak{A}&\\mathfrak{B}&\\mathfrak{C}&\\mathfrak{D}\\\\"
                "          \\boldsymbol{\\mathfrak{a}}&\\boldsymbol{\\mathfrak{b}}&\\boldsymbol{\\mathfrak{c}}&\\boldsymbol{\\mathfrak{d}}"
                "      \\end{pmatrix}"
                "      \\quad{(a+b)}^{\\frac{n}{2}}=\\sqrt{\\sum_{k=0}^n\\tbinom{n}{k}a^kb^{n-k}}\\quad "
                "          \\Biggl(\\biggl(\\Bigl(\\bigl(()\\bigr)\\Bigr)\\biggr)\\Biggr)\\\\"
                "      &\\forall\\varepsilon\\in\\mathbb{R}_+^*\\ \\exists\\eta>0\\ |x-x_0|\\leq\\eta\\Longrightarrow|f(x)-f(x_0)|\\leq\\varepsilon\\\\"
                "      &\\det"
                "      \\begin{bmatrix}"
                "          a_{11}&a_{12}&\\cdots&a_{1n}\\\\"
                "          a_{21}&\\ddots&&\\vdots\\\\"
                "          \\vdots&&\\ddots&\\vdots\\\\"
                "          a_{n1}&\\cdots&\\cdots&a_{nn}"
                "      \\end{bmatrix}"
                "      \\overset{\\mathrm{def}}{=}\\sum_{\\sigma\\in\\mathfrak{S}_n}\\varepsilon(\\sigma)\\prod_{k=1}^n a_{k\\sigma(k)}\\\\"
                "      &\\Delta f(x,y)=\\frac{\\partial^2f}{\\partial x^2}+\\frac{\\partial^2f}{\\partial y^2}\\qquad\\qquad \\fcolorbox{noir}{gris}"
                "          {n!\\underset{n\\rightarrow+\\infty}{\\sim} {\\left(\\frac{n}{e}\\right)}^n\\sqrt{2\\pi n}}\\\\"
                "      &\\sideset{_\\alpha^\\beta}{_\\gamma^\\delta}{"
                "      \\begin{pmatrix}"
                "          a&b\\\\"
                "          c&d"
                "      \\end{pmatrix}}"
                "      \\xrightarrow[T]{n\\pm i-j}\\sideset{^t}{}A\\xleftarrow{\\overrightarrow{u}\\wedge\\overrightarrow{v}}"
                "          \\underleftrightarrow{\\iint_{\\mathds{R}^2}e^{-\\left(x^2+y^2\\right)}\\,\\mathrm{d}x\\mathrm{d}y}"
                "  \\end{split}\\\\"
                "  \\rotatebox{30}{\\sum_{n=1}^{+\\infty}}\\quad\\mbox{Mirror rorriM}\\reflectbox{\\mbox{Mirror rorriM}}"
                "\\end{array}",
                500.f, 20.f, 7.f, BLACK, false, "XITS Math", "XITS"
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