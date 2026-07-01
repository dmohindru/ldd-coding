use crate::app::App;
use ratatui::{
    Frame,
    layout::{Constraint, Flex, Layout, Rect},
    style::{Color, Style},
    text::Text,
    widgets::{Block, Borders, Paragraph},
};

pub fn render_status_bar(frame: &mut Frame, area: Rect, app: &App) {
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
        format!("Msg size: {} Bytes", &app.get_data_size()),
        Style::default().fg(Color::White),
    ))
    .block(Block::default());
    frame.render_widget(msg_size, status_bar_chunks[1]);

    let msg_count = Paragraph::new(Text::styled(
        format!("Msg Count: {}", &app.get_msg_count()),
        Style::default().fg(Color::White),
    ))
    .block(Block::default());

    frame.render_widget(msg_count, status_bar_chunks[2]);

    let capacity = Paragraph::new(Text::styled(
        format!("Capacity: {}", &app.get_capacity()),
        Style::default().fg(Color::White),
    ))
    .block(Block::default());

    frame.render_widget(capacity, status_bar_chunks[3]);
}
