use ratatui::{
    Frame,
    layout::{Constraint, Direction, Flex, Layout, Rect},
    style::{Color, Style},
    text::Text,
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

    render_top_panel(frame, chunks[0]);

    render_middle_panel(frame, chunks[1]);

    render_status(frame, chunks[2], app);
}

fn render_top_panel(frame: &mut Frame, area: Rect) {
    let top_panel_block = Block::default()
        .borders(Borders::ALL)
        .style(Style::default());

    let top_panel_title =
        Paragraph::new(Text::styled("Top Panel", Style::default().fg(Color::Gray)))
            .block(top_panel_block);

    frame.render_widget(top_panel_title, area);
}

fn render_middle_panel(frame: &mut Frame, area: Rect) {
    let middle_panel_block = Block::default()
        .borders(Borders::ALL)
        .style(Style::default());

    let middle_panel_title = Paragraph::new(Text::styled(
        "Middle Panel",
        Style::default().fg(Color::Gray),
    ))
    .block(middle_panel_block);

    frame.render_widget(middle_panel_title, area);
}

fn render_status(frame: &mut Frame, area: Rect, app: &App) {
    let status_block = Block::default()
        .title("Status")
        .borders(Borders::ALL)
        .border_style(Style::default())
        .style(Style::default());

    let status_block_area = status_block.inner(area);
    frame.render_widget(status_block, area);

    let status_bar_chunks = Layout::horizontal([
        Constraint::Percentage(25),
        Constraint::Percentage(25),
        Constraint::Percentage(25),
        Constraint::Percentage(25),
    ])
    .flex(Flex::SpaceEvenly)
    .split(status_block_area);

    let device_file = Paragraph::new(Text::styled(
        format!("Device: {}", &app.device.file_path),
        Style::default().fg(Color::White),
    ))
    .block(Block::default());
    frame.render_widget(device_file, status_bar_chunks[0]);

    let msg_size = Paragraph::new(Text::styled(
        format!("Msg size: {} Bytes", &app.data_size),
        Style::default().fg(Color::White),
    ))
    .block(Block::default());
    frame.render_widget(msg_size, status_bar_chunks[1]);

    let msg_count = Paragraph::new(Text::styled(
        format!("Msg Count: {}", &app.msg_count),
        Style::default().fg(Color::White),
    ))
    .block(Block::default());

    frame.render_widget(msg_count, status_bar_chunks[2]);

    let capacity = Paragraph::new(Text::styled(
        format!("Capacity: {}", &app.capacity),
        Style::default().fg(Color::White),
    ))
    .block(Block::default());

    frame.render_widget(capacity, status_bar_chunks[3]);
}
