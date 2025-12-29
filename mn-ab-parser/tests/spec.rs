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

    fn is_block_child(&self, b_type: BlockType) -> bool {
        matches!(
            b_type,
            BlockType::P | BlockType::H
                | BlockType::Hr
                | BlockType::Code
                | BlockType::Math
                | BlockType::Td
                | BlockType::Th
                | BlockType::Hidden
        )
    }

    fn print_block_html_enter(
        &mut self,
        b_type: BlockType,
        bounds: &Vec<Boundaries>,
        attributes: &Attributes,
        detail: &BlockDetail,
    ) {
        if b_type != BlockType::Doc {
            writeln!(self.html).unwrap();
        }

        write!(self.html, "{}", self.indent()).unwrap();
        write!(self.html, "<{}", block_to_html(b_type)).unwrap();

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
            if let Some(bound) = bounds.first() {
                for i in bound.beg..bound.end {
                    write!(self.html, "{}", self.txt.chars().nth(i).unwrap_or(' ')).unwrap();
                }
            }
        }
    }

    fn print_block_ast(
        &mut self,
        b_type: BlockType,
        bounds: &Vec<Boundaries>,
        _attributes: &Attributes,
        _detail: &BlockDetail,
    ) {
        write!(self.ast, "{}", self.indent()).unwrap();
        write!(self.ast, "{}", block_to_name(b_type)).unwrap();
        for bound in bounds {
            write!(
                self.ast,
                " {{{}: {}, {}, {}, {}}} ",
                bound.line_number, bound.pre, bound.beg, bound.end, bound.post
            )
            .unwrap();
        }
        writeln!(self.ast).unwrap();
    }

    fn print_block_html_close(&mut self, b_type: BlockType) {
        if !self.is_block_child(b_type) {
            writeln!(self.html).unwrap();
            write!(self.html, "{}", self.indent()).unwrap();
            write!(self.html, "</{}>", block_to_html(b_type)).unwrap();
        } else if b_type != BlockType::Hr {
            write!(self.html, "</{}>", block_to_html(b_type)).unwrap();
        }
    }

    fn print_span_html_enter(
        &mut self,
        s_type: SpanType,
        _bounds: &Vec<Boundaries>,
        attributes: &Attributes,
        detail: &SpanDetail,
    ) {
        write!(self.html, "<{}", span_to_html(s_type)).unwrap();

        match detail {
            SpanDetail::Url { href } => {
                write!(self.html, " href=\"{}\"", href).unwrap();
            }
            SpanDetail::Img { src, alt } => {
                write!(self.html, " src=\"{}\" alt=\"{}\"", src, alt).unwrap();
            }
            SpanDetail::Ref { name } => {
                write!(self.html, " name=\"{}\"", name).unwrap();
            }
            SpanDetail::None => {}
        }

        self.print_attrs(attributes);

        if matches!(s_type, SpanType::Img | SpanType::Ref) {
            write!(self.html, "/>").unwrap();
        } else {
            write!(self.html, ">").unwrap();
        }
    }

    fn print_span_ast(
        &mut self,
        s_type: SpanType,
        bounds: &Vec<Boundaries>,
        _attributes: &Attributes,
        _detail: &SpanDetail,
    ) {
        write!(self.ast, "{}", self.indent()).unwrap();
        write!(self.ast, "{}", span_to_name(s_type)).unwrap();
        for bound in bounds {
            write!(
                self.ast,
                " {{{}: {}, {}, {}, {}}} ",
                bound.line_number, bound.pre, bound.beg, bound.end, bound.post
            )
            .unwrap();
        }
        writeln!(self.ast).unwrap();
    }

    fn print_span_html_close(&mut self, s_type: SpanType) {
        if !matches!(s_type, SpanType::Img | SpanType::Ref) {
            write!(self.html, "</{}>", span_to_html(s_type)).unwrap();
        }
    }

    fn print_text_ast(&mut self, t_type: TextType, bounds: &Vec<Boundaries>) {
        write!(self.ast, "{}", self.indent()).unwrap();
        let name = text_to_name(t_type);
        write!(self.ast, "{}", name).unwrap();
        for bound in bounds {
            write!(
                self.ast,
                " {{{}: {}, {}, {}, {}}} ",
                bound.line_number, bound.pre, bound.beg, bound.end, bound.post
            )
            .unwrap();
        }
        writeln!(self.ast).unwrap();
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
        self.print_block_html_enter(b_type, bounds, attributes, detail);
        self.print_block_ast(b_type, bounds, attributes, detail);
        self.level += 1;
        Ok(())
    }

    fn leave_block(&mut self, b_type: BlockType) -> Result<(), String> {
        self.level -= 1;
        self.print_block_html_close(b_type);
        Ok(())
    }

    fn text(&mut self, t_type: TextType, bounds: &Vec<Boundaries>) -> Result<(), String> {
        self.print_text_ast(t_type, bounds);
        
        for (j, bound) in bounds.iter().enumerate() {
            if t_type == TextType::Math {
                if j > 0 {
                    write!(self.html, " ").unwrap();
                }
            } else if t_type == TextType::Code {
                if j > 0 {
                    writeln!(self.html).unwrap();
                }
            } else if j > 0 {
                write!(self.html, "<br />").unwrap();
            }
            
            for i in bound.beg..bound.end {
                write!(self.html, "{}", self.txt.chars().nth(i).unwrap_or(' ')).unwrap();
            }
        }
        Ok(())
    }

    fn enter_span(
        &mut self,
        s_type: SpanType,
        bounds: &Vec<Boundaries>,
        attributes: &Attributes,
        detail: &SpanDetail,
    ) -> Result<(), String> {
        self.print_span_html_enter(s_type, bounds, attributes, detail);
        self.print_span_ast(s_type, bounds, attributes, detail);
        self.level += 1;
        Ok(())
    }

    fn leave_span(&mut self, s_type: SpanType) -> Result<(), String> {
        self.level -= 1;
        self.print_span_html_close(s_type);
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
        assert!(false, "{} tests failed.", failures.len());
    }
}
