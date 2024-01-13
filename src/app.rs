use egui::{ScrollArea, TextBuffer, TextFormat};

use crate::editor::TextEditor;

#[derive(serde::Deserialize, serde::Serialize)]
#[serde(default)] // if we add new fields, give them default values when deserializing old state
pub struct App {
    // Example stuff:
    label: String,
    editor: TextEditor,

    #[serde(skip)] // This how you opt-out of serialization of a field
    value: f32,
}

impl Default for App {
    fn default() -> Self {
        Self {
            // Example stuff:
            label: "Hello World!".to_owned(),
            value: 2.7,
            editor: TextEditor::default(),
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
        use egui::text::LayoutJob;

        let text = TO_BE_OR_NOT_TO_BE.as_str();

        egui::CentralPanel::default().show(ctx, |ui: &mut egui::Ui| {
            ui.heading("Text editor");

            let pixels_per_point = ui.ctx().pixels_per_point();
            let points_per_pixel = 1.0 / pixels_per_point;

            egui::ScrollArea::vertical()
                .auto_shrink(false)
                .show(ui, |ui| {
                    let extra_letter_spacing = points_per_pixel * 1.0 as f32;
                    let line_height = None;

                    let mut job = egui::text::LayoutJob::default();
                    job.justify = true;
                    job.append(
                        &text,
                        0.0,
                        egui::TextFormat {
                            extra_letter_spacing,
                            line_height,
                            ..Default::default()
                        },
                    );

                    // let mut job = LayoutJob::single_section(
                    //     text.to_owned(),
                    //     egui::TextFormat {
                    //         extra_letter_spacing,
                    //         line_height,
                    //         ..Default::default()
                    //     },
                    // );
                    job.wrap = egui::text::TextWrapping {
                        max_rows: usize::max_value(),
                        break_anywhere: false,
                        ..Default::default()
                    };

                    // NOTE: `Label` overrides some of the wrapping settings, e.g. wrap width
                    ui.label("Hello world");
                });

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
