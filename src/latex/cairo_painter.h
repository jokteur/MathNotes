#include "cairo.h"

#include "graphic_abstract.h"
#include "core/image.h"

namespace microtex {
    class Cairo_Painter : public Painter {
    private:
        cairo_t* m_context = nullptr;
        cairo_surface_t* m_surface = nullptr;

        ImVec2 m_offset, m_scale, m_dimensions;

        color m_color;
        Stroke m_stroke;
        std::vector<float> m_dash;

        bool m_painting = false;

        ARGB_Imageptr m_image_data;

        float m_dx = 0.f;
        float m_dy = 0.f;

        float m_sx = 1.f;
        float m_sy = 1.f;

        inline ImVec2 getRealPos(float x, float y);

        void roundRect(float x, float y, float w, float h, float rx, float ry);
        void destroy();
    public:
        Cairo_Painter();
        ~Cairo_Painter();

        ARGB_Imageptr getImageDataPtr() { return m_image_data; }

        ImVec2 getImageDimensions() { return m_dimensions; }

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

        void fillPath(i32 id) override;

        /** Draw text */
        void drawText(const std::string& src, float x, float y);

        virtual void drawLine(float x, float y1, float x2, float y2) override;

        virtual void drawRect(float x, float y, float w, float h) override;

        virtual void fillRect(float x, float y, float w, float h) override;

        virtual void drawRoundRect(float x, float y, float w, float h, float rx, float ry) override;

        virtual void fillRoundRect(float x, float y, float w, float h, float rx, float ry) override;

        virtual void start(ImVec2 top_left, ImVec2 bottom_right, ImVec2 scale = ImVec2(1.f, 1.f), ImVec2 inner_padding = ImVec2(20.f, 20.f)) override;

        virtual void finish() override;
    };
}