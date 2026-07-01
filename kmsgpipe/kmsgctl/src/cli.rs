use clap::{Parser, Subcommand, ValueEnum};

#[derive(ValueEnum, Clone, Debug)]
pub enum IoctlGetCommands {
    DataSize,
    Capacity,
    MsgCount,
    Readers,
    Writers,
    ExpiryMs,
}

#[derive(ValueEnum, Clone, Debug)]
pub enum IoctlSetCommands {
    ExpiryMs,
}

#[derive(Parser)]
#[command(version, about, long_about = None)]
pub struct KmsgpipeCli {
    /// Device file to operate on
    #[arg(short, long)]
    pub device: String,

    /// Command line switch for interactive session
    #[arg(short, long)]
    pub interactive: bool,

    /// The subcommand to run
    #[command(subcommand)]
    pub command: Option<IoctlCommands>,
}

#[derive(Subcommand, Debug)]
pub enum IoctlCommands {
    /// IOCTL get command
    Get { op: IoctlGetCommands },
    /// IOCTL set command
    Set { op: IoctlSetCommands, value: i64 },
    /// IOCTL clear command
    Clear,
}
