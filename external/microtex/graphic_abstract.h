#pragma once

#include <set>
#include <unordered_map>
#include <string>
#include <memory>

#include <vector>

#define IMGUI_USE_WCHAR32
#include <tempo.h>
#include "microtex.h"
#include "graphic/graphic.h"

namespace microtex {
    /**
     * @brief Stores a function argument safely until retrieval
     *
     */
    class MICROTEX_EXPORT Argument {
    private:
        std::shared_ptr<void> m_data;
        bool is_empty = true;
    public:
        enum Type { FLOAT, FLOATV, STROKE, INT, STRING, COLOR, U16 };
        Argument() {}
        Argument(const std::string& data);
        Argument(float data);
        Argument(const std::vector<float>& data);
        Argument(const Stroke& s);
        Argument(int data);
        Argument(color data);
        Argument(u16 data);

        bool isempty() { return is_empty; }

        template<typename T>
        auto getData() {
            return *std::static_pointer_cast<T>(m_data);
        }
    };

    struct Call {
        std::string fct_name;
        std::vector<Argument> arguments;
    };

    class MICROTEX_EXPORT Font_abstract : public Font {
    private:
        static std::unordered_map<std::string, sptr<Font_abstract>> fonts_sptr;
        std::string m_path;
        std::string m_family;
        int m_style;
        float m_font_size;
    public:
        Font_abstract(const std::string& family, int style, float size);

        Font_abstract(const std::string& file, float size);

        void setSize(float size);

        std::string getFamily() const;

        int getStyle() const { return m_style; }

        float getSize() const { return m_font_size; }

        std::string getPath() const { return m_path; }

        virtual bool operator==(const Font& f) const override; // Not implemented

        static sptr<Font_abstract> getOrCreate(const std::string& file);

        virtual ~Font_abstract() {};
    };

    /**************************************************************************************************/


    class MICROTEX_EXPORT TextLayout_abstract : public TextLayout {
    private:
        std::string m_txt;
        sptr<Font_abstract> m_font;
        FontStyle m_style;
        float m_size;

    public:
        TextLayout_abstract(const std::string& src, FontStyle style, float size); // Not implemented

        virtual void getBounds(Rect& bounds) override; // Not implemented

        virtual void draw(Graphics2D& g2, float x, float y) override; // Not implemented
    };

    /**************************************************************************************************/

    class MICROTEX_EXPORT PlatformFactory_abstract : public PlatformFactory {
    public:
        sptr<Font> createFont(const std::string& file) override;

        sptr<TextLayout> createTextLayout(const std::string& src, FontStyle style, float size) override;
    };

    /**
     * Slightly modified abstract class (from MicroTeX) to represents a graphics (2D) context, all the TeX drawing operations will on it.
     */
    class Painter {
    public:
        virtual ~Painter() = default;

        /**
         * Set the color of the context
         *
         * @param c required color
         */
        virtual void setColor(color c) = 0;

        /** Set the stroke of the context */
        virtual void setStroke(const Stroke& s) = 0;

        /** Set the stroke width of the context */
        virtual void setStrokeWidth(float w) = 0;

        /**
         * Set the dash style to draw the lines.
         * Each line can be drawn with a different pen dash. It defines the style of the line.
         * The pattern is set by the dash array, which is an array of positive floating point values.
         * They set the on and off parts of the dash pattern. We also specify the length of the array
         * and the offset value. If the length is 0, the dashing is disabled. If it is 1, a symmetric
         * pattern is assumed with alternating on and off portions of the size specified by the single
         * value in dashes.
         *
         * @param dash the dash pattern
         */
        virtual void setDash(const std::vector<float>& dash) = 0;

        /** Set the font of the context */
        virtual void setFont(const std::string& path, float size, int style = -1, const std::string& family = "") = 0;

        /** Set font size */
        virtual void setFontSize(float size) = 0;

        /**
         * Translate the context by dx, dy
         *
         * @param dx distance in x-direction to translate
         * @param dy distance in y-direction to translate
         */
        virtual void translate(float dx, float dy) = 0;

        /**
         * Scale the context by sx, sy
         *
         * @param sx scale ratio in x-direction
         * @param sy scale ratio in y-direction
         */
        virtual void scale(float sx, float sy) = 0;

        /**
         * Rotate the context with the given angle (in radian) and pivot (0, 0).
         *
         * @param angle angle (in radian) amount to rotate
         */
        virtual void rotate(float angle) = 0;

        /**
         * Rotate the context with the given angle (in radian) and pivot (px, py).
         *
         * @param angle angle (in radian) amount to rotate
         * @param px pivot in x-direction
         * @param py pivot in y-direction
         */
        virtual void rotate(float angle, float px, float py) = 0;

        /** Reset transformations */
        virtual void reset() = 0;

        /**
         * Draw glyph, is baseline aligned
         *
         * @param glyph glyph index in the current font
         * @param x x-coordinate
         * @param y y-coordinate, is baseline aligned
         */
        virtual void drawGlyph(u16 glyph, float x, float y) = 0;

        // region path commands

        /**
         * Begin a new path. We promise every path begin with function `beginPath` and
         * end with `fillPath`. You may cache the path via its id.
         *
         * @param id the path id, if id < 0 that means the path is not cacheable.
         */
        virtual void beginPath(i32 id) = 0;

        /** Begin a path, move to point (x, y). */
        virtual void moveTo(float x, float y) = 0;

        /** Add a line to path. */
        virtual void lineTo(float x, float y) = 0;

        /**
         * Add a cubic Bezier spline to path, with control points (x1, y1), (x2, y2)
         * and the final point (x3, y3).
         */
        virtual void cubicTo(float x1, float y1, float x2, float y2, float x3, float y3) = 0;

        /**
         * Add a quadratic Bezier spline to path, with control point (x1, y1) and the
         * final point (x2, y2).
         */
        virtual void quadTo(float x1, float y1, float x2, float y2) = 0;

        /** Close the path. */
        virtual void closePath() = 0;

        /** Fill the path. */
        virtual void fillPath(i32 id) = 0;

        // endregion

        // region shape commands

        /**
         * Draw line
         *
         * @param x1 start point in x-direction
         * @param y1 start point in y-direction
         * @param x2 end point in x-direction
         * @param y2 end point in y-direction
         */
        virtual void drawLine(float x1, float y1, float x2, float y2) = 0;

        /**
         * Draw rectangle
         *
         * @param x left position
         * @param y top position
         * @param w width
         * @param h height
         */
        virtual void drawRect(float x, float y, float w, float h) = 0;

        /**
         * Fill rectangle
         *
         * @param x left position
         * @param y top position
         * @param w width
         * @param h height
         */
        virtual void fillRect(float x, float y, float w, float h) = 0;

        /**
         * Draw round rectangle
         *
         * @param x left position
         * @param y top position
         * @param w width
         * @param h height
         * @param rx radius in x-direction
         * @param ry radius in y-direction
         */
        virtual void drawRoundRect(float x, float y, float w, float h, float rx, float ry) = 0;

        /**
         * Fill round rectangle
         *
         * @param x left position
         * @param y top position
         * @param w width
         * @param h height
         * @param rx radius in x-direction
         * @param ry radius in y-direction
         */
        virtual void fillRoundRect(float x, float y, float w, float h, float rx, float ry) = 0;

        /**
         * @brief Start the painting process
         *
         * @param top_left minimum coordinate
         * @param bottom_right maximum coordinate
         * @param scale scale the whole image
         * @param inner_padding horizontal and vertical inner padding
         */
        virtual void start(ImVec2 dimensions, ImVec2 scale = ImVec2(1.f, 1.f), ImVec2 inner_padding = ImVec2(20.f, 20.f)) = 0;
        /**
         * @brief Tells the painter that it has finished painting
         *
         */
        virtual void finish() = 0;
        // endregion
    };

    struct FontInfo {
        float max_real_size = 10.f;
        std::vector<u16> glyphs;
        std::string text = "";
    };

    using FontInfos = std::unordered_map<std::string, FontInfo>;


    /**************************************************************************************************/

    /**
     * @brief Abstract Graphics2D class that only registrers the (microtex) calls without acting on it
     * It is possible to redistribute the calls to a Painter which does the drawing
     */
    class MICROTEX_EXPORT Graphics2D_abstract : public Graphics2D {
    private:
        static Font_abstract m_default_font;

        color m_color;
        Stroke m_stroke;
        std::vector<float> m_dash;
        Font_abstract* m_font;

        float m_sx = 1.f;
        float m_sy = 1.f;
        float m_dx = 0.f;
        float m_dy = 0.f;

        float m_min_x = 1e6;
        float m_max_x = -1e6;
        float m_min_y = 1e6;
        float m_max_y = -1e6;

        ImVec2 m_dimensions;

        FontInfos m_font_infos;

        std::vector<Call> m_calls;

        i32 m_path_id;

        inline void pushMinMax(float x, float y);

        void updateFontInfo(const std::string& text);
        void updateFontInfo(u32 c);

    public:
        Graphics2D_abstract();
        ~Graphics2D_abstract();

        ImVec2 getScaledMin();
        ImVec2 getScaledMax();

        /**
         * @brief Get the (functions) call list
         * Must be called after render->draw(graphics_abstract, ...) has been called
         * @return std::vector<Call> list of all draw calls
         */
        std::vector<Call> getCallList();

        /**
         * @brief Returns information on all the fonts used, with the max size and the glyphs used
         *
         * @return FontInfos key is the path of the font -> FontInfo
         */
        FontInfos getFontInfos() {
            return m_font_infos;
        }

        /**
         * @brief Distributes the (functions) call list to the painter
         *
         * @param painter must be a child of Painter
         */
        void distributeCallList(Painter* painter);

        /**
         * @brief Empties the call list
         *
         */
        void resetCallList();

        virtual void setColor(color c) override;

        virtual color getColor() const override;

        virtual void setStroke(const Stroke& s) override;

        virtual const Stroke& getStroke() const override;

        virtual void setStrokeWidth(float w) override;

        void setDash(const std::vector<float>& dash) override;

        std::vector<float> getDash() override;

        sptr<Font> getFont() const override;

        void setFont(const sptr<Font>& font) override;

        float getFontSize() const override;

        void setFontSize(float size) override;

        virtual void translate(float dx, float dy) override;

        virtual void scale(float sx, float sy) override;

        virtual void rotate(float angle) override;

        virtual void rotate(float angle, float px, float py) override;

        virtual void reset() override;

        virtual float sx() const override;

        virtual float sy() const override;

        void drawGlyph(u16 glyph, float x, float y) override;

        bool beginPath(i32 id) override;

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
    };

}  // namespace tex