#ifndef USB_H
#define USB_H

#include <cstdint>
#include <cstddef>

namespace app {

void usb_init();
bool usb_send(const void* data, size_t size);
bool usb_send_finalize();

}

#endif