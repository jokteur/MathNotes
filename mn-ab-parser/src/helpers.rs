use std::collections::HashSet;
use std::sync::OnceLock;

/// Returns the integer value of a roman letter.
/// Returns 0 if the character is not a valid roman numeral.
/// Does not work for values larger than 1000.
pub fn roman_value(c: char) -> i32 {
    match c {
        'I' | 'i' => 1,
        'V' | 'v' => 5,
        'X' | 'x' => 10,
        'L' | 'l' => 50,
        'C' | 'c' => 100,
        'D' | 'd' => 500,
        'M' | 'm' => 1000,
        _ => 0,
    }
}

/// Converts a decimal number to its roman numeral representation.
/// Returns an empty string for numbers outside the range 1-3999.
pub fn decimal_to_roman(mut number: i32, lower: bool) -> String {
    if number > 3999 || number < 1 {
        return String::new();
    }

    let mapping = [
        (1000, "M"),
        (900, "CM"),
        (500, "D"),
        (400, "CD"),
        (100, "C"),
        (90, "XC"),
        (50, "L"),
        (40, "XL"),
        (10, "X"),
        (9, "IX"),
        (5, "V"),
        (4, "IV"),
        (1, "I"),
    ];

    let mut out = String::new();
    for (val, sym) in mapping {
        while number >= val {
            out.push_str(sym);
            number -= val;
        }
    }

    if lower {
        out.to_lowercase()
    } else {
        out
    }
}

/// Converts a decimal number to its alphabetic representation (A, B, ..., Z, AA, AB, ...).
/// Returns an empty string for numbers less than 1.
pub fn decimal_to_alpha(mut number: i32, lower: bool) -> String {
    if number < 1 {
        return String::new();
    }

    let mut res = Vec::new();
    while number > 0 {
        number -= 1; // Adjust for 1-based indexing (A=1, B=2)
        res.push(((number % 26) as u8 + if lower { b'a' } else { b'A' }) as char);
        number /= 26;
    }
    res.into_iter().rev().collect()
}

/// Converts a roman numeral string to its decimal integer value.
/// Returns 0 for invalid roman numeral strings.
pub fn roman_to_decimal(s: &str) -> i32 {
    if s.is_empty() {
        return 0;
    }

    let chars: Vec<char> = s.chars().collect();
    let mut res = 0;

    for i in 0..chars.len() {
        let val = roman_value(chars[i]);
        if val == 0 {
            return 0;
        }

        if i + 1 < chars.len() && val < roman_value(chars[i + 1]) {
            res -= val;
        } else {
            res += val;
        }
    }
    res
}

/// Converts an alphabetic string (A, B, ..., Z, AA, AB, ...) to its decimal integer value.
pub fn alpha_to_decimal(s: &str) -> i32 {
    let mut res = 0;
    for c in s.chars() {
        let val = if c.is_ascii_uppercase() {
            c as i32 - 64
        } else if c.is_ascii_lowercase() {
            c as i32 - 96
        } else {
            return -1;
        };
        res = res * 26 + val;
    }
    res
}

#[derive(PartialEq, PartialOrd, Clone, Copy)]
enum State {
    Unit,
    Ten,
    Hundred,
    Thousand,
}

pub fn validate_roman_enumeration(s: &str) -> bool {
    if s.is_empty() {
        return false;
    }
    // Static sets for validation
    static UNITS: OnceLock<HashSet<&'static str>> = OnceLock::new();
    static TENS: OnceLock<HashSet<&'static str>> = OnceLock::new();
    static HUNDREDS: OnceLock<HashSet<&'static str>> = OnceLock::new();
    static THOUSANDS: OnceLock<HashSet<&'static str>> = OnceLock::new();

    let units =
        UNITS.get_or_init(|| ["I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX"].into());
    let tens = TENS.get_or_init(|| ["X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC"].into());
    let hundreds =
        HUNDREDS.get_or_init(|| ["C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM"].into());
    let thousands = THOUSANDS.get_or_init(|| ["M", "MM", "MMM"].into());

    let value_to_state = |val| match val {
        v if v >= 1000 => State::Thousand,
        v if v >= 100 => State::Hundred,
        v if v >= 10 => State::Ten,
        _ => State::Unit,
    };

    let get_set = |state| match state {
        State::Unit => units,
        State::Ten => tens,
        State::Hundred => hundreds,
        State::Thousand => thousands,
    };

    let chars: Vec<char> = s.chars().collect();
    let is_lower = chars[0].is_lowercase();
    let mut current_state = value_to_state(roman_value(chars[0]));
    let mut accumulator = String::new();

    for i in 0..chars.len() {
        let c = chars[i];
        let val = roman_value(c);
        // Non-valid roman character, we do not mix lower and upper chars
        if val == 0 || c.is_lowercase() != is_lower {
            return false;
        }

        let upper_c = c.to_ascii_uppercase();
        let prev_accumulator = accumulator.clone();
        accumulator.push(upper_c);

        // We have to go from thousands, to hundreds, to tens, to units
        // If we break this rule, the roman numeral is not valid
        let next_state = value_to_state(val);
        if next_state > current_state {
            if !get_set(current_state).contains(accumulator.as_str()) {
                return false;
            }
        } else if next_state == current_state {
            if !get_set(next_state).contains(accumulator.as_str()) {
                return false;
            }
        } else {
            // Non-valid sequence of characters
            if !prev_accumulator.is_empty()
                && !get_set(current_state).contains(prev_accumulator.as_str())
            {
                return false;
            }
            current_state = next_state;
            accumulator = upper_c.to_string();
        }
    }
    get_set(current_state).contains(accumulator.as_str())
}

pub fn validate_alpha_enumeration(s: &str, max_length: usize) -> bool {
    if s.is_empty() || s.len() > max_length {
        return false;
    }

    let is_lower = s.chars().next().unwrap().is_lowercase();
    s.chars()
        .all(|c| c.is_ascii_alphabetic() && c.is_lowercase() == is_lower)
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_alpha_enumeration() {
        // Alpha <-> Decimal roundtrip
        for i in 1..10000 {
            let alpha = decimal_to_alpha(i, false);
            assert_eq!(
                alpha_to_decimal(&alpha), 
                i, 
                "Failed for i={} (generated string: {})", i, alpha
            );
        }

        // Alpha verification
        assert!(validate_alpha_enumeration("abc", 3));
        assert!(!validate_alpha_enumeration("aBc", 3), "Mixed case should fail");
        assert!(!validate_alpha_enumeration("aaaz", 3), "Length exceeding max should fail");
        assert!(!validate_alpha_enumeration("ab.", 3), "Non-alpha characters should fail");
        assert!(!validate_alpha_enumeration("12", 3), "Numbers should fail");
    }

    #[test]
    fn test_roman_enumeration() {
        // Roman <-> Decimal roundtrip
        for i in 1..3999 {
            let roman = decimal_to_roman(i, false);
            assert_eq!(
                roman_to_decimal(&roman), 
                i, 
                "Failed for i={} (generated string: {})", i, roman
            );
        }

        // Roman verification (Valid strings)
        for i in 1..3999 {
            let str_upper = decimal_to_roman(i, false);
            let str_lower = decimal_to_roman(i, true);
            
            assert!(
                validate_roman_enumeration(&str_upper), 
                "Failed valid check for i={} (Uppercase: {})", i, str_upper
            );
            assert!(
                validate_roman_enumeration(&str_lower), 
                "Failed valid check for i={} (Lowercase: {})", i, str_lower
            );
        }

        // Roman verification (Invalid strings)
        assert!(!validate_roman_enumeration("XM"), "Should have failed for XM (invalid subtraction)");
        assert!(!validate_roman_enumeration("MMMXD"), "Should have failed for MMMXD");
        assert!(!validate_roman_enumeration("MMA"), "Should have failed for MMA (invalid char)");
        assert!(!validate_roman_enumeration("MMDxi"), "Should have failed for MMDxi (mixed case)");
        assert!(!validate_roman_enumeration("ABC"), "Should have failed for ABC");
        assert!(!validate_roman_enumeration("IIII"), "Should have failed for IIII (standard max 3)");
    }
}