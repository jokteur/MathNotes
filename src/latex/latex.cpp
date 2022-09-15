#include "latex.h"

namespace Latex {
    bool is_initialized = false;

    std::string init() {
        using namespace microtex;

        microtex::MicroTeX::setRenderGlyphUsePath(true);
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


            PlatformFactory::registerFactory("abstract", std::make_unique<PlatformFactory_abstract>());
            PlatformFactory::activate("abstract");
            is_initialized = true;
            return "";
        }
        catch (std::exception& e) {
            return e.what();
        }
    }

    bool isInitialized() {
        return is_initialized;
    }

    void LatexImage::render(ImVec2 scale, ImVec2 inner_padding) {
        m_painter.start(ImVec2(round(m_render->getWidth()), round(m_render->getHeight())), scale, inner_padding);
        m_graphics.distributeCallList(&m_painter);
        m_painter.finish();
        if (!m_painter.getImageDataPtr()->empty())
            m_image->setImage(m_painter.getImageDataPtr(), m_painter.getImageDimensions().x, m_painter.getImageDimensions().y, Image::FILTER_BILINEAR);
    }

    LatexImage::LatexImage(const std::string& latex_src, float font_size, float line_space, microtex::color text_color, ImVec2 scale, ImVec2 inner_padding) {
        if (!is_initialized) {
            m_latex_error_msg = "LateX has not been initialized";
            return;
        }
        m_image = std::make_shared<Image>();
        using namespace microtex;
        try {
            // Default width large enough
            m_render = MicroTeX::parse(
                latex_src,
                2000.f, font_size, line_space, text_color
            );
            float height = m_render->getHeight(); // total height of the box = ascent + descent
            m_descent = m_render->getDepth();   // depth = descent
            m_ascent = height - m_descent;

            m_render->draw(m_graphics, 0.f, 0.f);

        }
        catch (std::exception& e) {
            m_latex_error_msg = e.what();
        }
        if (m_latex_error_msg.empty())
            render(scale, inner_padding);
    }

    std::shared_ptr<Image> LatexImage::getImage() {
        return m_image;
    }

    ImVec2 LatexImage::getDimensions() {
        if (m_latex_error_msg.empty())
            return m_painter.getImageDimensions();
        else
            return ImVec2(0, 0);
    }

    void LatexImage::forgetImage() {
        m_image->reset();
    }

    void LatexImage::redraw(ImVec2 scale, ImVec2 inner_padding) {
        if (m_latex_error_msg.empty())
            render(scale, inner_padding);
    }
}