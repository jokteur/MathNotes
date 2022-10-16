#pragma once

#include <tempo.h>
#include "widgets_enum.h"
#include "chars/drawable_char.h"
#include "wrapper.h"
#include "ui/drawable.h"
#include "ui/fonts.h"
#include "ui/colors.h"

namespace RichText {
    using SafeString = std::shared_ptr<std::string>;

    struct AbstractWidget;
    using AbstractWidgetPtr = std::shared_ptr<AbstractWidget>; // replace with unique_ptr?

    struct SizeProperties {
        ImVec2 dimensions;
        ImVec2 position;
        ImVec2 h_paddings;
        ImVec2 v_paddings;
        float scale = 1.f;
        float window_width = 1.f;
    };

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

        void virtual buildWidget();
        
        // For display, start not implemented yet
        // Returns false if not succesfully build chars
        bool virtual buildAndAddChars(std::vector<WrapCharPtr>& wrap_chars, std::vector<DrawableCharPtr>& draw_chars);
        void virtual draw(ImDrawList* draw_list, float& cursor_y_pos, float x_offset);


        // Font infos
        Fonts::FontRequestInfo m_font_request;
        Colors::color m_font_color = Colors::BLACK;
        Colors::color m_bg_color = Colors::TRANSPARENT;
        bool m_font_underline = false;
        float m_line_space = 1.5f;

        bool m_widget_dirty = true;
        WrapAlgorithm m_wrapper;

        SizeProperties m_size_props;
        ImVec2 m_position;

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

        void buildWidget() override;
        void draw(ImDrawList* draw_list, float& cursor_y_pos, float x_offset) override;

        void setWidth(float width) override;
    };

    struct AbstractSpan : public AbstractWidget {
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