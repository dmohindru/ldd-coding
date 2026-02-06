mod cli;
mod ioctl;

use crate::cli::{IoctlCommands, IoctlGetCommands, IoctlSetCommands, KmsgpipeCli};
use crate::ioctl::KmsgpipeDevice;
use clap::Parser;
use nix::libc::c_long;
use std::process;

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
    match cli.command {
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
