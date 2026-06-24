mod app;
mod cli;
mod ioctl;
mod ui;

use crate::app::App;
use crate::cli::{IoctlCommands, IoctlGetCommands, IoctlSetCommands, KmsgpipeCli};
use crate::ioctl::KmsgpipeDevice;
use crate::ui::ui;
use clap::Parser;
use nix::libc::c_long;
use ratatui::crossterm::execute;
use ratatui::{
    Terminal,
    backend::{Backend, CrosstermBackend},
    crossterm::{
        event::{self, DisableMouseCapture, EnableMouseCapture},
        terminal::{EnterAlternateScreen, LeaveAlternateScreen, disable_raw_mode, enable_raw_mode},
    },
};
use std::io;
use std::{error::Error, process};

fn main() {
    let cli = KmsgpipeCli::parse();
    let device_path = cli.device;
    let device = match KmsgpipeDevice::open(device_path) {
        Ok(device) => device,
        Err(e) => {
            eprintln!("{}", e);
            process::exit(1);
        }
    };
    let interactive_mode = cli.interactive;
    if interactive_mode {
        run_interactive_mode().unwrap();
        return;
    }

    if cli.command.is_none() {
        eprintln!("Missing sub commands. For more information, try '--help'. ");
        process::exit(1);
    }
    match cli.command.unwrap() {
        IoctlCommands::Get { op } => match op {
            IoctlGetCommands::DataSize => process_get_command(device.data_size()),
            IoctlGetCommands::Capacity => process_get_command(device.capacity()),
            IoctlGetCommands::MsgCount => process_get_command(device.msg_count()),
            IoctlGetCommands::ExpiryMs => process_get_command(device.expiry_ms()),
            IoctlGetCommands::Readers => process_get_command(device.readers()),
            IoctlGetCommands::Writers => process_get_command(device.writers()),
        },
        IoctlCommands::Set { op, value } => match op {
            IoctlSetCommands::ExpiryMs => process_set_command(device.set_expiry_ms(value)),
        },
        IoctlCommands::Clear => process_set_command(device.clear()),
    }
}

fn process_get_command(op_result: nix::Result<c_long>) {
    match op_result {
        Ok(resp) => println!("{}", resp),
        Err(e) => {
            eprintln!("{}", e);
            process::exit(1);
        }
    }
}

fn process_set_command(op_result: nix::Result<()>) {
    if let Err(e) = op_result {
        eprintln!("{}", e);
        process::exit(1);
    }
}

fn run_interactive_mode() -> Result<(), Box<dyn Error>> {
    enable_raw_mode()?;
    let mut stderr = io::stderr();
    execute!(stderr, EnterAlternateScreen, EnableMouseCapture)?;
    let backend = CrosstermBackend::new(stderr);
    let mut terminal = Terminal::new(backend)?;

    let mut app = App::new();
    run_app(&mut terminal, &mut app)?;

    disable_raw_mode()?;
    execute!(
        terminal.backend_mut(),
        LeaveAlternateScreen,
        DisableMouseCapture
    )?;
    terminal.show_cursor()?;
    Ok(())
}

fn run_app<B: Backend>(terminal: &mut Terminal<B>, app: &mut App) -> std::io::Result<()> {
    loop {
        // terminal
        //     .draw(|f| f.render_widget("kmsgctl tui", f.area()))
        //     .unwrap();
        terminal.draw(|f| ui(f, app)).unwrap();

        if event::read()?.is_key_press() {
            break;
        }
    }
    Ok(())
}
