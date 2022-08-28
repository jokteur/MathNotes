#include "imgui_painter.h"

using namespace microtex;

void ImGui_Painter::setColor(color color) {
}
void ImGui_Painter::setStroke(const Stroke& s) {
}
void ImGui_Painter::setStrokeWidth(float w) {
}
void ImGui_Painter::setDash(const std::vector<float>& dash) {
}
void ImGui_Painter::setFont(const std::string& path, float size, int style, const std::string& family) {
}
void ImGui_Painter::setFontSize(float size) {
}
void ImGui_Painter::translate(float dx, float dy) {
}
void ImGui_Painter::scale(float sx, float sy) {
}
void ImGui_Painter::rotate(float angle) {
}
void ImGui_Painter::rotate(float angle, float px, float py) {
}
void ImGui_Painter::reset() {
}
void ImGui_Painter::drawGlyph(u16 c, float x, float y) {
}
void ImGui_Painter::beginPath(i32 id) {
}
void ImGui_Painter::moveTo(float x, float y) {
}
void ImGui_Painter::lineTo(float x, float y) {
}
void ImGui_Painter::cubicTo(float x1, float y1, float x2, float y2, float x3, float y3) {
}
void ImGui_Painter::quadTo(float x1, float y1, float x2, float y2) {
}
void ImGui_Painter::closePath() {
}
void ImGui_Painter::fillPath() {
}
void ImGui_Painter::drawText(const std::string& t, float x, float y) {
}
void ImGui_Painter::drawLine(float x1, float y1, float x2, float y2) {
}
void ImGui_Painter::drawRect(float x, float y, float w, float h) {
}
void ImGui_Painter::fillRect(float x, float y, float w, float h) {
}
void ImGui_Painter::drawRoundRect(float x, float y, float w, float h, float rx, float ry) {
}
void ImGui_Painter::fillRoundRect(float x, float y, float w, float h, float rx, float ry) {
}