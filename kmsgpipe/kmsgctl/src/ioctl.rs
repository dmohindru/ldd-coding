use std::fs::{File, OpenOptions};
use std::os::fd::{AsRawFd, RawFd};

use nix::libc::c_long;
use nix::{Result, ioctl_none, ioctl_read, ioctl_write_ptr};

const KMSGPIPE_IOC_MAGIC: u8 = b'K';

ioctl_read!(kmsgpipe_ioc_g_data_size, KMSGPIPE_IOC_MAGIC, 1, c_long);
ioctl_read!(kmsgpipe_ioc_g_capacity, KMSGPIPE_IOC_MAGIC, 2, c_long);
ioctl_read!(kmsgpipe_ioc_g_msg_count, KMSGPIPE_IOC_MAGIC, 3, c_long);
ioctl_read!(kmsgpipe_ioc_g_readers, KMSGPIPE_IOC_MAGIC, 4, c_long);
ioctl_read!(kmsgpipe_ioc_g_writers, KMSGPIPE_IOC_MAGIC, 5, c_long);
ioctl_read!(kmsgpipe_ioc_g_expiry_ms, KMSGPIPE_IOC_MAGIC, 6, c_long);
ioctl_write_ptr!(kmsgpipe_ioc_s_expiry_ms, KMSGPIPE_IOC_MAGIC, 7, c_long);
ioctl_none!(kmsgpipe_ioc_clear, KMSGPIPE_IOC_MAGIC, 8);

pub struct KmsgpipeDevice {
    file: File,
}

impl KmsgpipeDevice {
    pub fn open(path: String) -> std::io::Result<Self> {
        let file = OpenOptions::new().read(true).write(true).open(path)?;
        Ok(Self { file })
    }

    fn fd(&self) -> RawFd {
        self.file.as_raw_fd()
    }

    pub fn data_size(&self) -> Result<c_long> {
        let mut v: c_long = 0;
        unsafe {
            kmsgpipe_ioc_g_data_size(self.fd(), &mut v)?;
        }
        Ok(v)
    }

    pub fn capacity(&self) -> Result<c_long> {
        let mut v: c_long = 0;
        unsafe {
            kmsgpipe_ioc_g_capacity(self.fd(), &mut v)?;
        }
        Ok(v)
    }

    pub fn msg_count(&self) -> Result<c_long> {
        let mut v: c_long = 0;
        unsafe {
            kmsgpipe_ioc_g_msg_count(self.fd(), &mut v)?;
        }
        Ok(v)
    }

    pub fn readers(&self) -> Result<c_long> {
        let mut v: c_long = 0;
        unsafe {
            kmsgpipe_ioc_g_readers(self.fd(), &mut v)?;
        }
        Ok(v)
    }

    pub fn writers(&self) -> Result<c_long> {
        let mut v: c_long = 0;
        unsafe {
            kmsgpipe_ioc_g_writers(self.fd(), &mut v)?;
        }
        Ok(v)
    }

    pub fn expiry_ms(&self) -> Result<c_long> {
        let mut v: c_long = 0;
        unsafe {
            kmsgpipe_ioc_g_expiry_ms(self.fd(), &mut v)?;
        }
        Ok(v)
    }

    pub fn set_expiry_ms(&self, ms: c_long) -> Result<()> {
        unsafe {
            kmsgpipe_ioc_s_expiry_ms(self.fd(), &ms)?;
        }
        Ok(())
    }

    pub fn clear(&self) -> Result<()> {
        unsafe {
            kmsgpipe_ioc_clear(self.fd())?;
        }
        Ok(())
    }
}
