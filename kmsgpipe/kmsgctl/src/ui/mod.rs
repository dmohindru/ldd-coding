mod menu_bar;
mod middle_panel;
mod status_bar;
mod top_panel;

use ratatui::{
    Frame,
    layout::{Constraint, Direction, Layout},
};

use crate::app::App;
use menu_bar::render_menu_bar;
use middle_panel::render_middle_panel;
use status_bar::render_status_bar;
use top_panel::render_top_panel;

pub fn ui(frame: &mut Frame, app: &App) {
    let chunks = Layout::default()
        .direction(Direction::Vertical)
        // .margin(1)
        .constraints([
            Constraint::Length(3),
            Constraint::Fill(1),
            Constraint::Length(3),
            Constraint::Length(3),
        ])
        .split(frame.area());

    render_top_panel(frame, chunks[0], app);

    render_middle_panel(frame, chunks[1], app);

    render_status_bar(frame, chunks[2], app);

    render_menu_bar(frame, chunks[3], app);
}
