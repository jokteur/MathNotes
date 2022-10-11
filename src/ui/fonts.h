#pragma once

#include <tempo.h>

#ifndef FONT_ICON_FILE_NAME_MD
#include "IconsMaterialDesign.h"
#endif

namespace Fonts {
    enum Family { F_REGULAR, F_CONDENSED, F_MONOSPACE };
    enum Weight { W_THIN, W_LIGHT, W_REGULAR, W_MEDIUM, W_BOLD };
    enum Style { S_NORMAL, S_ITALIC };
    enum Range {
        R_LATIN,
        R_GREEK,
        R_KOREAN,
        R_CHINESE_FULL,
        R_CHINESE_SIMPLIFIED,
        R_JAPANESE,
        R_CYRILLIC,
        R_THAI,
        R_VIETNAMESE,
        R_ARABIC,
    };
    enum Error {
        E_OK,
        E_PATH_NOT_FOUND,
        E_FILE_NOT_VALID,
        E_FONT_NOT_FOUND,
        W_FALLBACK_TO_DEFAULT_FONT
    };

    struct Font {
        // Todo, range to path
        std::string path;
        std::map<int, Tempo::FontID> font_sizes;
        ImVector<ImWchar> range;
        std::string icon_path;
        ImVector<ImWchar> icon_range;
    };
    struct FontStyling {
        Family family;
        Weight weight;
        Style style;
    };

    struct FontRequestInfo {
        FontStyling font_styling;
        float size_wish;
        bool auto_scaling = true;
        bool exact_request = false;
    };

    struct FontInfoOut {
        float size = 0.f;
        float ratio = 1.f; // Ratio between size wish and size out
        Tempo::FontID font_id = -1;
    };

    class FontManager {
    private:
        std::unordered_map<int, Font> m_fonts;

        int get_font_uuid(const FontStyling& font_styling);
        void get_font_info_from_uuid(int uuid, Family& family, Weight& weight, Style& style);

        void construct_font(int uuid, float size, bool auto_scaling);
    public:
        Error addIconsToFont(const FontStyling& font_styling, const std::string& path, const ImVector<ImWchar>& range);
        void setFontPath(const FontStyling& font_styling, const std::string& path, Range range = R_LATIN);
        void setFontPath(const FontStyling& font_styling, const std::string& path, const ImVector<ImWchar>& range);
        void setFallBack(const FontStyling& from, const FontStyling& to);
        void setWeightFallBack(Family family, Weight from, Weight to);
        void setStyleFallBack(Family family, Style from, Style to);
        Error setDefaultFont(FontRequestInfo& font_info);
        void addToFontRange(const FontRequestInfo& font_info, const ImVector<ImWchar>& range);
        void addToFontRange(const FontRequestInfo& font_info, Range range);
        void addToFontRangeFromStr(const FontRequestInfo& font_info, const char* str, int size);

        Error requestFont(const FontRequestInfo& font_info, FontInfoOut& font_info_out);
    };
}