use crate::ioctl::KmsgpipeDevice;
pub enum CurrentScreen {
    Main,
    Dashboard, //TODO come up with a better name here
    Ioctl,
    Automation,
}
pub struct App {
    pub device: KmsgpipeDevice,
    pub current_screen: CurrentScreen,
}

impl App {
    pub fn new(device: KmsgpipeDevice) -> App {
        App {
            device: device,
            current_screen: CurrentScreen::Main,
        }
    }

    pub fn get_data_size(&self) -> i64 {
        self.device.data_size().unwrap()
    }

    pub fn get_msg_count(&self) -> i64 {
        self.device.msg_count().unwrap()
    }

    pub fn get_capacity(&self) -> i64 {
        self.device.capacity().unwrap()
    }
}
