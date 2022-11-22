#include "helpers.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "internal_helpers.h"

namespace AB {

    /* Roman and alpha enumeration. */

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
    std::string decimal_to_roman(int number, bool lower) {
        if (number > 3999 || number < 1)
            return "";
        int numbers[] = { 1,4,5,9,10,40,50,90,100,400,500,900,1000 };
        static std::string symbols_lower[13] = { "i","iv","v","ix","x","xl","l","xc","c","cd","d","cm","m" };
        static std::string symbols_upper[13] = { "I","IV","V","IX","X","XL","L","XC","C","CD","D","CM","M" };
        std::string* symbols = (lower) ? &symbols_lower[0] : &symbols_upper[0];
        int i = 12;

        std::string out;
        while (number > 0) {
            int div = number / numbers[i];
            number = number % numbers[i];
            while (div--) {
                out += symbols[i];
            }
            i--;
        }
        return out;
    }
    std::string decimal_to_alpha(int number, bool lower) {
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

    int roman_to_decimal(const std::string& str) {
        if (str.empty())
            return 0;

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
    bool validate_roman_enumeration(const std::string& str) {
        if (str.empty())
            return false;

        enum STATE { UNIT, TEN, HUNDRED, THOUSAND };
        static std::unordered_set<std::string> units = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX" };
        static std::unordered_set<std::string> tens = { "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC" };
        static std::unordered_set<std::string> hundreds = { "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM" };
        static std::unordered_set<std::string> thousands = { "M", "MM", "MMM" };

        // Little helper functions
        static auto value_to_state = [](int value) {
            if (value > 500)
                return THOUSAND;
            else if (value >= 100)
                return HUNDRED;
            else if (value >= 10)
                return TEN;
            else return UNIT;
        };
        static auto get_roman_set = [](STATE state) -> std::unordered_set<std::string>*{
            if (state == UNIT)
                return &units;
            else if (state == TEN)
                return &tens;
            else if (state == HUNDRED)
                return &hundreds;
            else
                return &thousands;
        };

        std::string accumulator;
        STATE current_state = value_to_state(roman_value(str[0]));
        bool is_lower = ISLOWER_(str[0]);

        for (int i = 0;i < str.length();i++) {
            char c = str[i];
            int value = roman_value(c);
            // Non-valid roman character
            if (value == 0) {
                return false;
            }
            // We do not mix lower and upper chars
            if (is_lower != ISLOWER_(c)) {
                return false;
            }
            // Constraint all to upper case
            if (c > 90) {
                c -= 32;
            }
            std::string prev_accumulator = accumulator;
            accumulator += c;
            // if (i == str.length() - 1)
            //     prev_accumulator = accumulator;

            auto current_set = get_roman_set(current_state);
            // We have to go from thousands, to hundreds, to tens, to units
            // If we break this rule, the roman numeral is not valid
            STATE next_state = value_to_state(value);
            if (next_state > current_state) {
                if (current_set->find(accumulator) == current_set->end())
                    return false;
            }
            else if (next_state == current_state) {
                auto set = get_roman_set(next_state);
                // Non-valid sequence of characters
                if (set->find(accumulator) == set->end()) {
                    return false;
                }
            }
            else {
                if (prev_accumulator.empty())
                    continue;

                // Non-valid sequence of characters
                if (current_set->find(prev_accumulator) == current_set->end()) {
                    return false;
                }
                current_state = next_state;
                accumulator = accumulator.substr(prev_accumulator.length());
            }
        }
        return true;
    }
    bool validate_alpha_enumeration(const std::string& str, int max_length) {
        if (str.length() > max_length || str.length() == 0)
            return false;

        bool is_lower = ISLOWER_(str[0]);
        for (char c : str) {
            if (!ISALPHA_(c))
                return false;
            if (is_lower != ISLOWER_(c))
                return false;
        }
        return true;
    }
}