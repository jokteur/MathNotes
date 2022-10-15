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

    struct AbstractWidget : public Drawable {
        Type m_type;
        Category m_category;
        AbstractWidget(UIState_ptr ui_state) : Drawable(ui_state) {}

        // Informations about the tree structure
        std::vector<AbstractWidgetPtr> m_childrens;
        AbstractWidgetPtr m_parent = nullptr;

        void virtual buildWidget() {
            for (auto ptr : m_childrens) {
                ptr->buildWidget();
            }
        }

        // For display, start not implemented yet
        // Returns false if not succesfully build chars
        bool virtual buildAndAddChars(std::vector<DrawableCharPtr>& draw_string, std::vector<WrapCharPtr>& wrap_string, int start = -1) { return true; }
        void virtual draw(ImDrawList* draw_list) {
            for (auto ptr : m_childrens) {
                ptr->draw(draw_list);
            }
        }
        std::vector<DrawableCharPtr> m_draw_chars;
        std::vector<WrapCharPtr> m_wrap_chars;

        // Potential customizations
        Fonts::FontRequestInfo m_font_request;
        Colors::color m_font_color = Colors::BLACK;
        Colors::color m_bg_color = Colors::TRANSPARENT;
        bool m_font_underline = false;
        float m_line_space = 1.5f;

        ImVec2 m_dimensions;
        ImVec2 m_position;
        float m_scale = 1.f;
        float m_window_width = 1.f;

        // Internal
        SafeString m_safe_string;
        int m_raw_text_begin = 0;
        int m_raw_text_end = 0;

        // Events
        void virtual onClick() {}
        void virtual onSelect() {}
        void virtual onDeselect() {}
        void virtual setWidth(float width) {
            for (auto ptr : m_childrens) {
                ptr->setWidth(width);
            }
        }
    };

    struct AbstractBlock : public AbstractWidget {
    public:
        AbstractBlock(UIState_ptr ui_state) : AbstractWidget(ui_state) {
            m_category = C_BLOCK;
        }
        void setWidth(float width) {
            for (auto ptr : m_childrens) {
                ptr->setWidth(width);
            }
            m_wrapper.setWidth(width - m_x_offset);
            m_children_dirty = true;
        }

        float m_x_offset = 0.f;
        bool m_widget_dirty = true;
        bool m_children_dirty = false;
        WrapAlgorithm m_wrapper;
    };
    struct AbstractSpan : public AbstractWidget {
        AbstractSpan(UIState_ptr ui_state) : AbstractWidget(ui_state) {
            m_category = C_SPAN;
        }
    };
    struct RootNode : public AbstractWidget {
        RootNode(UIState_ptr ui_state);

        void virtual draw(ImDrawList* draw_list) override;
    };
}