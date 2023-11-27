#ifndef USB_H
#define USB_H

#include <cstdint>
#include <cstddef>

namespace app {

void usb_init();
bool usb_send(const uint8_t* data, size_t size);

}

#endif