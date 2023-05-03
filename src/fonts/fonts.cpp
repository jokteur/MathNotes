#include "fonts.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace Fonts {
    void FontManager::delete_chars() {
        for (auto pair : m_chars) {
            delete pair.second;
        }
    }

    int FontManager::get_font_uuid(const FontStyling& font_styling) {
        return (font_styling.family + 1) + 10 * (font_styling.weight + 1) + 100 * (font_styling.style + 1);
    }
    void FontManager::get_font_info_from_uuid(int uuid, Family& family, Weight& weight, Style& style) {
        int style_ = uuid / 100;
        int weight_ = (uuid - style_ * 100) / 10;
        int family_ = (uuid - style_ * 100 - weight_ * 10);
        style = Style(style_);
        weight = Weight(weight_);
        family = Family(family_);
    }
    void FontManager::setFontPath(const FontStyling& font_styling, const std::string& path, Range range) {
        Font font;
        font.path = path;
        m_fonts[get_font_uuid(font_styling)] = font;
        // TODO: check if previous font exists
        // TODO: check if font path is valid
        // TODO: range
    }
    Error FontManager::addIconsToFont(const FontStyling& font_styling, const std::string& path, const ImVector<ImWchar>& range) {
        int uuid = get_font_uuid(font_styling);

        // TODO: check if font is valid
        if (m_fonts.find(uuid) == m_fonts.end()) {
            return E_FONT_NOT_FOUND;
        }
        m_fonts[uuid].icon_path = path;
        m_fonts[uuid].icon_range = range;
        return E_OK;
    }
    void FontManager::setFallBack(const FontStyling& from, const FontStyling& to) {
        //TODO
    }
    void FontManager::setWeightFallBack(Family family, Weight from, Weight to) {
        //TODO
    }
    void FontManager::setStyleFallBack(Family family, Style from, Style to) {
        //TODO
    }
    Error FontManager::setDefaultFont(FontRequestInfo& font_info) {
        FontInfoOut tmp;
        // TODO, keep in memory
        font_info.exact_request = true;
        return requestFont(font_info, tmp);
    }

    Error FontManager::requestFont(const FontRequestInfo& font_info, FontInfoOut& font_info_out) {
        int uuid = get_font_uuid(font_info.font_styling);

        if (m_fonts.find(uuid) == m_fonts.end()) {
            // No fallback for now
            return E_FONT_NOT_FOUND;
        }
        auto& font = m_fonts[uuid];
        float font_size = 0.f;
        if (font_info.exact_request) {
            float size_wish = font_info.size_wish;
            font_info_out.size = size_wish;
            if (font.font_sizes.find(size_wish) != font.font_sizes.end()) {
                font_info_out.font_id = font.font_sizes[size_wish];
            }
            else {
                auto res = Tempo::AddFontFromFileTTF(font.path, size_wish, ImFontConfig{}, ImVector<ImWchar>(), !font_info.auto_scaling);
                font.font_sizes[size_wish] = res.value();
                font_info_out.font_id = res.value();
            }
            return E_OK;
        }

        for (auto& pair : font.font_sizes) {
            if (font_info.size_wish <= pair.first) {
                font_size = pair.first;
                break;
            }
        }
        if (font_info.size_wish <= font_size / 2 || font_size == 0.f) {
            // Build font
            font_size = font_info.size_wish;
            // TODO: check validity
            auto res = Tempo::AddFontFromFileTTF(font.path, font_info.size_wish, ImFontConfig{}, ImVector<ImWchar>());
            font.font_sizes[font_size] = res.value();
        }
        font_info_out.font_id = font.font_sizes[font_size];
        font_info_out.size = font_size;
        font_info_out.ratio = font_info.size_wish / font_size;

        return E_OK;
    }

    bool FontManager::requestCharString(std::vector<Character*>& chars, const std::string& str, int start, int end, FontStyling style, const emfloat& font_size, bool replace_spaces_by_points) {
        /* Request the dynamic DPI font */
        FontRequestInfo font_request;
        font_request.font_styling = style;
        font_request.size_wish = font_size.f;

        FontInfoOut font_out;
        requestFont(font_request, font_out);
        float actual_font_size = font_out.size * font_out.ratio * Tempo::GetScaling();

        auto font = Tempo::GetImFont(font_out.font_id);
        if (font->im_font == nullptr) {
            return false;
        }

        char* s = (char*)(str.c_str() + start);
        const char* text_end = (char*)(str.c_str() + end);
        while (s < text_end) {
            unsigned int c = (unsigned int)*s;
            if (c >= 0x80) {
                s += ImTextCharFromUtf8(&c, s, text_end);
                if (c == 0) // Malformed UTF-8?
                    break;
            }
            else {
                if (replace_spaces_by_points && c == ' ') {
                    c = 183;
                }
                s += 1;
            }
            if (c == '\r')
                continue;
            bool force_breakable = false;
            if (c == ',' || c == '|' || c == '-' || c == '.' || c == '!' || c == '?')
                force_breakable = true;

            /* Once the char has been defined, we can request information on it */
            CharId key{ font_out.font_id, c };
            auto it = m_chars.find(key);

            if (it != m_chars.end()) {
                chars.push_back(it->second);
            }
            else {
                auto char_ptr = new Character;
                fillCharInfos(char_ptr, c, actual_font_size, font, force_breakable);
                chars.push_back(char_ptr);
            }
        }
        return true;
    }
}