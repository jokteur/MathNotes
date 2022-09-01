#include <codecvt>
#include <locale>
#include <vector>
#include "graphic_abstract.h"

namespace microtex {
    class LatexFontManager {
    private:

    public:
        LatexFontManager() {}
        ~LatexFontManager() {}

    };

    class ImGui_Painter : public Painter {
    private:
        color m_color;
        Stroke m_stroke;
        std::vector<float> m_dash;
        bool m_is_prev_font = false;
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> m_converter;

        std::unordered_map<std::string, Tempo::FontID> m_fonts;

        float m_sx = 1.f;
        float m_sy = 1.f;

    public:
        ImGui_Painter() {}
        ~ImGui_Painter() {}

        void setOrigin(float x, float y);

        void setFontInfos(const FontInfos& font_infos);

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

        void drawGlyph(u32 glyph, float x, float y) override;

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

        virtual void finish() override;
    };
}