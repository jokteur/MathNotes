#pragma once

#include "wrapper.h"
#include "ui/drawable.h"


namespace RichText {
    class Widget : public Drawable {
    private:
        float m_width;
        float m_height;
    public:
        virtual void FrameUpdate() override;
    };
}