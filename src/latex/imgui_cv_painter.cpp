#include <iostream>
#include <opencv2/imgproc.hpp>

#include "imgui_cv_painter.h"

using namespace microtex;

void ImGuiCV_Painter::finishPath() {
    if (m_draw_list->_Path.empty())
        return;
    std::vector<cv::Point> positions;
    for (auto pt : m_draw_list->_Path) {
        positions.push_back(cv::Point(pt.x, pt.y));
    }

    ImVec4 color = ImGui::ColorConvertU32ToFloat4(m_color);
    auto col = cv::Scalar((int)(color.x * 255), (int)(color.y * 255), (int)(color.z * 255), (int)(color.w * 255));

    if (m_fill_path) {
        // positions.push_back(positions[0]);
        positions.pop_back();
        cv::fillPoly(m_canvas, positions, col);
    }
    else {
        cv::polylines(m_canvas, positions, false, col, m_stroke.lineWidth * m_sx * m_scale.x, cv::LINE_AA);
    }
    m_draw_list->PathClear();
}

ImVec2 ImGuiCV_Painter::getRealPos(float x, float y) {
    return ImVec2(
        m_scale.x * (m_dx + x * m_sx) + m_offset.x,
        m_scale.y * (m_dy + y * m_sy) + m_offset.y
    );
}

void ImGuiCV_Painter::draw(ImDrawList* draw_list) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    pos.x += 100;
    pos.y += 100;
    int width = (float)((int)m_image.width() / 1);
    int height = (float)((int)m_image.height() / 1);
    draw_list->AddImage(m_image.texture(), pos, ImVec2(pos.x + width, pos.y + height));
}


void ImGuiCV_Painter::setColor(color color) {
    m_color = color;
}
void ImGuiCV_Painter::setStroke(const Stroke& s) {
    m_stroke = s;
}
void ImGuiCV_Painter::setStrokeWidth(float w) {
    m_stroke.lineWidth = w;
}
void ImGuiCV_Painter::setDash(const std::vector<float>& dash) {
    m_dash = dash;
}
void ImGuiCV_Painter::setFont(const std::string& path, float size, int style, const std::string& family) {
    // Not implemented
    // Everything is drawn by Path
}
void ImGuiCV_Painter::setFontSize(float size) {
    // Not implemented
    // Everything is drawn by Path
}
void ImGuiCV_Painter::translate(float dx, float dy) {
    m_dx = m_sx * dx;
    m_dy = m_sy * dy;
}
void ImGuiCV_Painter::scale(float sx, float sy) {
    m_sx *= sx;
    m_sy *= sy;
}
void ImGuiCV_Painter::rotate(float angle) {
    // TODO
}
void ImGuiCV_Painter::rotate(float angle, float px, float py) {
    // TODO
}
void ImGuiCV_Painter::reset() {
    float m_sx = 1.f;
    float m_sy = 1.f;
    float m_dx = 0.f;
    float m_dy = 0.f;
}
void ImGuiCV_Painter::drawGlyph(u16 c, float x, float y) {
    // Not implemented
    // Everything is drawn by Path
}
void ImGuiCV_Painter::beginPath(i32 id) {
    if (m_prev_path) {
        finishPath();
    }
    m_fill_path = false;
    m_prev_path = true;
    m_draw_list->PathClear();
}
void ImGuiCV_Painter::moveTo(float x, float y) {
    ImVec2 to = getRealPos(x, y);
    m_draw_list->PathLineTo(to);
}
void ImGuiCV_Painter::lineTo(float x, float y) {
    ImVec2 to = getRealPos(x, y);
    m_draw_list->PathLineTo(to);
}
void ImGuiCV_Painter::cubicTo(float x1, float y1, float x2, float y2, float x3, float y3) {
    ImVec2 p2 = getRealPos(x1, y1);
    ImVec2 p3 = getRealPos(x2, y2);
    ImVec2 p4 = getRealPos(x3, y3);
    m_draw_list->PathBezierCubicCurveTo(p2, p3, p4, 20);
}
void ImGuiCV_Painter::quadTo(float x1, float y1, float x2, float y2) {
    ImVec2 p2 = getRealPos(x1, y1);
    ImVec2 p3 = getRealPos(x2, y2);
    m_draw_list->PathBezierQuadraticCurveTo(p2, p3, 20);
}
void ImGuiCV_Painter::closePath() {
    // Path is closed in fillPath or beginPath/finish because microtex
    // closePath before fillPath if it needs to be filled
}
void ImGuiCV_Painter::fillPath() {
    m_fill_path = true;
    finishPath();
}
void ImGuiCV_Painter::drawText(const std::string& t, float x, float y) {
    // Not implemented
    // Everything is drawn by Path
}
void ImGuiCV_Painter::drawLine(float x1, float y1, float x2, float y2) {
    // SetCursorPos(x1, y1);
    ImVec2 from = getRealPos(x1, y1);
    ImVec2 to = getRealPos(x2, y2);

    // m_draw_list->AddLine(from, to, m_color, m_stroke.lineWidth * m_sx);
}
void ImGuiCV_Painter::drawRect(float x, float y, float w, float h) {
    ImVec2 top_left = getRealPos(x, y);
    ImVec2 bottom_right = getRealPos(x + w, y + h);
    // m_draw_list->AddRect(top_left, bottom_right, m_color, 0.f, 0, m_stroke.lineWidth * m_sx);
}
void ImGuiCV_Painter::fillRect(float x, float y, float w, float h) {
    ImVec2 top_left = getRealPos(x, y);
    ImVec2 bottom_right = getRealPos(x + w, y + h);
    // m_draw_list->AddRectFilled(top_left, bottom_right, m_color, 0.f, 0);
}
void ImGuiCV_Painter::drawRoundRect(float x, float y, float w, float h, float rx, float ry) {
    ImVec2 top_left = getRealPos(x, y);
    ImVec2 bottom_right = getRealPos(x + w, y + h);
    // m_draw_list->AddRect(top_left, bottom_right, m_color, 0.f, rx * m_sx, m_stroke.lineWidth * m_sx);
}
void ImGuiCV_Painter::fillRoundRect(float x, float y, float w, float h, float rx, float ry) {
    ImVec2 top_left = getRealPos(x, y);
    ImVec2 bottom_right = getRealPos(x + w, y + h);
    // m_draw_list->AddRectFilled(top_left, bottom_right, m_color, rx * m_sx, rx * m_sx);
}
void ImGuiCV_Painter::start(ImVec2 top_left, ImVec2 bottom_right, ImVec2 scale, float oversampling) {
    m_image.reset();
    m_oversampling = oversampling;
    // Bake oversampling directly into the scale
    scale.x *= oversampling;
    scale.y *= oversampling;
    m_offset = ImVec2(-top_left.x, -top_left.y);
    m_dimensions = ImVec2(scale.x * (bottom_right.x - top_left.x), scale.y * (bottom_right.y - top_left.y));
    m_scale = scale;
    m_canvas = cv::Mat((int)m_dimensions.y, (int)m_dimensions.x, CV_8UC4);
}
void ImGuiCV_Painter::finish() {
    if (m_prev_path) {
        finishPath();
    }
    int width = (int)((float)m_canvas.cols / m_oversampling);
    int height = (int)((float)m_canvas.rows / m_oversampling);
    cv::Mat out(width, height, CV_8UC4);
    // cv::resize(m_canvas, out, cv::Size(), 1.f / m_oversampling, 1.f / m_oversampling, cv::INTER_AREA);
    m_image.setImage(m_canvas.data, m_canvas.cols, m_canvas.rows, Image::FILTER_BILINEAR);
}