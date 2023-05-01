#pragma once

#include <memory>

#include "state.h"

class Drawable {
protected:
    UIState& m_ui_state;
    long long int m_id = 0;
    static long long int counter;
public:
    Drawable(): m_ui_state(UIState::getInstance()) {
        counter++;
        m_id = counter;
    }
    virtual ~Drawable() {}

    virtual void FrameUpdate() {}
    virtual void BeforeFrameUpdate() {}
};