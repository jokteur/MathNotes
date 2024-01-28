use crate::editor::{ImageGlyph, TextEditor};
use crate::latex::{self, LatexDefaultColor, LatexImage};
use egui_extras::{Size, StripBuilder};
use std::env;
use std::fs;

#[derive(serde::Deserialize, serde::Serialize)]
#[serde(default)] // if we add new fields, give them default values when deserializing old state
pub struct App {
    // Example stuff:
    editor: TextEditor,
    dark_mode: bool,

    #[serde(skip)]
    image: LatexImage<'static>,
    #[serde(skip)]
    error_on_load: Option<String>,
    #[serde(skip)]
    first_frame: bool,
}

impl Default for App {
    fn default() -> Self {
        let font_file_path = "data/fonts/XITS_Math.otf";
        let font_file = std::fs::read(font_file_path).unwrap();
        let font = latex::load_font(&font_file);
        let image = latex::LatexImage::new(
            "\\sigma = \\sqrt{ \\color{orange}\\frac{1}{N} \\sum_{i=1}^N (x_i - \\mu)^2 }",
            font,
            16.0,
            2.0,
            LatexDefaultColor::White,
        );

        Self {
            editor: TextEditor::default(),
            dark_mode: false,
            image: image,
            error_on_load: prepare_folders(),
            first_frame: false,
        }
    }
}

impl App {
    /// Called once before the first frame.
    pub fn new(cc: &eframe::CreationContext<'_>) -> Self {
        // Load previous app state (if any)
        if let Some(storage) = cc.storage {
            return eframe::get_value(storage, eframe::APP_KEY).unwrap_or_default();
        }
        Default::default()
    }

    /// Called once on the first frame.
    pub fn first_frame(&mut self, ctx: &egui::Context) {
        if self.first_frame {
            return;
        }
        // Set light / dark mode
        if self.dark_mode != ctx.style().visuals.dark_mode {
            if self.dark_mode {
                ctx.set_visuals(egui::Visuals::dark());
            } else {
                ctx.set_visuals(egui::Visuals::light());
            }
        }
        self.first_frame = true;
    }
}

fn prepare_folders() -> Option<String> {
    // Not needed for now
    // let _ = fs::remove_dir_all("tmp_mn");
    // let res = fs::create_dir("tmp_mn");
    // // let _ = env::set_current_dir("tmp_mn");
    // match res {
    //     Ok(_) => None,
    //     Err(e) => Some(format!("Error creating tmp folder: {}", e.to_string())),
    // }
    None
}

impl eframe::App for App {
    /// Called by the frame work to save state before shutdown.
    fn save(&mut self, storage: &mut dyn eframe::Storage) {
        eframe::set_value(storage, eframe::APP_KEY, self);
    }

    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        egui_extras::install_image_loaders(ctx);

        self.first_frame(ctx);

        if self.error_on_load.is_some() {
            egui::CentralPanel::default().show(ctx, |ui: &mut egui::Ui| {
                ui.label(self.error_on_load.as_ref().unwrap());
                if ui.button("Quit").clicked() {
                    ctx.send_viewport_cmd(egui::ViewportCommand::Close);
                }
            });
        } else {
            egui::TopBottomPanel::top("top_panel").show(ctx, |ui| {
                egui::menu::bar(ui, |ui| {
                    ui.menu_button("File", |ui| {
                        if ui.button("Quit").clicked() {
                            ctx.send_viewport_cmd(egui::ViewportCommand::Close);
                        }
                    });
                    ui.add_space(16.0);

                    egui::widgets::global_dark_light_mode_switch(ui);
                });
            });

            egui::CentralPanel::default().show(ctx, |ui: &mut egui::Ui| {
                let pos = egui::Pos2::new(0.0, 30.0);
                let tint = if ctx.style().visuals.dark_mode {
                    egui::Color32::from_rgb(240, 240, 240)
                } else {
                    egui::Color32::from_rgb(50, 50, 50)
                };
                self.image.draw_at(ui, ctx, pos, tint);
            });
        }
    }
}
