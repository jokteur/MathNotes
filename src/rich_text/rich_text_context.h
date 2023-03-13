#pragma once
#include "wrapper.h"
#include "ui/drawable.h"
#include <unordered_map>

namespace RichText {
    struct RichTextInfo {
        std::unordered_map<int, WrapCharPtr> line_to_char;
    };
}