#ifndef USB_H
#define USB_H

namespace app {

void usb_init();
void usb_send(const uint8_t* data, size_t size);

}

#endif