#include "helpers.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace AB {
    /* Roman and alpha enumeration. */
    std::string decimal_to_roman(int number) {
        if (number > 3999 || number < 1)
            return "";

        return "";
    }
    std::string decimal_to_str(int number) {
        if (number < 1)
            return "";

        int tmp, power, order;
        power = 1; order = 0; tmp = 26;
        while (tmp < number) {
            order++;
            power = tmp;
            tmp *= 26;
        }
        std::vector<char> res;
        for (int i = order; i >= 0;i--) {
            int div = number / power;
            number = number % power;
            res.push_back(div);
            power /= 26;
        }
        for (int i = res.size() - 1; i > 0;i--) {
            if (res[i] == 0) {
                res[i] = 26;
                for (int j = i - 1; j >= 0; j--) {
                    bool carry_done = res[j] != 0;
                    if (carry_done)
                        res[j] = (res[j] - 1) % 26;
                    else
                        res[j] = 25;
                    if (carry_done)
                        break;
                }
            }
        }
        std::string str;
        for (auto i : res) {
            if (i > 0) {
                str += 64 + i;
            }
        }
        return str;
    }

    /**
     * Returns the integer value of a roman letter.
     *
     * If not valid, returns 0
    */
    int roman_value(const char c) {
        switch (c) {
        case 'I':
        case 'i':
            return 1;
        case 'V':
        case 'v':
            return 5;
        case 'X':
        case 'x':
            return 10;
        case 'L':
        case 'l':
            return 50;
        case 'C':
        case 'c':
            return 100;
        case 'D':
        case 'd':
            return 500;
        case 'M':
        case 'm':
            return 1000;
        };
        return 0;
    }

    int roman_to_decimal(const std::string& str) {
        int res = 0;
        for (int i = 0; i < str.length() - 1; ++i) {
            if (roman_value(str[i]) < roman_value(str[i + 1]))
                res -= roman_value(str[i]);
            else
                res += roman_value(str[i]);
        }
        res += roman_value(str[str.length() - 1]);
        return res;
    }
    int alpha_to_decimal(const std::string& str) {
        int res = 0;
        int multiplier = 1;
        for (int i = str.length() - 1;i >= 0;i--) {
            char letter = str[i];
            // Uppercase letter
            if (letter >= 65 && letter <= 90) {
                res += multiplier * (letter - 64);
            }
            // Lowercase letter
            else if (letter >= 97 && letter <= 122) {
                res += multiplier * (letter - 96);
            }
            else {
                return -1;
            }
            multiplier *= 26;
        }
        return res;
    }
    bool validate_roman_str(const std::string& str) {
        char prev = 0;
        int counter = 0;
        int prev_counter = 0;

        static std::unordered_set<const char*> units = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX" };
        static std::unordered_set<const char*> tens = { "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC" };
        static std::unordered_set<const char*> hundreds = { "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM" };

        enum STATE { UNIT, TEN, HUNDRED, THOUSAND };
        STATE current_state = THOUSAND;
        for (char c : str) {
            int value = roman_value(c);
            if (value == 0) {
                return false;
            }
        }

        return true;
    }
}