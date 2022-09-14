#include "cairo_painter.h"

using namespace microtex;


inline float max(float a, float b) {
    if (a > b)
        return a;
    return b;
}
inline float min(float a, float b) {
    if (a < b)
        return a;
    return b;
}

void Cairo_Painter::destroy() {
    if (m_surface != nullptr) {
        cairo_surface_destroy(m_surface);
        cairo_destroy(m_context);
    }
    m_image_data = std::make_shared<ARGB_Image>();
}

void Cairo_Painter::start(ImVec2 dimensions, ImVec2 scale, ImVec2 inner_padding) {
    destroy();
    m_painting = true;

    m_dimensions = ImVec2(
        int(scale.x * (dimensions.x + 2 * inner_padding.x)),
        int(scale.y * (dimensions.y + 2 * inner_padding.y))
    );
    m_scale = scale;
    m_offset = inner_padding;
    m_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, m_dimensions.x, m_dimensions.y);
    m_context = cairo_create(m_surface);

    setColor(BLACK);
    setStroke(Stroke());
}
void Cairo_Painter::finish() {
    if (m_dimensions.x > 0 && m_dimensions.y > 0 && m_painting) {
        unsigned char* data = cairo_image_surface_get_data(m_surface);
        // data is a borrowed pointer, its creation / destruction is managed by cairo
        // this is why we copy
        m_image_data = std::make_shared<ARGB_Image>();
        m_image_data->resize(m_dimensions.x * m_dimensions.y * 4);
        memcpy(&(*m_image_data)[0], data, sizeof(unsigned char) * m_dimensions.x * m_dimensions.y * 4);
        m_painting = false;
    }
}

ImVec2 Cairo_Painter::getRealPos(float x, float y) {
    return ImVec2(m_scale.x * (x + m_sx * m_offset.x), m_scale.y * (y + m_sy * m_offset.y));
}

Cairo_Painter::Cairo_Painter() {
}

Cairo_Painter::~Cairo_Painter() {
    destroy();
}

void Cairo_Painter::setColor(color c) {
    m_color = c;
    const double a = color_a(c) / 255.;
    const double r = color_r(c) / 255.;
    const double g = color_g(c) / 255.;
    const double b = color_b(c) / 255.;
    cairo_set_source_rgba(m_context, r, g, b, a);
}

void Cairo_Painter::setStroke(const Stroke& s) {
    m_stroke = s;
    cairo_set_line_width(m_context, (double)s.lineWidth * m_scale.x);

    // convert abstract line cap to platform line cap
    cairo_line_cap_t c;
    switch (s.cap) {
    case CAP_BUTT:
        c = CAIRO_LINE_CAP_BUTT;
        break;
    case CAP_ROUND:
        c = CAIRO_LINE_CAP_ROUND;
        break;
    case CAP_SQUARE:
        c = CAIRO_LINE_CAP_SQUARE;
        break;
    }
    cairo_set_line_cap(m_context, c);

    // convert abstract line join to platform line join
    cairo_line_join_t j;
    switch (s.join) {
    case JOIN_BEVEL:
        j = CAIRO_LINE_JOIN_BEVEL;
        break;
    case JOIN_ROUND:
        j = CAIRO_LINE_JOIN_ROUND;
        break;
    case JOIN_MITER:
        j = CAIRO_LINE_JOIN_MITER;
        break;
    }
    cairo_set_line_join(m_context, j);

    cairo_set_miter_limit(m_context, (double)s.miterLimit);
}

void Cairo_Painter::setStrokeWidth(float w) {
    m_stroke.lineWidth = w;
    cairo_set_line_width(m_context, (double)w * m_scale.x);
}

void Cairo_Painter::setDash(const std::vector<float>& dash) {
    if (dash.empty()) {
        cairo_set_dash(m_context, nullptr, 0, 0.);
    }
    else {
        const std::vector<double> ddash(dash.begin(), dash.end());
        cairo_set_dash(m_context, ddash.data(), ddash.size(), 0.);
    }
}

void Cairo_Painter::setFont(const std::string&, float, int, const std::string&) {
}


void Cairo_Painter::setFontSize(float) {
}

void Cairo_Painter::translate(float dx, float dy) {
    ImVec2 pos = getRealPos(dx, dy);
    cairo_translate(m_context, (double)pos.x, (double)pos.y);
}

void Cairo_Painter::scale(float sx, float sy) {
    m_sx *= sx;
    m_sy *= sy;
    cairo_scale(m_context, (double)sx, (double)sy);
}

void Cairo_Painter::rotate(float angle) {
    cairo_rotate(m_context, (double)angle);
}

void Cairo_Painter::rotate(float angle, float px, float py) {
    ImVec2 pos = getRealPos(px, py);
    cairo_translate(m_context, (double)pos.x, (double)pos.y);
    cairo_rotate(m_context, (double)angle);
    cairo_translate(m_context, (double)-pos.x, (double)-pos.y);
}

void Cairo_Painter::reset() {
    cairo_identity_matrix(m_context);
    m_sx = m_sy = 1.f;
}

void Cairo_Painter::drawGlyph(u16, float, float) {
}

void Cairo_Painter::beginPath(i32 id) {
    cairo_new_path(m_context);
}

void Cairo_Painter::moveTo(float x, float y) {
    ImVec2 pos = getRealPos(x, y);
    cairo_move_to(m_context, (double)pos.x, (double)pos.y);
}

void Cairo_Painter::lineTo(float x, float y) {
    ImVec2 pos = getRealPos(x, y);
    cairo_line_to(m_context, pos.x, pos.y);
}

void Cairo_Painter::cubicTo(float x1, float y1, float x2, float y2, float x3, float y3) {
    ImVec2 p1 = getRealPos(x1, y1);
    ImVec2 p2 = getRealPos(x2, y2);
    ImVec2 p3 = getRealPos(x3, y3);
    cairo_curve_to(m_context, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
}

void Cairo_Painter::quadTo(float x1, float y1, float x2, float y2) {
    // See https://en.wikipedia.org/wiki/B%C3%A9zier_curve#Degree_elevation
    // and https://lists.cairographics.org/archives/cairo/2010-April/019691.html
    // for details
    double x0, y0;
    cairo_get_current_point(m_context, &x0, &y0);
    ImVec2 p1 = getRealPos(x1, y1);
    ImVec2 p2 = getRealPos(x2, y2);
    cairo_curve_to(
        m_context,
        2.0 / 3.0 * p1.x + 1.0 / 3.0 * x0,
        2.0 / 3.0 * p1.y + 1.0 / 3.0 * y0,
        2.0 / 3.0 * p1.x + 1.0 / 3.0 * p2.x,
        2.0 / 3.0 * p1.y + 1.0 / 3.0 * p2.y,
        y1, y2
    );
}

void Cairo_Painter::closePath() {
    cairo_close_path(m_context);
}

void Cairo_Painter::fillPath(i32 id) {
    cairo_fill(m_context);
}

void Cairo_Painter::drawLine(float x1, float y1, float x2, float y2) {
    ImVec2 p1 = getRealPos(x1, y1);
    ImVec2 p2 = getRealPos(x2, y2);
    cairo_move_to(m_context, p1.x, p1.y);
    cairo_line_to(m_context, p2.x, p2.y);
    cairo_stroke(m_context);
}

void Cairo_Painter::drawRect(float x, float y, float w, float h) {
    ImVec2 p1 = getRealPos(x, y);
    cairo_rectangle(m_context, p1.x, p1.y, w * m_scale.x, h * m_scale.y);
    cairo_stroke(m_context);
}

void Cairo_Painter::fillRect(float x, float y, float w, float h) {
    ImVec2 p1 = getRealPos(x, y);
    cairo_rectangle(m_context, p1.x, p1.y, w * m_scale.x, h * m_scale.y);
    cairo_fill(m_context);
}

void Cairo_Painter::roundRect(float x, float y, float w, float h, float rx, float ry) {
    rx *= m_scale.x;
    ry *= m_scale.y;
    double r = max(rx, ry);
    double d = 3.1415926535f / 180.;
    cairo_new_sub_path(m_context);
    ImVec2 p = getRealPos(x, y);
    cairo_arc(m_context, p.x + r, p.y + r, r, 180 * d, 270 * d);
    cairo_arc(m_context, p.x + w - r, p.y + r, r, -90 * d, 0);
    cairo_arc(m_context, p.x + w - r, p.y + h - r, r, 0, 90 * d);
    cairo_arc(m_context, p.x + r, p.y + h - r, r, 90 * d, 180 * d);
    cairo_close_path(m_context);
}

void Cairo_Painter::drawRoundRect(float x, float y, float w, float h, float rx, float ry) {
    roundRect(x, y, w, h, rx, ry);
    cairo_stroke(m_context);
}

void Cairo_Painter::fillRoundRect(float x, float y, float w, float h, float rx, float ry) {
    roundRect(x, y, w, h, rx, ry);
    cairo_fill(m_context);
}