#pragma once

#include <memory>
#include <tempo.h>
#include "fonts.h"

#include "translations/translate.h"

struct UIState {
protected:
    UIState() {

    }
public:
    bool read_only = false;
    long long int imID = 100000000;
    float scaling = 1.f;

    UIState(UIState const&) = delete;
    void operator=(UIState const&) = delete;

    static UIState& getInstance() {
        static UIState instance;
        return instance;
    }

    // Fonts
    Fonts::FontManager font_manager;

    // Languages
    Language language = LANG_EN;
    Translator babel_fr = build_FR();
    Translator babel_default;
    Translator* babel_current = &this->babel_default;
};
typedef std::shared_ptr<UIState> UIState_ptr;