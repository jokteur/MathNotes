use mn_ab_parser::definitions::*;
use std::fmt::Write;
use std::collections::BTreeMap; 
use mn_ab_parser::Parser;

struct TestParser {
    level: usize,
    html: String,
    ast: String,
    txt: String, // Copy of the input text to extract content
}

impl TestParser {
    fn new(txt: &str) -> Self {
        Self {
            level: 0,
            html: String::new(),
            ast: String::new(),
            txt: txt.to_string(),
        }
    }

    fn indent(&self) -> String {
        "  ".repeat(self.level)
    }

    fn format_bounds(&self, bounds: &Vec<Boundaries>) -> String {
        let mut out = String::new();
        for b in bounds {
            // Format: {line: pre, beg, end, post}
            // Note: C++ uses " {0: 0, 0, 8, 8} " with trailing space
            write!(out, " {{{}: {}, {}, {}, {}}} ", b.line_number, b.pre, b.beg, b.end, b.post).unwrap();
        }
        out
    }

    fn print_attrs(&mut self, attributes: &Attributes) {
        // C++ uses std::map which sorts by key. Rust HashMap is arbitrary.
        // We must sort keys to match output.
        let sorted: BTreeMap<_, _> = attributes.iter().collect();
        for (k, v) in sorted {
            write!(self.html, " {}", k).unwrap();
            if !v.is_empty() {
                write!(self.html, "=\"{}\"", v).unwrap();
            }
        }
    }
}

impl Parser for TestParser {
    fn enter_block(&mut self, b_type: BlockType, bounds: &Vec<Boundaries>, attributes: &Attributes, detail: &BlockDetail) -> bool {
        // --- AST GENERATION ---
        write!(self.ast, "{}{}", self.indent(), block_to_name(b_type)).unwrap();
        write!(self.ast, "{}", self.format_bounds(bounds)).unwrap();
        writeln!(self.ast).unwrap();

        // --- HTML GENERATION ---
        if b_type != BlockType::Doc {
            writeln!(self.html).unwrap();
        }
        write!(self.html, "{}<{}", self.indent(), block_to_html(b_type)).unwrap();

        // Handle Details (Classes, Langs)
        match detail {
            BlockDetail::Div { name } => write!(self.html, " class=\"{}\"", name).unwrap(),
            BlockDetail::Code { lang, .. } => write!(self.html, " lang=\"{}\"", lang).unwrap(),
            _ => {}
        }

        self.print_attrs(attributes);

        if b_type == BlockType::Hr {
            write!(self.html, "/>").unwrap();
        } else {
            write!(self.html, ">").unwrap();
        }
        
        // Handle HIDDEN block content immediately (logic from t_testcases.cpp)
        if b_type == BlockType::Hidden {
            if let Some(first) = bounds.first() {
                let s = &self.txt[first.beg..first.end];
                write!(self.html, "{}", s).unwrap();
            }
        }

        self.level += 1;
        true
    }

    fn leave_block(&mut self, b_type: BlockType) -> bool {
        self.level -= 1;
        // logic from is_block_child in t_testcases.h
        let is_child = matches!(b_type, BlockType::Li | BlockType::Ul | BlockType::Ol | BlockType::Quote | BlockType::Def | BlockType::Div);
        
        if !is_child {
            writeln!(self.html).unwrap();
            write!(self.html, "{}", self.indent()).unwrap();
            writeln!(self.html, "</{}>", block_to_html(b_type)).unwrap();
        } else if b_type != BlockType::Hr {
             // For child blocks (li, quote), close on same line or simplified
            write!(self.html, "</{}>", block_to_html(b_type)).unwrap();
        }
        true
    }

    fn text(&mut self, t_type: TextType, bounds: &Vec<Boundaries>) -> bool {
        // --- AST ---
        let name = match t_type {
            TextType::Code => "TEXT_CODE",
            TextType::Math => "TEXT_LATEX",
            TextType::Normal => "TEXT;", // C++ output has a semicolon for normal text
        };
        write!(self.ast, "{}{}", self.indent(), name).unwrap();
        write!(self.ast, "{}", self.format_bounds(bounds)).unwrap();
        writeln!(self.ast).unwrap();

        // --- HTML ---
        for (i, bound) in bounds.iter().enumerate() {
            if i > 0 {
                match t_type {
                    TextType::Math => write!(self.html, " ").unwrap(), // Space for Latex
                    TextType::Code => writeln!(self.html).unwrap(),     // Newline for Code
                    _ => write!(self.html, "<br />").unwrap(),          // Break for Normal
                }
            }
            let s = &self.txt[bound.beg..bound.end];
            write!(self.html, "{}", s).unwrap();
        }
        true
    }

    // You would implement enter_span / leave_span similarly mapping to `span_to_name`
    fn enter_span(&mut self, _s: SpanType, _b: &Vec<Boundaries>, _a: &Attributes, _d: &SpanDetail) -> bool { 
        // Implement based on t_testcases.cpp:print_span_html_enter
        self.level += 1;
        true 
    }
    
    fn leave_span(&mut self, _s: SpanType) -> bool { 
        self.level -= 1;
        true 
    }
}

use std::fs;
use std::path::Path;

#[test]
fn test_specs() {
    let test_dir = Path::new("tests/files");
    // Create tmp directory
    
    let tmp_dir = Path::new("tmp");
    if !tmp_dir.exists() {
        fs::create_dir(tmp_dir).expect("Failed to create tmp directory");
    }
    let out_dir = Path::new("tmp/ast_out");

    // 1. Create/Clean the output directory (like C++: fs::remove_all("ast_out"); fs::create_directory("ast_out");)
    if out_dir.exists() {
        fs::remove_dir_all(out_dir).expect("Failed to clean ast_out");
    }
    fs::create_dir(out_dir).expect("Failed to create ast_out");

    let mut failures = Vec::new();

    let entries = fs::read_dir(test_dir).expect("tests/files directory not found");

    // Collect and sort entries to ensure deterministic order (C++ uses std::set)
    let mut paths: Vec<_> = entries
        .map(|e| e.unwrap().path())
        .filter(|p| p.extension().and_then(|s| s.to_str()) == Some("ab"))
        .collect();
    paths.sort();

    for path in paths {
        let file_stem = path.file_stem().unwrap().to_str().unwrap().to_string();

        // 2. Read Inputs
        let input = fs::read_to_string(&path).unwrap();
        
        let ast_path = path.with_extension("ast");
        let html_path = path.with_extension("html");

        let expected_ast = fs::read_to_string(&ast_path).unwrap_or_default();
        let expected_html = fs::read_to_string(&html_path).unwrap_or_default();

        // 3. Parse
        let mut parser_check = TestParser::new(&input);
        mn_ab_parser::parser(&input, 0, &mut parser_check);

        // 4. Write output files to `ast_out/`
        let out_ast_path = out_dir.join(format!("{}.ast", file_stem));
        let out_html_path = out_dir.join(format!("{}.html", file_stem));

        fs::write(&out_ast_path, &parser_check.ast).expect("Failed to write AST output");
        fs::write(&out_html_path, &parser_check.html).expect("Failed to write HTML output");

        // 5. Check Logic (Replicating exclusions from C++)
        if file_stem == "_testbench" || file_stem == "long_doc" {
            continue;
        }

        // Compare (Normalize newlines to handle Windows/Unix differences)
        let output_ast = parser_check.ast.replace("\r\n", "\n");
        let expected_ast_norm = expected_ast.replace("\r\n", "\n");
        let ast_failed = !expected_ast.is_empty() && output_ast != expected_ast_norm;

        let output_html = parser_check.html.replace("\r\n", "\n");
        let expected_html_norm = expected_html.replace("\r\n", "\n");
        let html_failed = !expected_html.is_empty() && output_html != expected_html_norm;

        // 6. Report Error (Matching C++ Message Format)
        if ast_failed && html_failed {
            let msg = format!("Failed both AST and HTML for test case '{}' ; see outputs in tmp/ast_out", file_stem);
            println!("{}", msg);
            failures.push(msg);
        } else if ast_failed {
            let msg = format!("Failed AST for test case '{}' ; see outputs in tmp/ast_out", file_stem);
            println!("{}", msg);
            failures.push(msg);
        } else if html_failed {
            let msg = format!("Failed HTML for test case '{}' ; see outputs in tmp/ast_out", file_stem);
            println!("{}", msg);
            failures.push(msg);
        }
    }

    // Panic at the end if any tests failed
    if !failures.is_empty() {
        panic!("{} tests failed.", failures.len());
    }
}