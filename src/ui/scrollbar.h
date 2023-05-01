#pragma once

#include "geometry/basic.h"
#include "drawable.h"
#include "draw_commands.h"
#include "types.h"

class VerticalScrollBar: public Drawable {
public:
    enum Mode { FIT, FIT_UNTIL_LAST_LINE };
private:
    float m_percentage;
    float m_min_scroll_height;
    float m_y_grab_pos = 0.f;
    bool m_grabbing = false;
    bool m_has_changed = false;
    bool m_grab_mode = false;
    Mode m_display_mode;
public:
    VerticalScrollBar(Mode display_mode);

    void FrameUpdate(const Rect& boundaries, Draw::DrawList& m_draw_list, float before, float after, const std::string& window_name);

    float getPercentage() { return m_percentage; }
    bool hasChanged() { return m_has_changed; }
    bool isGrabbing() { return m_grabbing; }

    void setMinScrollHeight(float min_height) { m_min_scroll_height = min_height; }
};