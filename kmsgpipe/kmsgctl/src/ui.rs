use ratatui::{
    Frame,
    layout::{Constraint, Direction, Layout},
    style::{Color, Style},
    text::{Line, Span, Text},
    widgets::{Block, Borders, Paragraph},
};

use crate::app::App;

pub fn ui(frame: &mut Frame, app: &App) {
    let chunks = Layout::default()
        .direction(Direction::Vertical)
        .margin(1)
        .constraints([
            Constraint::Fill(1),
            Constraint::Length(3),
            Constraint::Length(3),
        ])
        .split(frame.area());

    // Top Panel
    let top_panel_block = Block::default()
        .borders(Borders::ALL)
        .style(Style::default());

    let top_panel_title =
        Paragraph::new(Text::styled("Top Panel", Style::default().fg(Color::Gray)))
            .block(top_panel_block);

    frame.render_widget(top_panel_title, chunks[0]);

    // Middle Panel
    let middle_panel_block = Block::default()
        .borders(Borders::ALL)
        .style(Style::default());

    let middle_panel_title = Paragraph::new(Text::styled(
        "Middle Panel",
        Style::default().fg(Color::Gray),
    ))
    .block(middle_panel_block);

    frame.render_widget(middle_panel_title, chunks[1]);

    // Status bar
    let status_bar_block = Block::default()
        .borders(Borders::ALL)
        .style(Style::default());

    let status_bar_title =
        Paragraph::new(Text::styled("Status Bar", Style::default().fg(Color::Gray)))
            .block(status_bar_block);

    frame.render_widget(status_bar_title, chunks[2]);
}
