#pragma once

#include <tempo.h>
#include <unordered_set>

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
    using AbstractWidgetPtr = std::shared_ptr<AbstractWidget>;

    struct RawTextInfo {
        int pre = 0;
        int begin = 0;
        int end = 0;
        int post;
    };
    using RawTextPtr = std::shared_ptr<RawTextInfo>;

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

        // Returns false if not succesfully build chars
        bool virtual add_chars(std::vector<WrapCharPtr>& wrap_chars);
        void virtual draw(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries);

        // Draw hooks
        float virtual hk_set_position(float& cursor_y_pos, float& x_offset);
        void virtual hk_set_dimensions(float last_y_pos, float& cursor_y_pos, float x_offset);
        void virtual hk_build_widget(float x_offset);
        void virtual hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries);
        void virtual hk_draw_background(Draw::DrawList& draw_list);
        void virtual hk_draw_show_boundaries(Draw::DrawList& draw_list);

        Style m_style;

        bool m_is_selected = true;
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
        RawTextInfo m_raw_text_info;

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

        bool m_widget_dirty_before = true;
        bool m_widget_dirty_after = true;

        void hk_build_widget(float x_offset) override;
        void hk_draw_main(Draw::DrawList& draw_list, float& cursor_y_pos, float x_offset, const Rect& boundaries) override;
        void hk_draw_background(Draw::DrawList& draw_list) override;

        void setWidth(float width) override;
    };

    struct AbstractSpan : public AbstractWidget {
        std::string m_processed_text;
        AbstractSpan(UIState_ptr ui_state) : AbstractWidget(ui_state) {
            m_category = C_SPAN;
        }
        bool add_chars(std::vector<WrapCharPtr>& wrap_chars) override;
    };

    struct RootNode : public AbstractWidget {
        RootNode(UIState_ptr ui_state) : AbstractWidget(ui_state) {
            m_type = T_ROOT;
            m_category = C_ROOT;
        }
    };
}