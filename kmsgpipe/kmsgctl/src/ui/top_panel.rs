use crate::app::{App, CurrentScreen};
use ratatui::{
    Frame,
    layout::{Alignment, Constraint, Flex, Layout, Rect},
    style::{Color, Style},
    text::Text,
    widgets::{Block, Borders, Paragraph},
};

pub fn render_top_panel(frame: &mut Frame, area: Rect, app: &App) {
    let panel_block = Block::default()
        .borders(Borders::ALL)
        .style(Style::default());
    let panel_block_area = panel_block.inner(area);

    let title_chunks = Layout::horizontal([Constraint::Fill(1)])
        .flex(Flex::SpaceAround)
        .split(panel_block_area);

    frame.render_widget(panel_block, area);
    let title = match app.current_screen {
        CurrentScreen::Main => "Home",
        CurrentScreen::Dashboard => "Reader Writer Dashboard",
        CurrentScreen::Ioctl => "IOCTL Console",
        CurrentScreen::Automation => "Automation",
    };

    let panel_title = Paragraph::new(Text::styled(title, Style::default().fg(Color::Gray)))
        .alignment(Alignment::Center)
        .block(Block::default());

    frame.render_widget(panel_title, title_chunks[0]);
}
