use egui::{ScrollArea, TextBuffer, TextFormat, Vec2};

use crate::editor::{ImageGlyph, TextEditor};
use crate::latex::{self, LatexImage};
use egui_extras::{Size, StripBuilder};
use std::sync::Arc;

#[derive(serde::Deserialize, serde::Serialize)]
#[serde(default)] // if we add new fields, give them default values when deserializing old state
pub struct App {
    // Example stuff:
    label: String,
    editor: TextEditor,

    #[serde(skip)]
    value: f32,
    #[serde(skip)]
    image: LatexImage<'static>,
}

impl Default for App {
    fn default() -> Self {
        let font_file_path = "data/fonts/XITS_Math.otf";
        let font_file = std::fs::read(font_file_path).unwrap();
        let font = latex::load_font(&font_file);
        let image = latex::render_image("\\int_a^b x^2 dx", font, 16.0, 1.5);
        Self {
            // Example stuff:
            label: "Hello World!".to_owned(),
            value: 2.7,
            editor: TextEditor::default(),
            image: image,
        }
    }
}

impl App {
    /// Called once before the first frame.
    pub fn new(cc: &eframe::CreationContext<'_>) -> Self {
        // This is also where you can customize the look and feel of egui using
        // `cc.egui_ctx.set_visuals` and `cc.egui_ctx.set_fonts`.

        // Load previous app state (if any).
        // Note that you must enable the `persistence` feature for this to work.
        if let Some(storage) = cc.storage {
            return eframe::get_value(storage, eframe::APP_KEY).unwrap_or_default();
        }

        Default::default()
    }
}

impl eframe::App for App {
    /// Called by the frame work to save state before shutdown.
    fn save(&mut self, storage: &mut dyn eframe::Storage) {
        eframe::set_value(storage, eframe::APP_KEY, self);
    }

    /// Called each time the UI needs repainting, which may be many times per second.
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        // Put your widgets into a `SidePanel`, `TopBottomPanel`, `CentralPanel`, `Window` or `Area`.
        // For inspiration and more examples, go to https://emilk.github.io/egui

        egui_extras::install_image_loaders(ctx);

        egui::TopBottomPanel::top("top_panel").show(ctx, |ui| {
            // The top panel is often a good place for a menu bar:

            egui::menu::bar(ui, |ui| {
                ui.menu_button("File", |ui| {
                    if ui.button("Quit").clicked() {
                        ctx.send_viewport_cmd(egui::ViewportCommand::Close);
                    }
                });
                ui.add_space(16.0);

                egui::widgets::global_dark_light_mode_buttons(ui);
            });
        });

        let text = TO_BE_OR_NOT_TO_BE.as_str();

        egui::CentralPanel::default().show(ctx, |ui: &mut egui::Ui| {
            ui.heading("Text editor");

            // ui.image("file://out.svg");

            let pixels_per_point = ui.ctx().pixels_per_point();
            let points_per_pixel = 1.0 / pixels_per_point;

            // StripBuilder::new(ui)
            //     .size(Size::remainder())
            //     .vertical(|mut strip| {
            //         strip.cell(|ui| {
            ui.add(
                self.image
                    .image
                    .clone()
                    .max_size(Vec2::new(self.image.width, self.image.height)),
            );
            // });
            // });
            // ScrollArea::vertical().show(ui, |ui| {
            //     self.editor.ui(ui);
            // });
        });
    }
}

/// Excerpt from Dolores Ibárruri's farwel speech to the International Brigades:
const TO_BE_OR_NOT_TO_BE: &str = "Mothers! Women!\n
When the years pass by and the wounds of war are stanched; when the memory of the sad and bloody days dissipates in a present of liberty, of peace and of wellbeing; when the rancor have died out and pride in a free country is felt equally by all Spaniards, speak to your children. Tell them of these men of the International Brigades.\n\
\n\
Recount for them how, coming over seas and mountains, crossing frontiers bristling with bayonets, sought by raving dogs thirsting to tear their flesh, these men reached our country as crusaders for freedom, to fight and die for Spain’s liberty and independence threatened by German and Italian fascism. \
They gave up everything — their loves, their countries, home and fortune, fathers, mothers, wives, brothers, sisters and children — and they came and said to us: “We are here. Your cause, Spain’s cause, is ours. It is the cause of all advanced and progressive mankind.”\n\
\n\
- Dolores Ibárruri, 1938";
