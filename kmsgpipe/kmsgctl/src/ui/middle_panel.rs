use crate::app::App;
use ratatui::{
    Frame,
    layout::Rect,
    style::{Color, Style},
    text::Text,
    widgets::{Block, Borders, Paragraph},
};

pub fn render_middle_panel(frame: &mut Frame, area: Rect, app: &App) {
    let panel_block = Block::default()
        .borders(Borders::ALL)
        .style(Style::default());

    let panel_title = Paragraph::new(Text::styled(
        "Middle Panel",
        Style::default().fg(Color::Gray),
    ))
    .block(panel_block);

    frame.render_widget(panel_title, area);
}
