use crate::app::{App, CurrentScreen};
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

    let middle_screen = match app.current_screen {
        CurrentScreen::Main => "Home",
        CurrentScreen::Dashboard => "Reader Writer Screen",
        CurrentScreen::Ioctl => "IOCTL Screen",
        CurrentScreen::Automation => "Automation Screen",
    };

    let panel_title = Paragraph::new(Text::styled(
        middle_screen,
        Style::default().fg(Color::Gray),
    ))
    .block(panel_block);

    frame.render_widget(panel_title, area);
}
