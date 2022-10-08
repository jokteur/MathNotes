#pragma once

#include <tempo.h>

#include "widgets_enum.h"
#include "wrapper.h"

namespace RichText {
    struct AbstractWidget;
    using AbstractWidgetPtr = std::shared_ptr<AbstractWidget>; // replace with unique_ptr?

    struct AbstractWidget {
        Type type;

        // Informations about the tree structure
        std::vector<AbstractWidgetPtr> childrens;
        AbstractWidgetPtr parent = nullptr;
        int level = 0;

        // For display
        void virtual addCharPtrsToVector(std::vector<WrapCharPtr>& string) {}
        void virtual draw(ImDrawList* draw_list) {}
        ImVec2 dimensions;
        ImVec2 position;

        float scale = 1.f;

        // Internal
        int text_begin = 0;
        int text_end = 0;

        // Events
        void virtual onClick() {}
        void virtual onSelect() {}
        void virtual onDeselect() {}
    };

    struct AbstractBlock : public AbstractWidget {
    };
    struct AbstractSpan : public AbstractWidget {
    };
    struct RootNode : public AbstractWidget {
        RootNode() {
            type = T_ROOT;
        }
    };
}