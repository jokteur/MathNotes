#pragma once

#include <string>
#include <vector>
#include <tempo.h>

#include "core/image.h"
#include "cairo_painter.h"

namespace Latex {
    /**
     * @brief Initializes the latex fonts and parser
     *
     * @return std::string error message if failed
     */
    std::string init();

    /**
     * @brief returns true if latex has been initialized
     * otherwise false
     */
    bool isInitialized();

    /**
     * @brief A LatexImage generates an image from a latex source
     *
     * Can only be rescaled after creation
     *
     */
    class LatexImage {
    private:
        microtex::Render* m_render = nullptr;
        microtex::Graphics2D_abstract m_graphics;
        std::shared_ptr<Image> m_image;
        microtex::Cairo_Painter m_painter;
        float m_ascent;
        float m_descent;
        float m_baseline;

        std::string m_latex_error_msg;

        void render(ImVec2 scale, ImVec2 inner_padding);
    public:
        /**brief Create a Latex Image
         *
         latex source
         * @param font_size indicative font size for latex
         * @param text_color defaut text color
         * @param scale rescale the image (in x and y)
         * @param inner_padding horizontal and vertical inner padding (will be scaled)
         */
        LatexImage(const std::string& latex_src, float font_size = 20.f, float line_space = 7.f, microtex::color text_color = microtex::BLACK, ImVec2 scale = ImVec2(1.f, 1.f), ImVec2 inner_padding = ImVec2(20.f, 20.f));

        ~LatexImage() {}

        /**
         * @brief Returns the generated image
         *
         * Could be empty if forgetImage has been called or
         * if an latex error has occured
         *
         * @return const Image&
         */
        std::shared_ptr<Image> getImage();

        /**
         * @brief Returns the dimensions of the latex image
         *
         * Returns the correct dimensions even if the image has been forgotten with forgetImage()
         * If a latex error occured in the creation of the object, returns (0,0)
         *
         * @return ImVec2
         */
        ImVec2 getDimensions();

        /**
         * @brief Returns latex error message (if any, otherwise empty string)
         */
        std::string getLatexErrorMsg() { return m_latex_error_msg; }

        float getAscent() { return m_ascent; }
        float getDescent() { return m_descent; }

        /**
         * @brief Removes from memory the generated image
         * getImage will return an empty image
         *
         */
        void forgetImage();

        /**
         * @brief Redraws the parsed latex into an image
         * getImage will return a valid image (if no latex error occured)
         *
         * @param scale rescale the image (in x and y)
         * @param inner_padding horizontal and vertical inner padding (will be scaled)
         */
        void redraw(ImVec2 scale = ImVec2(1.f, 1.f), ImVec2 inner_padding = ImVec2(20.f, 20.f));
    };

    using LatexImagePtr = std::shared_ptr<LatexImage>;
}