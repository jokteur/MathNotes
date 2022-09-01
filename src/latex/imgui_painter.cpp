#include <iostream>

#include "imgui_painter.h"

using namespace microtex;

void ImGui_Painter::setFontInfos(const FontInfos& font_infos) {
    for (auto& pair : m_fonts) {
        Tempo::RemoveFont(pair.second);
    }
    for (auto& pair : font_infos) {
        auto& path = pair.first;
        ImVector<ImWchar> ranges;
        ImFontGlyphRangesBuilder builder;
        builder.AddText("a"); // For safety
        for (u16 c : pair.second.glyphs) {
            if (c == 0)
                continue;
            builder.AddChar(c);
        }
        if (!pair.second.text.empty())
            builder.AddText(pair.second.text.c_str());
        builder.BuildRanges(&ranges);

        Tempo::FontID font_id = Tempo::AddFontFromFileTTF(path, pair.second.max_real_size, ImFontConfig{}, ranges, true).value();
        m_fonts[path] = font_id;
    }
}

void ImGui_Painter::setColor(color color) {
}
void ImGui_Painter::setStroke(const Stroke& s) {
}
void ImGui_Painter::setStrokeWidth(float w) {
}
void ImGui_Painter::setDash(const std::vector<float>& dash) {
}
void ImGui_Painter::setFont(const std::string& path, float size, int style, const std::string& family) {
    if (m_is_prev_font) {
        Tempo::PopFont();
    }
    m_is_prev_font = true;
    Tempo::PushFont(m_fonts[path]);
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
void ImGui_Painter::drawGlyph(u32 c, float x, float y) {
    std::string u8str = m_converter.to_bytes(c);
    ImGui::Text(u8str.c_str());

    // drawText(u8"\u42", x, y);
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
    ImGui::Text(t.c_str());
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
void ImGui_Painter::finish() {
    if (m_is_prev_font) {
        Tempo::PopFont();
    }
    m_is_prev_font = false;
}