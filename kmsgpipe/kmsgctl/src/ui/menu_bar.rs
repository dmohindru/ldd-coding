use crate::app::{App, CurrentScreen};
use ratatui::{
    Frame,
    layout::{Constraint, Flex, Layout, Rect},
    style::{Color, Style},
    text::Text,
    widgets::{Block, Borders, Paragraph},
};

enum MenuItems {
    Dashboard,
    Ioctl,
    Automation,
    Quit,
    Home,
    Write,
    Read,
    StartAutomation,
    EndAutomation,
}

impl MenuItems {
    fn menu_text(&self) -> &'static str {
        match self {
            Self::Dashboard => "F1: Dashboard",
            Self::Ioctl => "F2: IOCTL",
            Self::Automation => "F3: Automation",
            Self::Quit => "CTRL+Q: Quit",
            Self::Home => "ESC: Home",
            Self::Write => "ENTER: Write",
            Self::Read => "CTRL+R: Read",
            Self::StartAutomation => "CTRL+S: Start",
            Self::EndAutomation => "CTRL+E: End",
        }
    }
}

pub fn render_menu_bar(frame: &mut Frame, area: Rect, app: &App) {
    let panel_block = Block::default()
        .borders(Borders::ALL)
        .style(Style::default());
    let panel_block_area = panel_block.inner(area);
    frame.render_widget(panel_block, area);

    let menu_items = match app.current_screen {
        CurrentScreen::Main => get_main_screen_menu_items(),
        CurrentScreen::Dashboard => get_dashboard_menu_items(),
        CurrentScreen::Ioctl => get_ioctl_console_menu_items(),
        CurrentScreen::Automation => get_automation_menu_items(),
    };

    let menu_item_constraints: Vec<Constraint> =
        menu_items.iter().map(|_| Constraint::Fill(1)).collect();

    let menu_items_layout_chunks = Layout::horizontal(menu_item_constraints)
        .flex(Flex::SpaceEvenly)
        .split(panel_block_area);

    let menu_item_text: Vec<Paragraph> = menu_items
        .iter()
        .map(|item| get_menu_widget(item))
        .collect();
    let menu_items_zipped: Vec<(&Rect, &Paragraph)> = menu_items_layout_chunks
        .iter()
        .zip(menu_item_text.iter())
        .collect();

    menu_items_zipped
        .iter()
        .for_each(|(chunk, widget)| frame.render_widget(*widget, **chunk));
}

fn get_main_screen_menu_items() -> Vec<MenuItems> {
    vec![
        MenuItems::Dashboard,
        MenuItems::Ioctl,
        MenuItems::Automation,
        MenuItems::Quit,
    ]
}

fn get_dashboard_menu_items() -> Vec<MenuItems> {
    vec![
        MenuItems::Home,
        MenuItems::Write,
        MenuItems::Read,
        MenuItems::Ioctl,
        MenuItems::Automation,
        MenuItems::Quit,
    ]
}

fn get_ioctl_console_menu_items() -> Vec<MenuItems> {
    vec![
        MenuItems::Home,
        MenuItems::Dashboard,
        MenuItems::Automation,
        MenuItems::Quit,
    ]
}

fn get_automation_menu_items() -> Vec<MenuItems> {
    vec![
        MenuItems::Home,
        MenuItems::StartAutomation,
        MenuItems::EndAutomation,
        MenuItems::Dashboard,
        MenuItems::Ioctl,
        MenuItems::Quit,
    ]
}

fn get_menu_widget(menu_item: &MenuItems) -> Paragraph<'_> {
    Paragraph::new(Text::styled(
        menu_item.menu_text(),
        Style::default().fg(Color::White),
    ))
    .block(Block::default())
}
