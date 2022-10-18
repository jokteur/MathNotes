#pragma once

#include <tempo.h>

#include "types.h"
#include "ui/drawable.h"
#include "ui/fonts.h"
#include "ui/draw_commands.h"
#include "ui/colors.h"

#include "widgets_enum.h"
#include "markdown_config.h"
#include "wrapper.h"
#include "chars/drawable_char.h"

namespace RichText {
    using SafeString = std::shared_ptr<std::string>;

    struct AbstractWidget;
    using AbstractWidgetPtr = std::shared_ptr<AbstractWidget>; // replace with unique_ptr?

    struct AbstractWidget : public Drawable {
        protected:
            std::vector<DrawableCharPtr> m_draw_chars;
            std::vector<WrapCharPtr> m_wrap_chars;
            bool virtual build_chars() { return true; }
        public:
        Type m_type;
        Category m_category;
        AbstractWidget(UIState_ptr ui_state) : Drawable(ui_state) {}
        ~AbstractWidget() {};

        // Informations about the tree structure
        std::vector<AbstractWidgetPtr> m_childrens;
        AbstractWidgetPtr m_parent = nullptr;

        void virtual buildWidgetChars(float x_offset);
        
        // For display, start not implemented yet
        // Returns false if not succesfully build chars
        bool virtual buildAndAddChars(std::vector<WrapCharPtr>& wrap_chars);
        void virtual draw(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries);

        Style m_style;

        bool m_widget_dirty = true;
        WrapAlgorithm m_wrapper;

        // Widget position and size
        ImVec2 m_position;
        ImVec2 m_dimensions;
        float m_scale = 1.f;
        float m_window_width = 1.f;

        // Debug
        bool m_show_boundaries = false;


        // Internal
        SafeString m_safe_string;
        int m_raw_text_begin = 0;
        int m_raw_text_end = 0;

        // Events
        void virtual onClick() {}
        void virtual onSelect() {}
        void virtual onDeselect() {}
        void virtual setWidth(float width);
    };

    struct AbstractBlock : public AbstractWidget {
    public:
        AbstractBlock(UIState_ptr ui_state) : AbstractWidget(ui_state) {
            m_category = C_BLOCK;
        }

        void buildWidgetChars(float x_offset) override;
        void draw(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) override;

        void setWidth(float width) override;
    };

    struct AbstractSpan : public AbstractWidget {
        std::string m_processed_text;
        AbstractSpan(UIState_ptr ui_state) : AbstractWidget(ui_state) {
            m_category = C_SPAN;
        }
    };

    struct RootNode : public AbstractWidget {
        RootNode(UIState_ptr ui_state) : AbstractWidget(ui_state) {
            m_type = T_ROOT;
            m_category = C_ROOT;
        }
    };
}