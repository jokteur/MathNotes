use mn_ab_parser::definitions::*;
use mn_ab_parser::Parser;
use std::collections::BTreeMap;
use std::fmt::Write;

struct TestParser {
    level: usize,
    html: String,
    ast: String,
    txt: String,
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
            write!(
                out,
                " {{{}: {}, {}, {}, {}}} ",
                b.line_number, b.pre, b.beg, b.end, b.post
            )
            .unwrap();
        }
        out
    }

    fn print_attrs(&mut self, attributes: &Attributes) {
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
    fn enter_block(
        &mut self,
        b_type: BlockType,
        bounds: &Vec<Boundaries>,
        attributes: &Attributes,
        detail: &BlockDetail,
    ) -> Result<(), String> {
        // --- AST GENERATION ---
        write!(self.ast, "{}{}", self.indent(), block_to_name(b_type)).unwrap();
        write!(self.ast, "{}", self.format_bounds(bounds)).unwrap();
        writeln!(self.ast).unwrap();

        if b_type != BlockType::Doc {
            writeln!(self.html).unwrap();
        }
        write!(self.html, "{}<{}", self.indent(), block_to_html(b_type)).unwrap();

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

        if b_type == BlockType::Hidden {
            if let Some(first) = bounds.first() {
                let s = &self.txt[first.beg..first.end];
                write!(self.html, "{}", s).unwrap();
            }
        }

        self.level += 1;
        Ok(())
    }

    fn leave_block(&mut self, b_type: BlockType) -> Result<(), String> {
        self.level -= 1;
        let is_child = matches!(
            b_type,
            BlockType::Li
                | BlockType::Ul
                | BlockType::Ol
                | BlockType::Quote
                | BlockType::Def
                | BlockType::Div
        );

        if !is_child {
            writeln!(self.html).unwrap();
            write!(self.html, "{}", self.indent()).unwrap();
            writeln!(self.html, "</{}>", block_to_html(b_type)).unwrap();
        } else if b_type != BlockType::Hr {
            write!(self.html, "</{}>", block_to_html(b_type)).unwrap();
        }
        Ok(())
    }

    fn text(&mut self, t_type: TextType, bounds: &Vec<Boundaries>) -> Result<(), String> {
        // --- AST ---
        let name = match t_type {
            TextType::Code => "TEXT_CODE",
            TextType::Math => "TEXT_LATEX",
            TextType::Normal => "TEXT;",
        };
        write!(self.ast, "{}{}", self.indent(), name).unwrap();
        write!(self.ast, "{}", self.format_bounds(bounds)).unwrap();
        writeln!(self.ast).unwrap();

        // --- HTML ---
        for (i, bound) in bounds.iter().enumerate() {
            if i > 0 {
                match t_type {
                    TextType::Math => write!(self.html, " ").unwrap(),
                    TextType::Code => writeln!(self.html).unwrap(),
                    _ => write!(self.html, "<br />").unwrap(),
                }
            }
            let s = &self.txt[bound.beg..bound.end];
            write!(self.html, "{}", s).unwrap();
        }
        Ok(())
    }

    fn enter_span(
        &mut self,
        _s: SpanType,
        _b: &Vec<Boundaries>,
        _a: &Attributes,
        _d: &SpanDetail,
    ) -> Result<(), String> {
        self.level += 1;
        Ok(())
    }

    fn leave_span(&mut self, _s: SpanType) -> Result<(), String> {
        self.level -= 1;
        Ok(())
    }
}

use std::fs;
use std::path::Path;

#[test]
fn test_specs() {
    let test_dir = Path::new("tests/files");

    let tmp_dir = Path::new("tmp");
    if !tmp_dir.exists() {
        fs::create_dir(tmp_dir).expect("Failed to create tmp directory");
    }
    let out_dir = Path::new("tmp/ast_out");

    if out_dir.exists() {
        fs::remove_dir_all(out_dir).expect("Failed to clean ast_out");
    }
    fs::create_dir(out_dir).expect("Failed to create ast_out");

    let mut failures = Vec::new();

    let entries = fs::read_dir(test_dir).expect("tests/files directory not found");

    // Collect and sort entries to ensure deterministic order
    let mut paths: Vec<_> = entries
        .map(|e| e.unwrap().path())
        .filter(|p| p.extension().and_then(|s| s.to_str()) == Some("ab"))
        .collect();
    paths.sort();

    for path in paths {
        let file_stem = path.file_stem().unwrap().to_str().unwrap().to_string();

        let input = fs::read_to_string(&path).unwrap();

        let ast_path = path.with_extension("ast");
        let html_path = path.with_extension("html");

        let expected_ast = fs::read_to_string(&ast_path).unwrap_or_default();
        let expected_html = fs::read_to_string(&html_path).unwrap_or_default();

        let mut parser_check = TestParser::new(&input);
        mn_ab_parser::parser(&input, &mut parser_check);

        let out_ast_path = out_dir.join(format!("{}.ast", file_stem));
        let out_html_path = out_dir.join(format!("{}.html", file_stem));

        fs::write(&out_ast_path, &parser_check.ast).expect("Failed to write AST output");
        fs::write(&out_html_path, &parser_check.html).expect("Failed to write HTML output");

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

        if ast_failed && html_failed {
            let msg = format!(
                "Failed both AST and HTML for test case '{}' ; see outputs in tmp/ast_out",
                file_stem
            );
            println!("{}", msg);
            failures.push(msg);
        } else if ast_failed {
            let msg = format!(
                "Failed AST for test case '{}' ; see outputs in tmp/ast_out",
                file_stem
            );
            println!("{}", msg);
            failures.push(msg);
        } else if html_failed {
            let msg = format!(
                "Failed HTML for test case '{}' ; see outputs in tmp/ast_out",
                file_stem
            );
            println!("{}", msg);
            failures.push(msg);
        }
    }

    if !failures.is_empty() {
        panic!("{} tests failed.", failures.len());
    }
}
