#include <codecvt>
#include <locale>
#include <vector>

#include <opencv2/core/mat.hpp>

#include "core/image.h"
#include "graphic_abstract.h"


namespace microtex {

    class LatexFontManager {
    private:

    public:
        LatexFontManager() {}
        ~LatexFontManager() {}

    };

    class ImGuiCV_Painter : public Painter {
    private:
        color m_color;
        Stroke m_stroke;
        std::vector<float> m_dash;

        ImVec2 m_offset, m_scale, m_dimensions;

        float m_dx = 0.f;
        float m_dy = 0.f;

        float m_sx = 1.f;
        float m_sy = 1.f;

        bool m_fill_path = false;
        bool m_prev_path = false;

        float m_oversampling = 2.f;

        ImDrawList* m_draw_list;
        Image m_image;
        cv::Mat m_canvas;

        inline ImVec2 getRealPos(float x, float y);
        void finishPath();
    public:
        ImGuiCV_Painter() {}
        ~ImGuiCV_Painter() {}

        /**
         * @brief The painter uses draw list for the convenient bezier implementations
         * But in the end, only a rasterized image is drawn to ImGui
         *
         * @param draw_list
         */
        void setPhantomDrawList(ImDrawList* draw_list) { m_draw_list = draw_list; }

        /**
         * @brief Draws the resulting image to draw_list
         *
         */
        void draw(ImDrawList* draw_list);

        virtual void setColor(color c) override;

        virtual void setStroke(const Stroke& s) override;

        virtual void setStrokeWidth(float w) override;

        void setDash(const std::vector<float>& dash) override;

        void setFont(const std::string& path, float size, int style = -1, const std::string& family = "") override;

        void setFontSize(float size) override;

        virtual void translate(float dx, float dy) override;

        virtual void scale(float sx, float sy) override;

        virtual void rotate(float angle) override;

        virtual void rotate(float angle, float px, float py) override;

        virtual void reset() override;

        void drawGlyph(u16 glyph, float x, float y) override;

        void beginPath(i32 id) override;

        void moveTo(float x, float y) override;

        void lineTo(float x, float y) override;

        void cubicTo(float x1, float y1, float x2, float y2, float x3, float y3) override;

        void quadTo(float x1, float y1, float x2, float y2) override;

        void closePath() override;

        void fillPath() override;

        /** Draw text */
        void drawText(const std::string& src, float x, float y);

        virtual void drawLine(float x, float y1, float x2, float y2) override;

        virtual void drawRect(float x, float y, float w, float h) override;

        virtual void fillRect(float x, float y, float w, float h) override;

        virtual void drawRoundRect(float x, float y, float w, float h, float rx, float ry) override;

        virtual void fillRoundRect(float x, float y, float w, float h, float rx, float ry) override;

        virtual void start(ImVec2 top_left, ImVec2 bottom_right, ImVec2 scale = ImVec2(1.f, 1.f), float oversampling = 8.f) override;

        virtual void finish() override;
    };
}