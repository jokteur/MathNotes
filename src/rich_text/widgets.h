#pragma once

#include <tempo.h>

#include "widgets_enum.h"
#include "chars/drawable_char.h"
#include "wrapper.h"
#include "ui/drawable.h"

namespace RichText {
    using SafeString = std::shared_ptr<std::string>;

    struct AbstractWidget;
    using AbstractWidgetPtr = std::shared_ptr<AbstractWidget>; // replace with unique_ptr?

    struct AbstractWidget : public Drawable {
        Type type;
        AbstractWidget(UIState_ptr ui_state) : Drawable(ui_state) {}

        // Informations about the tree structure
        std::vector<AbstractWidgetPtr> childrens;
        AbstractWidgetPtr parent = nullptr;
        int level = 0;

        // For display, start not implemented yet
        // Returns false if not succesfully build char
        bool virtual buildAndAddChars(std::vector<WrapCharPtr>& string, int start = -1) { return true; }
        void virtual draw(ImDrawList* draw_list) {}
        std::vector<DrawableCharPtr> chars;
        ImVec2 dimensions;
        ImVec2 position;

        float scale = 1.f;

        // Internal
        SafeString safe_string;
        int text_begin = 0;
        int text_end = 0;

        // Events
        void virtual onClick() {}
        void virtual onSelect() {}
        void virtual onDeselect() {}
    };

    struct AbstractBlock : public AbstractWidget {
        AbstractBlock(UIState_ptr ui_state) : AbstractWidget(ui_state) {}
    };
    struct AbstractSpan : public AbstractWidget {
        AbstractSpan(UIState_ptr ui_state) : AbstractWidget(ui_state) {}
    };
    struct RootNode : public AbstractWidget {
        RootNode(UIState_ptr ui_state) : AbstractWidget(ui_state) {
            type = T_ROOT;
        }
    };
}