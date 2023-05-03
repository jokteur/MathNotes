#pragma once
#include "fonts/wrapper.h"
#include "ui/drawable.h"
#include <unordered_map>

namespace RichText {

    struct RichTextInfo {
        std::unordered_map<int, WrapCharPtr> line_to_char;

        WrapCharPtr getFirstCharFromLine(int line_number) {
            auto it = line_to_char.find(line_number);
            if (it != line_to_char.end())
                return it->second;
            else
                return nullptr;
        }
    };

}