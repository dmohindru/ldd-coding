use crate::ioctl::KmsgpipeDevice;
pub struct App {
    pub device: KmsgpipeDevice,
    pub data_size: i64,
    pub msg_count: i64,
    pub capacity: i64,
}

impl App {
    pub fn new(device: KmsgpipeDevice) -> App {
        let data_size = device.data_size().unwrap();
        let msg_count = device.msg_count().unwrap();
        let capacity = device.capacity().unwrap();
        App {
            device,
            data_size,
            msg_count,
            capacity,
        }
    }
}
