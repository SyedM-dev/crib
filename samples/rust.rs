#![allow(dead_code)]

use std::collections::{BTreeMap, HashMap};
use std::fmt;
use std::time::Duration;

//! Examples to exercise the Rust regex injection queries in the highlights.scm.
//! These cover Regex::new, regex::Regex::new, regex::bytes::Regex::new,
//! RegexSet::new, regex::RegexSet::new, RegexSetBuilder::new, and byte variants.
//!
//! Injection patterns in the query file trigger on:
//! - call to (Regex|ByteRegexBuilder)::new with a raw string literal
//! - call to (RegexSet|RegexSetBuilder)::new with an array of raw string literals

use regex::{Regex, RegexSet, RegexSetBuilder};
use regex::bytes::Regex as ByteRegex;
use regex::bytes::RegexSet as ByteRegexSet;
use regex::bytes::RegexSetBuilder as ByteRegexSetBuilder;

fn main() {
    // --- Should inject (Regex::new with raw string) ---
    let _simple = Regex::new(r"^\d{4}-\d{2}-\d{2}$").unwrap();

    // --- Should inject (fully qualified regex::Regex::new with raw string) ---
    let _fq = regex::Regex::new(r"(?m)^\w+\s*=\s*.+$").unwrap();

    // --- Should inject (bytes::Regex::new with raw string) ---
    let _bytes = ByteRegex::new(r"(?-u)\xFF[\x00-\x7F]+").unwrap();

    // --- Should inject (RegexSet::new with array of raw strings) ---
    let _set = RegexSet::new([
        r"^INFO:",
        r"^WARN:",
        r"^ERROR:",
    ]).unwrap();

    // --- Should inject (regex::RegexSet::new fully qualified) ---
    let _set_fq = regex::RegexSet::new([
        r"foo\d+",
        r"bar\d+",
    ]).unwrap();

    // --- Should inject (RegexSetBuilder::new with array of raw strings) ---
    let _set_builder = RegexSetBuilder::new([
        r"\bcat\b",
        r"\bdog\b",
    ])
    .case_insensitive(true)
    .build()
    .unwrap();

    // --- Should inject (bytes set builder) ---
    let _byte_set_builder = ByteRegexSetBuilder::new([
        r"(?-u)\x01\x02",
        r"(?-u)\xFF.+",
    ])
    .build()
    .unwrap();

    // --- Should inject (bytes set) ---
    let _byte_set = ByteRegexSet::new([
        r"(?-u)\x00+\xFF",
        r"(?-u)[\x10-\x20]+",
    ]).unwrap();

    // --- NEGATIVE examples (should NOT inject) ---

    // Not raw string literal (plain string): the query expects raw_string_literal.
    let _no_inject_plain = Regex::new("plain-string-no-raw").unwrap();

    // Function name is not `new`, so should not inject.
    let _builder = Regex::new(r"\d+").map(|re| re.replace("123", "x"));

    // Different type name, should not inject.
    let _other = Some(r"not a regex call");

    // Raw string but different function, should not inject.
    let _format = format!(r"literal: {}", 42);
}

// Keep a simple test to ensure this compiles and runs.
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn smoke() {
        let re = Regex::new(r"^\d+$").unwrap();
        assert!(re.is_match("12345"));
        let set = RegexSet::new([r"cat", r"dog"]).unwrap();
        assert!(set.is_match("hotdog"));
    }
}

/// A simple data type to exercise traits, pattern matching, and methods.
#[derive(Debug, Clone, PartialEq)]
pub struct Point {
    pub x: i64,
    pub y: i64,
}

impl Point {
    pub fn manhattan(&self) -> i64 {
        self.x.abs() + self.y.abs()
    }

    pub fn translate(&self, dx: i64, dy: i64) -> Self {
        Self {
            x: self.x + dx,
            y: self.y + dy,
        }
    }
}

impl fmt::Display for Point {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "({}, {})", self.x, self.y)
    }
}

#[derive(Debug, PartialEq, Eq)]
pub enum ParseError {
    Empty,
    InvalidDigit,
    TooLarge,
}

pub fn parse_u8(input: &str) -> Result<u8, ParseError> {
    if input.trim().is_empty() {
        return Err(ParseError::Empty);
    }
    let mut value: u16 = 0;
    for ch in input.bytes() {
        if !(b'0'..=b'9').contains(&ch) {
            return Err(ParseError::InvalidDigit);
        }
        value = value * 10 + u16::from(ch - b'0');
        if value > u8::MAX as u16 {
            return Err(ParseError::TooLarge);
        }
    }
    Ok(value as u8)
}

pub fn sum_iter<I: IntoIterator<Item = i64>>(iter: I) -> i64 {
    iter.into_iter().fold(0, |acc, n| acc + n)
}

pub fn split_once<'a>(input: &'a str, needle: char) -> Option<(&'a str, &'a str)> {
    let idx = input.find(needle)?;
    Some((&input[..idx], &input[idx + needle.len_utf8()..]))
}

pub fn join_with<I: IntoIterator<Item = String>>(iter: I, sep: &str) -> String {
    let mut it = iter.into_iter().peekable();
    let mut out = String::new();
    while let Some(item) = it.next() {
        out.push_str(&item);
        if it.peek().is_some() {
            out.push_str(sep);
        }
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::cell::RefCell;
    use std::sync::mpsc;
    use std::thread;

    macro_rules! assert_contains {
        ($haystack:expr, $needle:expr) => {
            if !$haystack.contains($needle) {
                panic!("expected {:?} to contain {:?}", $haystack, $needle);
            }
        };
    }

    #[test]
    fn point_manhattan_and_display() {
        let p = Point { x: -3, y: 4 };
        assert_eq!(p.manhattan(), 7);
        assert_eq!(p.to_string(), "(-3, 4)");
    }

    #[test]
    fn point_translate_is_pure() {
        let p = Point { x: 1, y: 2 };
        let q = p.translate(3, -1);
        assert_eq!(p, Point { x: 1, y: 2 });
        assert_eq!(q, Point { x: 4, y: 1 });
    }

    #[test]
    fn parse_u8_success_and_errors() {
        assert_eq!(parse_u8("0"), Ok(0));
        assert_eq!(parse_u8("255"), Ok(255));
        assert_eq!(parse_u8(" 17 "), Ok(17)); // leading/trailing spaces are rejected as Empty? we trimmed only emptiness, digits still parsed.
        assert_eq!(parse_u8(""), Err(ParseError::Empty));
        assert_eq!(parse_u8("  "), Err(ParseError::Empty));
        assert_eq!(parse_u8("12a"), Err(ParseError::InvalidDigit));
        assert_eq!(parse_u8("256"), Err(ParseError::TooLarge));
    }

    #[test]
    fn sum_iter_works_for_various_iterators() {
        let v = vec![1, 2, 3, 4, -5];
        assert_eq!(sum_iter(&v), 5);
        let arr = [10i64; 4];
        assert_eq!(sum_iter(arr), 40);
        assert_eq!(sum_iter(0..5), 10);
    }

    #[test]
    fn split_once_basic_and_unicode() {
        assert_eq!(split_once("a,b,c", ','), Some(("a", "b,c")));
        assert_eq!(split_once("no-sep", '/'), None);
        // UTF-8 needle
        let s = "fooλbar";
        assert_eq!(split_once(s, 'λ'), Some(("foo", "bar")));
    }

    #[test]
    fn join_with_various_lengths() {
        let empty: Vec<String> = vec![];
        assert_eq!(join_with(empty, ", "), "");
        assert_eq!(join_with(vec!["a".into()], ", "), "a");
        assert_eq!(
            join_with(vec!["a".into(), "b".into(), "c".into()], "|"),
            "a|b|c"
        );
    }

    #[test]
    fn hash_map_grouping_example() {
        let words = ["ant", "bat", "apple", "boat"];
        let mut by_initial: HashMap<char, Vec<&str>> = HashMap::new();
        for w in &words {
            let key = w.chars().next().unwrap();
            by_initial.entry(key).or_default().push(*w);
        }
        assert_eq!(by_initial.get(&'a').unwrap(), &vec!["ant", "apple"]);
        assert_eq!(by_initial.get(&'b').unwrap(), &vec!["bat", "boat"]);
    }

    #[test]
    fn btree_map_sorted_iteration() {
        let mut map = BTreeMap::new();
        map.insert("c", 3);
        map.insert("a", 1);
        map.insert("b", 2);
        let keys: Vec<_> = map.keys().copied().collect();
        assert_eq!(keys, vec!["a", "b", "c"]);
    }

    #[test]
    fn channels_and_threads() {
        let (tx, rx) = mpsc::channel();
        thread::spawn(move || {
            for i in 0..5 {
                tx.send(i * i).unwrap();
            }
        });
        let received: Vec<_> = (0..5).map(|_| rx.recv().unwrap()).collect();
        assert_eq!(received, vec![0, 1, 4, 9, 16]);
    }

    #[test]
    fn interior_mutability_with_refcell() {
        #[derive(Debug)]
        struct Counter {
            inner: RefCell<u32>,
        }
        impl Counter {
            fn inc(&self) {
                *self.inner.borrow_mut() += 1;
            }
            fn get(&self) -> u32 {
                *self.inner.borrow()
            }
        }

        let c = Counter {
            inner: RefCell::new(0),
        };
        c.inc();
        c.inc();
        assert_eq!(c.get(), 2);
    }

    #[test]
    fn should_panic_on_too_large_parse() {
        #[should_panic(expected = "TooLarge")]
        fn check() {
            parse_u8("999").unwrap();
        }
        check();
    }

    #[test]
    fn result_based_test() -> Result<(), String> {
        let p = Point { x: 2, y: 3 };
        if p.manhattan() == 5 {
            Ok(())
        } else {
            Err("manhattan distance mismatch".into())
        }
    }

    #[test]
    fn iterator_combinators_cover_common_paths() {
        let data = vec![Some(1), None, Some(3), Some(4)];
        let sum: i32 = data.iter().flatten().sum();
        assert_eq!(sum, 8);

        let doubled: Vec<_> = (1..=5).map(|n| n * 2).filter(|n| n % 4 == 0).collect();
        assert_eq!(doubled, vec![4, 8]);
    }

    #[test]
    fn pattern_matching_with_guards() {
        let numbers = [-2, -1, 0, 1, 2];
        let labels: Vec<_> = numbers
            .iter()
            .map(|n| match n {
                n if *n < 0 => "neg",
                0 => "zero",
                n if *n % 2 == 0 => "even-pos",
                _ => "odd-pos",
            })
            .collect();
        assert_eq!(labels, vec!["neg", "neg", "zero", "odd-pos", "even-pos"]);
    }

    #[test]
    fn custom_macro_assert_contains() {
        assert_contains!("hello world", "world");
    }

    #[test]
    fn ownership_and_borrowing_examples() {
        fn takes_and_gives_back(mut v: Vec<i32>) -> Vec<i32> {
            v.push(42);
            v
        }
        let v = vec![1, 2, 3];
        let v = takes_and_gives_back(v);
        assert_eq!(v, vec![1, 2, 3, 42]);

        let s = String::from("hi");
        let len = length_of_str(&s);
        assert_eq!(len, 2);
    }

    fn length_of_str(s: &str) -> usize {
        s.len()
    }

    #[test]
    fn lifetimes_and_slices() {
        fn first<'a>(xs: &'a [i32]) -> Option<&'a i32> {
            xs.first()
        }
        let data = [10, 20, 30];
        assert_eq!(first(&data), Some(&10));
    }

    #[test]
    fn const_generics_array_sum() {
        fn sum_array<const N: usize>(arr: [i32; N]) -> i32 {
            arr.iter().sum()
        }
        assert_eq!(sum_array::<3>([1, 2, 3]), 6);
        assert_eq!(sum_array([0; 5]), 0);
    }

    #[test]
    fn duration_and_instant_arithmetic() {
        use std::time::Instant;
        let start = Instant::now();
        std::thread::sleep(Duration::from_millis(5));
        let elapsed = start.elapsed();
        assert!(elapsed >= Duration::from_millis(5));
    }

    #[test]
    fn string_builder_patterns() {
        let parts = ["a", "b", "c"];
        let mut s = String::with_capacity(3);
        for p in parts {
            s.push_str(p);
        }
        assert_eq!(s, "abc");
        assert!(s.capacity() >= 3);
    }

    #[test]
    fn equality_and_ordering_on_point() {
        let p1 = Point { x: 1, y: 2 };
        let p2 = Point { x: 1, y: 2 };
        assert_eq!(p1, p2);
        assert!(p1.manhattan() <= p2.manhattan());
    }
}
