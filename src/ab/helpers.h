#pragma once

#include <string>

namespace AB {
    std::string decimal_to_roman(int number, bool lower = false);
    std::string decimal_to_alpha(int number, bool lower = true);

    int roman_to_decimal(const std::string& str);
    int alpha_to_decimal(const std::string& str);

    bool validate_roman_enumeration(const std::string& str);
    bool validate_alpha_enumeration(const std::string& str, int max_length = 3);
}