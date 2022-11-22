#pragma once

#include <string>

namespace AB {
    std::string decimal_to_roman(int number);
    std::string decimal_to_alpha(int number);

    int roman_to_decimal(const std::string& str);
    int alpha_to_decimal(const std::string& str);

    bool validate_roman_str(const std::string& str);
}