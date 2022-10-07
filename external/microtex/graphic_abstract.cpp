#include <cmath>
#include "unimath/uni_char.h"

#include "graphic_abstract.h"
#include "utils/utils.h"

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

Argument::Argument(const std::string& data) {
    m_data = std::make_shared<std::string>(data);
}
Argument::Argument(float data) {
    m_data = std::make_shared<float>(data);
}
Argument::Argument(const std::vector<float>& data) {
    m_data = std::make_shared<std::vector<float>>(data);
}
Argument::Argument(const Stroke& data) {
    m_data = std::make_shared<Stroke>(data);
}
Argument::Argument(int data) {
    m_data = std::make_shared<int>(data);
}
Argument::Argument(color data) {
    m_data = std::make_shared<color>(data);
}
Argument::Argument(u16 data) {
    m_data = std::make_shared<u16>(data);
}

std::unordered_map<std::string, sptr<Font_abstract>> Font_abstract::fonts_sptr = std::unordered_map<std::string, sptr<Font_abstract>>();

/************ FONT ************/
Font_abstract::Font_abstract(const std::string& family, int style, float size) {
    m_family = family;
    m_style = style;
    m_font_size = size;
}
Font_abstract::Font_abstract(const std::string& file, float size) {
    m_path = file;
    m_font_size = size;
    m_style = -1;
}
void Font_abstract::setSize(float size) {
    m_font_size = size;
}
std::string Font_abstract::getFamily() const {
    return m_family;
}
bool Font_abstract::operator==(const Font& f) const {
    return false; //static_cast<Font_abstract>(f).m_path == m_path; //f.m_font_id == m_font_id;
}
sptr<Font_abstract> Font_abstract::getOrCreate(const std::string& file) {
    if (fonts_sptr.find(file) != fonts_sptr.end()) {
        return fonts_sptr[file];
    }
    else {
        return sptrOf<Font_abstract>(file, 10.f);
    }
}

/************ TextLayout ***********/

TextLayout_abstract::TextLayout_abstract(const std::string& src, FontStyle style, float size)
    : m_txt(src), m_style(style), m_size(size) {
}
void TextLayout_abstract::getBounds(Rect& r) {

}
void TextLayout_abstract::draw(Graphics2D& g2, float x, float y) {
    auto& g = static_cast<Graphics2D_abstract&>(g2);
    auto prev = g2.getFont();
    auto prevSize = g2.getFontSize();
    g.setFont(m_font);
    g.setFontSize(m_size);
    g.drawText(m_txt, x, y);
    g.setFontSize(prevSize);
    g.setFont(prev);
}

/************ PlatformFactory ************/
sptr<microtex::Font> PlatformFactory_abstract::createFont(const std::string& file) {
    return Font_abstract::getOrCreate(file);
}

sptr<TextLayout> PlatformFactory_abstract::createTextLayout(const std::string& src, FontStyle style, float size) {
    return sptrOf<TextLayout_abstract>(src, style, size);
}

/************ Graphics2D ************/
Graphics2D_abstract::Graphics2D_abstract() {

}
Graphics2D_abstract::~Graphics2D_abstract() {

}

ImVec2 Graphics2D_abstract::getScaledMin() {
    return ImVec2(m_min_x, m_min_y);
}
ImVec2 Graphics2D_abstract::getScaledMax() {
    return ImVec2(m_max_x, m_max_y);
}

void Graphics2D_abstract::pushMinMax(float x, float y) {
    m_min_x = min(x * m_sx + m_dx, m_min_x);
    m_max_x = max(x * m_sx + m_dx, m_max_x);
    m_min_y = min(y * m_sy + m_dy, m_min_y);
    m_max_y = max(y * m_sy + m_dy, m_max_y);
}

void Graphics2D_abstract::updateFontInfo(const std::string& text) {
    auto& font_infos = m_font_infos[m_font->getPath()];
    font_infos.text += text;
    float actual_font_size = max(abs(m_sx) * m_font->getSize(), abs(m_sy) * m_font->getSize());
    if (actual_font_size > font_infos.max_real_size)
        font_infos.max_real_size = actual_font_size;
}
void Graphics2D_abstract::updateFontInfo(u32 c) {
    auto& font_infos = m_font_infos[m_font->getPath()];
    font_infos.glyphs.push_back(c);
    float actual_font_size = max(abs(m_sx) * m_font->getSize(), abs(m_sy) * m_font->getSize());
    if (actual_font_size > font_infos.max_real_size)
        font_infos.max_real_size = actual_font_size;
}


std::vector<Call> Graphics2D_abstract::getCallList() {
    return m_calls;
}

void Graphics2D_abstract::distributeCallList(Painter* painter) {
    for (auto& call : m_calls) {
        if (call.fct_name == "setColor") {
            painter->setColor(call.arguments[0].getData<color>());
        }
        else if (call.fct_name == "setStroke") {
            painter->setStroke(call.arguments[0].getData<Stroke>());
        }
        else if (call.fct_name == "setStrokeWidth") {
            painter->setStrokeWidth(call.arguments[0].getData<float>());
        }
        else if (call.fct_name == "setDash") {
            painter->setDash(call.arguments[0].getData<std::vector<float>>());
        }
        else if (call.fct_name == "setFont") {
            painter->setFont(
                call.arguments[0].getData<std::string>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<int>(),
                call.arguments[3].getData<std::string>()
            );
        }
        else if (call.fct_name == "setFontSize") {
            painter->setFontSize(call.arguments[0].getData<float>());
        }
        else if (call.fct_name == "translate") {
            painter->translate(call.arguments[0].getData<float>(), call.arguments[1].getData<float>());
        }
        else if (call.fct_name == "scale") {
            painter->scale(call.arguments[0].getData<float>(), call.arguments[1].getData<float>());
        }
        else if (call.fct_name == "rotate") {
            painter->rotate(call.arguments[0].getData<float>());
        }
        else if (call.fct_name == "rotateAroundPt") {
            painter->rotate(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>()
            );
        }
        else if (call.fct_name == "reset") {
            painter->reset();
        }
        else if (call.fct_name == "drawGlyph") {
            painter->drawGlyph(
                call.arguments[0].getData<u32>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>()
            );
        }
        else if (call.fct_name == "beginPath") {
            painter->beginPath(call.arguments[0].getData<i32>());
        }
        else if (call.fct_name == "moveTo") {
            painter->moveTo(call.arguments[0].getData<float>(), call.arguments[1].getData<float>());
        }
        else if (call.fct_name == "lineTo") {
            painter->lineTo(call.arguments[0].getData<float>(), call.arguments[1].getData<float>());
        }
        else if (call.fct_name == "cubicTo") {
            painter->cubicTo(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>(),
                call.arguments[4].getData<float>(),
                call.arguments[5].getData<float>()
            );
        }
        else if (call.fct_name == "quadTo") {
            painter->quadTo(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>()
            );
        }
        else if (call.fct_name == "closePath") {
            painter->closePath();
        }
        else if (call.fct_name == "fillPath") {
            painter->fillPath(call.arguments[0].getData<i32>());
        }
        else if (call.fct_name == "drawLine") {
            painter->drawLine(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>()
            );
        }
        else if (call.fct_name == "drawRect") {
            painter->drawRect(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>()
            );
        }
        else if (call.fct_name == "fillRect") {
            painter->fillRect(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>()
            );
        }
        else if (call.fct_name == "drawRoundRect") {
            painter->drawRoundRect(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>(),
                call.arguments[4].getData<float>(),
                call.arguments[5].getData<float>()
            );
        }
        else if (call.fct_name == "fillRoundRect") {
            painter->fillRoundRect(
                call.arguments[0].getData<float>(),
                call.arguments[1].getData<float>(),
                call.arguments[2].getData<float>(),
                call.arguments[3].getData<float>(),
                call.arguments[4].getData<float>(),
                call.arguments[5].getData<float>()
            );
        }
    }
}
void Graphics2D_abstract::resetCallList() {
    m_calls.clear();
}
void Graphics2D_abstract::setColor(color color) {
    m_color = color;

    std::vector<Argument> arguments;
    arguments.push_back(Argument(color));
    m_calls.push_back(Call{ "setColor", arguments });
}
color Graphics2D_abstract::getColor() const {
    return m_color;
}
void Graphics2D_abstract::setStroke(const Stroke& s) {
    m_stroke = s;

    std::vector<Argument> arguments;
    arguments.push_back(Argument(s));
    m_calls.push_back(Call{ "setStroke", arguments });
}
const Stroke& Graphics2D_abstract::getStroke() const {
    return m_stroke;
}
void Graphics2D_abstract::setStrokeWidth(float w) {
    m_stroke.lineWidth = w;

    std::vector<Argument> arguments;
    arguments.push_back(Argument(w));
    m_calls.push_back(Call{ "setStrokeWidth", arguments });
}

void Graphics2D_abstract::setDash(const std::vector<float>& dash) {
    m_dash = dash;

    std::vector<Argument> arguments;
    arguments.push_back(Argument(dash));
    m_calls.push_back(Call{ "setDash", arguments });
}
std::vector<float> Graphics2D_abstract::getDash() {
    return m_dash;
}

sptr<Font> Graphics2D_abstract::getFont() const {
    return sptr<Font>(m_font);
}
void Graphics2D_abstract::setFont(const sptr<Font>& font) {
    m_font = (Font_abstract*)font.get();

    if (m_font_infos.find(m_font->getPath()) == m_font_infos.end()) {
        m_font_infos[m_font->getPath()] = FontInfo();
    }

    std::vector<Argument> arguments;
    arguments.push_back(Argument(m_font->getPath()));
    arguments.push_back(Argument(m_font->getSize()));
    arguments.push_back(Argument(m_font->getStyle()));
    arguments.push_back(Argument(m_font->getFamily()));
    m_calls.push_back(Call{ "setFont", arguments });
}
float Graphics2D_abstract::getFontSize() const {
    return m_font->getSize();
}
void Graphics2D_abstract::setFontSize(float size) {
    m_font->setSize(size);

    std::vector<Argument> arguments;
    arguments.push_back(Argument(size));
    m_calls.push_back(Call{ "setFontSize", arguments });
}
void Graphics2D_abstract::translate(float dx, float dy) {
    std::vector<Argument> arguments;
    m_dx = dx * m_sx;
    m_dy = dy * m_sy;

    arguments.push_back(Argument(dx));
    arguments.push_back(Argument(dy));
    m_calls.push_back(Call{ "translate", arguments });
}
void Graphics2D_abstract::scale(float sx, float sy) {
    m_sx *= sx;
    m_sy *= sy;

    std::vector<Argument> arguments;
    arguments.push_back(Argument(sx));
    arguments.push_back(Argument(sy));
    m_calls.push_back(Call{ "scale", arguments });
}
void Graphics2D_abstract::rotate(float angle) {
    std::vector<Argument> arguments;
    arguments.push_back(Argument(angle));
    m_calls.push_back(Call{ "rotate", arguments });
}
void Graphics2D_abstract::rotate(float angle, float px, float py) {
    std::vector<Argument> arguments;
    arguments.push_back(Argument(angle));
    arguments.push_back(Argument(px));
    arguments.push_back(Argument(py));
    m_calls.push_back(Call{ "rotateAroundPt", arguments });
}
void Graphics2D_abstract::reset() {
    std::vector<Argument> arguments;
    m_sx = 1.f;
    m_sy = 1.f;
    m_dx = 0.f;
    m_dy = 0.f;
    m_calls.push_back(Call{ "reset", arguments });
}
float Graphics2D_abstract::sx() const {
    return m_sx;
}
float Graphics2D_abstract::sy() const {
    return m_sy;
}
void Graphics2D_abstract::drawGlyph(u16 c, float x, float y) {
    // Convert to u8 string
    updateFontInfo(c);

    float size = m_font->getSize();
    pushMinMax(x, y);
    pushMinMax(x + 0.5f * size, y + size);

    std::vector<Argument> arguments;
    arguments.push_back(Argument(c));
    arguments.push_back(Argument(x));
    arguments.push_back(Argument(y));
    m_calls.push_back(Call{ "drawGlyph", arguments });
}
bool Graphics2D_abstract::beginPath(i32 id) {
    std::vector<Argument> arguments;
    m_path_id = id;
    arguments.push_back(Argument(id));
    m_calls.push_back(Call{ "beginPath", arguments });
    return false;
}
void Graphics2D_abstract::moveTo(float x, float y) {
    pushMinMax(x, y);
    std::vector<Argument> arguments;
    arguments.push_back(Argument(x));
    arguments.push_back(Argument(y));
    m_calls.push_back(Call{ "moveTo", arguments });
}
void Graphics2D_abstract::lineTo(float x, float y) {
    pushMinMax(x, y);
    std::vector<Argument> arguments;
    arguments.push_back(Argument(x));
    arguments.push_back(Argument(y));
    m_calls.push_back(Call{ "lineTo", arguments });
}
void Graphics2D_abstract::cubicTo(float x1, float y1, float x2, float y2, float x3, float y3) {
    pushMinMax(x1, y1);
    pushMinMax(x2, y2);
    pushMinMax(x3, y3);
    std::vector<Argument> arguments;
    arguments.push_back(Argument(x1));
    arguments.push_back(Argument(y1));
    arguments.push_back(Argument(x2));
    arguments.push_back(Argument(y2));
    arguments.push_back(Argument(x3));
    arguments.push_back(Argument(y3));
    m_calls.push_back(Call{ "cubicTo", arguments });
}
void Graphics2D_abstract::quadTo(float x1, float y1, float x2, float y2) {
    pushMinMax(x1, y1);
    pushMinMax(x2, y2);
    std::vector<Argument> arguments;
    arguments.push_back(Argument(x1));
    arguments.push_back(Argument(y1));
    arguments.push_back(Argument(x2));
    arguments.push_back(Argument(y2));
    m_calls.push_back(Call{ "quadTo", arguments });
}
void Graphics2D_abstract::closePath() {
    std::vector<Argument> arguments;
    m_calls.push_back(Call{ "closePath", arguments });
}
void Graphics2D_abstract::fillPath(i32 id) {
    std::vector<Argument> arguments;
    arguments.push_back(Argument(m_path_id));
    m_calls.push_back(Call{ "fillPath", arguments });
}
void Graphics2D_abstract::drawText(const std::string& t, float x, float y) {
    updateFontInfo(t);

    // Calculate the number of utf8 characters
    int count = 0;
    if (!t.empty()) {
        for (auto p : t)
            count += ((p & 0xc0) != 0x80);
    }

    float size = m_font->getSize();
    pushMinMax(x, y);
    pushMinMax(x + 0.5f * size * count, y + size);

    std::vector<Argument> arguments;
    arguments.push_back(Argument(t));
    arguments.push_back(Argument(x));
    arguments.push_back(Argument(y));
    m_calls.push_back(Call{ "drawText", arguments });
}
void Graphics2D_abstract::drawLine(float x1, float y1, float x2, float y2) {
    std::vector<Argument> arguments;
    pushMinMax(x1, y1);
    pushMinMax(x2, y2);
    arguments.push_back(Argument(x1));
    arguments.push_back(Argument(y1));
    arguments.push_back(Argument(x2));
    arguments.push_back(Argument(y2));
    m_calls.push_back(Call{ "drawLine", arguments });
}
void Graphics2D_abstract::drawRect(float x, float y, float w, float h) {
    std::vector<Argument> arguments;
    pushMinMax(x, y);
    pushMinMax(x + w, y + h);
    arguments.push_back(Argument(x));
    arguments.push_back(Argument(y));
    arguments.push_back(Argument(w));
    arguments.push_back(Argument(h));
    m_calls.push_back(Call{ "drawRect", arguments });
}
void Graphics2D_abstract::fillRect(float x, float y, float w, float h) {
    std::vector<Argument> arguments;
    pushMinMax(x, y);
    pushMinMax(x + w, y + h);
    arguments.push_back(Argument(x));
    arguments.push_back(Argument(y));
    arguments.push_back(Argument(w));
    arguments.push_back(Argument(h));
    m_calls.push_back(Call{ "fillRect", arguments });
}
void Graphics2D_abstract::drawRoundRect(float x, float y, float w, float h, float rx, float ry) {
    std::vector<Argument> arguments;
    pushMinMax(x, y);
    pushMinMax(x + w, y + h);
    arguments.push_back(Argument(x));
    arguments.push_back(Argument(y));
    arguments.push_back(Argument(w));
    arguments.push_back(Argument(h));
    arguments.push_back(Argument(rx));
    arguments.push_back(Argument(ry));
    m_calls.push_back(Call{ "drawRoundRect", arguments });
}
void Graphics2D_abstract::fillRoundRect(float x, float y, float w, float h, float rx, float ry) {
    std::vector<Argument> arguments;
    pushMinMax(x, y);
    pushMinMax(x + w, y + h);
    arguments.push_back(Argument(x));
    arguments.push_back(Argument(y));
    arguments.push_back(Argument(w));
    arguments.push_back(Argument(h));
    arguments.push_back(Argument(rx));
    arguments.push_back(Argument(ry));
    m_calls.push_back(Call{ "fillRoundRect", arguments });
}