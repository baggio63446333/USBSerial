/*
 * USBSerial.cpp - USB CDC-ACM Serial port I/O
 *
 * This is modified for USB Serial based on HardwareSerial.cpp.
 */
/*
  HardwareSerial.cpp - Serial port I/O for the Spresense SDK
  Copyright (C) 2018 Sony Semiconductor Solutions Corp.
  Copyright (c) 2017 Sony Corporation  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <sdk/config.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <nuttx/serial/tioctl.h>
#include <sys/boardctl.h>
#include "USBSerial.h"

USBSerial::USBSerial()
: _fd(-1),
 _peek_buffer(-1),
 _available(0),
 _wbuf_size(0),
 _handle(NULL)
{
}

void USBSerial::begin(unsigned long baud, uint8_t config)
{
    int ret;
    struct termios tio;
    const char* dev = 0;
    char node[16];
    struct boardioc_usbdev_ctrl_s ctrl;

    if (_fd >= 0) {
        ::close(_fd);
        _fd = -1;
    }

    if (_handle == NULL) {
        /* Initialize the USB CDC/ACM serial driver */
        ctrl.usbdev   = BOARDIOC_USBDEV_CDCACM;
        ctrl.action   = BOARDIOC_USBDEV_CONNECT;
        ctrl.instance = 0;
        ctrl.handle   = &_handle;

        ret = boardctl(BOARDIOC_USBDEV_CONTROL, (uintptr_t)&ctrl);
        if (ret != 0)
            return;
    }

    // Wait for a while until device is ready
    sleep(1);

    snprintf(node, sizeof(node), "/dev/ttyACM0");
    dev = node;

    _fd = open(dev, O_RDWR);
    if (_fd < 0) {
        printf("ERROR: _fd=%d\n", _fd);
        return;
    }

    // Apply baud rate
    ret = ioctl(_fd, TCGETS, (long unsigned int)&tio);
    if (ret != 0)
        return;
    tio.c_speed = baud;
    tio.c_cflag = config;
    tio.c_oflag &= ~OPOST;
    ioctl(_fd, TCSETS, (long unsigned int)&tio);
    ioctl(_fd, TCFLSH, NULL);

    _wbuf_size = availableForWrite();
}

void USBSerial::end(void)
{
    struct boardioc_usbdev_ctrl_s ctrl;

    if (_fd >= 0) {
        close(_fd);
        _fd = -1;

        /* Disconnect the device */
        ctrl.usbdev   = BOARDIOC_USBDEV_CDCACM;
        ctrl.action   = BOARDIOC_USBDEV_DISCONNECT;
        ctrl.instance = 0;
        ctrl.handle   = &_handle;
        boardctl(BOARDIOC_USBDEV_CONTROL, (uintptr_t)&ctrl);
        _handle = NULL;
    }
}

USBSerial::operator bool() const
{
    return (_fd >= 0);
}

int USBSerial::available(void)
{
    int ret;
    int count = 0;

    if (_fd < 0)
        return 0;

    ret = ioctl(_fd, FIONREAD, (long unsigned int)&count);
    if (ret)
        printf("Serial FIONREAD not supported\n");

    return count;
}

int USBSerial::peek(void)
{
    char buf[1];

    if (_fd < 0)
        return -1;

    if (_peek_buffer > 0)
        return _peek_buffer;

    if (_available <= 0) {
        _available = available();
        if (_available <= 0)
            return -1;
    }

    ::read(_fd, buf, 1);
    --_available;
    _peek_buffer = (unsigned int)buf[0];

    return _peek_buffer;
}

int USBSerial::read(void)
{
    char buf[1];

    if (_fd < 0)
        return -1;

    if (_peek_buffer >= 0) {
        int t = _peek_buffer;
        _peek_buffer = -1;
        return t;
    }

    if (_available <= 0)
        _available = available();

    if (_available <= 0)
        return -1;

    ::read(_fd, buf, 1);
    --_available;

    return (unsigned int)buf[0];
}

int USBSerial::availableForWrite(void)
{
    int ret;
    int count = 0;

    if (_fd < 0)
        return 0;

    ret = ioctl(_fd, FIONSPACE, (long unsigned int)&count);
    if (ret)
        printf("Serial FIONSPACE not supported\n");

    return count;
}

void USBSerial::flush(void)
{
    ioctl(_fd, TCFLSH, NULL);
    while (availableForWrite() != _wbuf_size)
        usleep(1000);
}

size_t USBSerial::write(const char* str)
{
    if (_fd < 0)
        return 0;

    return ::write(_fd, str, strlen((const char*)str));
}

size_t USBSerial::write(uint8_t c)
{
    return write(&c, 1);
}

size_t USBSerial::write(const uint8_t* buffer, size_t size)
{
    if (_fd < 0)
        return 0;

    return ::write(_fd, buffer, size);
}

