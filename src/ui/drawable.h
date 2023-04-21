#pragma once

#include <memory>

#include "state.h"

class Drawable {
protected:
    std::shared_ptr<UIState> m_ui_state;
    long long int m_id = 0;
    static long long int counter;
public:
    Drawable(std::shared_ptr<UIState> ui_state): m_ui_state(ui_state) {
        counter++;
        m_id = counter;
    }
    virtual ~Drawable() {}

    virtual void FrameUpdate() {}
    virtual void BeforeFrameUpdate() {}
};