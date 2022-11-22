#include <doctest/doctest.h>
#include <string>
#include "ab/ab_interpreter.h"


TEST_SUITE("AB") {
    TEST_CASE("Alpha enumeration") {
        SUBCASE("Alpha <-> Decimal") {
            for (int i = 1;i < 10000;i++) {
                CHECK_MESSAGE(AB::alpha_to_decimal(AB::decimal_to_alpha(i)) == i, "Failed for i=", i);
            }
        }
        SUBCASE("Alpha verification") {
            CHECK(AB::validate_alpha_enumeration("abc", 3));
            CHECK(!AB::validate_alpha_enumeration("aBc", 3));
            CHECK(!AB::validate_alpha_enumeration("aaaz", 3));
            CHECK(!AB::validate_alpha_enumeration("ab.", 3));
        }
    }
    TEST_CASE("Roman enumeration") {
        SUBCASE("Roman <-> Decimal") {
            for (int i = 1;i < 3999;i++) {
                CHECK_MESSAGE(AB::roman_to_decimal(AB::decimal_to_roman(i)) == i, "Failed for i=", i);
            }
        }
        SUBCASE("Roman verification") {
            // First check for all valid roman strings
            for (int i = 1;i < 3999;i++) {
                std::string str = AB::decimal_to_roman(i);
                CHECK_MESSAGE(AB::validate_roman_enumeration(str), "Failed for i=", i, " or ", str);
            }
            // Check for some non-obvious non-valid strings
            CHECK_MESSAGE(!AB::validate_roman_enumeration("XM"), "Should have failed for XM");
            CHECK_MESSAGE(!AB::validate_roman_enumeration("MMMXD"), "Should have failed for MMMXD");
            CHECK_MESSAGE(!AB::validate_roman_enumeration("MMA"), "Should have failed for MMA");
            CHECK_MESSAGE(!AB::validate_roman_enumeration("MMDxi"), "Should have failed for MMDxi");
        }
    }
}
