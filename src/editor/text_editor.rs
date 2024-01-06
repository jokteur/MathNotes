#[cfg_attr(feature = "serde", derive(serde::Deserialize, serde::Serialize))]
#[cfg_attr(feature = "serde", serde(default))]
#[derive(serde::Deserialize, serde::Serialize)]
pub struct TextEditor {
    text: String,
    // #[cfg_attr(feature = "serde", serde(skip))]
    // highlighter: crate::easy_mark::MemoizedEasymarkHighlighter,
}

impl TextEditor {
    pub fn ui(&mut self, ui: &mut egui::Ui) {
        ui.add(egui::TextEdit::multiline(&mut self.text).desired_rows(20));
    }
}

impl Default for TextEditor {
    fn default() -> Self {
        Self {
            text: "test hello world".trim().to_owned(),
        }
    }
}
